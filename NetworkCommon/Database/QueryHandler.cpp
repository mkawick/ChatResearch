
#include <time.h>
#include "../Utils/Utils.h"
#include "QueryHandler.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QueryHandler :: QueryHandler( U32 type, int periodicity, Queryer* parent ) : m_queryType( type ), m_periodicitySeconds( periodicity ), m_parent( parent ) 
{
   time( &m_lastTimeStamp );
}

QueryHandler :: ~QueryHandler()
{
}

void     QueryHandler::Update( time_t currentTime, bool& flagToSet )
{
   if( m_parent == NULL || flagToSet == true )
      return;

   if( difftime( currentTime, m_lastTimeStamp ) >= m_periodicitySeconds ) 
   {
      m_lastTimeStamp = currentTime;

      flagToSet = true;
      PacketDbQuery* dbQuery = new PacketDbQuery;
      dbQuery->id = 0;
      dbQuery->lookup = m_queryType;

      dbQuery->query = m_queryString;

      m_parent->AddQueryToOutput( dbQuery );
   }
}

void     QueryHandler::Fire()// not based on a timer.
{
   if( m_parent == NULL )
      return;

   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id = 0;
   dbQuery->lookup = m_queryType;

   dbQuery->query = m_queryString;

   m_parent->AddQueryToOutput( dbQuery );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
