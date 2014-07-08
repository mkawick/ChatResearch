// AnalyticsMainLoop.cpp

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
#include "../NetworkCommon/Packets/AnalyticsPacket.h"


#include "AnalyticsMainLoop.h"
#include "AnalyticsCommon.h"

#include "../NetworkCommon/Database/StringLookup.h"

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////


DiplodocusStat::DiplodocusStat( const string& serverName, U32 serverId ): Diplodocus< KhaanStat >( serverName, serverId, 0,  ServerType_Analytics )
{
   time( &m_lastDbWriteTimeStamp );
   m_lastDbWriteTimeStamp = ZeroOutMinutes( m_lastDbWriteTimeStamp );
   SetSleepTime( 100 );
}

DiplodocusStat :: ~DiplodocusStat()
{
   //delete m_staticAssets;
   //delete m_dynamicAssets;
}
//---------------------------------------------------------------

void     DiplodocusStat::ServerWasIdentified( IChainedInterface* khaan )
{
   BasePacket* packet = NULL;
   PackageForServerIdentification( m_serverName, m_localIpAddress, m_externalIpAddress, m_serverId, m_listeningPort, m_gameProductId, m_isGame, m_isControllerApp, true, m_gatewayType, &packet );
   ChainedType* localKhaan = static_cast< ChainedType* >( khaan );
   localKhaan->AddOutputChainData( packet, 0 );
   m_clientsNeedingUpdate.push_back( localKhaan->GetServerId() );
}

//---------------------------------------------------------------

bool     DiplodocusStat::AddInputChainData( BasePacket* packet, U32 connectionId )
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

   if( packet->packetType == PacketType_GatewayWrapper ) // this should not happen on the stat server
   {
      assert( 0 );
      // we handle all packets from the gateway here.
      return false;
   }
   
   return false;
}

//---------------------------------------------------------------

bool  DiplodocusStat::HandlePacketFromOtherServer( BasePacket* packet, U32 connectionId )// not thread safe
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
   
   if( packetType == PacketType_Analytics )
   {
      PacketCleaner cleaner( unwrappedPacket );

      PacketAnalytics* statPacket = static_cast< PacketAnalytics* >( unwrappedPacket );

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
   }

   return false;
}

//---------------------------------------------------------------
//---------------------------------------------------------------

void     DiplodocusStat::PeriodicWriteToDB()
{
   time_t currentTime;
   time( &currentTime );

   if( difftime( currentTime, m_lastDbWriteTimeStamp ) >= timeoutDBWriteStatisics ) 
   {
      m_lastDbWriteTimeStamp = ZeroOutMinutes( currentTime );// advance the hour.
   
      HistoricalAnalyisList::iterator historyIt = m_history.begin();
      while( historyIt != m_history.end() )
      {
         const AnalyticsPacketList& list = historyIt->second;
         historyIt++;

         const PacketAnalytics& statPacket = *list.begin();

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

      m_history.clear();
   }
}

//---------------------------------------------------------------
//---------------------------------------------------------------

bool     DiplodocusStat::AddQueryToOutput( PacketDbQuery* packet )
{
   ChainLinkIteratorType itOutputs = m_listOfOutputs.begin();
   while( itOutputs != m_listOfOutputs.end() )// only one output currently supported.
   {
      ChainType* outputPtr = static_cast< ChainType*> ( (*itOutputs).m_interface );
      if( outputPtr->AddInputChainData( packet, m_chainId ) == true )
      {
         return true;
      }
      itOutputs++;
   }

   delete packet;/// normally, we'd leave this up to the invoker to cleanup. 
   return false;
}

//---------------------------------------------------------------
//---------------------------------------------------------------

int      DiplodocusStat::CallbackFunction()
{
   UpdateAllConnections();

   time_t currentTime;
   time( &currentTime );

   int numClients = static_cast< int >( m_connectedClients.size() );
   UpdateConsoleWindow( m_timeOfLastTitleUpdate, m_uptime, m_numTotalConnections, numClients, m_listeningPort, m_serverName );

   PeriodicWriteToDB();

   return 1;
}


//---------------------------------------------------------------
//---------------------------------------------------------------