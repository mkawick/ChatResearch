#include "DiplodocusGame.h"
#include "KhaanGame.h"

#include "../NetworkCommon/Packets/DbPacket.h"
#include "../NetworkCommon/Packets/ServerToServerPacket.h"
#include "../NetworkCommon/Packets/LoginPacket.h"

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

DiplodocusGame::DiplodocusGame( const string& serverName, U32 serverId, U8 gameProductId ): Diplodocus< KhaanGame >( serverName, serverId, gameProductId, ServerType_GameInstance ) 
{
}


//---------------------------------------------------------------

int      DiplodocusGame::CallbackFunction()
{
   UpdateInputPacketToBeProcessed();
   UpdateOutputPacketToBeProcessed();
   return 1;
}

//---------------------------------------------------------------

void     DiplodocusGame::InputRemovalInProgress( IChainedInterface* chainedInput )
{
}

//---------------------------------------------------------------

void     DiplodocusGame::InputConnected( IChainedInterface* chainedInput )
{
}
//---------------------------------------------------------------

//---------------------------------------------------------------

bool  DiplodocusGame::HandleCommandFromGateway( BasePacket* packet, U32 connectionId )
{
   Threading::MutexLock locker( m_mutex );

   // delete connections, etc.
   assert( 0 );// incomplete

   return false;
}

//---------------------------------------------------------------

// this will always be data coming from the gateway or at least from the outside in.
bool   DiplodocusGame::AddInputChainData( BasePacket* packet, U32 gatewayId ) 
{
   m_mutex.lock();
   m_inputPacketsToBeProcessed.push_back( PacketStorage( packet, gatewayId ) );
   m_mutex.unlock();
   return true;
}

//---------------------------------------------------------------

bool     DiplodocusGame:: ProcessPacket( PacketStorage& storage )
{
   BasePacket* packet = storage.packet;
   U32 gatewayId = storage.gatewayId;

   U8 packetType = packet->packetType;

   if( packetType == PacketType_GatewayWrapper )
   {
      PacketGatewayWrapper* wrapper = static_cast< PacketGatewayWrapper* >( packet );
      BasePacket* pPacket = wrapper->pPacket;

      if( pPacket->packetType == PacketType_Gameplay )
      {
         // fro simplicity, we are simply going to send packets onto the chat server
         ChainLinkIteratorType itOutputs = m_listOfOutputs.begin();
         while( itOutputs != m_listOfOutputs.end() )// only one output currently supported.
         {
            ChainType* outputPtr = static_cast< ChainType* >( itOutputs->m_interface );
            if( outputPtr->AddOutputChainData( pPacket, -1 ) == true )
            {
               
               return true;
            }
            itOutputs++;
         }
         assert( 0 );
         return false;
      }
      else
      {
         assert( 0 );
      }
      return true;
   }

   if( packet->packetType == PacketType_GatewayInformation )
   {
      PacketCleaner cleaner( packet );
      HandleCommandFromGateway( packet, gatewayId );
      return false;
   }

   if( packet->packetType == PacketType_ServerJobWrapper )
   {
      PacketCleaner cleaner( packet );
      HandlePacketFromOtherServer( packet, gatewayId );
      return true;
   }

   PacketCleaner cleaner( packet );

   return true;
}

//---------------------------------------------------------------

bool   DiplodocusGame::AddOutputChainData( BasePacket* packet, U32 connectionId ) 
{
   Threading::MutexLock locker( m_mutex );

   if( packet->packetType == PacketType_DbQuery )
   {
      if( packet->packetSubType == BasePacketDbQuery::QueryType_Result )
      {
         /*PacketDbQueryResult* result = reinterpret_cast<PacketDbQueryResult*>( packet );
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
         }*/
      }
      return true;
   }
   if( packet->packetType == PacketType_ServerJobWrapper )
   {
      return HandlePacketToOtherServer( packet, connectionId );
   }

   
   return false;
}

//---------------------------------------------------------------


bool  DiplodocusGame::HandlePacketFromOtherServer( BasePacket* packet, U32 connectionId )// not thread safe
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
         ConnectUser( static_cast< PacketPrepareForUserLogin* >( actualPacket ) );
         return true;

      case PacketLogin::LoginType_PrepareForUserLogout:
         DisconnectUser( static_cast< PacketPrepareForUserLogout* >( actualPacket ) );
         return true;

      case PacketLogin::LoginType_ExpireUserLogin:
         ExpireUser( static_cast< PacketLoginExpireUser* >( actualPacket ) );
         return true;

      case PacketLogin::LoginType_RequestServiceToFlushAllUserLogins:
         DeleteAllUsers();
         return true;
      }
   }
   return false;
}

//---------------------------------------------------------------

bool  DiplodocusGame::ConnectUser( const PacketPrepareForUserLogin* loginPacket )
{
   U32 connectionId = loginPacket->connectionId;
   string uuid = loginPacket->uuid;
   U32 gatewayId = loginPacket->gatewayId;
   cout << "Prep for logon: " << connectionId << ", " << loginPacket->userName << ", " << uuid << ", " << loginPacket->password << endl;

   ConnectionMapIterator it = m_connectionMap.find( connectionId );
   if( it != m_connectionMap.end() )
   {
      // we already have this which should be a major problem
      Log( "Chat server: Attempt to add a user connection fails" );
      return false;
   }

   bool found = false;
   // if the user is already here but relogged, simply 
 /*  m_mutex.lock();
      it = m_connectionMap.begin();
      while( it != m_connectionMap.end() )
      {
         if( it->second-> == loginPacket->uuid ) 
         {
            found = true;
            m_connectionMap.insert( ConnectionPair( connectionId, it->second ) );
            m_connectionMap.erase( it );
         }
      }
   m_mutex.unlock();*/

   if( found == false )
   {
  /* UserConnection* connection = new UserConnection( connectionId );
   connection->SetupFromLogin( loginPacket->username, loginPacket->uuid, loginPacket->loginKey, loginPacket->lastLoginTime );

   m_mutex.lock();
      m_connectionMap.insert( ConnectionPair ( connectionId, connection ) );
   m_mutex.unlock();*/
   }
   return false;
}

//---------------------------------------------------------------

bool  DiplodocusGame::DisconnectUser( const PacketPrepareForUserLogout* logoutPacket )
{
   cout << "Prep for logout: " << logoutPacket->connectionId << ", " << logoutPacket->uuid << endl;
  /* U32 connectionId = logoutPacket->connectionId;
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
   m_mutex.unlock();*/

   return false;
}

//---------------------------------------------------------------

bool  DiplodocusGame::ExpireUser( const PacketLoginExpireUser* actualPacket )
{
   return true;
}

//---------------------------------------------------------------

bool  DiplodocusGame::DeleteAllUsers()
{
   return true;
}

//---------------------------------------------------------------
