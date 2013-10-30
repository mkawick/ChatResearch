#include "DiplodocusAsset.h"
#include "../NetworkCommon/Packets/ServerToServerPacket.h"
#include "../NetworkCommon/Packets/AssetPacket.h"
#include "../NetworkCommon/Packets/LoginPacket.h"
#include "../NetworkCommon/Packets/Packetfactory.h"

#include "AssetOrganizer.h"

#include <iostream>
#include <time.h>

using namespace std;

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

DiplodocusAsset::DiplodocusAsset( const string& serverName, U32 serverId ): Diplodocus< KhaanAsset >( serverName, serverId, 0,  ServerType_Contact ), m_staticAssets( NULL ), m_dynamicAssets( NULL )
/*, 
                        //m_initializationStage( InitializationStage_Begin ),
                        m_queryPeriodicity( 10000 ),
                        m_isExecutingQuery( false )*/
{
    //time( &m_lastTimeStamp );
   SetSleepTime( 100 );
}

DiplodocusAsset :: ~DiplodocusAsset()
{
   delete m_staticAssets;
   delete m_dynamicAssets;
}
//---------------------------------------------------------------

void     DiplodocusAsset::ServerWasIdentified( ChainedInterface* khaan )
{
   BasePacket* packet = NULL;
   PackageForServerIdentification( m_serverName, m_serverId, m_gameProductId, m_isGame, m_isControllerApp, true, m_isGateway, &packet );
   khaan->AddOutputChainData( packet, 0 );
   m_serversNeedingUpdate.push_back( static_cast<InputChainType*>( khaan )->GetServerId() );
}

//---------------------------------------------------------------

string AssembleFullPath( const string& path, const string& fileName )
{
   string finalPath = path;

   char endValue = *path.rbegin() ;   
   if( endValue != '/' && endValue != '\\' ) // the back slash is windows support. Obviously will not work on linux.
   {
      char beginValue = *fileName.begin(); // note... different value from above
      if( beginValue != '/' && beginValue != '\\' )
      {
         finalPath += '/';
      }
   }

   finalPath += fileName;
   std::replace( finalPath.begin(), finalPath.end(), '\\', '/'); // convert everything to forward slashes

   return finalPath;
}

bool     DiplodocusAsset::SetIniFilePath( const string& assetPath, const string& assetDictionary, const string& dynamicAssetDictionary )
{
   if( m_staticAssets )
   {
      delete m_staticAssets;
   }
   m_staticAssets = new AssetOrganizer();

   if( m_dynamicAssets )
   {
      delete m_dynamicAssets;
   }
   m_dynamicAssets = new AssetOrganizer();

   string   finalPath = AssembleFullPath( assetPath, assetDictionary );

   bool success = m_staticAssets->Init( finalPath );

   //-----------------------------------------------

   finalPath = AssembleFullPath( assetPath, dynamicAssetDictionary );

   success &= m_dynamicAssets->Init( finalPath );

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
      return HandlePacketFromOtherServer( packet, connectionId );
      //return true;
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
            break;
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
               SendErrorToClient( connectionId, PacketErrorReport::ErrorType_Asset_BadLoginKey );
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

      case PacketLogin::LoginType_ListOfProductsS2S:
         StoreUserProductsOwned( static_cast< PacketListOfUserProductsS2S* >( unwrappedPacket ) );
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
      ut.userName =        loginPacket->userName;
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

bool     DiplodocusAsset::StoreUserProductsOwned( PacketListOfUserProductsS2S* productNamesPacket )
{
   string uuid = productNamesPacket->uuid;
   U64 hashForUser = GenerateUniqueHash( uuid );

   Threading::MutexLock locker( m_mutex );
   UAADMapIterator it = m_userTickets.find( hashForUser );
   if( it == m_userTickets.end() )
      return false;

   it->second.SetupProductFilterNames( productNamesPacket->products );

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
         IChainedInterface* interfacePtr = chainedInput.m_interface;
         KhaanAsset* khaan = static_cast< KhaanAsset* >( interfacePtr );
         if( khaan->GetServerId() == m_connectionIdGateway )
         {
            khaan->AddOutputChainData( packet );
            //khaan->Update();// the gateway may not have a proper connection id.

            AddServerNeedingUpdate( khaan->GetServerId() );
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
         UAADMapIterator currentIt = it++;
         if( currentIt->second.LogoutExpired() )
         {
            //delete it->second;// bad idea
            m_userTickets.erase( currentIt );
         }
      }
   }

   return false;
}

void     DiplodocusAsset::AddServerNeedingUpdate( U32 serverId )
{
   deque< U32 >::iterator it = m_serversNeedingUpdate.begin();
   while( it != m_serversNeedingUpdate.end() )
   {
      if( *it == serverId )
         return;
      it++;
   }
   m_serversNeedingUpdate.push_back( serverId );

}

//---------------------------------------------------------------

int      DiplodocusAsset::CallbackFunction()
{
   while( m_serversNeedingUpdate.size() )
   {
      Threading::MutexLock locker( m_mutex );

      U32 serverId = m_serversNeedingUpdate.front();
      m_serversNeedingUpdate.pop_front();
      
      ChainLinkIteratorType itInputs = m_listOfInputs.begin();
      while( itInputs != m_listOfInputs.end() )
      {
         ChainLink& chainedInput = *itInputs++;
         IChainedInterface* interfacePtr = chainedInput.m_interface;
         KhaanAsset* khaan = static_cast< KhaanAsset* >( interfacePtr );
         if( khaan->GetServerId() == serverId )
         {
            if( khaan->Update() == false )
            {
               AddServerNeedingUpdate( serverId );// more updating needed
            }
         }
      }
   }
   UpdateAllConnections();

   if( m_staticAssets->IsFullyLoaded() == false )
   {
      m_staticAssets->Update();
   }
   if( m_dynamicAssets->IsFullyLoaded() == false )
   {
      m_dynamicAssets->Update();
   }

   UpdateConsoleWindow( m_timeOfLastTitleUpdate, m_uptime, m_numTotalConnections, m_connectedClients.size(), m_listeningPort, m_serverName );
   /*ContinueInitialization();*/
   // check for new friend requests and send a small list of notifications

   return 1;
}


//---------------------------------------------------------------
//---------------------------------------------------------------