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

   void  SetQuery( string& _query ) { m_queryString = _query; }
   void  SetParent( Queryer* parent ) { m_parent = parent; }
   void  SetPeriodicty( int timeoutSeconds ) { m_periodicitySeconds = timeoutSeconds; }

   virtual void     Update( time_t currentTime );
   virtual void     Fire();// not based on a timer.
   virtual bool     HandleResult( const PacketDbQueryResult* dbResult ) = 0;

protected:
   U32      m_queryType;
   int      m_periodicitySeconds;
   time_t   m_lastTimeStamp;
   string   m_queryString;
   Queryer* m_parent;
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
