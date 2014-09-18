// Analytics.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <list>
#include <vector>
using namespace std;

#include <assert.h>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
using boost::format;

#include "../NetworkCommon/Version.h"

#include "../NetworkCommon/Utils/CommandLineParser.h"
#include "../NetworkCommon/DataTypes.h"
#include "../NetworkCommon/Utils/Utils.h"
#include "../NetworkCommon/NetworkUtils.h"

#include "../NetworkCommon/Daemon/Daemonizer.h"
#include "../NetworkCommon/Logging/server_log.h"

#include "../NetworkCommon/NetworkIn/DiplodocusServerToServer.h"
#include "../NetworkCommon/Database/Deltadromeus.h"

#include "../NetworkCommon/Daemon/Daemonizer.h"

#include "AnalyticsMainLoop.h"

#if PLATFORM == PLATFORM_WINDOWS
#include <conio.h>
#pragma warning (disable:4996)
#endif

////////////////////////////////////////////////////////////////////////

#if PLATFORM == PLATFORM_WINDOWS
#include <conio.h>
#endif   

////////////////////////////////////////////////////////////////////////

void  PrintInstructions()
{
   cout << "Analytics Server:" << endl;
   cout << "* This may be the simplest server that we have. It only accepts" << endl;
   cout << "* connections on the s2s endpoint and then saves off the analytics." << endl;
   cout << "* Once per hour on the hour, it writes all analytics to the db." << endl;
   cout << "* After writing the analytics once per hour, the analytics are cleared." << endl;
   cout << "* All analytics arriving have a particular format which are then used" << endl;
   cout << "* for future lookup (key-value indexing) and all future analytics-posts" << endl;
   cout << "* add to or modify that data. You may write as often as you see fit." << endl;   

   cout << "analytics takes the following parameters using this format:" << endl;
   cout << "> analytics_server param1=localhost param1.extension='path' ... " << endl;

   cout << endl << endl;
   cout << ": params are as follows:" << endl;
   cout << "    server.name       - allows a new name reported in logs and connections" << endl;
   cout << "    listen.address    - what is the ipaddress that this app should be using;" << endl;
   cout << "                        usually localhost or null" << endl;
   cout << "    listen.port       - listen on which port" << endl;
   cout << "    s2s.address       - where is the for gateway, login and others" << endl;
   cout << "    s2s.port          - where is the port for gateway, login and others" << endl;
   
   cout << "    db.address        - database ipaddress" << endl;
   cout << "    db.port           - database port" << endl;
   cout << "    db.username       - database username" << endl;
   cout << "    db.password       - database password" << endl;
   cout << "    db.schema         - database schema-table collection" << endl;

   cout << " -h, -help, -? for help " << endl;
}
////////////////////////////////////////////////////////////////////////

int main( int argc, const char* argv[] )
{
   daemonize( "analytics_serverd" );

	CommandLineParser    parser( argc, argv );

   string serverName = "Analytics Server";

   string listenPortString = "7800";    // this connection is for consistency, it may not be used at all
   string listenAddress = "localhost";

   string listenForS2SPort = "7802";
   string listenForS2SAddress = "localHost";

   //---------------------------------------

   if( parser.IsRequestingInstructions() == true )
   {
      PrintInstructions();
      return 0;
   }

   //--------------------------------------------------------------
   
   parser.FindValue( "server.name", serverName );

   parser.FindValue( "listen.port", listenPortString );
   parser.FindValue( "listen.address", listenAddress );

   parser.FindValue( "s2s.port", listenForS2SPort );
   parser.FindValue( "s2s.address", listenForS2SAddress );

   string dbPortString = "16384";
   string dbIpAddress = "localhost";
   string dbUsername = "root";
   string dbPassword = "password";
   string dbSchema = "playdek";

   parser.FindValue( "db.address", dbIpAddress );
   parser.FindValue( "db.port", dbPortString );
   parser.FindValue( "db.username", dbUsername );
   parser.FindValue( "db.password", dbPassword );
   parser.FindValue( "db.schema", dbSchema );


   FileLogOpen( serverName.c_str() );
   int listenPort = 7800, dbPortAddress = 3306, listenS2SPort= 7802;
   try 
   {
      listenS2SPort = boost::lexical_cast<int>( listenForS2SPort );
      listenPort = boost::lexical_cast<int>( listenPortString );
      dbPortAddress = boost::lexical_cast<int>( dbPortString );
   } 
   catch( boost::bad_lexical_cast const& ) 
   {
       LogMessage( LOG_PRIO_INFO, "Error: input string was not valid" );
   }

   //--------------------------------------------------------------

   Database::Deltadromeus* delta = new Database::Deltadromeus;
   delta->SetConnectionInfo( dbIpAddress, dbPortAddress, dbUsername, dbPassword, dbSchema );
   if( delta->IsConnected() == false )
   {
      //cout << "Error: Database connection is invalid." << endl;
      LogMessage(LOG_PRIO_ERR, "Error: Database connection is invalid.\n");
      getch();
      return 1;
   }

   //----------------------------------------------------------------
   
   U64 serverUniqueHashValue = GenerateUniqueHash( serverName );
   U32 serverId = (U32)serverUniqueHashValue;

   LogMessage( LOG_PRIO_INFO, serverName.c_str() );
   LogMessage( LOG_PRIO_INFO, "Server stack version %s" , ServerStackVersion );
   LogMessage( LOG_PRIO_INFO, "ServerId %u" , serverId );
   LogMessage( LOG_PRIO_INFO, "Db %s : %d", dbIpAddress.c_str(), dbPortAddress );
   LogMessage( LOG_PRIO_INFO, "Network protocol version: %d:%d" , (int)NetworkVersionMajor , ":" , (int)NetworkVersionMinor );
      
   LogMessage( LOG_PRIO_INFO, "------------------------------------------------------------------" );

   InitializeSockets();
   bool isBusy = IsPortBusy( listenPort ) | IsPortBusy( listenS2SPort );
   ShutdownSockets();

   if( isBusy == false )
   {
      DiplodocusStat*    statServer = new DiplodocusStat( serverName, serverId );
      statServer->SetupListening( listenPort );

      DiplodocusServerToServer* s2s = new DiplodocusServerToServer( serverName, serverId, 0, ServerType_Analytics );
      s2s->SetupListening( listenS2SPort );
      
      //----------------------------------------------------------------
      
      statServer->Init();

      s2s->AddOutputChain( statServer );
      statServer->AddOutputChain( delta );

      s2s->Resume();
      statServer->Run();
   }
   else
   {
      LogMessage( LOG_PRIO_ERR, "***********************************************" );
      LogMessage( LOG_PRIO_ERR, " error: that server port is busy " );
      LogMessage( LOG_PRIO_ERR, "  port: %d", listenPort );
      LogMessage( LOG_PRIO_ERR, "  port: %d", listenS2SPort );
      LogMessage( LOG_PRIO_ERR, " Note: you may have an instance already running" );
      LogMessage( LOG_PRIO_ERR, "        we must exit now" );
      LogMessage( LOG_PRIO_ERR, "***********************************************" );
      LogMessage( LOG_PRIO_ERR, "\nPress any key to exit" );
      getch();
   }
	return 0;
}

////////////////////////////////////////////////////////////////////////