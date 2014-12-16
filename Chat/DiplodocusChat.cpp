// DiplodocusChat.cpp

#include <iostream>
#include <time.h>
#include <string>
using namespace std;


#include "../NetworkCommon/Packets/ServerToServerPacket.h"
#include "../NetworkCommon/Packets/PacketFactory.h"

#include "../NetworkCommon/Packets/DbPacket.h"
#include "../NetworkCommon/Packets/ChatPacket.h"
#include "../NetworkCommon/Packets/InvitationPacket.h"
#include "../NetworkCommon/Packets/LoginPacket.h"
#include "../NetworkCommon/Packets/AnalyticsPacket.h"

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
//using namespace boost;

#include "DiplodocusChat.h"
#include "ChatUser.h"
#include "ChatRoomManager.h"

#include "../NetworkCommon/NetworkIn/DiplodocusTools.h"

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

DiplodocusChat::DiplodocusChat( const string& serverName, U32 serverId ): ChainedType( serverName, serverId, 0,  ServerType_Chat ),
                                             StatTrackingConnections(),
                                             m_chatRoomManager( NULL ),
                                             m_chatRoomManagerNeedsUpdate( true ),
                                             m_invitationManager( NULL ),
                                             m_invitationManagerNeedsUpdate( true )
{
   this->SetSleepTime( 45 );

   time_t currentTime;
   time( &currentTime );
   m_timestampDailyStatServerStatisics = ZeroOutHours( currentTime );
   m_timestampHourlyStatServerStatisics = ZeroOutMinutes( currentTime );
   
}

//---------------------------------------------------------------

void  DiplodocusChat :: Init()
{
   ChatRoomManager::Set( this );
   m_chatRoomManager = new ChatRoomManager();
   m_chatRoomManager->SetDbIdentifier( 1 );
   //m_chatRoomManager->Init();

   InvitationManager::Set( this );
   InvitationManager::Set( m_chatRoomManager );
   m_invitationManager = new InvitationManager( Invitation::InvitationType_ChatRoom );
   m_invitationManager->SetDbIdentifier( 2 );
   m_invitationManager->Init();
   

   ChatUser::Set( this );
   ChatUser::Set( m_chatRoomManager );
   ChainedType::Init();

   m_chatRoomManager->Set( m_invitationManager );
}

//---------------------------------------------------------------

ChatUser* DiplodocusChat::CreateNewUser( U32 connectionId, U32 gatewayId )
{
   // Threading::MutexLock locker( m_mutex ); // must be locked from the outside
   UserMapIterator it = m_users.find( connectionId );
   if( it != m_users.end() )
      return it->second;

   ChatUser* user = new ChatUser( connectionId, gatewayId );
   m_users.insert( UserMapPair( connectionId, user ) );

   return user;
}

//---------------------------------------------------------------

ChatUser* DiplodocusChat::UpdateExistingUsersConnectionId( const string& uuid, U32 connectionId, U32 gatewayId )
{
   // Threading::MutexLock locker( m_mutex ); // must be locked from the outside
   UserMapIterator it = m_users.begin(); 
   while( it != m_users.end() )
   {
      if( it->second->GetUuid() == uuid )
      {
         ChatUser* user = it->second;
         user->SetConnectionId( connectionId );
         user->SetGatewayId( gatewayId );
         m_users.insert( UserMapPair( connectionId, user ) );
         m_users.erase( it );
         return user;
      }
      it++;
   }

   return NULL;
}

//---------------------------------------------------------------

ChatUser* DiplodocusChat::GetUser( U32 connectionId )
{
   //Threading::MutexLock locker( m_mutex );
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

ChatUser*   DiplodocusChat::GetUserById( U32 userId )
{
   //Threading::MutexLock locker( m_mutex );
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

ChatUser*   DiplodocusChat::GetUserByUuid( const string& uuid )
{
   //Threading::MutexLock locker( m_mutex );
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

ChatUser*   DiplodocusChat::GetUserByUsername( const string& userName )
{
   //Threading::MutexLock locker( m_mutex );
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

ChatUser*    DiplodocusChat::GetUserByConnectionId( U32 ConnectionId )
{
   //Threading::MutexLock locker( m_mutex );
   UserMapIterator it = m_users.begin(); 
   while( it != m_users.end() )
   {
      if( it->second->GetConnectionId() == ConnectionId )
         return it->second;
      it++;
   }

   return NULL;
}

string      DiplodocusChat::GetUserUuidByConnectionId( U32 connectionId )
{
   //Threading::MutexLock locker( m_mutex );
   const ChatUser*  user = GetUserByConnectionId( connectionId );
   if( user == NULL )
      return string();
   return user->GetUuid();
}

void        DiplodocusChat::GetUserConnectionId( const string& uuid, U32& connectionId, U32& gatewayId )
{
   //Threading::MutexLock locker( m_mutex );
   connectionId = 0;

   ChatUser* user = GetUserByUuid( uuid );
   if( user )
   {
      connectionId = user->GetConnectionId();
      gatewayId = user->GetGatewayId();
   }
}

string      DiplodocusChat::GetUserName( const string& uuid )
{
   //Threading::MutexLock locker( m_mutex );
   const ChatUser*  user = GetUserByUuid( uuid );
   if( user == NULL )
      return string();
   return user->GetUuid();
}

//---------------------------------------------------------------
//---------------------------------------------------------------

void     DiplodocusChat::ServerWasIdentified( IChainedInterface* khaan )
{
   /*
   BasePacket* packet = NULL;
   PackageForServerIdentification( m_serverName, m_localIpAddress, m_externalIpAddress, m_serverId, m_serverType, m_listeningPort, m_gameProductId, m_isGame, m_isControllerApp, true, m_gatewayType, &packet );
   ChainedType* localKhaan = static_cast< ChainedType* >( khaan );
   localKhaan->AddOutputChainData( packet, 0 );
   m_clientsNeedingUpdate.push_back( localKhaan->GetServerId() );*/
   BasePacket* packet = NULL;
   PackageForServerIdentification( m_serverName, m_localIpAddress, m_externalIpAddress, m_serverId, m_serverType, m_listeningPort, m_gameProductId, m_isGame, m_isControllerApp, true, m_gatewayType, &packet );
   Khaan* localKhaan = static_cast< Khaan* >( khaan );
   localKhaan->AddOutputChainDataNoLock( packet );
   // this is not thread safe, but will be invoked from within the same thread.
   m_clientsNeedingUpdate.push_back( localKhaan->GetChainedId() );
}

/////////////////////////////////////////////////////////////////////////////////

void     DiplodocusChat::InputRemovalInProgress( IChainedInterface * chainedInput )
{
   KhaanChat* khaan = static_cast< KhaanChat* >( chainedInput );

   SetupClientWaitingToBeRemoved( khaan );

   string currentTime = GetDateInUTC();
   string printer = "DiplodocusChat::Client disconnection at time:" + currentTime + " from " + inet_ntoa( khaan->GetIPAddress().sin_addr );
   LogMessage( LOG_PRIO_ERR, printer.c_str() );

   LogMessage( LOG_PRIO_ERR, "** InputRemovalInProgress" );
}

//---------------------------------------------------------------

bool     DiplodocusChat::HandleChatPacket( BasePacket* packet, U32 gatewayId )
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
            m_chatRoomManager->CreateNewRoom( pPacket );
         }
         break;
      case PacketChatToServer::ChatType_AddUserToChatChannelGameServer:
         {
            PacketChatAddUserToChatChannelGameServer* pPacket = static_cast< PacketChatAddUserToChatChannelGameServer* > ( packet );
            m_chatRoomManager->AddUserToRoom( pPacket );
         }
         break;
      case PacketChatToServer::ChatType_RemoveUserFromChatChannelGameServer:
         {
            PacketChatRemoveUserFromChatChannelGameServer* pPacket = static_cast< PacketChatRemoveUserFromChatChannelGameServer* > ( packet );
            m_chatRoomManager->RemoveUserFromRoom( pPacket );
         }
         break;
      case PacketChatToServer::ChatType_DeleteChatChannelFromGameServer:
         {
            PacketChatDeleteChatChannelFromGameServer* pPacket = static_cast< PacketChatDeleteChatChannelFromGameServer* > ( packet );
            m_chatRoomManager->DeleteRoom( pPacket );
         }
         break;
      case PacketChatToServer::ChatType_ListAllMembersInChatChannel:
         {
            PacketChatListAllMembersInChatChannel* pPacket = static_cast< PacketChatListAllMembersInChatChannel* > ( packet );
            //pPacket->
            m_chatRoomManager->RequestChatRoomInfo( pPacket, gatewayId );
         }
  /* ChatType_InviteUserToChatChannel,
   ChatType_InviteUserToChatChannelResponse,*/
      }
   }

   return true;
}

//---------------------------------------------------------------

bool     DiplodocusChat::HandleInvitationPacket( BasePacket* packet, U32 connectionId, U32 gatewayId )
{
   UserMapIterator it = m_users.find( connectionId );
   if( it != m_users.end() )
   {
      ChatUser* user = it->second;
      if( user )
      {
         gatewayId = user->GetGatewayId();
      }
   }
   PacketInvitation* invitation = static_cast< PacketInvitation* > ( packet );
   if( m_invitationManager->HandlePacketRequest( invitation, connectionId, gatewayId ) == false )
   {
      PacketFactory factory;
      factory.CleanupPacket( packet );
   }

   return true;
}
//---------------------------------------------------------------

bool     DiplodocusChat::HandleLoginPacket( BasePacket* packet, U32 gatewayId )
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
            cout << "Login: " << pPacket->userName << ", " << pPacket->uuid << ", " << pPacket->password );
            //pPacket->connectionId;
            //m_chatRoomManager->CreateNewChannel( pPacket );
            
         }
         return true;
      case PacketLogin::LoginType_Logout:
         {
            PacketLogout* pPacket = static_cast< PacketLogout* > ( packet );
            cout << "Logout: " << pPacket->wasDisconnectedByError );
            //pPacket->connectionId;
            //m_chatRoomManager->CreateNewChannel( pPacket );
         }
         return true;*/
      case PacketLogin::LoginType_PrepareForUserLogin:
         {
            PacketPrepareForUserLogin* loginPacket = static_cast< PacketPrepareForUserLogin* > ( packet );
            U32 userConnectionId = loginPacket->connectionId;
            string uuid = loginPacket->uuid;
            U32 whichGateway = loginPacket->gatewayId;
            
            //LogMessage( LOG_PRIO_INFO, "Prep for logon: %d, %s, %s, %s", userConnectionId, pPacket->userName.c_str(), uuid.c_str(), pPacket->password.c_str() );
            
            LogMessage_LoginPacket( loginPacket );
            
            LockMutex();
            // TODO: verify that the user isn't already in the list and if s/he is, assign the new connectionId.
            ChatUser* user = UpdateExistingUsersConnectionId( uuid, userConnectionId, whichGateway );
            //ChatUser* user = GetUserByUsername( pPacket->userName );
            if( user == NULL )
            {
               user = CreateNewUser( userConnectionId, whichGateway );
            }

            user->Init( loginPacket->userId, loginPacket->userName, loginPacket->uuid, loginPacket->lastLoginTime );            
            user->LoggedIn();
            UnlockMutex();
         }
         return true;
      case PacketLogin::LoginType_PrepareForUserLogout:
         {
            PacketPrepareForUserLogout* logoutPacket = static_cast< PacketPrepareForUserLogout* > ( packet );
            LogMessage_LogoutPacket( logoutPacket );
            //LogMessage( LOG_PRIO_INFO, "Prep for logout: %d, %s", logoutPacket->connectionId, logoutPacket->uuid.c_str() );
            U32 userConnectionId = logoutPacket->connectionId;
            LockMutex();
            UserMapIterator iter = m_users.find( userConnectionId );
         
            if( iter != m_users.end() )// a bad user login can cause this so don't worry
            {
               iter->second->LoggedOut();
            }
            else
            {
               string str = "Log user out failed: user not found. userUuid: ";
               str += logoutPacket->uuid.c_str();
               Log( str, 4 );
            }
            UnlockMutex();
            
         }
         return true;
      }
   }
   return false;
}

//---------------------------------------------------------------

bool     DiplodocusChat::AddInputChainData( BasePacket* packet, U32 connectionId )
{
   m_mutex.lock();
   m_inputPacketsToBeProcessed.push_back( PacketStorage( packet, connectionId ) );
   m_mutex.unlock();

   return true;
}

//---------------------------------------------------------------

//---------------------------------------------------------------

bool     DiplodocusChat:: ProcessPacket( PacketStorage& storage )
{
   BasePacket* packet = storage.packet;
   U32 gatewayId = storage.gatewayId;

   U8 packetType = packet->packetType;

   //cout << "DiplodocusChat:: ProcessPacket <<<" << endl;

   switch( packetType )
   {
   case PacketType_GatewayInformation:
      {
         PacketCleaner cleaner( packet );
         HandleCommandFromGateway( packet, gatewayId );
         return true;
      }
   case PacketType_ServerJobWrapper:
      {
         PacketCleaner cleaner( packet );
         HandlePacketFromOtherServer( packet, gatewayId );
         return true;
      }
   case PacketType_GatewayWrapper:
      {
         PacketCleaner cleaner( packet );
         HandlePacketFromClient( packet );
         return true;
      }
   }

   //cout << "DiplodocusChat:: ProcessPacket >>>" << endl;
   PacketCleaner cleaner( packet );
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

   if( packet->packetType == PacketType_DbQuery )
   {
      //cout << "DiplodocusChat::AddOutputChainData.. PacketType_DbQuery" << endl;
      if( packet->packetSubType == BasePacketDbQuery::QueryType_Result )
      {
         PacketDbQueryResult* result = static_cast<PacketDbQueryResult*>( packet );

         //cout << "DiplodocusChat::AddOutputChainData.. locker( m_mutex ) <<<" << endl;
         Threading::MutexLock locker( m_mutex );
         //cout << "DiplodocusChat::AddOutputChainData.. locker( m_mutex ) >>>" << endl;

         m_dbQueries.push_back( result );
       /*  if( result->customData != NULL )
            cout << "AddOutputChainData: Non-null custom data " );*/
      }
      return true;
   }

   
   return false;
}

//---------------------------------------------------------------

bool     DiplodocusChat::SendMessageToClient( BasePacket* packet, U32 connectionId, U32 gatewayId )
{
   if( packet->packetType == PacketType_GatewayWrapper )// this is already wrapped up and ready for the gateway... send it on.
   {
      //Threading::MutexLock locker( m_inputChainListMutex );
      ClientMapIterator itInputs = m_connectedClients.begin();
      while( itInputs != m_connectedClients.end() )// only one output currently supported.
      {
         KhaanChat* khaan = static_cast< KhaanChat* >( itInputs->second );
         if( khaan->GetChainedType() == ChainedType_InboundSocketConnector && 
            khaan->GetServerId() == gatewayId )
         {
            khaan->AddOutputChainData( packet );
            MarkConnectionAsNeedingUpdate( khaan->GetChainedId() );
            return true;
         }
         itInputs++;
      }
      return true;
   }

  /* if( packet->packetType == PacketType_DbQuery )
   {
      Threading::MutexLock locker( m_outputChainListMutex );
      // we don't do much interpretation here, we simply pass output data onto our output, which should be the DB or other servers.
      ChainLinkIteratorType itOutputs = m_listOfOutputs.begin();
      while( itOutputs != m_listOfOutputs.end() )// only one output currently supported.
      {
         ChainType* outputPtr = static_cast< ChainType*> ( (*itOutputs).m_interface );
         if( outputPtr->AddInputChainData( packet, m_chainId ) == true )
         {
            break;
         }
         itOutputs++;
      }
      return true;
   }*/

   return false;
}

//---------------------------------------------------------------

void  DiplodocusChat::UpdateDbResults()
{
   PacketFactory factory;

   m_mutex.lock();
      deque< PacketDbQueryResult* > tempQueue = m_dbQueries;
      m_dbQueries.clear();
   m_mutex.unlock();

   deque< PacketDbQueryResult* >::iterator it = tempQueue.begin();
   while( it != tempQueue.end() )
   {
      PacketDbQueryResult* dbResult = *it++;
    /*  if( dbResult->customData != NULL )
            cout << "UpdateDbResults: Non-null custom data " );*/
      BasePacket* packet = static_cast<BasePacket*>( dbResult );

      U32 connectionId = dbResult->id;

      if( dbResult->serverLookup == m_chatRoomManager->GetDbIdentifier() ) //&& connectionId == ChatChannelManagerUniqueId )
      {
         if( m_chatRoomManager->HandleDbResult( dbResult ) == false )
         {
            factory.CleanupPacket( packet );
         }
         m_chatRoomManagerNeedsUpdate = true;
      }
      else if( dbResult->serverLookup == m_invitationManager->GetDbIdentifier() ) //&& connectionId == ChatChannelManagerUniqueId )
      {
         if( m_invitationManager->HandleDbResult( dbResult ) == false )
         {
            factory.CleanupPacket( packet );
         }
         m_invitationManagerNeedsUpdate = true;
      }
      else
      {
         ChatUser* user = GetUser( connectionId );
         if( user )
         {
            user->HandleDbResult( dbResult );
         }
         else
         {
            factory.CleanupPacket( packet );
         }
      }
   }
}

//---------------------------------------------------------------

bool  DiplodocusChat::HandlePacketFromOtherServer( BasePacket* packet, U32 gatewayId )
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
      if( HandleLoginPacket( unwrappedPacket, gatewayId ) == false )
      {
         factory.CleanupPacket( packet );
      }      
      return true;
   }

   if( packetType == PacketType_Chat )
   {
      if( HandleChatPacket( unwrappedPacket, gatewayId ) == false )
      {
         factory.CleanupPacket( packet );
      }
      return true;
   }

   if( packetType == PacketType_Invitation )
   {
      if( HandleInvitationPacket( unwrappedPacket, gatewayId, 0 ) == false )
      {
         factory.CleanupPacket( packet );
      }
      return true;
   }

   return false;
}

//---------------------------------------------------------------

bool  DiplodocusChat::HandlePacketFromClient( BasePacket* packet )
{
   if( packet->packetType != PacketType_GatewayWrapper )
   {
      return false;
   }

   //cout << "DiplodocusChat::HandlePacketFromClient <<<" << endl;

   PacketGatewayWrapper* wrapper = static_cast< PacketGatewayWrapper* >( packet );
   BasePacket* unwrappedPacket = wrapper->pPacket;

   U32 connectionId = wrapper->connectionId;
   //wrapper->

   ChatUser* user = NULL;
   //m_mutex.lock();// no longer needed
   UserMapIterator it = m_users.find( connectionId );
   user = it->second;
   //m_mutex.unlock();
   if( user )
   {
      // could be a switch but we only have two cases.
      if( unwrappedPacket->packetType == PacketType_Invitation )
      {
         m_invitationManager->HandlePacketRequest( unwrappedPacket, connectionId, user->GetGatewayId() );
      }
      else
      {
      //PacketCleaner cleaner( packet );
         bool result = user->HandleClientRequest( unwrappedPacket );
         result = result;
      }

      //cout << "DiplodocusChat::HandlePacketFromClient >>>+" << endl;
      return true;
   }

   //cout << "DiplodocusChat::HandlePacketFromClient >>>-" << endl;

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

   Threading::MutexLock locker( m_mutex );
   UserMapIterator it = m_users.begin(); 
   while( it != m_users.end() )
   {
      UserMapIterator currentUserIt = it++;
      bool wasRemoved = false;
      time_t loggedOutTime = currentUserIt->second->GetLoggedOutTime();
      if( loggedOutTime != 0 )
      {
         if( difftime( currentTime, loggedOutTime ) >= logoutTimeout )
         {
            wasRemoved = true;
            m_users.erase( currentUserIt );
         }
      }
   }
}
//---------------------------------------------------------------
//---------------------------------------------------------------
/*
bool     DiplodocusChat::AddQueryToOutput( PacketDbQuery* dbQuery, U32 connectionId )
{
   PacketFactory factory;
   dbQuery->id = connectionId;

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
}*/
bool     DiplodocusChat::AddQueryToOutput( PacketDbQuery* dbQuery, U32 connectionId )
{
   //cout << "DiplodocusChat::AddQueryToOutput <<<" << endl;
   PacketFactory factory;
   dbQuery->id = connectionId;

   ChainLinkIteratorType itOutputs = m_listOfOutputs.begin();
   while( itOutputs != m_listOfOutputs.end() )// only one output currently supported.
   {
      ChainType* outputPtr = static_cast< ChainType*> ( (*itOutputs).m_interface );
      //ChainedInterface* interfacePtr = static_cast< ChainedInterface* >( outputPtr );
      if( outputPtr->GetChainedType() == ChainedType_DatabaseConnector )
      {
         bool isValidConnection = false;
         Database::Deltadromeus* delta = static_cast< Database::Deltadromeus* >( outputPtr );
         if( dbQuery->dbConnectionType != 0 )
         {
            //cout << "DiplodocusChat::delta->WillYouTakeThisQuery <<<" << endl;
            if( delta->WillYouTakeThisQuery( dbQuery->dbConnectionType ) )
            {
               isValidConnection = true;
            }
         }
         else // if this query is not set, default to true
         {
            isValidConnection = true;
         }
         if( isValidConnection == true )
         {
            //cout << "DiplodocusChat::outputPtr->AddInputChainData <<<" << endl;
            if( outputPtr->AddInputChainData( dbQuery, m_chainId ) == true )
            {
               //cout << "DiplodocusChat::AddQueryToOutput >>>+" << endl;
               return true;
            }
         }
      }
      itOutputs++;
   }

   BasePacket* deleteMe = static_cast< BasePacket*>( dbQuery );

   factory.CleanupPacket( deleteMe );
   //cout << "DiplodocusChat::AddQueryToOutput >>>-" << endl;
   return false;
}

//---------------------------------------------------------------

bool     DiplodocusChat::SendErrorToClient( U32 connectionId, U32 gatewayId, PacketErrorReport::ErrorType error )
{
   return ChainedType:: SendErrorToClient( connectionId, gatewayId, error, 0 );
}

//---------------------------------------------------------------

void     DiplodocusChat::UpdateChatChannelManager()
{
   if( m_chatRoomManagerNeedsUpdate == false || m_chatRoomManager == NULL )
      return;

   if( m_chatRoomManager->Update() == true )
   {
      m_chatRoomManagerNeedsUpdate = false;
   }
}

//---------------------------------------------------------------

void     DiplodocusChat::UpdateInvitationManager()
{
   if( m_invitationManagerNeedsUpdate == false || m_invitationManager == NULL )
      return;

   if( m_invitationManager->Update() == true )
   {
      m_invitationManagerNeedsUpdate = false;
   }
}

//---------------------------------------------------------------

void     DiplodocusChat::UpdateAllChatUsers()
{
   Threading::MutexLock locker( m_mutex );
   UserMapIterator it = m_users.begin();
   while( it != m_users.end() )
   {
      //string userID = it->first;

      ChatUser* user = it->second;
      it++;
      user->Update();
   }
}

//-----------------------------------------------------------------------------------------

void     DiplodocusChat::TrackCountStats( StatTracking stat, float value, int sub_category )
{
   StatTrackingConnections::TrackCountStats( m_serverName, m_serverId, stat, value, sub_category );
}

//---------------------------------------------------------------

void     DiplodocusChat::RunHourlyStats()
{
   if( m_chatRoomManager == NULL )
      return;

   time_t currentTime;
   time( &currentTime );

   if( difftime( currentTime, m_timestampHourlyStatServerStatisics ) >= timeoutHourlyStatisics ) 
   {
      m_timestampHourlyStatServerStatisics = ZeroOutMinutes( currentTime );

      //--------------------------------

      int totalCount = m_chatRoomManager->GetNumChannelChatsSent() + m_chatRoomManager->GetNumP2PChatsSent();
      TrackCountStats( StatTracking_ChatNumberOfChatsSentPerHour, static_cast< float >( totalCount ), 0 );
      TrackCountStats( StatTracking_ChatNumberOfChannelChatsSentPerHour, static_cast< float >( m_chatRoomManager->GetNumChannelChatsSent() ), 0 );
      TrackCountStats( StatTracking_ChatNumberOfP2PChatsSentPerHour, static_cast< float >( m_chatRoomManager->GetNumP2PChatsSent() ), 0 );
      TrackCountStats( StatTracking_ChatNumberOfChatChannelChangesPerHour, static_cast< float >( m_chatRoomManager->GetNumChangesToChatRoom() ), 0 );
      m_chatRoomManager->ClearStats();
   }
}

//---------------------------------------------------------------

void     DiplodocusChat::RunDailyStats()
{
   if( m_chatRoomManager == NULL )
      return;

   time_t currentTime;
   time( &currentTime );
   if( difftime( currentTime, m_timestampDailyStatServerStatisics ) >= timeoutDailyStatisics ) 
   {
      m_timestampDailyStatServerStatisics = ZeroOutHours( currentTime );

     /* float numUniqueUsers = static_cast< float >( m_uniqueUsers.size() );
      if( numUniqueUsers == 0 )
         return;

      ClearOutUniqueUsersNotLoggedIn();

      TrackCountStats( StatTracking_UniquesUsersPerDay, numUniqueUsers, 0 );*/
   }
}

//---------------------------------------------------------------
//---------------------------------------------------------------

int      DiplodocusChat::CallbackFunction()
{
   CommonUpdate();

   CleanupOldClientConnections( "KhaanChat" );

   UpdateAllConnections( "KhaanChat" );

   time_t currentTime;
   time( &currentTime );

   //int numClients = static_cast< int >( m_connectedClients.size() );
   //UpdateConsoleWindow( m_timeOfLastTitleUpdate, m_uptime, m_numTotalConnections, numClients, m_listeningPort, m_serverName );

   UpdateInputPacketToBeProcessed();

   PeriodicWriteToDB();
   RemoveLoggedOutUsers();
   UpdateChatChannelManager();
   UpdateInvitationManager();
   UpdateDbResults();
   UpdateAllChatUsers();

   RunHourlyStats();
   RunDailyStats();
   StatTrackingConnections::SendStatsToStatServer( m_listOfOutputs, m_serverName, m_serverId, m_serverType );

   return 1;
}

///////////////////////////////////////////////////////////////////