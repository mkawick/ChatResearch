#include "DiplodocusGame.h"
#include "KhaanGame.h"

#include "../NetworkCommon/Packets/ServerToServerPacket.h"
#include <iostream>
using namespace std;

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

DiplodocusGame::DiplodocusGame( const string& serverName, U32 serverId ): Diplodocus< KhaanGame >( serverName, serverId, ServerType_GameInstance ),
                  m_callbacks( NULL )
{
}


//---------------------------------------------------------------

int      DiplodocusGame::CallbackFunction()
{
   return 1;
}

//---------------------------------------------------------------

void     DiplodocusGame::ClientConnectionIsAboutToRemove( KhaanGame* khaan )
{
   cout << "Gateway has disconnected" << endl;

}

//---------------------------------------------------------------

void     DiplodocusGame::ClientConnectionFinishedAdding( KhaanGame* khaan )
{
   cout << "Gateway has connected" << endl;
}

//---------------------------------------------------------------

//---------------------------------------------------------------

void  DiplodocusGame::HandleCommandFromGateway( BasePacket* packet, U32 connectionId )
{
   Threading::MutexLock locker( m_mutex );

   // delete connections, etc.
   assert( 0 );// incomplete
}

//---------------------------------------------------------------

bool  DiplodocusGame::IsConnectionValid( U32 connectionId ) const
{
   ConnectionMap::const_iterator it = m_connectionMap.find( connectionId );
   if( it != m_connectionMap.end() )
   {
      return true;
   }
   return false;
}

//---------------------------------------------------------------

// this will always be data coming from the gateway or at least from the outside in.
bool   DiplodocusGame::AddInputChainData( BasePacket* packet, U32 connectionId ) 
{
   if( packet->packetType == PacketType_GatewayInformation )
   {
      HandleCommandFromGateway( packet, connectionId );
      return false;
   }

   if( packet->packetType == PacketType_ServerJobWrapper )
   {
      HandlePacketFromOtherServer( packet, connectionId );
      return true;
   }

   if( packet->packetType == PacketType_GatewayWrapper )
   {
      PacketGatewayWrapper* wrapper = static_cast< PacketGatewayWrapper* >( packet );
      BasePacket* pPacket = wrapper->pPacket;

      if( pPacket->packetType == PacketType_Gameplay )
      {
         // for simplicity, we are simply going to send packets onto the chat server
         ChainLinkIteratorType itOutputs = m_listOfOutputs.begin();
         while( itOutputs != m_listOfOutputs.end() )// only one output currently supported.
         {
            ChainedInterface* outputPtr = itOutputs->m_interface;
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
   }
   return false;
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
      }
   }
   return false;
}

//---------------------------------------------------------------

bool  DiplodocusGame::HandlePacketToOtherServer( BasePacket* packet, U32 connectionId )// not thread safe
{
   ChainLinkIteratorType itInputs = m_listOfInputs.begin();
   while( itInputs != m_listOfInputs.end() )// only one output currently supported.
   {
      ChainedInterface* inputPtr = itInputs->m_interface;
      if( inputPtr->GetConnectionId() == ServerToServerConnectionId )
      {
         if( inputPtr->AddOutputChainData( packet, connectionId ) == true )
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

void  DiplodocusGame::ConnectUser( PacketPrepareForUserLogin* loginPacket )
{
   U32 connectionId = loginPacket->connectionId;
   ConnectionMapIterator it = m_connectionMap.find( connectionId );
   if( it != m_connectionMap.end() )
   {
      // we already have this which should be a major problem
      Log( "Chat server: Attempt to add a user connection fails" );
      return;
   }

   if( m_callbacks )
   {
      UserInfo ui;
      ui.username = loginPacket->username;
      ui.uuid = loginPacket->uuid;
      ui.apple_id = "";
      ui.connectionId = connectionId;
      m_callbacks->UserConnected( &ui, connectionId );
   }
}

//---------------------------------------------------------------

void  DiplodocusGame::DisconnectUser( PacketPrepareForUserLogout* logoutPacket )
{
   U32 connectionId = logoutPacket->connectionId;
   ConnectionMapIterator it = m_connectionMap.find( connectionId );
   if( it == m_connectionMap.end() )
   {
      // we already have this which should be a major problem
      Log( "Chat server: Attempt to remove a user connection fails" );
      return;
   }

   if( logoutPacket->gameInstanceId == m_serverId )
   {
      if( m_callbacks )
      {
         m_callbacks->UserDisconnected( connectionId );
      }
   }
}

//---------------------------------------------------------------
