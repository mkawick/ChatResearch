// UserStatsMainThread.cpp

#include <iostream>
#include <time.h>
#include <string>
using namespace std;

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include "UserStatsMainThread.h"

#include "../NetworkCommon/Packets/ServerToServerPacket.h"
#include "../NetworkCommon/Packets/PacketFactory.h"

#include "../NetworkCommon/Packets/DbPacket.h"
#include "../NetworkCommon/Packets/ChatPacket.h"
#include "../NetworkCommon/Packets/InvitationPacket.h"
#include "../NetworkCommon/Packets/LoginPacket.h"
#include "../NetworkCommon/Packets/AnalyticsPacket.h"
#include "../NetworkCommon/Packets/UserStatsPacket.h"
#include "../NetworkCommon/Packets/GamePacket.h"

#include "server_stats_plugins.h"
#include "../NetworkCommon/NetworkIn/DiplodocusTools.h"
#include "../NetworkCommon/Utils/StringUtils.h"


UserStatsMainThread::UserStatsMainThread( const string& serverName, U32 serverId ): ChainedType( serverName, serverId, 0,  ServerType_UserStats )
{
   //time( &m_timestampStatsPrint );
   //m_timestampSelectPreferredGateway = m_timestampStatsPrint;
}

UserStatsMainThread::~UserStatsMainThread()
{
}

void UserStatsMainThread::Init()
{
   ChainedType::Init();

   DbHandle *pStatDB = GetDbConnectionByType( Database::Deltadromeus::DbConnectionType_StatData );

   InitStatsPlugins( pStatDB );
}
//---------------------------------------------------------------

void     UserStatsMainThread::ServerWasIdentified( IChainedInterface* khaan )
{
   BasePacket* packet = NULL;
   PackageForServerIdentification( m_serverName, m_localIpAddress, m_externalIpAddress, m_serverId, m_serverType, m_listeningPort, m_gameProductId, m_isGame, m_isControllerApp, true, m_gatewayType, &packet );
   Khaan* localKhaan = static_cast< Khaan* >( khaan );
   localKhaan->AddOutputChainDataNoLock( packet );
   // this is not thread safe, but will be invoked from within the same thread.
   m_clientsNeedingUpdate.push_back( localKhaan->GetChainedId() );
}

/////////////////////////////////////////////////////////////////////////////////

void     UserStatsMainThread::InputRemovalInProgress( IChainedInterface * chainedInput )
{
   KhaanServerToServer* khaan = static_cast< KhaanServerToServer* >( chainedInput );

   SetupClientWaitingToBeRemoved( khaan );

   string currentTime = GetDateInUTC();
   string printer = "UserStatsMainThread::Client disconnection at time:" + currentTime + " from " + inet_ntoa( khaan->GetIPAddress().sin_addr );
   cout << printer << endl;

   PrintDebugText( "** InputRemovalInProgress" , 1 );
}

//---------------------------------------------------------------

int      UserStatsMainThread::CallbackFunction()
{
   //UpdateDbResults();
   
   UpdateAllConnections( "KhaanServerToServer" );

   time_t currentTime;
   time( &currentTime );

   int numClients = static_cast< int >( m_connectedClients.size() );
   UpdateConsoleWindow( m_timeOfLastTitleUpdate, m_uptime, m_numTotalConnections, numClients, m_listeningPort, m_serverName );

   UpdateInputPacketToBeProcessed();

   CleanupOldClientConnections( "KhaanServerToServer" );
   
   UpdateDbResults();
   /*
   PeriodicWriteToDB();
   RemoveLoggedOutUsers();
   UpdateChatChannelManager();
   UpdateAllChatUsers();

   RunHourlyStats();
   RunDailyStats();
   StatTrackingConnections::SendStatsToStatServer( m_listOfOutputs, m_serverName, m_serverId, m_serverType );

   */
   return 0;
}


//---------------------------------------------------------------

bool     UserStatsMainThread::AddInputChainData( BasePacket* packet, U32 gatewayId )
{
   m_mutex.lock();
   m_inputPacketsToBeProcessed.push_back( PacketStorage( packet, gatewayId ) );
   m_mutex.unlock();

   return true;
}

//---------------------------------------------------------------

bool     UserStatsMainThread:: ProcessPacket( PacketStorage& storage )
{
   BasePacket* packet = storage.packet;
   U32 gatewayId = storage.gatewayId;
   // out of process call
   U8 packetType = packet->packetType;

   switch( packetType )
   {
   case PacketType_GatewayInformation:
      {
         PacketCleaner cleaner( packet );
         //HandleCommandFromGateway( packet, connectionId );
         return true;
      }
   case PacketType_ServerJobWrapper:
      {
         PacketCleaner cleaner( packet );
         HandlePacketFromOtherServer( packet, gatewayId );
         return true;
      }
 
   case PacketType_DbQuery:
      {
      }
   case PacketType_GatewayWrapper:
      {
         PacketCleaner cleaner( packet );
         HandlePacketFromClient( packet, gatewayId ); // TODO: place packets in deque
         return true;
      }
   }
   PacketCleaner cleaner( packet );
   return false;
}

//---------------------------------------------------------------

bool  UserStatsMainThread::HandlePacketFromClient( BasePacket* packet, U32 gatewayId )// not thread safe
{
   if( packet->packetType != PacketType_GatewayWrapper )
   {
      return false;
   }

   PacketGatewayWrapper* wrapper = static_cast< PacketGatewayWrapper* >( packet );
   BasePacket* unwrappedPacket = wrapper->pPacket;
   //U32  serverIdLookup = wrapper->serverId;
   //serverIdLookup = serverIdLookup;

   U8 packetType = unwrappedPacket->packetType;
   U8 packetSubType = unwrappedPacket->packetSubType;
   U32 connectionId = wrapper->connectionId;

   if( packetType == PacketType_UserStats )
   {
      switch( packetSubType )
      {
         case PacketUserStats::UserStatsType_RequestListOfUserStats:
         {
            PacketUserStats_RequestListOfUserStats* stats = static_cast< PacketUserStats_RequestListOfUserStats* >( unwrappedPacket );
            PacketUserStats_RequestListOfUserStatsResponse* response = new PacketUserStats_RequestListOfUserStatsResponse;
            response->userUuid = stats->userUuid;
            response->whichGame = stats->whichGame;
            response->stats.insert( "stat1", "100" );            
            response->stats.insert( "stat2", "1000" );
            response->stats.insert( "stat3", "-9" );
            SendPacketToGateway( response, connectionId, gatewayId );
            //DbQuery
         }
         break;
      case PacketUserStats::UserStatsType_RequestGameFactionStats:
         {
            PacketUserStats_RequestGameFactionStats *pRequestFaction = static_cast< PacketUserStats_RequestGameFactionStats* >( unwrappedPacket );

            if( pRequestFaction->gameType < 0 || pRequestFaction->gameType >= k_statsPluginCount )
            {
               break;
            }

            if( pRequestFaction->userId == 0 )
            {
               if( s_StatsPlugins[pRequestFaction->gameType].onRequestGlobalFactionStats != NULL )
               {
                  s_StatsPlugins[pRequestFaction->gameType].onRequestGlobalFactionStats( this, connectionId, gatewayId );
               }
            }
            else
            {
               if( s_StatsPlugins[pRequestFaction->gameType].onRequestPlayerFactionStats != NULL )
               {
                  s_StatsPlugins[pRequestFaction->gameType].onRequestPlayerFactionStats( this, connectionId, gatewayId, pRequestFaction->userId );
               }
            }
         }
         break;
      case PacketUserStats::UserStatsType_RequestGameProfile:
         {
            PacketUserStats_RequestGameProfile *pRequestGameProfile = static_cast< PacketUserStats_RequestGameProfile* >( unwrappedPacket );

            if( pRequestGameProfile->gameType < 0 || pRequestGameProfile->gameType >= k_statsPluginCount )
            {
               break;
            }

            if( s_StatsPlugins[pRequestGameProfile->gameType].onRequestGameProfile != NULL )
            {
               s_StatsPlugins[pRequestGameProfile->gameType].onRequestGameProfile( this, connectionId, gatewayId,
                  pRequestGameProfile->profileUserId, pRequestGameProfile->requestUserId );
            }
         }
         break;
      case PacketUserStats::UserStatsType_RequestUserProfileStats:
         {
            PacketUserStats_RequestUserProfileStats *pRequestUserProfileStats = static_cast< PacketUserStats_RequestUserProfileStats* >( unwrappedPacket );

            if( pRequestUserProfileStats->gameType < 0 || pRequestUserProfileStats->gameType >= k_statsPluginCount )
            {
               break;
            }

            if( s_StatsPlugins[pRequestUserProfileStats->gameType].onRequestUserProfileStats != NULL )
            {
               if( s_StatsPlugins[pRequestUserProfileStats->gameType].onRequestUserProfileStats(this,connectionId,gatewayId,pRequestUserProfileStats->profileUserId) )
               {
                  break;
               }
            }

            PacketUserStats_RequestUserProfileStatsResponse* response = new PacketUserStats_RequestUserProfileStatsResponse;
            response->profileUserId = pRequestUserProfileStats->profileUserId;
            response->gameType = pRequestUserProfileStats->gameType;
            SendPacketToGateway( response, connectionId, gatewayId );
         }
         break;
      }
      return true;
   }
   return false;
}

//---------------------------------------------------------------

bool  UserStatsMainThread::HandlePacketFromOtherServer( BasePacket* packet, U32 connectionId )// not thread safe
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
      if( HandleLoginPacket( unwrappedPacket, serverIdLookup ) == false )
      {
         factory.CleanupPacket( packet );
      }
      
      return true;
   }

   if( packetType == PacketType_UserStats )
   {
      if( HandleUserStatPacket( unwrappedPacket, serverIdLookup ) == false )
      {
         factory.CleanupPacket( packet );
      }
      return true;
   }

   return false;
}

//---------------------------------------------------------------

bool     UserStatsMainThread::HandleLoginPacket( BasePacket* packet, U32 serverIdLookup )
{
   U32 packetType = packet->packetType;
   U32 packetSubType = packet->packetSubType;

   if( packetType == PacketType_Login )
   {
      switch( packetSubType )
      {
      case PacketLogin::LoginType_PrepareForUserLogin:
         {
            PacketPrepareForUserLogin* loginPacket = static_cast< PacketPrepareForUserLogin* > ( packet );
            U32 connectionId = loginPacket->connectionId;
            U32 gatewayId = loginPacket->gatewayId;

            m_inputChainListMutex.lock();
            m_userConnectionList.push_front( ConnectionPair( connectionId, gatewayId ) );
            m_inputChainListMutex.unlock();

            cout << " --------------------------------- " << endl;
            cout << "Prep for User login: " << endl;
            cout << "    id: " << loginPacket->userId << endl;
            cout << "  name: " << loginPacket->userName << endl;
            cout << "  uuid: " << loginPacket->uuid << endl;
            cout << "  login time: " << loginPacket->lastLoginTime << endl;
            cout << " --------------------------------- " << endl;
            LogMessage_LoginPacket( loginPacket );
         }
         return true;
      case PacketLogin::LoginType_PrepareForUserLogout:
         {
            PacketPrepareForUserLogout* logoutPacket = static_cast< PacketPrepareForUserLogout* > ( packet );
            U32 connectionId = logoutPacket->connectionId;

           /* cout << " --------------------------------- " << endl;
            cout << "Prep for logout: " << logoutPacket->connectionId << ", " << logoutPacket->uuid << endl;
            
            cout << "User login: " << endl;
            cout << "  uuid: " << logoutPacket->uuid << endl;
            cout << " --------------------------------- " << endl;*/
            LogMessage_LogoutPacket( logoutPacket );

            m_inputChainListMutex.lock();   
            deque< ConnectionPair >::iterator it = m_userConnectionList.begin();
            while( it != m_userConnectionList.end() )
            {
               if( it->connectionId == connectionId )
               {
                  m_userConnectionList.erase( it );
                  break;
               }
               it++;
            }
            m_inputChainListMutex.unlock();
            
         }
         return true;
      }
   }
   return false;
}

//---------------------------------------------------------------

bool     UserStatsMainThread::HandleUserStatPacket( BasePacket* packet, U32 serverIdLookup )
{
   U32 packetType = packet->packetType;
   U32 packetSubType = packet->packetSubType;

   if( packetType == PacketType_UserStats )
   {
      switch( packetSubType )
      {
    /*  case PacketUserStats::UserStatsType_RequestListOfUserStats:
         {
            PacketUserStats_RequestListOfUserStats* stats = static_cast< PacketUserStats_RequestListOfUserStats* >( packet );
            PacketUserStats_RequestListOfUserStatsResponse* response = new PacketUserStats_RequestListOfUserStatsResponse;
            response->userUuid = stats->userUuid;
            response->whichGame = stats->whichGame;

            response->stats.insert( "stat1", "100" );            
            response->stats.insert( "stat2", "1000" );
            response->stats.insert( "stat3", "-9" );
            
            PacketServerJobWrapper* wrapper = new PacketServerJobWrapper;
            wrapper->pPacket = response;
            wrapper->serverId = serverIdLookup;
            AddOutputChainData( wrapper, serverIdLookup );
         }
         break;
      case PacketUserStats::UserStatsType_RecordUserStats:
         {
            PacketUserStats_RecordUserStats* stats = static_cast< PacketUserStats_RecordUserStats* >( packet );
            
            PacketUserStats_RecordUserStatsResponse* response = new PacketUserStats_RecordUserStatsResponse;
            response->userUuid = stats->userUuid;
            response->success = true;
            PacketServerJobWrapper* wrapper = new PacketServerJobWrapper;
            wrapper->pPacket = response;
            wrapper->serverId = serverIdLookup;
            AddOutputChainData( wrapper, serverIdLookup );

            // lookup user and store stats
            const string userUuid = userUuid;
            cout << "Record user stats for user uuid: " << userUuid << endl;
            SerializedKeyValueVector< BoundedString64 >& statList = stats->stats;
            SerializedKeyValueVector< BoundedString64 >::KVIterator iter = statList.begin();
            while( iter != statList.end() )
            {
               const string& key = iter->key;
               const string& value = iter->value;
               iter++;
               cout << "stat: " << key << ", value: " << value << endl;
            }
            cout << "End user stats" << endl;
         }
         break;*/
      case PacketUserStats::UserStatsType_ReportGameResult:
         {
            PacketUserStats_ReportGameResult *pReportResult = static_cast< PacketUserStats_ReportGameResult* >( packet );

            if( s_StatsPlugins[pReportResult->gameType].onReportGameResult != NULL )
            {
               s_StatsPlugins[pReportResult->gameType].onReportGameResult( pReportResult->gameId, pReportResult->playerCount,
                  pReportResult->resultOrder, pReportResult->playerFactions );
            }
         }
         break;
      case PacketUserStats::UserStatsType_ReportUserForfeit:
         {
            PacketUserStats_ReportUserForfeit *pReportForfeit = static_cast< PacketUserStats_ReportUserForfeit* >( packet );

            if( s_StatsPlugins[pReportForfeit->gameType].onReportUserForfeit != NULL )
            {
               s_StatsPlugins[pReportForfeit->gameType].onReportUserForfeit( pReportForfeit->gameId, pReportForfeit->userId );
            }
         }
         break;
      }
   }

   return true;
}

//---------------------------------------------------------------

bool   UserStatsMainThread::AddOutputChainData( BasePacket* packet, U32 connectionId ) 
{
   if( packet->packetType == PacketType_ServerJobWrapper )
   {
      return HandlePacketToOtherServer( packet, connectionId );
   }

   if( packet->packetType == PacketType_GatewayWrapper )
   {
      return SendMessageToClient( packet, connectionId );
   }

   if( packet->packetType == PacketType_DbQuery )
   {
      // here we save the query

      Threading::MutexLock locker( m_mutex );
      if( packet->packetSubType == BasePacketDbQuery::QueryType_Result )
      {
         PacketDbQueryResult* result = static_cast< PacketDbQueryResult* >( packet );
         m_dbQueries.push_back( result );
      /*   if( result->customData != NULL )
            cout << "AddOutputChainData: Non-null custom data " << endl;*/
      }
      return true;
      //return false;
   }

   if( packet->packetType == PacketType_GatewayWrapper )
   {
      return SendMessageToClient( packet, connectionId );
   }

   
   return false;
}

//---------------------------------------------------------------

void     UserStatsMainThread::UpdateDbResults()
{
   // this function is unnecessary in this design
   PacketFactory factory;

   m_mutex.lock();
   deque< PacketDbQueryResult* > tempQueue = m_dbQueries;
   m_dbQueries.clear();
   m_mutex.unlock();

   deque< PacketDbQueryResult* >::iterator it = tempQueue.begin();
   while( it != tempQueue.end() )
   {
      PacketDbQueryResult* dbResult = *it++;
 /*     if( dbResult->customData != NULL )
            cout << "UpdateDbResults: Non-null custom data " << endl;*/
      BasePacket* packet = static_cast<BasePacket*>( dbResult );

      U32 connectionId = dbResult->id;
      connectionId = connectionId;

      factory.CleanupPacket( packet );

   }
}

//---------------------------------------------------------------

bool     UserStatsMainThread::SendMessageToClient( BasePacket* packet, U32 connectionId )
{
   if( packet->packetType == PacketType_GatewayWrapper )// this is already wrapped up and ready for the gateway... send it on.
   {
      U32 gatewayId = 0;
      m_inputChainListMutex.lock();
      ClientMap tempInputContainer = m_connectedClients;    

      deque< ConnectionPair >::iterator it = m_userConnectionList.begin();
      while( it != m_userConnectionList.end() )
      {
         if( it->connectionId == connectionId )
         {
            gatewayId = it->gatewayId;
            break;
         }
         it++;
      }
      m_inputChainListMutex.unlock();
      // we need to look the user connection up.

      ClientMapIterator itInputs = tempInputContainer.begin();
      if( itInputs != tempInputContainer.end() )// only one output currently supported.
      {
         KhaanServerToServer* khaan = static_cast< KhaanServerToServer* >( itInputs->second );
         if( khaan->GetChainedType() == ChainedType_InboundSocketConnector &&
            khaan->GetServerId() == gatewayId )
         {
            khaan->AddOutputChainData( packet );
            m_clientsNeedingUpdate.push_back( khaan->GetChainedId() );
            return true;
         }
         itInputs++;
      }
      return true;
   }

 /*  if( packet->packetType == PacketType_DbQuery )
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

bool  UserStatsMainThread::SendGameData( U32 connectionId, U32 gatewayId, int packetSize, const U8* packet )
{
   const int MaxSize = PacketGameplayRawData::MaxBufferSize  - sizeof( PacketGatewayWrapper );
   return SendRawData< PacketGameplayRawData, UserStatsMainThread > 
      ( packet, packetSize, PacketGameplayRawData::Game, MaxSize, GetServerId(), GetGameProductId(), "raw", connectionId, gatewayId, this );
}

////////////////////////////////////////////////////////////////////////////////////////

bool     UserStatsMainThread::AddQueryToOutput( PacketDbQuery* dbQuery )
{
   PacketFactory factory;
   m_outputChainListMutex.lock();
   BaseOutputContainer tempOutputContainer = m_listOfOutputs;
   m_outputChainListMutex.unlock();

   ChainLinkIteratorType itOutputs = tempOutputContainer.begin();
   while( itOutputs != tempOutputContainer.end() )// only one output currently supported.
   {
      ChainType* outputPtr = static_cast< ChainType*> ( (*itOutputs).m_interface );
      if( outputPtr->GetChainedType() == ChainedType_DatabaseConnector )
      {
         bool isValidConnection = false;
         Database::Deltadromeus* delta = static_cast< Database::Deltadromeus* >( outputPtr );
         if( dbQuery->dbConnectionType != 0 )
         {
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
            if( outputPtr->AddInputChainData( dbQuery, m_chainId ) == true )
            {
               return true;
            }
         }
      }
      itOutputs++;
   }

   BasePacket* deleteMe = static_cast< BasePacket*>( dbQuery );

   factory.CleanupPacket( deleteMe );
   return false;
}

////////////////////////////////////////////////////////////////////////////////////////

DbHandle*   UserStatsMainThread::GetDbConnectionByType( Database::Deltadromeus::DbConnectionType type )
{
   const char* searchName = "Deltadromeus";
   if( type == Database::Deltadromeus::DbConnectionType_none )// special case
   {
      // just grab any connection
      ChainLinkIteratorType itOutputs = m_listOfOutputs.begin();
      while( itOutputs != m_listOfOutputs.end() )// only one output currently supported.
      {
         ChainType* outputPtr = static_cast< ChainType*> ( (*itOutputs).m_interface );
         if( outputPtr->DoesNameMatch( searchName ) )
         {
            Database::Deltadromeus* delta = static_cast< Database::Deltadromeus* >( outputPtr );
            return delta->GetDbHandle();
         }
         itOutputs++;
      }
   }

   // loop once giving preference to connections that fit the type exactly.
   // this is the normal case.
   ChainLinkIteratorType itOutputs = m_listOfOutputs.begin();
   while( itOutputs != m_listOfOutputs.end() )// only one output currently supported.
   {
      //ChainedInterface* interfacePtr = static_cast< ChainedInterface* >( (*itOutputs).m_interface );
      ChainType* outputPtr = static_cast< ChainType*> ( (*itOutputs).m_interface );
      if( outputPtr->DoesNameMatch( searchName ) )
      {
         Database::Deltadromeus* delta = static_cast< Database::Deltadromeus* >( outputPtr );
         if( delta->GetConnectionType() == type ) // exact match
            return delta->GetDbHandle();
      }
      itOutputs++;
   }

   // loop a second time giving preference to those who have a bit matching
   itOutputs = m_listOfOutputs.begin();
   while( itOutputs != m_listOfOutputs.end() )// only one output currently supported.
   {
      //ChainedInterface* interfacePtr = static_cast< ChainedInterface* >( (*itOutputs).m_interface );
      ChainType* outputPtr = static_cast< ChainType*> ( (*itOutputs).m_interface );
      if( outputPtr->DoesNameMatch( searchName ) )
      {
         Database::Deltadromeus* delta = static_cast< Database::Deltadromeus* >( outputPtr );
         if( delta->WillYouTakeThisQuery( type ) ) // bit operator here.. inexact match
            return delta->GetDbHandle();
      }
      itOutputs++;
   }

   cout << "a db connection of that type does not exist. Please reconfigure." << endl;
   assert( 0 );// a db connection of that type does not exist. Please reconfigure.

   // lastly, grab any open connections
   return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////
