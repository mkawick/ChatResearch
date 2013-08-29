#include "DiplodocusAsset.h"
#include "../NetworkCommon/Packets/ServerToServerPacket.h"
#include "../NetworkCommon/Packets/AssetPacket.h"
#include "../NetworkCommon/Packets/Packetfactory.h"

#include "AssetOrganizer.h"

#include <iostream>
using namespace std;
#include <time.h>

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

DiplodocusAsset::DiplodocusAsset( const string& serverName, U32 serverId ): Diplodocus< KhaanAsset >( serverName, serverId, 0,  ServerType_Contact ), m_assets( NULL )
/*, 
                        //m_initializationStage( InitializationStage_Begin ),
                        m_queryPeriodicity( 10000 ),
                        m_isExecutingQuery( false )*/
{
    //time( &m_lastTimeStamp );
}

DiplodocusAsset :: ~DiplodocusAsset()
{
   delete m_assets;
}
//---------------------------------------------------------------

void     DiplodocusAsset::ServerWasIdentified( KhaanAsset* khaan )
{
   BasePacket* packet = NULL;
   PackageForServerIdentification( m_serverName, m_serverId, m_gameProductId, m_isGame, m_isControllerApp, true, m_isGateway, &packet );
   khaan->AddOutputChainData( packet, 0 );
   m_serversNeedingUpdate.push_back( khaan->GetServerId() );
}

//---------------------------------------------------------------

bool     DiplodocusAsset::SetIniFilePath( const string& assetPath, const string& assetDictionary )
{
   if( m_assets )
   {
      delete m_assets;
   }
   m_assets = new AssetOrganizer();

   string   finalPath = assetPath;

   char endValue = *assetPath.rbegin() ;   
   if( endValue != '/' && endValue != '\\' ) // the back slash is windows support. Obviously will not work on linux.
   {
      char beginValue = *assetDictionary.begin(); // note... different value from above
      if( beginValue != '/' && beginValue != '\\' )
      {
         finalPath += '/';
      }
   }

   finalPath += assetDictionary;
   std::replace( finalPath.begin(), finalPath.end(), '\\', '/'); // convert everything to forward slashes

   bool success = m_assets->Init( finalPath );

   return success;
   
}

//---------------------------------------------------------------

bool     DiplodocusAsset::AddInputChainData( BasePacket* packet, U32 connectionId )
{
   if( packet->packetType == PacketType_GatewayInformation )
   {
      PacketCleaner cleaner( packet );
      return HandleCommandFromGateway( packet, connectionId );
   }

   if( packet->packetType == PacketType_ServerJobWrapper )
   {
      PacketCleaner cleaner( packet );
      HandlePacketFromOtherServer( packet, connectionId );
      return true;
   }

   if( packet->packetType == PacketType_GatewayWrapper )
   {
      PacketCleaner cleaner( packet );
      PacketGatewayWrapper* wrapper = static_cast< PacketGatewayWrapper* >( packet );
      BasePacket* unwrappedPacket = wrapper->pPacket;
      U32 connectionIdToUse = wrapper->connectionId;
      
      if( unwrappedPacket->packetType == PacketType_Asset)
      {
         int type = unwrappedPacket->packetSubType;
         string uuid;
         string loginKey;

         switch( type )
         {
         case PacketAsset::AssetType_GetListOfStaticAssets:
            {
               PacketAsset_GetListOfStaticAssets* packetAsset = static_cast< PacketAsset_GetListOfStaticAssets* >( unwrappedPacket );
               uuid = packetAsset->uuid;
               loginKey = packetAsset->loginKey;
            }
         case PacketAsset::AssetType_GetListOfDynamicAssets:
            {
               PacketAsset_GetListOfDynamicAssets* packetAsset = static_cast< PacketAsset_GetListOfDynamicAssets* >( unwrappedPacket );
               uuid = packetAsset->uuid;
               loginKey = packetAsset->loginKey;
            }
            break;
         case PacketAsset::AssetType_RequestAsset:
            {
               PacketAsset_RequestAsset* packetAsset = static_cast< PacketAsset_RequestAsset* >( unwrappedPacket );
               uuid = packetAsset->uuid;
               loginKey = packetAsset->loginKey;
            }
            break;
         }
         if( uuid.size() )
         {
            U64 userHash = GenerateUniqueHash( uuid );
            Threading::MutexLock locker( m_mutex );
            UAADMapIterator found = m_userTickets.find( userHash );
            if( found == m_userTickets.end() )
               return true;
            if( found->second.LoginKeyMatches( loginKey ) == false )
            {
               SendErrorToClient( connectionId, PacketErrorReport::ErrorType_Contact_Asset_BadLoginKey );
            }
            else
            {
               found->second.SetConnectionId( connectionId );
               Threading::MutexLock locker( m_mutex );
               bool result = found->second.HandleRequestFromClient( static_cast< PacketAsset* >( unwrappedPacket ) );
            }
         }
         

         
        // we will cleanup here... see cleaner
         return true;
      }
      else
      {
         assert( 0 );
      }
      // we handle all packets from the gateway here.
      return true;
   }
   return false;
}

//---------------------------------------------------------------

bool  DiplodocusAsset::HandlePacketFromOtherServer( BasePacket* packet, U32 connectionId )// not thread safe
{
   if( packet->packetType != PacketType_ServerJobWrapper )
   {
      return false;
   }

   PacketServerJobWrapper* wrapper = static_cast< PacketServerJobWrapper* >( packet );
   BasePacket* unwrappedPacket = wrapper->pPacket;
   U32  serverIdLookup = wrapper->serverId;

   bool success = false;

   if( unwrappedPacket->packetType == PacketType_Login )
   {
      switch( unwrappedPacket->packetSubType )
      {
      case PacketLogin::LoginType_PrepareForUserLogin:
         ConnectUser( static_cast< PacketPrepareForUserLogin* >( unwrappedPacket ) );
         return true;

      case PacketLogin::LoginType_PrepareForUserLogout:
         DisconnectUser( static_cast< PacketPrepareForUserLogout* >( unwrappedPacket ) );
         return true;
      }
   }
 /*  else if( unwrappedPacket->packetType == PacketType_Contact)
   {
      return HandlePacketRequests( static_cast< PacketContact* >( packet ), connectionId );
   }*/

   return false;
}

//---------------------------------------------------------------

bool     DiplodocusAsset::ConnectUser( PacketPrepareForUserLogin* loginPacket )
{
   U64 hashForUser = GenerateUniqueHash( loginPacket->uuid );

   Threading::MutexLock locker( m_mutex );
   UAADMapIterator it = m_userTickets.find( hashForUser );
   if( it != m_userTickets.end() )// user may be reloggin and such.. no biggie.. just ignore
   {
      it->second.SetConnectionId( 0 );
      return false;
   }

   bool found = false;
   if( found == false )
   {

      UserTicket ut;
      ut.username =        loginPacket->username;
      ut.uuid =            loginPacket->uuid;
      ut.userTicket =      loginPacket->loginKey;
      ut.connectionId =    0;
      ut.gameProductId =   loginPacket->gameProductId;
      ut.userId =          loginPacket->userId;

      UserAccountAssetDelivery user( ut );
      user.SetServer( this );

      m_mutex.lock();
      m_userTickets.insert( UAADPair( hashForUser, user ) );
      m_mutex.unlock();
   }
   return true;
}

//---------------------------------------------------------------

bool     DiplodocusAsset::DisconnectUser( PacketPrepareForUserLogout* loginPacket )
{
   U32 connectionId = loginPacket->connectionId;
   string uuid = loginPacket->uuid;
   U64 hashForUser = GenerateUniqueHash( uuid );

   Threading::MutexLock locker( m_mutex );
   UAADMapIterator it = m_userTickets.find( hashForUser );
   if( it == m_userTickets.end() )
      return false;

   //it->second.NeedsUpdate();
   it->second.UserLoggedOut();
   // we need to send notifications

   return true;
}


//---------------------------------------------------------------

// make sure to follow the model of the account server regarding queries. Look at CreateAccount
// 
bool     DiplodocusAsset::AddOutputChainData( BasePacket* packet, U32 connectionId )
{
   if( packet->packetType == PacketType_GatewayWrapper )
   {
      if( m_connectionIdGateway == 0 )
         return false;

      Threading::MutexLock locker( m_mutex );

      ChainLinkIteratorType itInputs = m_listOfInputs.begin();
      while( itInputs != m_listOfInputs.end() )
      {
         ChainLink& chainedInput = *itInputs++;
         ChainedInterface* interfacePtr = chainedInput.m_interface;
         KhaanAsset* khaan = static_cast< KhaanAsset* >( interfacePtr );
         if( khaan->GetServerId() == m_connectionIdGateway )
         {
            interfacePtr->AddOutputChainData( packet );
            //khaan->Update();// the gateway may not have a proper connection id.

            m_serversNeedingUpdate.push_back( khaan->GetServerId() );
            return true;
         }
      }
      return false;
   }
   if( m_userTickets.size() )
   {
      Threading::MutexLock locker( m_mutex );
      UAADMapIterator it = m_userTickets.begin();
      while( it != m_userTickets.end() )
      {
         if( it->second.LogoutExpired() )
         {
            //delete it->second;// bad idea
            m_userTickets.erase( it );
         }
         else
         {
            it++;
         }
      }
   }

   return false;
}

//---------------------------------------------------------------

int      DiplodocusAsset::CallbackFunction()
{
   while( m_serversNeedingUpdate.size() )
   {
      U32 serverId = m_serversNeedingUpdate.front();
      m_serversNeedingUpdate.pop_front();

      Threading::MutexLock locker( m_mutex );
      ChainLinkIteratorType itInputs = m_listOfInputs.begin();
      while( itInputs != m_listOfInputs.end() )
      {
         ChainLink& chainedInput = *itInputs++;
         ChainedInterface* interfacePtr = chainedInput.m_interface;
         KhaanAsset* khaan = static_cast< KhaanAsset* >( interfacePtr );
         if( khaan->GetServerId() == serverId )
         {
            khaan->Update();
         }
      }
   }
   UpdateAllConnections();

   /*ContinueInitialization();*/
   // check for new friend requests and send a small list of notifications

   return 1;
}


//---------------------------------------------------------------
//---------------------------------------------------------------