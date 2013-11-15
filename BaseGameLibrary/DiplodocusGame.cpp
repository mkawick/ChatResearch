#include "DiplodocusGame.h"
#include "KhaanGame.h"

#include "../NetworkCommon/Packets/ServerToServerPacket.h"
#include "../NetworkCommon/Packets/GamePacket.h"
#include "../NetworkCommon/Packets/LoginPacket.h"
#include "../NetworkCommon/Packets/PacketFactory.h"
#include "../NetworkCommon/Packets/TournamentPacket.h"

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
   PacketFactory factory;
   if( packet->packetType == PacketType_GatewayInformation )
   {
      return HandleCommandFromGateway( packet, connectionId );
   }

   if( packet->packetType == PacketType_ServerJobWrapper )
   {
      PacketServerJobWrapper* wrapper = static_cast< PacketServerJobWrapper* >( packet );
      BasePacket* unwrappedPacket = wrapper->pPacket;
      U32  serverIdLookup = wrapper->serverId;
      if( HandlePacketFromOtherServer( unwrappedPacket, serverIdLookup ) == true )
      {
         wrapper->pPacket = NULL;
      }
      factory.CleanupPacket( packet );

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
      }
      else if( unwrappedPacket->packetType == PacketType_Tournament )
      {
         HandleUserRequestedTournamentInfo( unwrappedPacket, connectionIdToUse );
      }
      else
      {
         assert( 0 );
      }
   }
   
   return false;
}

void   DiplodocusGame::HandleUserRequestedTournamentInfo( BasePacket* packet, U32 connectionId )
{
   switch( packet->packetSubType )
   {
   case PacketTournament::TournamentType_RequestListOfTournaments:
      {
         if( m_callbacks )
         {
            m_callbacks->UserWantsAListOfTournaments( connectionId );
         }
      }
      break;
   case PacketTournament::TournamentType_RequestTournamentDetails:
      {
         if( m_callbacks )
         {
            const PacketTournament_RequestTournamentDetails* tournamentRequest = static_cast< PacketTournament_RequestTournamentDetails* > ( packet );
            m_callbacks->UserWantsTournamentDetails( connectionId, tournamentRequest->tournamentUuid );
         }
      }
      break;
   case PacketTournament::TournamentType_RequestListOfTournamentEntrants:
      {
         if( m_callbacks )
         {
            const PacketTournament_UserRequestsEntryInTournament* tournamentRequest = static_cast< PacketTournament_UserRequestsEntryInTournament* > ( packet );
            m_callbacks->UserWantsAListOfTournamentEntrants( connectionId, tournamentRequest->tournamentUuid );
         }
      }
      break;
   case PacketTournament::TournamentType_UserRequestsEntryInTournament:
      {
         if( m_callbacks )
         {
            const PacketTournament_UserRequestsEntryInTournament* tournamentRequest = static_cast< PacketTournament_UserRequestsEntryInTournament* > ( packet );
            m_callbacks->UserWantsToJoinTournament( connectionId, tournamentRequest->tournamentUuid );
         }
      }
      break;
   }
}
//---------------------------------------------------------------

bool   DiplodocusGame::AddOutputChainData( BasePacket* packet, U32 connectionId ) 
{
   Threading::MutexLock locker( m_mutex );
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

   return HandlePacketFromOtherServer( packet, connectionId );
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

   UpdateConsoleWindow( m_timeOfLastTitleUpdate, m_uptime, m_numTotalConnections, m_connectedClients.size(), m_listeningPort, m_serverName );

   return 1;
}

//---------------------------------------------------------------


bool  DiplodocusGame::HandlePacketFromOtherServer( BasePacket* packet, U32 connectionId )// not thread safe
{
  /* if( packet->packetType != PacketType_ServerJobWrapper )
   {
      return false;
   }*/

   PacketCleaner cleaner( packet );
 /*  PacketServerJobWrapper* wrapper = static_cast< PacketServerJobWrapper* >( packet );
   BasePacket* unwrappedPacket = wrapper->pPacket;
   U32  serverIdLookup = wrapper->serverId;*/

   bool success = false;

   if( packet->packetType == PacketType_Login )
   {
      switch( packet->packetSubType )
      {
      case PacketLogin::LoginType_PrepareForUserLogin:
         ConnectUser( static_cast< PacketPrepareForUserLogin* >( packet ) );
         return true;

      case PacketLogin::LoginType_PrepareForUserLogout:
         DisconnectUser( static_cast< PacketPrepareForUserLogout* >( packet ) );
         return true;
      }
   }
   else if( packet->packetType == PacketType_Gameplay )
   {
      switch( packet->packetSubType )
      {
      case PacketGameToServer::GamePacketType_ListOfGames:
         IsUserAllowedToUseThisProduct( static_cast< PacketListOfGames* >( packet ) );
         return true;
      }
   }
  /* else if( packet->packetType == PacketType_Tournament ) // s2s tournament packets are limited here.
   {
      switch( packet->packetSubType )
      {
      case PacketTournament::TournamentType_PurchaseTournamentEntryResponse:
         TournamentPurchaseResult( static_cast< PacketTournament_PurchaseTournamentEntryResponse* >( packet ) );
         return true;
      }
   }*/

   if( m_callbacks ) // allows game specific functionality
   {
      
      Threading::MutexLock locker( m_mutex );
      return m_callbacks->HandlePacketFromOtherServer( packet );
   }

   return true;// cleaner will cleanup this memory
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
//---------------------------------------------------------------
/*
bool  DiplodocusGame::TournamentPurchaseResult( const PacketTournament_PurchaseTournamentEntryResponse* tournamentPurchase )
{
   if( m_callbacks == NULL )
      return false;

   string transactionId = tournamentPurchase->uniqueTransactionId;

   bool found = false;
   Threading::MutexLock locker( m_mutex );

   list< TournamentPurchaseRequest >::iterator it = m_purchaseRequests.begin();
   while( it != m_purchaseRequests.end() )
   {
      if( it->transactionId == transactionId )
      {
         found = true;
         break;
      }
      it++;
   }

   if( found == false )
      return false;

   TournamentPurchaseRequest pr = *it;
   if( m_callbacks )
   {
      m_callbacks->PurchaseTournamentResult( pr.connectionId, pr.userUuid, pr.tournamentUuid, tournamentPurchase->result );
   }

   return true;
   
}
*/
//---------------------------------------------------------------
//---------------------------------------------------------------
/*
bool     DiplodocusGame::SendListOfTournamentsToClient( U32 connectionId, const list< TournamentOverview >& tournamentList )
{
   ChainLinkIteratorType itInputs = FindInputConnection( connectionId );
   if( itInputs != m_listOfInputs.end() )
   {
      ChainType* inputPtr = static_cast< ChainType*> ( itInputs->m_interface );
      PacketTournament_RequestListOfTournamentsResponse* response = new PacketTournament_RequestListOfTournamentsResponse;
      PacketCleaner cleaner( response );

      list< TournamentOverview >::const_iterator it = tournamentList.begin();
      while( it != tournamentList.end() )
      {
         TournamentInfo ti;
         ti.tournamentName = it->name;
         ti.tournamentUuid = it->uuid;
         response->tournaments.insert( ti.tournamentUuid, ti );
         it++;
      }

      if( inputPtr->AddOutputChainData( response, connectionId ) == true )
      {
         
         return true;
      }
   }
   return false;
}
*/
//---------------------------------------------------------------
/*
bool     DiplodocusGame::RequestPurchaseServerMakeTransaction( U32 connectionId, const string& userUuid, const string& tournamentUuid, int numTicketsRequired )
{
   PacketTournament_PurchaseTournamentEntry* entry = new PacketTournament_PurchaseTournamentEntry;
   //entry->gameInstanceId =       GetServerId();
   //entry->gameId =               GetGameProductId();

   U32 ms = GetCurrentMilliseconds();
   U32 randomValue = rand();
   string transactionId = GenerateUUID( ms + static_cast<U32>( GenerateUniqueHash( userUuid + tournamentUuid ) ) + randomValue );

   entry->numTicketsRequired =   numTicketsRequired;   
   entry->tournamentUuid =       tournamentUuid;
   entry->uniqueTransactionId =  transactionId;
   entry->userUuid  =            userUuid;


   // todo, verify that no transactions are in progress

   TournamentPurchaseRequest* pr = new TournamentPurchaseRequest;
   pr.connectionId   = connectionId;
   pr.userUuid = userUuid;
   pr.timeSent = GetDateInUTC();
   pr.transactionId = transactionId;
   pr.tournamentUuid = tournamentUuid;
   m_purchaseRequests.push_back( pr );

   //this->m
   return HandlePacketToOtherServer( wrapper, 0 );// based on the type, this should go to purchasing
}*/


//---------------------------------------------------------------
/*
bool     DiplodocusGame::SendPurchaseResultToClient( U32 connectionId, const string& tournamentUuid, int purchaseResult ) // see PacketTournament_UserRequestsEntryInTournamentResponse
{
   ChainLinkIteratorType   cl = FindInputConnection( connectionId );
   if( cl != m_listOfInputs.end() )
   {
      PacketTournament_UserRequestsEntryInTournamentResponse* response = new PacketTournament_UserRequestsEntryInTournamentResponse;
      ChainType* inputPtr = static_cast< ChainType*> ( cl->m_interface );
      if( inputPtr->AddOutputChainData( response, connectionId ) == true )
      {
         return true;
      }
   }
   return false;
}*/

//---------------------------------------------------------------

void     DiplodocusGame::ConnectUser( const PacketPrepareForUserLogin* loginPacket )
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
