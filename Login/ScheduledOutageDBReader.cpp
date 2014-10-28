// ScheduledOutageDBReader.cpp

#include <time.h>
#include <iostream>
#include <boost/lexical_cast.hpp>

#include "../NetworkCommon/Utils/Utils.h"
#include "../NetworkCommon/Packets/DbPacket.h"

#include "DiplodocusLogin.h"
#include "ScheduledOutageDBReader.h"

const int howFarInThePastToPullOutages_InHours = 4;

///////////////////////////////////////////////////////////////////////////////////////////

class TableAdminScheduledServiceOutage
{
public:
   enum Columns
   {
      Column_server_type,
      Column_game_id,
      Column_cancelled,
      Column_timestamp,
      Column_begin_time,
      Column_length_of_outage_in_minutes,
      Column_end
   };
   static const char* const column_names[];
};

typedef Enigmosaurus <TableAdminScheduledServiceOutage> OutageTableParser;

//////////////////////////////////////////////////////////////

ScheduledOutageDBReader::ScheduledOutageDBReader() : m_timeLastQuery( 0 )
{
}

ScheduledOutageDBReader::~ScheduledOutageDBReader()
{
}

void  ScheduledOutageDBReader::Update()
{
   time_t currentTime;
   time( &currentTime );

   if( HasTimeWindowExpired( currentTime, m_timeLastQuery, m_timeBetweenQueries ) == true )
   {
      m_timeLastQuery = currentTime;

      PacketDbQuery* dbQuery = new PacketDbQuery;
      dbQuery->id =     0;
      dbQuery->lookup = DiplodocusLogin::QueryType_ScheduledOutage;

      string date = GetDateInUTC( 0, -howFarInThePastToPullOutages_InHours, 0 );// look at up to 4 hours ago.

      string queryString = "SELECT server_type, game_id, cancelled, timestamp, begin_time, length_of_outage_in_minutes FROM admin_scheduled_service_outage WHERE begin_time>'";
      queryString += date;
      queryString += "' ORDER BY begin_time DESC";

      dbQuery->query =  queryString;
      m_mainLoop->AddQueryToOutput( dbQuery );
   }
}

//////////////////////////////////////////////////////////////

bool     ScheduledOutageDBReader::HandleResult( const PacketDbQueryResult* dbResult )
{
   U32 queryType = static_cast< U32 >( dbResult->lookup );
   if( queryType == DiplodocusLogin::QueryType_ScheduledOutage )
   {
      IndexTableParser              enigma( dbResult->bucket );
      IndexTableParser::iterator    it = enigma.begin();
      if( enigma.size() == 0 )
         return true;

      PacketServerConnectionInfo_ServerOutageSchedule* packet = 
         new PacketServerConnectionInfo_ServerOutageSchedule;

      while( it != enigma.end() )
      {
         OutageTableParser::row      row = *it++;

         U32   serverType =         boost::lexical_cast< U32 >( row[ TableAdminScheduledServiceOutage::Column_server_type ] );
         U32   gameId =             boost::lexical_cast< U32 >( row[ TableAdminScheduledServiceOutage::Column_game_id ] );
         string isCancelled =       row[ TableAdminScheduledServiceOutage::Column_cancelled ];
         bool  cancelled = false;
         if( isCancelled.size() && isCancelled != "0" )
            cancelled = true;

         string timestamp =         row[ TableAdminScheduledServiceOutage::Column_timestamp ];
         string start =             row[ TableAdminScheduledServiceOutage::Column_begin_time ];
         time_t beginTime =         GetDateFromString( start.c_str() );

         int timeWindowInMinutes = boost::lexical_cast< int >( row[ TableAdminScheduledServiceOutage::Column_length_of_outage_in_minutes ] );

         ScheduledOutage outage;
         outage.beginTime = beginTime;
         outage.cancelled = cancelled;
         outage.downTimeInSeconds = timeWindowInMinutes * 60;
         outage.gameId = gameId;
         outage.type = static_cast< ServerType >( serverType );

         packet->scheduledOutages.push_back( outage );
      }
      if( m_mainLoop->SendPacketToGateway( packet, 0, 0, 0 ) == false )
      {
         PacketFactory factory;
         BasePacket* pPacket = packet;
         factory.CleanupPacket( pPacket );
      }

      return true;
   }
   return false;
}

//////////////////////////////////////////////////////////////