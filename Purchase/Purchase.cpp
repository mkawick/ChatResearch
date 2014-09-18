// Purchase.cpp : Defines the entry point for the console application.
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

#include "PurchaseMainThread.h"



#if PLATFORM == PLATFORM_WINDOWS
#pragma warning (disable:4996)
#include <conio.h>
#endif   

////////////////////////////////////////////////////////////////////////

void  PrintInstructions()
{
   cout << "purchase takes params as follows:" << endl;
   cout << "> purchase_server listen.port=7700 s2s.port=7702 s2s.address=localhost" << endl;
   cout << "    db.address=10.16.4.44 db.port=3306 db.username=incinerator" << endl;
   cout << "    db.password=Cm8235 db.schema=playdek" << endl;
   cout << " NOTE: purchase is a server-to-games, who connect to it and trade items." << endl;

   cout << endl << endl;
   cout << ": params are as follows:" << endl;
   cout << "    server.name       - allows a new name reported in logs and connections" << endl;
   cout << "    listen.address    - what is the ipaddress that this app should be using;" << endl;
   cout << "                        usually localhost or null" << endl;
   cout << "    listen.port       - listen on which port to gateway connections" << endl;
   cout << "    s2s.address       - where is the load balancer" << endl;
   cout << "    s2s.port          - load balancer" << endl;
   
   cout << " - for simple single DB connection - " << endl;
   cout << "    db.address        - database ipaddress" << endl;
   cout << "    db.port           - database port" << endl;
   cout << "    db.username       - database username" << endl;
   cout << "    db.password       - database password" << endl;
   cout << "    db.schema         - database schema-table collection" << endl;

   cout << " - for multiple DB connections " << endl;
   cout << " A single db connection would look like this... note the 'all' designation" << endl;
   cout << " databaselist=[all:192.168.1.0:21000:root:password:playdek]" << endl;
   cout << " Multiple connections look like this" << endl;
   cout << " databaselist=[user:192.168.1.0:21000:root:password:playdek, game:192.168.1.0:21000:root:password:playdek ]" << endl;


   cout << " -h, -help, -? for help " << endl;
}

////////////////////////////////////////////////////////////////////////

int main( int argc, const char* argv[] )
{
   daemonize( "purchase_serverd" );

	CommandLineParser    parser( argc, argv );

   string serverName = "Purchase Server";

   string listenPortString = "7700";
   string listenAddress = "localhost";

   string listenForS2SPort = "7702";
   string listenForS2SAddress = "localHost";

   //---------------------------------------
   
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

   int listenPort = 7700, dbPortAddress = 3306, listenS2SPort = 7702;
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
      DiplodocusPurchase*    purchaseServer = new DiplodocusPurchase( serverName, serverId );
      purchaseServer->SetupListening( listenPort );

      DiplodocusServerToServer* s2s = new DiplodocusServerToServer( serverName, serverId, 0, ServerType_Purchase );
      s2s->SetupListening( listenS2SPort );
      
      //----------------------------------------------------------------
      
      s2s->AddOutputChain( purchaseServer );
      
      //----------------------------------------------------------------

      if( Database::ConnectToMultipleDatabases< DiplodocusPurchase > ( parser, purchaseServer ) == false )
      {
         Database::Deltadromeus* delta = new Database::Deltadromeus;
         delta->SetConnectionInfo( dbIpAddress, dbPortAddress, dbUsername, dbPassword, dbSchema );
         delta->SetConnectionType( Database::Deltadromeus::DbConnectionType_All );
         if( delta->IsConnected() == false )
         {
            LogMessage( LOG_PRIO_ERR, "Error: Database connection is invalid." );
            getch();
            return 1;
         }
         purchaseServer->AddOutputChain( delta );
      }

      //----------------------------------------------------------------

      s2s->Resume();

      purchaseServer->Init();
      purchaseServer->Run();
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
