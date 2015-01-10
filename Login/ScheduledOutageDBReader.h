// ScheduledOutageDBReader.h

#pragma once

#include "../NetworkCommon/Packets/DbPacket.h"

class LoginMainThread;

///////////////////////////////////////////////////////////////////////////////////////////

class ScheduledOutageDBReader
{
public:
   ScheduledOutageDBReader();
   ~ScheduledOutageDBReader();

   void  SetMainLoop( LoginMainThread* mainLoop ) { m_mainLoop = mainLoop; }
   void  Update();

   bool  HandleResult( const PacketDbQueryResult* dbResult );

private:
   LoginMainThread*  m_mainLoop;
   time_t            m_timeLastQuery;
   static const int  m_timeBetweenQueries = 12; /// 12 seconds
};

///////////////////////////////////////////////////////////////////////////////////////////
