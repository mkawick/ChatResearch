#include "NotificationMainThread.h"

#include <iostream>
#include <time.h>
#include <string>
using namespace std;

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/replace.hpp>

#include "../NetworkCommon/Utils/CommandLineParser.h"

#include "../NetworkCommon/Packets/ServerToServerPacket.h"
#include "../NetworkCommon/Packets/PacketFactory.h"
#include "../NetworkCommon/Packets/NotificationPacket.h"


#include "NotificationMainThread.h"
//#include "StatsCommon.h"

#include "../NetworkCommon/Database/StringLookup.h"

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

NotificationMainThread::NotificationMainThread( const string& serverName, U32 serverId ): Queryer(), Diplodocus< KhaanServerToServer >( serverName, serverId, 0,  ServerType_Notification )
{
   time( &m_lastNotificationCheck_TimeStamp );
   m_lastNotificationCheck_TimeStamp = ZeroOutMinutes( m_lastNotificationCheck_TimeStamp );
   SetSleepTime( 100 );
}

NotificationMainThread :: ~NotificationMainThread()
{
}
//---------------------------------------------------------------

void     NotificationMainThread::ServerWasIdentified( IChainedInterface* khaan )
{
   BasePacket* packet = NULL;
   PackageForServerIdentification( m_serverName, m_localIpAddress, m_serverId, m_listeningPort, m_gameProductId, m_isGame, m_isControllerApp, true, m_isGateway, &packet );
   ChainedType* localKhaan = static_cast< ChainedType* >( khaan );
   localKhaan->AddOutputChainData( packet, 0 );
   m_clientsNeedingUpdate.push_back( localKhaan->GetServerId() );
}

//---------------------------------------------------------------

bool     NotificationMainThread::AddInputChainData( BasePacket* packet, U32 connectionId )
{
   if( packet->packetType == PacketType_GatewayInformation )
   {
      PacketCleaner cleaner( packet );// we do not accept any data from the gateway
      HandleCommandFromGateway( packet, connectionId );
      return true;
   }

   if( packet->packetType == PacketType_ServerJobWrapper )
   {
      PacketCleaner cleaner( packet );
      HandlePacketFromOtherServer( packet, connectionId );
      return true;
   }

   if( packet->packetType == PacketType_GatewayWrapper ) // this should not happen on the stat server
   {
      assert( 0 );
      // we handle all packets from the gateway here.
      return false;
   }
   
   return false;
}

//---------------------------------------------------------------

bool   NotificationMainThread::AddOutputChainData( BasePacket* packet, U32 connectionId ) 
{
  /* if( packet->packetType == PacketType_ServerJobWrapper )
   {
      return HandlePacketToOtherServer( packet, connectionId );
   }*/

   if( packet->packetType == PacketType_DbQuery )
   {
      Threading::MutexLock locker( m_mutex );
      if( packet->packetSubType == BasePacketDbQuery::QueryType_Result )
      {
         PacketDbQueryResult* result = static_cast<PacketDbQueryResult*>( packet );
         //m_dbQueries.push_back( result );
         if( result->customData != NULL )
            cout << "AddOutputChainData: Non-null custom data " << endl;
      }
      return true;
   }

   
   return false;
}


//---------------------------------------------------------------

bool  NotificationMainThread::HandlePacketFromOtherServer( BasePacket* packet, U32 connectionId )// not thread safe
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
   
   /*if( packetType == PacketType_Stat )
   {
      PacketCleaner cleaner( unwrappedPacket );

      PacketStat* statPacket = static_cast< PacketStat* >( unwrappedPacket );

      cout << "***********************************************"      << endl;
      cout << "* stat packet contents *" << endl;
      cout << "  statName       :"  << statPacket->statName          << endl;
      cout << "  serverReporting:"  << statPacket->serverReporting   << endl;
      cout << "  category       :"  << statPacket->category          << endl;
      cout << "  subCategory    :"  << statPacket->subCategory       << endl;
      cout << "  value          :"  << statPacket->value             << endl;
      cout << "  timestamp      :"  << statPacket->timestamp         << endl;
      cout << "***********************************************"      << endl;

      AddStatUtil( m_history, *statPacket );

      return true;
   }*/

   return false;
}

//---------------------------------------------------------------
//---------------------------------------------------------------

void     NotificationMainThread::PeriodicCheckForNewNotifications()
{
   time_t currentTime;
   time( &currentTime );

   if( difftime( currentTime, m_lastNotificationCheck_TimeStamp ) >= timeoutDBWriteStatisics ) 
   {
      m_lastNotificationCheck_TimeStamp = ZeroOutMinutes( currentTime );// advance the hour.
   
      /*HistoricalStats::iterator historyIt = m_history.begin();
      while( historyIt != m_history.end() )
      {
         const StatPacketList& list = historyIt->second;
         historyIt++;

         const PacketStat& statPacket = *list.begin();

         CalculatedStats means = CalcStats( list );

         string query = "INSERT INTO playdek.stats_server (stat_name, server_reporting, category, sub_category, mean, final_value, min_value, max_value, num_values, std_dev, begin_time, end_time) VALUES ( \'";
         query += statPacket.statName;
         query += "\',\'";
         query += statPacket.serverReporting;
         query += "\',";
         query += boost::lexical_cast<string>( (int) statPacket.category );
         query += ",";

         query += boost::lexical_cast<string>( (int) statPacket.subCategory );
         query += ",";
         query += boost::lexical_cast<string>( (int) means.mean );
         query += ",";
         query += boost::lexical_cast<string>( (int) means.finalValue );
         query += ",";
         query += boost::lexical_cast<string>( (int) means.minValue );
         query += ",";
         query += boost::lexical_cast<string>( (int) means.maxValue );
         query += ",";
         query += boost::lexical_cast<string>( (int) means.numValues );
         query += ",";
         query += boost::lexical_cast<string>( (int) means.stdDev);
         query += ",\"";
         query += means.beginTime;
         query += "\",\"";
         query += means.endTime;
         query += "\");";
         cout << "Query:" << query << endl;
         cout << "***********************************************" << endl;
         PacketDbQuery* dbQuery = new PacketDbQuery;
         dbQuery->isFireAndForget = true;
         dbQuery->id = 0;
         dbQuery->lookup = 0x1;// unimportant
         //dbQuery->meta;

         dbQuery->query = query;
         AddQueryToOutput( dbQuery );
      }

      m_history.clear();*/
   }
}

//---------------------------------------------------------------
//---------------------------------------------------------------

bool     NotificationMainThread::AddQueryToOutput( PacketDbQuery* query )
{
   ChainLinkIteratorType itOutputs = m_listOfOutputs.begin();
   while( itOutputs != m_listOfOutputs.end() )// only one output currently supported.
   {
      ChainType* outputPtr = static_cast< ChainType*> ( (*itOutputs).m_interface );
      if( outputPtr->AddInputChainData( query, m_chainId ) == true )
      {
         return true;
      }
      itOutputs++;
   }

   BasePacket* packet = static_cast<BasePacket*>( query );
   PacketFactory factory;
   factory.CleanupPacket( packet );/// normally, we'd leave this up to the invoker to cleanup. 
   return false;
}

//---------------------------------------------------------------
//---------------------------------------------------------------

int     NotificationMainThread::MainLoop_InputProcessing()
{
   return 1;
}

//---------------------------------------------------------------

int      NotificationMainThread::MainLoop_OutputProcessing()
{
   UpdateAllConnections();

   time_t currentTime;
   time( &currentTime );

   int numClients = static_cast< int >( m_connectedClients.size() );
   UpdateConsoleWindow( m_timeOfLastTitleUpdate, m_uptime, m_numTotalConnections, numClients, m_listeningPort, m_serverName );

   PeriodicCheckForNewNotifications();

   return 1;
}


//---------------------------------------------------------------
//---------------------------------------------------------------