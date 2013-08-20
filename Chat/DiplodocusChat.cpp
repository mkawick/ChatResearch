// DiplodocusChat .cpp

#include "DiplodocusChat.h"
#include "ChatChannelManager.h"
#include "../NetworkCommon/NetworkIn/DiplodocusServerToServer.h"

#include "../NetworkCommon/Packets/DbPacket.h"
#include "../NetworkCommon/Packets/GamePacket.h"
#include "../NetworkCommon/Packets/ChatPacket.h"
#include "../NetworkCommon/Packets/ServerToServerPacket.h"

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

DiplodocusChat :: DiplodocusChat( const string& serverName, U32 serverId ) : Diplodocus< KhaanChat >( serverName, serverId, 0,  ServerType_Chat ), m_inputsNeedUpdate( false ), m_chatChannelManagerNeedsUpdate( false )
{
   this->SetSleepTime( 33 );// 30 fps
}

void  DiplodocusChat :: Init()
{
   ChatChannelManager::SetDiplodocusChat( this );
   
   m_chatChannelManager = new ChatChannelManager();
   m_chatChannelManager->SetConnectionId( ChatChannelManagerConnectionId );

   UserConnection::SetDiplodocusChat( this );
   UserConnection::SetChatManager( m_chatChannelManager );
   m_chatChannelManager->Init();
}
//---------------------------------------------------------------

void  DiplodocusChat::ClientConnectionFinishedAdding( KhaanChat* khaan )
{
}

void  DiplodocusChat::ClientConnectionIsAboutToRemove( KhaanChat* khaan )
{
   // inheritance is hurting us here. the base class GetConnectionId which is invoked later
   // hides the derived version due to the type of pointers.
   
   ClientMapIterator it = m_connectedClients.find( khaan->GetConnectionId() );
   if( it != m_connectedClients.end() )
   {
      LockMutex();
      m_connectedClients.erase( it );
      UnlockMutex();
   }
}

//---------------------------------------------------------------

// this will always be data coming from the gateway or at least from the outside in.
bool   DiplodocusChat::AddInputChainData( BasePacket* packet, U32 connectionId ) 
{
   if( packet->packetType == PacketType_GatewayInformation )
   {
      return HandleCommandFromGateway( packet, connectionId );
   }

   if( packet->packetType == PacketType_ServerJobWrapper )
   {
      HandlePacketFromOtherServer( packet, connectionId );
      return true;
   }
   
   m_mutex.lock();

      ConnectionMapIterator it = m_connectionMap.find( connectionId );
      //assert( it != m_connectionMap.end() );// this had better not happen

      

   m_mutex.unlock();

   if( it == m_connectionMap.end() )
   {
      Log( "Chat server: User attempting to send data on unauthorized channel" );
      return false;
   }
   UserConnection* conn = (*it).second;
   
   conn->AddInputChainData( packet, connectionId );
   m_connectionsNeedingUpdate.push_back( connectionId );

   return true;
}

//---------------------------------------------------------------

bool     DiplodocusChat::SendErrorReportToClient( PacketErrorReport::ErrorType error, int connectionId )
{
   PacketGatewayWrapper* wrapper = new PacketGatewayWrapper;
   wrapper->connectionId = connectionId;
   wrapper->pPacket = new PacketErrorReport( error );
   AddPacketFromUserConnection( wrapper, connectionId );

   return true;
}

//---------------------------------------------------------------

//---------------------------------------------------------------

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
         U32 connectionId = result->id;
         if( connectionId == m_chatChannelManager->GetConnectionId() )
         {
            m_chatChannelManager->AddInputChainData( packet );
            m_chatChannelManagerNeedsUpdate = true;
         }
         else
         {
            m_mutex.lock();
            ConnectionMapIterator it = m_connectionMap.find( connectionId );
            if( it == m_connectionMap.end() )
            {
               Log( "Missing connection" );
               assert( 0 );
            }
            else
            {
               (*it).second->AddInputChainData( packet, connectionId );
               m_connectionsNeedingUpdate.push_back( connectionId );
            }
            m_mutex.unlock();
         }
      }
      return true;
   }

   
   return false;
}

//---------------------------------------------------------------

void  DiplodocusChat::SetupForNewUserConnection( PacketPrepareForUserLogin* loginPacket )
{
   U32 connectionId = loginPacket->connectionId;
   ConnectionMapIterator it = m_connectionMap.find( connectionId );
   if( it != m_connectionMap.end() )
   {
      // we already have this which should be a major problem
      Log( "Chat server: Attempt to add a user connection fails" );
      return;
   }

   bool found = false;
   // if the user is already here but relogged, simply 
   m_mutex.lock();
      it = m_connectionMap.begin();
      while( it != m_connectionMap.end() )
      {
         if( it->second->GetUuid() == loginPacket->uuid ) 
         {
            found = true;
            it->second->SetConnectionId( connectionId );
            it->second->InformUserOfSuccessfulLogin();

            m_connectionMap.insert( ConnectionPair( connectionId, it->second ) );
            m_connectionMap.erase( it );
            break;
         }
         it++;
      }
   m_mutex.unlock();

   if( found == false )// almost 100% true
   {
      UserConnection* connection = new UserConnection( connectionId );

      m_mutex.lock();// the user must be in the list before any further communicatins can continue.
         m_connectionMap.insert( ConnectionPair ( connectionId, connection ) );
      m_mutex.unlock();

      connection->SetupFromLogin( loginPacket->userId, loginPacket->username, loginPacket->uuid, loginPacket->lastLoginTime );
   }
}

//---------------------------------------------------------------

void  DiplodocusChat::HandleUserDisconnection( PacketPrepareForUserLogout* logoutPacket )
{
   U32 connectionId = logoutPacket->connectionId;
   ConnectionMapIterator it = m_connectionMap.find( connectionId );
   if( it == m_connectionMap.end() )
   {
      // we already have this which should be a major problem
      Log( "Chat server: Attempt to remove a user connection fails" );
      return;
   }

   const string& username = it->second->GetName();
   const string& uuid = it->second->GetUuid();
   
   m_chatChannelManager->UserLoggedOut( username, uuid, it->second );

   m_mutex.lock();
      //************************************************
      // danger here: this object being deleted can cause lots of problems
      //************************************************

      FinishedLogout( connectionId, uuid );
   m_mutex.unlock();

}

//---------------------------------------------------------------

bool  DiplodocusChat::HandlePacketFromOtherServer( BasePacket* packet, U32 connectionId )// not thread safe
{
   if( packet->packetType != PacketType_ServerJobWrapper )
   {
      return false;
   }

   PacketServerJobWrapper* wrapper = static_cast< PacketServerJobWrapper* >( packet );
   BasePacket* actualPacket = wrapper->pPacket;
   U32  serverIdLookup = wrapper->serverId;

   delete wrapper;
   bool success = false;

   if( actualPacket->packetType == PacketType_Login )
   {
      switch( actualPacket->packetSubType )
      {
      case PacketLogin::LoginType_PrepareForUserLogin:
         SetupForNewUserConnection( static_cast< PacketPrepareForUserLogin* >( actualPacket ) );
         return true;
      case PacketLogin::LoginType_PrepareForUserLogout:
         HandleUserDisconnection( static_cast< PacketPrepareForUserLogout* >( actualPacket ) );
         return true;
      }
      return false;
   }
   else if( actualPacket->packetType == PacketType_Gameplay )
   {
     /* switch( actualPacket->packetSubType )
      {
      case PacketGameToServer::GamePacketType_CreateGame:
         {
            PacketCreateGame* pPacket = static_cast< PacketCreateGame* > ( actualPacket );
            //success = m_chatChannelManager->CreateNewChannel( pPacket->name, serverIdLookup );
         }
         break;
      case PacketGameToServer::GamePacketType_DeleteGame:
         {
            PacketDeleteGame* pPacket = static_cast< PacketDeleteGame* > ( actualPacket );
            success = m_chatChannelManager->DeleteChannel( pPacket->uuid, serverIdLookup );
         }
         break;
      case PacketGameToServer::GamePacketType_ForfeitGame:
         {
            PacketForfeitGame* pPacket = static_cast< PacketForfeitGame* > ( actualPacket );
            assert( 0 );// not complete
            //success = m_chatChannelManager->CreateNewChannel( actualPacket->name, serverIdLookup );
         }
         break;
      case PacketGameToServer::GamePacketType_QuitGame:
         {
            PacketQuitGame* pPacket = static_cast< PacketQuitGame* > ( actualPacket );
            assert( 0 );// not complete
            //success = m_chatChannelManager->CreateNewChannel( actualPacket->name, serverIdLookup );
         }
         break;
      case PacketGameToServer::GamePacketType_AddUser:
         {
            PacketAddUserToGame* pPacket = static_cast< PacketAddUserToGame* > ( actualPacket );
            //success = m_chatChannelManager->AddUserToChannel( pPacket->gameUuid, pPacket->userUuid, serverIdLookup );
         }
         break;
      case PacketGameToServer::GamePacketType_RemoveUser:
         {
            PacketRemoveUserFromGame* pPacket = static_cast< PacketRemoveUserFromGame* > ( actualPacket );
            //success = m_chatChannelManager->RemoveUserFromChannel( pPacket->gameUuid, pPacket->userUuid, serverIdLookup );
         }
         break;
      case PacketGameToServer::GamePacketType_AdvanceTurn:
         {
            PacketGameAdvanceTurn* pPacket = static_cast< PacketGameAdvanceTurn* > ( actualPacket );
            success = m_chatChannelManager->AdvanceGameTurn( pPacket->gameUuid, serverIdLookup );
         }
         break;
      case PacketGameToServer::GamePacketType_RequestListOfGames:
         {
            PacketRequestListOfGames* pPacket = static_cast< PacketRequestListOfGames* > ( actualPacket );
            m_chatChannelManager->RequestChatChannelList( serverIdLookup );
            success = true;
         }
         break;
      }*/

   }
   if( actualPacket->packetType == PacketType_Chat )
   {
      switch( actualPacket->packetSubType )
      {
      case PacketChatToServer::ChatType_CreateChatChannelFromGameServer:
         {
            PacketChatCreateChatChannelFromGameServer* pPacket = static_cast< PacketChatCreateChatChannelFromGameServer* > ( actualPacket );
            m_chatChannelManager->CreateNewChannel( pPacket );
         }
         break;
      case PacketChatToServer::ChatType_CreateChatChannelFromGameServerResponse:
         {
            assert( 0 );
         }
         break;
      case PacketChatToServer::ChatType_AddUserToChatChannelGameServer:
         {
            PacketChatAddUserToChatChannelGameServer* pPacket = static_cast< PacketChatAddUserToChatChannelGameServer* > ( actualPacket );
            m_chatChannelManager->AddUserToChannel( pPacket );
         }
         break;
      case PacketChatToServer::ChatType_AddUserToChatChannelGameServerResponse:
         {
            assert( 0 );
         }
         break;
      case PacketChatToServer::ChatType_RemoveUserFromChatChannelGameServer:
         {
            PacketChatRemoveUserFromChatChannelGameServer* pPacket = static_cast< PacketChatRemoveUserFromChatChannelGameServer* > ( actualPacket );
            m_chatChannelManager->RemoveUserFromChannel( pPacket );
         }
         break;
      case PacketChatToServer::ChatType_RemoveUserFromChatChannelGameServerResponse:
         {
            assert( 0 );
         }
         break;
  /* ChatType_InviteUserToChatChannel,
   ChatType_InviteUserToChatChannelResponse,*/
      }
   }

   delete actualPacket;
   return success;
}

//---------------------------------------------------------------

bool  DiplodocusChat::HandlePacketToOtherServer( BasePacket* packet, U32 connectionId )// not thread safe
{
   ChainLinkIteratorType itInputs = m_listOfInputs.begin();
   while( itInputs != m_listOfInputs.end() )// only one output currently supported.
   {
      ChainedInterface* inputPtr = itInputs->m_interface;
      if( inputPtr->GetConnectionId() == ServerToServerConnectionId )
      {
         if( inputPtr->AddOutputChainData( packet, connectionId ) )
         {
            return true;
         }
      }

      itInputs++;
   }
   assert( 0 );// should not happen
   //delete packet;
   return false;
}

//---------------------------------------------------------------

bool  DiplodocusChat::AddPacketFromUserConnection( BasePacket* packet, U32 connectionId )// not thread safe
{
   if( packet->packetType == PacketType_GatewayWrapper )// this is already wrapped up and ready for the gateway... send it on.
   {
      Threading::MutexLock locker( m_mutex );

      ClientMapIterator itInputs = m_connectedClients.begin();
      if( itInputs != m_connectedClients.end() )// only one output currently supported.
      {
         KhaanChat* khaan = static_cast< KhaanChat* >( itInputs->second );
         khaan->AddOutputChainData( packet );
         m_clientsNeedingUpdate.push_back( khaan->GetConnectionId() );
         itInputs++;
      }
      return true;
   }

   if( packet->packetType == PacketType_DbQuery )
   {
      Threading::MutexLock locker( m_outputChainListMutex );
      // we don't do much interpretation here, we simply pass output data onto our output, which should be the DB or other servers.
      ChainLinkIteratorType itOutputs = m_listOfOutputs.begin();
      while( itOutputs != m_listOfOutputs.end() )// only one output currently supported.
      {
         ChainedInterface* outputPtr = (*itOutputs).m_interface;
         if( outputPtr->AddInputChainData( packet, m_chainId ) == true )
         {
            break;
         }
         itOutputs++;
      }
      return true;
   }

   return false;
}

//---------------------------------------------------------------

bool  DiplodocusChat::HandleCommandFromGateway( BasePacket* packet, U32 connectionId )
{
   Threading::MutexLock locker( m_mutex );

   // delete connections, etc.
   assert( 0 );// incomplete

   return true;
}

//---------------------------------------------------------------

void     DiplodocusChat::FinishedLogin( U32 connectionId, const string& uuidUser )
{
   ConnectionMapIterator it = m_connectionMap.find( connectionId );
   if( it != m_connectionMap.end() )
   { 
      m_chatChannelManager->UserLoggedIn( it->second->GetName(), uuidUser, it->second );
      m_connectionLookup.insert( UuidConnectionIdPair( uuidUser, connectionId ) );
      m_uuidLookup.insert( ConnectionIdUuidPair( connectionId, uuidUser ) );
   }
   
   // notify other servers that the user has logged in... this emulates a login server.
   // However, this functionality should also be done to inform the diplodocus chat.
}

//---------------------------------------------------------------

void     DiplodocusChat::FinishedLogout( U32 connectionId, const string& uuidUser )
{
   Threading::MutexLock locker( m_mutex );
   ConnectionMapIterator it = m_connectionMap.find( connectionId );
   if( it != m_connectionMap.end() )
   { 
      m_oldConenctions.push_back( (*it).second );
      m_connectionMap.erase( it );
   }
   UuidConnectionIdMap::iterator connIter = m_connectionLookup.find( uuidUser );
   if( connIter != m_connectionLookup.end() )
   {
      m_connectionLookup.erase( connIter );
   }
   ConnectionIdUuidMap::iterator uuidIter = m_uuidLookup.find( connectionId );
   if( uuidIter != m_uuidLookup.end() )
   {
      m_uuidLookup.erase( uuidIter );
   }
}

//---------------------------------------------------------------

int   DiplodocusChat::CallbackFunction()
{
   while( m_connectionsNeedingUpdate.size() )
   {
      int index = m_connectionsNeedingUpdate.front();
      m_connectionsNeedingUpdate.pop_front();

      LockMutex();
      ConnectionMapIterator it = m_connectionMap.find( index );
      if( it != m_connectionMap.end() )
      {
         (*it).second->Update();
      }
      UnlockMutex();
   }


   if( m_inputsNeedUpdate )// should only be one, or however many gateways are connected.
   {
      LockMutex();
      int num = m_connectedClients.size();
      ClientMapIterator it = m_connectedClients.begin();
      while( it != m_connectedClients.end() )
      {
         KhaanChat* khaan = it->second;
         khaan->Update();
         
         it++;
      }
      UnlockMutex();
      //UpdateAllConnections();
   }

   if( m_chatChannelManagerNeedsUpdate )
   {
      m_chatChannelManager->Update();
   }

   CleanupOldConnections();

   return 1;
}

//---------------------------------------------------------------

void     DiplodocusChat::CleanupOldConnections()
{
   list< UserConnection* >::iterator it = m_oldConenctions.begin();
   while( it != m_oldConenctions.end() )
   {
      UserConnection* connection = *it++;
      delete connection;
   }
   m_oldConenctions.clear();
}

//---------------------------------------------------------------

//---------------------------------------------------------------


//---------------------------------------------------------------


//---------------------------------------------------------------
