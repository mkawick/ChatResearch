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

#include "DiplodocusPurchase.h"



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


   int listenPort = 7700, dbPortAddress = 3306, listenS2SPort = 7702;
   try 
   {
      listenS2SPort = boost::lexical_cast<int>( listenForS2SPort );
      listenPort = boost::lexical_cast<int>( listenPortString );
      dbPortAddress = boost::lexical_cast<int>( dbPortString );
   } 
   catch( boost::bad_lexical_cast const& ) 
   {
       std::cout << "Error: input string was not valid" << std::endl;
   }

   //--------------------------------------------------------------

   Database::Deltadromeus* delta = new Database::Deltadromeus;
   delta->SetConnectionInfo( dbIpAddress, dbPortAddress, dbUsername, dbPassword, dbSchema );
   if( delta->IsConnected() == false )
   {
      cout << "Error: Database connection is invalid." << endl;
      LogMessage(LOG_PRIO_ERR, "Error: Database connection is invalid.\n");
      getch();
      return 1;
   }

   //----------------------------------------------------------------
   
   U64 serverUniqueHashValue = GenerateUniqueHash( serverName );
   U32 serverId = (U32)serverUniqueHashValue;

   cout << serverName << ":" << endl;
   //cout << "Version " << version << endl;
   cout << "Server stack version " << ServerStackVersion << endl;
   cout << "ServerId " << serverId << endl;
   cout << "Db " << dbIpAddress << ":" << dbPortAddress << endl;
   cout << "Network protocol version: " << (int)GlobalNetworkProtocolVersion << endl;
   cout << "------------------------------------------------------------------" << endl << endl << endl;

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
      
      purchaseServer->Init();

      s2s->AddOutputChain( purchaseServer );
      purchaseServer->AddOutputChain( delta );

      s2s->Resume();
      purchaseServer->Run();
   }
   else
   {
      cout << "***********************************************" << endl;
      cout << " error: that server port is busy " << endl;
      cout << "  port: " << listenPort << endl;
      cout << "  port: " << listenS2SPort << endl;
      cout << " Note: you may have an instance already running" << endl;
      cout << "        we must exit now" << endl;
      cout << "***********************************************" << endl;
      cout << endl << "Press any key to exit" << endl;
      getch();
   }
	return 0;
}

////////////////////////////////////////////////////////////////////////
