// QueryHandler.h

#pragma once

#include "../Packets/DbPacket.h"
#include "../ChainedArchitecture/ChainedThread.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Queryer// : public Threading::CChainedThread < BasePacket* >
{
public:

   //Queryer() : Threading::CChainedThread < BasePacket* >( false, DefaultSleepTime, false ) {}

   virtual bool AddQueryToOutput( PacketDbQuery* packet ) = 0;
   virtual bool AddOutputChainData( BasePacket* packet, U32 connectionId ) = 0;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template< typename QueryParentPtr = Queryer* >
class QueryHandler
{
public:
   typedef QueryParentPtr  ParentQueryerPtr;
public:

   QueryHandler( U32 type, int periodicity, ParentQueryerPtr parent, bool fireImmediately = false );
   virtual ~QueryHandler();

   void  SetQuery( string& _query ) { m_queryString = _query; }
   void  SetParent( Queryer* parent ) { m_parent = parent; }
   void  SetPeriodicty( int timeoutSeconds ) { m_periodicitySeconds = timeoutSeconds; }


   void  SetRunSlower( bool runSlower ) { m_runSlower = runSlower; }
   bool  IsRunningSlower() const { return m_runSlower; }
   void  OnlyRunOneTime() { m_onlyRunsOnce = true; }

   virtual void     Update( time_t currentTime, bool& flagToSet );
   virtual void     Fire();// not based on a timer.
   

protected:

   virtual bool     HandleResult( const PacketDbQueryResult* dbResult ) = 0;
   virtual void     SubmitQuery();

   U32                  m_queryType;
   int                  m_periodicitySeconds;
   bool                 m_runSlower;// there are various timeout reasons to run slower.
   bool                 m_onlyRunsOnce;
   int                  m_countOfTimesExecuted;
   time_t               m_lastTimeStamp;
   string               m_queryString;
   ParentQueryerPtr     m_parent;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template< typename QueryParentPtr >
QueryHandler <QueryParentPtr>:: QueryHandler( U32 type, int periodicity, ParentQueryerPtr parent, bool fireImmediately ) : 
                              m_queryType( type ), 
                              m_periodicitySeconds( periodicity ), 
                              m_runSlower( false ),
                              m_parent( parent ),
                              m_onlyRunsOnce( false ),
                              m_countOfTimesExecuted( 0 )
{
   if( fireImmediately == false )// look at how the timers work and if this is set to 0, then we will execute the query on first run, else we wait for the timeout period.
   {
      time( &m_lastTimeStamp );
   }
   else
   {
      m_lastTimeStamp = 0;
   }
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

   if( m_onlyRunsOnce == true && m_countOfTimesExecuted > 0 )
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
      m_countOfTimesExecuted ++;
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////