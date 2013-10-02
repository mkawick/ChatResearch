#include "DiplodocusGame.h"
#include "KhaanGame.h"

#include "../NetworkCommon/Packets/ServerToServerPacket.h"
#include "../NetworkCommon/Packets/GamePacket.h"
#include "../NetworkCommon/Packets/LoginPacket.h"
#include "../NetworkCommon/Packets/PacketFactory.h"

#include <iostream>
#include <time.h>
using namespace std;

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

DiplodocusGame::DiplodocusGame( const string& serverName, U32 serverId, U8 gameProductId ): Diplodocus< KhaanGame >( serverName, serverId, gameProductId, ServerType_GameInstance ),
                  m_callbacks( NULL )
{
}

//---------------------------------------------------------------

void     DiplodocusGame::AddTimer( U32 timerId, U32 callbackTimeMs )
{
   TimerInfo timer;
   timer.scheduleTimeMs = callbackTimeMs;
   timer.timerId = timerId;
   timer.lastTimeMs = GetCurrentMilliseconds();

   m_timers.push_back( timer );
   if( GetSleepTime() > static_cast< int >( callbackTimeMs ) )
   {
      SetSleepTime( callbackTimeMs ); // never less granularity than the smallest
   }
}

//---------------------------------------------------------------

void     DiplodocusGame::InputRemovalInProgress( IChainedInterface* chainedInput )
{
   cout << "Gateway has disconnected" << endl;

}

//---------------------------------------------------------------

void     DiplodocusGame::InputConnected( IChainedInterface* chainedInput )
{
   cout << "Gateway has connected" << endl;
   //m_connectionIdGateway = khaan->GetServerId();
}

//---------------------------------------------------------------

void     DiplodocusGame::ServerWasIdentified( ChainedInterface* khaan )
{
   BasePacket* packet = NULL;
   PackageForServerIdentification( m_serverName, m_serverId, m_gameProductId, m_isGame, m_isControllerApp, true, m_isGateway, &packet );
   khaan->AddOutputChainData( packet, 0 );
   m_serversNeedingUpdate.push_back( static_cast<InputChainType*>( khaan )->GetServerId() );
}

//---------------------------------------------------------------

//---------------------------------------------------------------

bool  DiplodocusGame::HandleCommandFromGateway( BasePacket* packet, U32 connectionId )
{
   Threading::MutexLock locker( m_mutex );

   // delete connections, etc.
   assert( 0 );// incomplete

   return true;
}

//---------------------------------------------------------------

// this will always be data coming from the gateway or at least from the outside in.
bool   DiplodocusGame::AddInputChainData( BasePacket* packet, U32 connectionId ) 
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

   if( packet->packetType == PacketType_GatewayWrapper )
   {
      PacketCleaner cleaner( packet );
      PacketGatewayWrapper* wrapper = static_cast< PacketGatewayWrapper* >( packet );
      BasePacket* unwrappedPacket = wrapper->pPacket;
      U32 connectionIdToUse = wrapper->connectionId;

      if( unwrappedPacket->packetType == PacketType_Gameplay )
      {
         // we validate the raw data format...
         if( unwrappedPacket->packetSubType == PacketGameToServer::GamePacketType_RawGameData )
         {
            PacketGameplayRawData* rawData = static_cast< PacketGameplayRawData* > ( unwrappedPacket );
            if( m_callbacks )
            {
               MarshalledData data;
               data.m_data = rawData->data;
               data.m_sizeOfData = rawData->size;
               m_callbacks->DataFromClient( connectionIdToUse, &data );
            }
            return true;
         }
         else if( unwrappedPacket->packetSubType == PacketGameToServer::GamePacketType_ListOfGames )
         {
            PacketListOfGames* packet = static_cast< PacketListOfGames* > ( unwrappedPacket );
            bool  isUserValidForThisGame = false;
            KeyValueVectorIterator it = packet->games.begin();
            while( it != packet->games.end() )
            {
               if( it->key == m_gameUuid )
               {
                  isUserValidForThisGame = true;
               }
               it++;
            }
            if( m_callbacks )
            {
               m_callbacks->UserConfirmedToOwnThisProduct( packet->connectionId, isUserValidForThisGame );
            }
            return true;
         }
        /* // for simplicity, we are simply going to send packets onto the chat server
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
         return false;*/
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

  /* if( packet->packetType == PacketType_DbQuery )
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
   if( packet->packetType == PacketType_ServerJobWrapper )
   {
      return HandlePacketToOtherServer( packet, connectionId );
   }*/

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
         KhaanGame* khaan = static_cast< KhaanGame* >( interfacePtr );
         if( khaan->GetServerId() == m_connectionIdGateway )
         {
            khaan->AddOutputChainData( packet );
            //khaan->Update();// the gateway may not have a proper connection id.

            m_serversNeedingUpdate.push_back( khaan->GetServerId() );
            return true;
         }
      }
      return false;
   }
   return false;
}

//---------------------------------------------------------------

// copy and paste from Diplodocus BUT the connection ids don't work in all cases.
void	DiplodocusGame::UpdateAllConnections()
{
   if( m_clientsNeedingUpdate.size() == 0 )// no locking a mutex if you don't need to do it.
      return;

   LockMutex();
   while( m_clientsNeedingUpdate.size() )// threads can remove themselves.
   {
      U32 id = m_clientsNeedingUpdate.front();      
      m_clientsNeedingUpdate.pop_front();

      ClientMapIterator it = m_connectedClients.end();
      if( m_connectedClients.size() )// preventing removal crashes.
      {
         it = m_connectedClients.find( id );
      }

      //*********************************************

      if( it == m_connectedClients.end() )
      {
         ClientMapIterator searchIt = m_connectedClients.begin();
         while( searchIt != m_connectedClients.end() )
         {
            if( searchIt->second->GetServerId() == id )
            {
               it = searchIt;
               break;
            }
            searchIt++;
         }
      }

      //*********************************************
      
      if( it != m_connectedClients.end() )
      {
         InputChainType* connection = it->second;
         connection->Update();
      }
   }
   UnlockMutex();
}


//---------------------------------------------------------------

void     DiplodocusGame::UpdateAllTimers()
{
   if( m_timers.size() == 0 || m_callbacks == NULL )
      return;

   U32 currentTimeMs = GetCurrentMilliseconds();

   list< TimerInfo >:: iterator it = m_timers.begin();
   while( it != m_timers.end() )
   {
      TimerInfo& timer = *it++;
      time_t currentTime;
      time( &currentTime );
      if( timer.lastTimeMs < (currentTimeMs - timer.scheduleTimeMs ) )
      {
         m_mutex.lock();
         m_callbacks->TimerCallback( timer.timerId, currentTime );
         timer.lastTimeMs = currentTimeMs;
         m_mutex.unlock();
      }
   }
}

//---------------------------------------------------------------

int   DiplodocusGame::CallbackFunction()
{
   // I would do this with a map, but we'll only ever have one or two of these.
   while( m_serversNeedingUpdate.size() )
   {
      U32 serverId = m_serversNeedingUpdate.front();
      m_serversNeedingUpdate.pop_front();

      Threading::MutexLock locker( m_mutex );
      ChainLinkIteratorType itInputs = m_listOfInputs.begin();
      while( itInputs != m_listOfInputs.end() )
      {
         ChainLink& chainedInput = *itInputs++;
         IChainedInterface* interfacePtr = chainedInput.m_interface;
         KhaanGame* khaan = static_cast< KhaanGame* >( interfacePtr );
         if( khaan->GetServerId() == serverId )
         {
            if( khaan->Update() == false )
            {
               m_serversNeedingUpdate.push_back( serverId );
            }
         }
      }
   }
   UpdateAllConnections();
   UpdateAllTimers();

   return 1;
}

//---------------------------------------------------------------


bool  DiplodocusGame::HandlePacketFromOtherServer( BasePacket* packet, U32 connectionId )// not thread safe
{
   if( packet->packetType != PacketType_ServerJobWrapper )
   {
      return false;
   }

   PacketServerJobWrapper* wrapper = static_cast< PacketServerJobWrapper* >( packet );
   BasePacket* unwrappedPacket = wrapper->pPacket;
   U32  serverIdLookup = wrapper->serverId;

   delete wrapper;
   bool success = false;

   if( unwrappedPacket->packetType == PacketType_Login )
   {
      switch( unwrappedPacket->packetSubType )
      {
      case PacketLogin::LoginType_PrepareForUserLogin:
         ConnectUser( static_cast< PacketPrepareForUserLogin* >( unwrappedPacket ) );
         delete unwrappedPacket;
         return true;

      case PacketLogin::LoginType_PrepareForUserLogout:
         DisconnectUser( static_cast< PacketPrepareForUserLogout* >( unwrappedPacket ) );
         delete unwrappedPacket;
         return true;
      }
   }
   else if( unwrappedPacket->packetType == PacketType_Gameplay )
   {
      switch( unwrappedPacket->packetSubType )
      {
      case PacketGameToServer::GamePacketType_ListOfGames:
         IsUserAllowedToUseThisProduct( static_cast< PacketListOfGames* >( unwrappedPacket ) );
         delete unwrappedPacket;
         return true;
      }
   }

   return false;
}

//---------------------------------------------------------------

void  DiplodocusGame::IsUserAllowedToUseThisProduct( const PacketListOfGames* packet )
{
   bool  isUserValidForThisGame = false;
   KeyValueConstIterator it = packet->games.begin();
   while( it != packet->games.end() )
   {
      if( it->key == m_gameUuid )
      {
         isUserValidForThisGame = true;
         break;
      }
      it++;
   }
   if( m_callbacks )
   {
      m_callbacks->UserConfirmedToOwnThisProduct( packet->connectionId, isUserValidForThisGame );
   }

}

//---------------------------------------------------------------

bool  DiplodocusGame::HandlePacketToOtherServer( BasePacket* packet, U32 connectionId )// not thread safe
{
   ChainLinkIteratorType itInputs = m_listOfInputs.begin();
   while( itInputs != m_listOfInputs.end() )// only one output currently supported.
   {
      ChainType* inputPtr = static_cast< ChainType*> ( itInputs->m_interface );
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

void  DiplodocusGame::ConnectUser( const PacketPrepareForUserLogin* loginPacket )
{
   U32 connectionId = loginPacket->connectionId;
   if( m_callbacks )
   {
      UserInfo ui;
      ui.userName =        loginPacket->userName;
      ui.uuid =            loginPacket->uuid;
      ui.apple_id = "";
      ui.connectionId =    connectionId;
      ui.gameProductId =   loginPacket->gameProductId;
      ui.active =          loginPacket->active;
      ui.email =           loginPacket->email;
      ui.passwordHash =    loginPacket->password;
      ui.id =              loginPacket->userId;

      m_mutex.lock();
      m_callbacks->UserConnected( &ui, connectionId );
      m_mutex.unlock();
   }
}

//---------------------------------------------------------------

void  DiplodocusGame::DisconnectUser( const PacketPrepareForUserLogout* logoutPacket )
{
   U32 connectionId = logoutPacket->connectionId;
   bool  errorDisconnect = logoutPacket->wasDisconnectedByError;
   if( m_callbacks )
   {
      m_mutex.lock();
      m_callbacks->UserDisconnected( connectionId, errorDisconnect );
      m_mutex.unlock();
   }
}

//---------------------------------------------------------------
