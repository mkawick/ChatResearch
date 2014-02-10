// DiplodocusChat.cpp

#include <iostream>
#include <time.h>

using namespace std;


#include "../NetworkCommon/Packets/ServerToServerPacket.h"
#include "../NetworkCommon/Packets/PacketFactory.h"

#include "../NetworkCommon/Packets/DbPacket.h"
#include "../NetworkCommon/Packets/ChatPacket.h"
#include "../NetworkCommon/Packets/LoginPacket.h"
#include "../NetworkCommon/Packets/StatPacket.h"

#include "DiplodocusChat.h"
#include "ChatUser.h"
#include "ChatChannelManager.h"


///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

DiplodocusChat::DiplodocusChat( const string& serverName, U32 serverId ): Diplodocus< KhaanChat >( serverName, serverId, 0,  ServerType_Chat ),
                                             StatTrackingConnections(),
                                             m_chatChannelManagerNeedsUpdate( true )
{
   this->SetSleepTime( 66 );
}

void  DiplodocusChat :: Init()
{
   ChatUser::Set( this );
   ChatChannelManager::Set( this );
}

//---------------------------------------------------------------

ChatUser* DiplodocusChat::CreateNewUser( U32 connectionId )
{
   UserMapIterator it = m_users.find( connectionId );
   if( it != m_users.end() )
      return it->second;

   ChatUser* user = new ChatUser( connectionId );
   m_users.insert( UserMapPair( connectionId, user ) );

   return user;
}

//---------------------------------------------------------------

ChatUser* DiplodocusChat::UpdateExistingUsersConnectionId( const string& uuid, U32 connectionId )
{
   UserMapIterator it = m_users.begin(); 
   while( it != m_users.end() )
   {
      if( it->second->GetUuid() == uuid )
      {
         ChatUser* user = it->second;
         user->SetConnectionId( connectionId );
         m_users.insert( UserMapPair( connectionId, user ) );
         m_users.erase( it );
         return it->second;
      }
      it++;
   }

   return NULL;
}

//---------------------------------------------------------------

ChatUser* DiplodocusChat::GetUser( U32 connectionId )
{
   UserMapIterator it = m_users.find( connectionId );
   if( it == m_users.end() )
      return NULL;

   return it->second;
}

//---------------------------------------------------------------
/*
ChatUser* DiplodocusChat::GetUser( U32 connectionId )
{
   UserMapIterator it = m_users.find( connectionId );
   if( it == m_users.end() )
      return NULL;

   return it->second;
}*/

//---------------------------------------------------------------

ChatUser* DiplodocusChat::GetUserById( U32 userId )
{
   UserMapIterator it = m_users.begin();
   while( it != m_users.end() )
   {
      if( it->second->GetUserId() == userId )
      return it->second;
      it++;
   }

   return NULL;
}

//---------------------------------------------------------------

ChatUser* DiplodocusChat::GetUserByUsername( const string& userName )
{
   UserMapIterator it = m_users.begin(); 
   while( it != m_users.end() )
   {
      if( it->second->GetUserName() == userName )
         return it->second;
      it++;
   }

   return NULL;
}


//---------------------------------------------------------------

ChatUser* DiplodocusChat::GetUserByUuid( const string& uuid )
{
   UserMapIterator it = m_users.begin(); 
   while( it != m_users.end() )
   {
      if( it->second->GetUuid() == uuid )
         return it->second;
      it++;
   }

   return NULL;
}

//---------------------------------------------------------------

void     DiplodocusChat::ServerWasIdentified( IChainedInterface* khaan )
{
   BasePacket* packet = NULL;
   PackageForServerIdentification( m_serverName, m_localIpAddress, m_serverId, m_listeningPort, m_gameProductId, m_isGame, m_isControllerApp, true, m_isGateway, &packet );
   ChainedType* localKhaan = static_cast< ChainedType* >( khaan );
   localKhaan->AddOutputChainData( packet, 0 );
   m_clientsNeedingUpdate.push_back( localKhaan->GetServerId() );
}

//---------------------------------------------------------------

bool     DiplodocusChat::HandleChatPacket( BasePacket* packet, U32 connectionId )
{
   U32 packetType = packet->packetType;
   U32 packetSubType = packet->packetSubType;

   if( packetType == PacketType_Chat )
   {
      switch( packetSubType )
      {
      case PacketChatToServer::ChatType_CreateChatChannelFromGameServer:
         {
            PacketChatCreateChatChannelFromGameServer* pPacket = static_cast< PacketChatCreateChatChannelFromGameServer* > ( packet );
            //m_chatChannelManager->CreateNewChannel( pPacket );
         }
         break;
      case PacketChatToServer::ChatType_CreateChatChannelFromGameServerResponse:
         {
            assert( 0 );
         }
         break;
      case PacketChatToServer::ChatType_AddUserToChatChannelGameServer:
         {
            PacketChatAddUserToChatChannelGameServer* pPacket = static_cast< PacketChatAddUserToChatChannelGameServer* > ( packet );
            //m_chatChannelManager->AddUserToChannel( pPacket );
         }
         break;
      case PacketChatToServer::ChatType_AddUserToChatChannelGameServerResponse:
         {
            assert( 0 );
         }
         break;
      case PacketChatToServer::ChatType_RemoveUserFromChatChannelGameServer:
         {
            PacketChatRemoveUserFromChatChannelGameServer* pPacket = static_cast< PacketChatRemoveUserFromChatChannelGameServer* > ( packet );
            //m_chatChannelManager->RemoveUserFromChannel( pPacket );
         }
         break;
      case PacketChatToServer::ChatType_RemoveUserFromChatChannelGameServerResponse:
         {
            assert( 0 );
         }
         break;
      case PacketChatToServer::ChatType_DeleteChatChannelFromGameServer:
         {
            PacketChatDeleteChatChannelFromGameServer* pPacket = static_cast< PacketChatDeleteChatChannelFromGameServer* > ( packet );
            //m_chatChannelManager->DeleteChannel( pPacket );
         }
         break;
      case PacketChatToServer::ChatType_DeleteChatChannelFromGameServerResponse:
         {
            assert( 0 );
         }
         break;
  /* ChatType_InviteUserToChatChannel,
   ChatType_InviteUserToChatChannelResponse,*/
      }
   }

   return true;
}

//---------------------------------------------------------------

bool     DiplodocusChat::HandleLoginPacket( BasePacket* packet, U32 connectionId )
{
   //packet->
   U32 packetType = packet->packetType;
   U32 packetSubType = packet->packetSubType;

   if( packetType == PacketType_Login )
   {
      switch( packetSubType )
      {
    /*  case PacketLogin::LoginType_Login:
         {
            PacketLogin* pPacket = static_cast< PacketLogin* > ( packet );
            cout << "Login: " << pPacket->userName << ", " << pPacket->uuid << ", " << pPacket->password << endl;
            //pPacket->connectionId;
            //m_chatChannelManager->CreateNewChannel( pPacket );
            
         }
         return true;
      case PacketLogin::LoginType_Logout:
         {
            PacketLogout* pPacket = static_cast< PacketLogout* > ( packet );
            cout << "Logout: " << pPacket->wasDisconnectedByError << endl;
            //pPacket->connectionId;
            //m_chatChannelManager->CreateNewChannel( pPacket );
         }
         return true;*/
      case PacketLogin::LoginType_PrepareForUserLogin:
         {
            PacketPrepareForUserLogin* pPacket = static_cast< PacketPrepareForUserLogin* > ( packet );
            U32 conectionId = pPacket->connectionId;
            string uuid = pPacket->uuid;
            
            cout << "Prep for logon: " << connectionId << ", " << pPacket->userName << ", " << uuid << ", " << pPacket->password << endl;
            
            // TODO: verify that the user isn't already in the list and if s/he is, assign the new connectionId.
            ChatUser* user = UpdateExistingUsersConnectionId( uuid, connectionId );
            //ChatUser* user = GetUserByUsername( pPacket->userName );
            if( user == NULL )
            {
               user = CreateNewUser( connectionId );                        
               user->Init( pPacket->userId, pPacket->userName, pPacket->uuid, pPacket->lastLoginTime );
            }
            
            user->LoggedIn();

            
         }
         return true;
      case PacketLogin::LoginType_PrepareForUserLogout:
         {
            PacketPrepareForUserLogout* pPacket = static_cast< PacketPrepareForUserLogout* > ( packet );
            cout << "Prep for logout: " << pPacket->connectionId << ", " << pPacket->uuid << ", " << endl;
            U32 conectionId = pPacket->connectionId;
            Threading::MutexLock locker( m_mutex );
            UserMapIterator iter = m_users.find( connectionId );
            assert( iter != m_users.end() );
            iter->second->LoggedOut();
         }
         return true;
      }
   }
   return false;
}

//---------------------------------------------------------------

bool     DiplodocusChat::AddInputChainData( BasePacket* packet, U32 connectionId )
{

   if( packet->packetType == PacketType_GatewayInformation )
   {
      PacketCleaner cleaner( packet );
      HandleCommandFromGateway( packet, connectionId );
      return true;
   }

   if( packet->packetType == PacketType_ServerJobWrapper )
   {
      PacketCleaner cleaner( packet );
      HandlePacketFromOtherServer( packet, connectionId );
      return true;
   }
 
   if( packet->packetType == PacketType_DbQuery )
   {
   }

   return false;
}

//---------------------------------------------------------------

// data going out can go only a few directions
// coming from the DB, we can have a result or possibly a different packet meant for a single chat UserConnection
// otherwise, coming from a UserConnection, to go out, it will already be packaged as a Gateway Wrapper and then 
// we simply send it on.
bool   DiplodocusChat::AddOutputChainData( BasePacket* packet, U32 connectionId ) 
{
   if( packet->packetType == PacketType_ServerJobWrapper )
   {
      return HandlePacketToOtherServer( packet, connectionId );
   }

   Threading::MutexLock locker( m_mutex );

   if( packet->packetType == PacketType_DbQuery )
   {
      if( packet->packetSubType == BasePacketDbQuery::QueryType_Result )
      {
         PacketDbQueryResult* result = static_cast<PacketDbQueryResult*>( packet );
         Threading::MutexLock locker( m_mutex );
         m_dbQueries.push_back( result );
      }
      return true;
   }

   
   return false;
}

//---------------------------------------------------------------

void  DiplodocusChat::UpdateDbResults()
{
   PacketFactory factory;
   Threading::MutexLock locker( m_mutex );

   deque< PacketDbQueryResult* >::iterator it = m_dbQueries.begin();
   while( it != m_dbQueries.end() )
   {
      PacketDbQueryResult* result = *it++;
      BasePacket* packet = static_cast<BasePacket*>( result );

      U32 connectionId = result->id;
      bool isChatChannelManager = result->serverLookup > 0 ? true:false;

      if( isChatChannelManager )
      {
         m_chatChannelManager->HandleDbResult( result );
         m_chatChannelManagerNeedsUpdate = true;
      }
      else
      {
         ChatUser* user = GetUser( connectionId );
         if( user )
         {
            user->HandleDbResult( result );
         }
         else
         {
            factory.CleanupPacket( packet );
         }
      }
   }
}

//---------------------------------------------------------------

bool  DiplodocusChat::HandlePacketFromOtherServer( BasePacket* packet, U32 connectionId )// not thread safe
{
   if( packet->packetType != PacketType_ServerJobWrapper )
   {
      return false;
   }

   PacketServerJobWrapper* wrapper = static_cast< PacketServerJobWrapper* >( packet );
   BasePacket* unwrappedPacket = wrapper->pPacket;
   U32  serverIdLookup = wrapper->serverId;
   serverIdLookup = serverIdLookup;

   U8 packetType = unwrappedPacket->packetType;
   PacketFactory factory;
   
   if( packetType == PacketType_Login )
   {
      if( HandleLoginPacket( unwrappedPacket, connectionId ) == false )
      {
         factory.CleanupPacket( packet );
      }
      
      return true;
   }

   if( packetType == PacketType_Chat )
   {
      if( HandleChatPacket( unwrappedPacket, connectionId ) == false )
      {
         factory.CleanupPacket( packet );
      }
      return true;
   }

   return false;
}

//---------------------------------------------------------------
//---------------------------------------------------------------

void     DiplodocusChat::PeriodicWriteToDB()
{
  /* time_t currentTime;
   time( &currentTime );

   if( difftime( currentTime, m_lastDbWriteTimeStamp ) >= timeoutDBWriteStatisics ) 
   {
      m_lastDbWriteTimeStamp = ZeroOutMinutes( currentTime );// advance the hour.
   
      
   }*/
}

void     DiplodocusChat::RemoveLoggedOutUsers()
{
   time_t currentTime;
   time( &currentTime );

   UserMapIterator it = m_users.begin(); 
   while( it != m_users.end() )
   {
      bool wasRemoved = false;
      time_t loggedOutTime = it->second->GetLoggedOutTime();
      if( loggedOutTime != 0 )
      {
         if( difftime( currentTime, loggedOutTime ) >= logoutTimeout )
         {
            wasRemoved = true;
            m_users.erase( it );
         }
      }
      if( wasRemoved == false )
      {
         it++;
      }
   }
}
//---------------------------------------------------------------
//---------------------------------------------------------------

bool     DiplodocusChat::AddQueryToOutput( PacketDbQuery* dbQuery, U32 connectionId, bool isChatChannelManager )
{
   PacketFactory factory;
   dbQuery->id = m_connectionId;
   dbQuery->serverLookup = isChatChannelManager ? 1 : 0;

   ChainLinkIteratorType itOutputs = m_listOfOutputs.begin();
   while( itOutputs != m_listOfOutputs.end() )// only one output currently supported.
   {
      ChainType* outputPtr = static_cast< ChainType*> ( (*itOutputs).m_interface );
      if( outputPtr->AddInputChainData( dbQuery, m_chainId ) == true )
      {
         return true;
      }
      itOutputs++;
   }

   BasePacket* deleteMe = static_cast< BasePacket*>( dbQuery );

   factory.CleanupPacket( deleteMe );
   return false;
}

//---------------------------------------------------------------

void     DiplodocusChat::UpdateChatChannelManager()
{
   if( m_chatChannelManagerNeedsUpdate == false )
      return;
   m_chatChannelManager->Update();

   m_chatChannelManagerNeedsUpdate = false;
}

//---------------------------------------------------------------
//---------------------------------------------------------------

int      DiplodocusChat::CallbackFunction()
{
   UpdateAllConnections();

   time_t currentTime;
   time( &currentTime );


   UpdateConsoleWindow( m_timeOfLastTitleUpdate, m_uptime, m_numTotalConnections, m_connectedClients.size(), m_listeningPort, m_serverName );

   PeriodicWriteToDB();
   RemoveLoggedOutUsers();
   UpdateChatChannelManager();
   UpdateDbResults();

   return 1;
}

///////////////////////////////////////////////////////////////////