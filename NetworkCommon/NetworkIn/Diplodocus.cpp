// Diplodocus.cpp


#include <iostream>
#include <memory.h>
#include <time.h>
#include <iostream>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
using boost::format;

#include "../Platform.h"
#include "../DataTypes.h"
#include "../Serialize.h"
#include "Diplodocus.h"
#include "../Utils/OsLevelUtils.h"

using namespace std;

#if PLATFORM == PLATFORM_WINDOWS
#pragma comment( lib, "ws2_32.lib" )
#pragma comment(lib, "libevent.lib")
#endif



event_base* BasePacketChainHandler::m_LibEventInstance = NULL;

void  UpdateConsoleWindow( time_t& timeOfLastTitleUpdate, time_t uptime, int numTotalConnections, int numCurrentConnections, int listeningPort, const string& serverName )
{
   static format titleFormat("%1% -- ( port:%2% ) -- connections[%3%:%4%] -- uptime(%5$3d:%6$2d:%7$2d)");

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
