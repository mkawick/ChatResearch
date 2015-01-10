// DiplodocusGame.cpp

#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
using boost::format;

#include "../NetworkCommon/NetworkOut/Fruitadens.h"
#include "../NetworkCommon/NetworkOut/FruitadensServerToServer.h"
#include "../NetworkCommon/Packets/ServerToServerPacket.h"
#include "../NetworkCommon/Packets/GamePacket.h"
#include "../NetworkCommon/Packets/LoginPacket.h"
#include "../NetworkCommon/Packets/PacketFactory.h"
#include "../NetworkCommon/Packets/TournamentPacket.h"
#include "../NetworkCommon/Packets/UserStatsPacket.h"

#include "DiplodocusGame.h"
#include "KhaanGame.h"
#include "GameFramework.h"
#include "../NetworkCommon/NetworkIn/DiplodocusTools.h"
#include "../NetworkCommon/Utils/StringUtils.h"

#include <iostream>
#include <time.h>
using namespace std;

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

DiplodocusGame::DiplodocusGame( const string& serverName, U32 serverId, U8 gameProductId ): 
                  ChainedType( serverName, serverId, gameProductId, ServerType_GameInstance ),
                  StatTrackingConnections(),
                  m_callbacks( NULL )
{
   this->SetSleepTime( 15 );
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
   KhaanGame* khaan = static_cast< KhaanGame* >( chainedInput );

   SetupClientWaitingToBeRemoved( khaan );

   string currentTime = GetDateInUTC();
   string printer = "DiplodocusGame::Client disconnection at time:" + currentTime + " from " + inet_ntoa( khaan->GetIPAddress().sin_addr );
   LogMessage( LOG_PRIO_INFO, printer.c_str() );

   PrintDebugText( "** InputRemovalInProgress" , 1 );
}

//---------------------------------------------------------------

void     DiplodocusGame::InputConnected( IChainedInterface* chainedInput )
{
   LogMessage( LOG_PRIO_INFO, "Gateway has connected" );
   LogMessage( LOG_PRIO_INFO, "DiplodocusGame::InputConnected" );
   Khaan* khaan = static_cast< Khaan* >( chainedInput );
   string currentTime = GetDateInUTC();

   string printer = "Accepted connection at time:" + currentTime + " from " + inet_ntoa( khaan->GetIPAddress().sin_addr );
   LogMessage( LOG_PRIO_INFO, printer.c_str() );

   int outputBufferSize = 128 * 1024;
   LogMessage( LOG_PRIO_INFO, "DiplodocusGame::SetOutputBufferSize( %d )", outputBufferSize );
   khaan->SetOutboudBufferSize( outputBufferSize );
}

//---------------------------------------------------------------

void     DiplodocusGame::ServerWasIdentified( IChainedInterface* chainedInput )
{
   cout << "DiplodocusGame::ServerWasIdentified <<<" << endl;
   BasePacket* packet = NULL;
   PackageForServerIdentification( m_serverName, m_localIpAddress, m_externalIpAddress, m_serverId, m_serverType, m_listeningPort, m_gameProductId, m_isGame, m_isControllerApp, true, m_gatewayType, &packet );
   Khaan* khaan = static_cast< Khaan* >( chainedInput );
   khaan->AddOutputChainDataNoLock( packet );
   // this is not thread safe, but will be invoked from within the same thread.
   m_clientsNeedingUpdate.push_back( khaan->GetChainedId() );
   cout << "DiplodocusGame::ServerWasIdentified >>>" << endl;
}

//---------------------------------------------------------------

struct st_mysql * DiplodocusGame::GetDBConnection( Database::Deltadromeus::DbConnectionType type )
{
   ChainLinkIteratorType itOutputs = m_listOfOutputs.begin();
   while( itOutputs != m_listOfOutputs.end() )// only one output currently supported.
   {
      ChainType* outputPtr = static_cast< ChainType*> ( (*itOutputs).m_interface );
      if( outputPtr->GetChainedType() == ChainedType_DatabaseConnector )
      {
         Database::Deltadromeus* delta = static_cast< Database::Deltadromeus* >( outputPtr );
         if( type != 0 )
         {
            if( delta->WillYouTakeThisQuery( type ) )
            {
               return delta->GetDbHandle();
            }
         }
         else // if this query is not set, default to true
         {
            return delta->GetDbHandle();
         }
         
      }
      itOutputs++;
   }
   return NULL;
}
//---------------------------------------------------------------

bool  DiplodocusGame::HandleCommandFromGateway( BasePacket* packet, U32 gatewayId )
{
   Threading::MutexLock locker( m_mutex );

   // delete connections, etc.
   assert( 0 );// incomplete

   return true;
}

//---------------------------------------------------------------

// this will always be data coming from the gateway or at least from the outside in.
//---------------------------------------------------------------

bool     DiplodocusGame::AddInputChainData( BasePacket* packet, U32 gatewayId )
{
   m_mutex.lock();
   m_inputPacketsToBeProcessed.push_back( PacketStorage( packet, gatewayId ) );
   m_mutex.unlock();

   return true;
}

bool     DiplodocusGame:: ProcessPacket( PacketStorage& storage )
{
   BasePacket* packet = storage.packet;
   U32 gatewayId = storage.gatewayId;

   PacketFactory factory;
   if( packet->packetType == PacketType_GatewayInformation )
   {
      return HandleCommandFromGateway( packet, gatewayId );
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

   if( packet->packetType == PacketType_ServerToServerWrapper )
   {
      PacketServerToServerWrapper* wrapper = static_cast< PacketServerToServerWrapper* >( packet );
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
      U32 connectionId = wrapper->connectionId;
      U8 packetType = unwrappedPacket->packetType;
      U8 packetSubType = unwrappedPacket->packetSubType;

      if( packetType == PacketType_Gameplay )
      {
         // we validate the raw data format...
         if( packetSubType == PacketGameToServer::GamePacketType_RawGameData )
         {
            PacketGameplayRawData* rawData = static_cast< PacketGameplayRawData* > ( unwrappedPacket );
            if( m_callbacks )
            {
               MarshalledData data;
               data.m_data = rawData->data;
               data.m_sizeOfData = rawData->size;
               m_callbacks->DataFromClient( connectionId, &data );
            }
            return true;
         }
         else if( packetSubType == PacketGameToServer::GamePacketType_ListOfGames )
         {
            PacketListOfGames* packet = static_cast< PacketListOfGames* > ( unwrappedPacket );
            bool  isUserValidForThisGame = false;
            SerializedKeyValueVector< BoundedString32 >::KeyValueVectorIterator it = packet->games.begin();
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
         else if( packetSubType == PacketGameToServer::GamePacketType_EchoToServer )
         {
            EchoHandler( connectionId );
            return true;
         }
         else if( packetSubType == PacketGameToServer::GamePacketType_Notification )
         {
            PacketGame_Notification* notification = static_cast< PacketGame_Notification* > ( unwrappedPacket );
            SendNotification( notification, connectionId );
            return true;
         }
         else if( packetSubType == PacketGameToServer::GamePacketType_TestHook )
         {
            RunTest();
            return true;
         }
         return true;
      }
      else if( packetType == PacketType_Tournament )
      {
         HandleUserRequestedTournamentInfo( unwrappedPacket, connectionId );
         return true;
      }
      else
      {
         assert( 0 );
      }
   }
   
   return false;
}

//---------------------------------------------------------------

void  DiplodocusGame::EchoHandler( U32 connectionId )
{
   PacketGame_EchoToClient* echo = new PacketGame_EchoToClient;
   PacketFactory factory;
   PacketGatewayWrapper* wrapper = new PacketGatewayWrapper();
   wrapper->SetupPacket( echo, connectionId );

   AddOutputChainData( wrapper, connectionId );
}

//---------------------------------------------------------------

void  DiplodocusGame::SendNotification( const PacketGame_Notification* notification, U32 connectionId )
{
   if( m_callbacks )
   {
      m_callbacks->UserWantsNotification( connectionId, notification->notificationType, notification->additionalText );
   }
}

//---------------------------------------------------------------

void  DiplodocusGame::RunTest()
{
   bool user9User14ChatchannelCreateTest = true;
   if( user9User14ChatchannelCreateTest == true )
   {
    /*  list< const char* > listOfUserUuids;
      listOfUserUuids.push_back( "user9" );
      listOfUserUuids.push_back( "a150f79095fa1235" );*/
      //GameFramework::Instance()->SendChatChannelCreate( "summonwar", 5, listOfUserUuids );
      PacketChatCreateChatChannelFromGameServer* createChannel = new PacketChatCreateChatChannelFromGameServer;
      createChannel->gameName = "summonwar";
      createChannel->gameId = 5;

      //createChannel->userUuidList.insert( uuid );
      createChannel->userUuidList.insert( "user9" );
      createChannel->userUuidList.insert( "a150f79095fa1235" );

      GameFramework::Instance()->SendChatData( createChannel );
      return;
   }

   static bool  testChoice = false ;

   BasePacket* packet = NULL;
   if( testChoice == false )
   {
      PacketUserStats_RecordUserStats* stats = new PacketUserStats_RecordUserStats;
      stats->userUuid = "user1";
      stats->stats.insert( "test1", "value1" );
      packet = stats;
   }
   else
   {
      PacketUserStats_RequestListOfUserStats* statRequest = new PacketUserStats_RequestListOfUserStats;
      statRequest->userUuid = "user1";
      statRequest->whichGame = GetGameProductId();
      packet = statRequest;
   }
   
   if( packet != NULL )
   {
      PacketServerToServerWrapper* wrapper = new PacketServerToServerWrapper;
      wrapper->pPacket = packet;
      wrapper->serverId = GetServerId();
      GameFramework::Instance()->GetUserStats()->AddOutputChainData( wrapper, 0 );
   }
}

//---------------------------------------------------------------

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
            m_callbacks->UserWantsTournamentDetails( connectionId, tournamentRequest->tournamentUuid.c_str() );
         }
      }
      break;
   case PacketTournament::TournamentType_RequestListOfTournamentEntrants:
      {
         if( m_callbacks )
         {
            const PacketTournament_UserRequestsEntryInTournament* tournamentRequest = static_cast< PacketTournament_UserRequestsEntryInTournament* > ( packet );
            m_callbacks->UserWantsAListOfTournamentEntrants( connectionId, tournamentRequest->tournamentUuid.c_str() );
         }
      }
      break;
   case PacketTournament::TournamentType_UserRequestsEntryInTournament:
      {
         if( m_callbacks )
         {
            const PacketTournament_UserRequestsEntryInTournament* tournamentRequest = static_cast< PacketTournament_UserRequestsEntryInTournament* > ( packet );
            m_callbacks->UserWantsToJoinTournament( connectionId, tournamentRequest->tournamentUuid.c_str(), tournamentRequest->itemsToSpend, tournamentRequest->customDeck, NULL );
         }
      }
      break;
   }
}
//---------------------------------------------------------------

bool   DiplodocusGame::AddOutputChainData( BasePacket* packet, U32 connectionId ) 
{
   //Threading::MutexLock locker( m_mutex );
   if( packet->packetType == PacketType_GatewayWrapper )
   {
      if( m_connectionIdGateway == 0 )
         return false;

      U32 gatewayId = m_callbacks->GetGatewayId( connectionId );

      if( gatewayId == 0 )
         gatewayId = m_connectionIdGateway;

      Threading::MutexLock locker( m_inputChainListMutex );     

      ChainLinkIteratorType itInputs = m_listOfInputs.begin();
      while( itInputs != m_listOfInputs.end() )
      {
         ChainType* outputPtr = static_cast< ChainType*> ( (*itInputs).m_interface );
         if( outputPtr->GetChainedType() == ChainedType_InboundSocketConnector )
         {
            KhaanGame* khaan = static_cast< KhaanGame* >( outputPtr );
            if( khaan->GetChainedType() == ChainedType_InboundSocketConnector && 
               //khaan->GetConnectionId() == connectionId && on the game, we do not match the connection id.
               khaan->GetServerId() == gatewayId )
            {
               khaan->AddOutputChainDataNoLock( packet );
               MarkConnectionAsNeedingUpdate( khaan->GetChainedId() );
               return true;
            }
         }
         itInputs++;
      }
      return false;
   }

   if( packet->packetType == PacketType_ServerToServerWrapper )
   {
      assert( 0 );// this is a bad design and should not be used.
      // but I need to be sure that this is never invoked before I remove it.

      PacketServerToServerWrapper* wrapper = static_cast< PacketServerToServerWrapper* >( packet );
      BasePacket* unwrappedPacket = wrapper->pPacket;
      U32  serverIdLookup = wrapper->serverId;

      // this is where the problem is: AddOutputChainData should be sending data
      // onto other servers... not routing internally. If we do this,
      // we should have a different mechanism.
      return HandlePacketFromOtherServer( unwrappedPacket, serverIdLookup );
   }

   LogMessage( LOG_PRIO_INFO, "Output layer is seonding unhandled packet up" );
   return HandlePacketFromOtherServer( packet, connectionId );
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
         //m_mutex.lock();
         m_callbacks->TimerCallback( timer.timerId, currentTime );
         timer.lastTimeMs = currentTimeMs;
         //m_mutex.unlock();
      }
   }
}

int   DiplodocusGame::CallbackFunction()
{
   CleanupOldClientConnections( "KhaanGame" );

   UpdateInputPacketToBeProcessed();

   UpdateAllConnections("KhaanGame");
   UpdateAllTimers();

   StatTrackingConnections::SendStatsToStatServer( m_listOfOutputs, m_serverName, m_serverId, m_serverType );

   int numClients = static_cast< int >( m_connectedClients.size() );
   UpdateConsoleWindow( m_timeOfLastTitleUpdate, m_uptime, m_numTotalConnections, numClients, m_listeningPort, m_serverName );

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
   U8 packetType = packet->packetType;

   if( packetType == PacketType_Login )
   {
      switch( packet->packetSubType )
      {
      case PacketLogin::LoginType_PrepareForUserLogin:
         ConnectUser( static_cast< PacketPrepareForUserLogin* >( packet ) );
         return true;

      case PacketLogin::LoginType_PrepareForUserLogout:
         DisconnectUser( static_cast< PacketPrepareForUserLogout* >( packet ) );
         return true;

      case PacketLogin::LoginType_ExpireUserLogin:
         ExpireUser( static_cast< const PacketLoginExpireUser* >( packet ) );
         return true;

      case PacketLogin::LoginType_RequestServiceToFlushAllUserLogins:
         DeleteAllUsers();
         return true;
      }
   }
   else if( packetType == PacketType_Gameplay )
   {
      switch( packet->packetSubType )
      {
      case PacketGameToServer::GamePacketType_ListOfGames:
         IsUserAllowedToUseThisProduct( static_cast< PacketListOfGames* >( packet ) );
         return true;
      }
   }
   else if( packetType == PacketType_UserStats )
   {
      switch( packet->packetSubType )
      {
      case PacketUserStats::UserStatsType_RecordUserStatsResponse:
         {
            PacketUserStats_RecordUserStatsResponse* statusUpdated = static_cast< PacketUserStats_RecordUserStatsResponse* >( packet );
            if( m_callbacks )
            {
               m_callbacks->UserStatsSaved( statusUpdated->userUuid, statusUpdated->success );
            }
         }
         return true;
      case PacketUserStats::UserStatsType_RequestListOfUserStatsResponse:
         {
            PacketUserStats_RequestListOfUserStatsResponse* stats = static_cast< PacketUserStats_RequestListOfUserStatsResponse* >( packet );
            if( m_callbacks )
            {
               KeyValueVector vec;
               SerializedKeyValueVector< BoundedString64 >::KVIterator iter = stats->stats.begin();
               while( iter != stats->stats.end() )
               {
                  const string& key = iter->key;
                  const string& value = iter->value;
                  vec.push_back( KeyValueString( key, value ) );
                  iter ++;
               }
               m_callbacks->UserStatsLoaded( stats->userUuid, vec );
            }
         }
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
      //Threading::MutexLock locker( m_mutex );
      m_callbacks->HandlePacketFromOtherServer( packet );
   }

   return true;// cleaner will cleanup this memory
}

//---------------------------------------------------------------

void  DiplodocusGame::IsUserAllowedToUseThisProduct( const PacketListOfGames* packet )
{
   bool  isUserValidForThisGame = false;
   SerializedKeyValueVector< BoundedString32 >::const_KVIterator it = packet->games.begin();
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

void     DiplodocusGame::LockMutex()
{
 /*  if( m_mutex.isLocked() ) // never hang
      return;*/
   //m_mutex.lock();
}
void     DiplodocusGame::UnlockMutex()
{
   //m_mutex.unlock();
}

//---------------------------------------------------------------

void     DiplodocusGame::ConnectUser( const PacketPrepareForUserLogin* loginPacket )
{
   U32 connectionId = loginPacket->connectionId;
   string uuid = loginPacket->uuid;
   U32 gatewayId = loginPacket->gatewayId;
   //LogMessage( LOG_PRIO_INFO, "Prep for logon: %d, %s, %s, %s", connectionId, loginPacket->userName.c_str(), uuid.c_str(), loginPacket->password.c_str() );
   LogMessage_LoginPacket( loginPacket );

   if( m_callbacks )
   {
      UserInfo ui;
      ui.userName =        loginPacket->userName;
      ui.uuid =            loginPacket->uuid.c_str();
      ui.apple_id = "";
      ui.connectionId =    connectionId;
      ui.gameProductId =   loginPacket->gameProductId;
      ui.active =          loginPacket->active;
      ui.email =           loginPacket->email;
      ui.passwordHash =    loginPacket->password;
      ui.id =              loginPacket->userId;

      //m_mutex.lock();
      if( m_callbacks->UserConnected( &ui, connectionId, gatewayId ) == true )
      {
         ChainLinkIteratorType itInputs = m_listOfInputs.begin();
         while( itInputs != m_listOfInputs.end() )
         {
            ChainType* outputPtr = static_cast< ChainType*> ( (*itInputs).m_interface );
            if( outputPtr->GetChainedType() == ChainedType_InboundSocketConnector )
            {
               KhaanGame* khaan = static_cast< KhaanGame* >( outputPtr );
               if( khaan->GetChainedType() == ChainedType_InboundSocketConnector && 
                  khaan->GetServerId() == gatewayId )
               {
                  //LogMessage( LOG_PRIO_INFO, "InformClientWhoThisServerIs %d <<<", connectionId );

                  PacketGameIdentification* idPacket;
                  GameFramework::Instance()->PackGameIdentificationPack( idPacket );
                  PacketGatewayWrapper* wrapper = new PacketGatewayWrapper();
                  wrapper->SetupPacket( idPacket, connectionId );
                  khaan->AddOutputChainDataNoLock( wrapper );

                  //LogMessage( LOG_PRIO_INFO, "InformClientWhoThisServerIs %d >>>", connectionId );

               }
            }
            itInputs++;
         }
         
      }
      //m_mutex.unlock();
   }
}

//---------------------------------------------------------------

void  DiplodocusGame::DisconnectUser( const PacketPrepareForUserLogout* logoutPacket )
{
   //LogMessage( LOG_PRIO_INFO, "Prep for logout: %d, %s", logoutPacket->connectionId, logoutPacket->uuid.c_str() );
   LogMessage_LogoutPacket( logoutPacket );

   U32 connectionId = logoutPacket->connectionId;
   bool  errorDisconnect = logoutPacket->wasDisconnectedByError;
   if( m_callbacks )
   {
      //m_mutex.lock();
      m_callbacks->UserDisconnected( connectionId, errorDisconnect );
      //m_mutex.unlock();
   }
}

//---------------------------------------------------------------

void  DiplodocusGame::ExpireUser( const PacketLoginExpireUser* expireUserPacket )
{
   const UuidString& uuid = expireUserPacket->uuid;

   if( m_callbacks )
   {
      m_callbacks->ExpireUser( uuid );
   }
}

//---------------------------------------------------------------

bool  DiplodocusGame::DeleteAllUsers()
{
   if( m_callbacks )
   {
      m_callbacks->DeleteAllUsers();
   }
   return true;
}

//---------------------------------------------------------------
//---------------------------------------------------------------
//---------------------------------------------------------------
