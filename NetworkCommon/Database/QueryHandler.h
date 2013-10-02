// QueryHandler.h

#pragma once

#include "../Packets/DbPacket.h"
#include "../ChainedArchitecture/ChainedThread.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Queryer : public Threading::CChainedThread < BasePacket* >
{
public:
   enum { DefaultSleepTime = 30 };

public:

   Queryer() : Threading::CChainedThread < BasePacket* >( false, DefaultSleepTime, false ) {}

   virtual bool AddQueryToOutput( PacketDbQuery* packet ) = 0;
   virtual bool AddOutputChainData( BasePacket* packet, U32 connectionId ) = 0;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class QueryHandler
{
public:

   QueryHandler( U32 type, int periodicity, Queryer* parent );
   virtual ~QueryHandler();

   void  SetQuery( string& _query ) { m_queryString = _query; }
   void  SetParent( Queryer* parent ) { m_parent = parent; }
   void  SetPeriodicty( int timeoutSeconds ) { m_periodicitySeconds = timeoutSeconds; }


   void  SetRunSlower( bool runSlower ) { m_runSlower = runSlower; }
   bool  IsRunningSlower() const { return m_runSlower; }

   virtual void     Update( time_t currentTime, bool& flagToSet );
   virtual void     Fire();// not based on a timer.
   

protected:

   virtual bool     HandleResult( const PacketDbQueryResult* dbResult ) = 0;
   virtual void     SubmitQuery();

   U32      m_queryType;
   int      m_periodicitySeconds;
   bool     m_runSlower;// there are various timeout reasons to run slower.
   time_t   m_lastTimeStamp;
   string   m_queryString;
   Queryer* m_parent;
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
