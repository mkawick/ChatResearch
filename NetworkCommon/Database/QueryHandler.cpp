
#include <time.h>
#include "../Utils/Utils.h"
#include "QueryHandler.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
template< typename QueryParentPtr >
QueryHandler <QueryParentPtr>:: QueryHandler( U32 type, int periodicity, ParentQueryerPtr parent ) : 
                              m_queryType( type ), 
                              m_periodicitySeconds( periodicity ), 
                              m_runSlower( false ),
                              m_parent( parent )
{
   time( &m_lastTimeStamp );
}

template< typename QueryParentPtr >
QueryHandler <QueryParentPtr> :: ~QueryHandler()
{
}

template< typename QueryParentPtr >
void     QueryHandler <QueryParentPtr>::Update( time_t currentTime, bool& flagToSet )
{
   if( m_parent == NULL || flagToSet == true )
      return;

   int testTime = m_periodicitySeconds;
   if( m_runSlower )
   {
      testTime *= 2;
   }

   if( difftime( currentTime, m_lastTimeStamp ) >= testTime ) 
   {
      m_lastTimeStamp = currentTime;

      flagToSet = true;
      SubmitQuery();
   }
}

template< typename QueryParentPtr >
void     QueryHandler <QueryParentPtr>::SubmitQuery()
{
   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id = 0;
   dbQuery->lookup = m_queryType;

   dbQuery->query = m_queryString;

   m_parent->AddQueryToOutput( dbQuery );
}

template< typename QueryParentPtr >
void     QueryHandler <QueryParentPtr> :: Fire()// not based on a timer.
{
   if( m_parent == NULL )
      return;

   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id = 0;
   dbQuery->lookup = m_queryType;

   dbQuery->query = m_queryString;

   m_parent->AddQueryToOutput( dbQuery );
}
*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
