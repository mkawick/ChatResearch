#include "DiplodocusAsset.h"
#include "../NetworkCommon/Packets/ServerToServerPacket.h"
#include "../NetworkCommon/Packets/AssetPacket.h"
#include "../NetworkCommon/Packets/Packetfactory.h"

#include <iostream>
using namespace std;
#include <time.h>

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

DiplodocusAsset::DiplodocusAsset( const string& serverName, U32 serverId ): Diplodocus< KhaanAsset >( serverName, serverId, 0,  ServerType_Contact )
/*, 
                        //m_initializationStage( InitializationStage_Begin ),
                        m_queryPeriodicity( 10000 ),
                        m_isExecutingQuery( false )*/
{
    //time( &m_lastTimeStamp );
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
         switch( unwrappedPacket->packetSubType )
         {
         case PacketAsset::AssetType_GetListOfStaticAssets:
            {
               PacketAsset_GetListOfStaticAssets* packetAsset = static_cast< PacketAsset_GetListOfStaticAssets* >( unwrappedPacket );
               U64 userHash = GenerateUniqueHash( packetAsset->uuid );

               UAADMapIterator found = m_userTickets.find( userHash );
               if( found == m_userTickets.end() )
                  return true;

               //bool result = found->second.HandleRequestFromClient( packetContact, userHash );
            }
         case PacketAsset::AssetType_GetListOfDynamicAssets:
            {
            }
            break;
         }
         

         
        
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

   UAADMapIterator it = m_userTickets.find( hashForUser );
   if( it != m_userTickets.end() )// user may be reloggin and such.. no biggie.. just ignore
   {
      it->second.SetConnectionId( 0 );
      return false;
   }

   bool found = false;
   // if the user is already here but relogged, simply 
  /* m_mutex.lock();
      it = m_users.begin();
      while( it != m_users.end() )
      {
         if( it->second.GetUserInfo().uuid == loginPacket->uuid ) 
         {
            found = true;
            U32 id = it->second.GetUserInfo().id;
            UserIdToContactMapIterator itIdToContact = m_userLookupById.find( id );
            if( itIdToContact != m_userLookupById.end() )
            {
               itIdToContact->second = connectionId;
            }
            it->second.SetConnectionId( connectionId );
            it->second.FinishLoginBySendingUserFriendsAndInvitations();
            
            m_users.insert( UAADPair( connectionId, it->second ) );
            m_users.erase( it );
            break;
         }
         it++;
      }
   m_mutex.unlock();*/

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
      //m_userLookupById.insert( UserIdToContactPair( ui.id, connectionId ) );
      m_mutex.unlock();
   }
   return true;
}

//---------------------------------------------------------------

bool     DiplodocusAsset::DisconnectUser( PacketPrepareForUserLogout* loginPacket )
{
   U32 connectionId = loginPacket->connectionId;

   UAADMapIterator it = m_userTickets.find( connectionId );
   if( it == m_userTickets.end() )
      return false;

   //it->second.NeedsUpdate();
   it->second.UserLoggedOut();
   // we need to send notifications

   return true;
}


//---------------------------------------------------------------
//---------------------------------------------------------------