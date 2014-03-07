// Diplodocus.cpp


#include <iostream>
#include <memory.h>
#include <time.h>
#include <iostream>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
using boost::format;

#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/thread.h>

#if PLATFORM != PLATFORM_WINDOWS
#include <event2/util.h>
#include <event2/event-config.h>
#endif

#include "../Platform.h"
#include "../DataTypes.h"
#include "../Serialize.h"
#include "../Utils/OsLevelUtils.h"
#include "../Utils/CommandLineParser.h"
#include "Diplodocus.h"

using namespace std;


event_base* BasePacketChainHandler::m_LibEventInstance = NULL;

void  UpdateConsoleWindow( time_t& timeOfLastTitleUpdate, time_t uptime, int numTotalConnections, int numCurrentConnections, int listeningPort, const string& serverName )
{
   static format titleFormat("%1% -- ( port:%2% ) -- connections[%3%:%4%] -- uptime(%5$003d:%6$02d:%7$02d)");

   time_t currentTime;
   time( &currentTime );
   
   if( difftime( currentTime, timeOfLastTitleUpdate ) >= 1 ) // once per second
   {
      timeOfLastTitleUpdate = currentTime;
      int totalUptime = static_cast< int >( difftime( currentTime, uptime ) );
      int hours = totalUptime/3600;
      int minutes = (totalUptime - hours*3600 ) / 60;
      int seconds = totalUptime - hours*3600 - minutes * 60;
      
#ifndef _DISABLE_TITLE_OVERLOAD_
      string title = str( titleFormat
                        % serverName 
                        % listeningPort 
                        % numCurrentConnections 
                        % numTotalConnections 
                        % hours
                        % minutes
                        % seconds );

      SetConsoleWindowTitle( title.c_str() );
#endif //_DISABLE_TITLE_OVERLOAD_
   }
}

void  EnableThreadingInLibEvent()
{
#if PLATFORM == PLATFORM_WINDOWS
      // signal to the libevent system that we may be sending threaded signals to it.
	   // see http://stackoverflow.com/questions/7645217/user-triggered-event-in-libevent
	   evthread_use_windows_threads();
#elif PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX
      evthread_use_pthreads();
#endif
}
