// Stat.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <list>
#include <vector>
#pragma warning (disable:4996)
using namespace std;

#include <assert.h>

#include <boost/lexical_cast.hpp>


#include "../NetworkCommon/Version.h"

#include "../NetworkCommon/Utils/CommandLineParser.h"
#include "../NetworkCommon/DataTypes.h"
#include "../NetworkCommon/Utils/Utils.h"

#include "../NetworkCommon/Daemon/Daemonizer.h"
#include "../NetworkCommon/Logging/server_log.h"

#include "../NetworkCommon/NetworkIn/DiplodocusServerToServer.h"
#include "../NetworkCommon/Database/Deltadromeus.h"

#include "DiplodocusStat.h"


////////////////////////////////////////////////////////////////////////

#if PLATFORM == PLATFORM_WINDOWS
#include <conio.h>
#endif   

////////////////////////////////////////////////////////////////////////

void  PrintInstructions()
{
   cout << "Stat Server:" << endl;
   cout << "* This may be the simplest server that we have. It only accepts" << endl;
   cout << "* connections on the s2s endpoint and then saves off the stats." << endl;
   cout << "* Once per hour on the hour, it writes all stats to the db." << endl;
   cout << "* After writing the stats once per hour, the stats are cleared." << endl;
   cout << "* All stats arriving have a particular format which are then used" << endl;
   cout << "* for future lookup (key-value indexing) and all future stat-posts" << endl;
   cout << "* add to or modify that data. You may write as often as you see fit." << endl;   

   cout << "Stat takes the following parameters using this format:" << endl;
   cout << "> stat_server param1=localhost param1.extension='path' ... " << endl;

   cout << endl << endl;
   cout << ": params are as follows:" << endl;
   cout << "    server.name       - allows a new name reported in logs and connections" << endl;
   cout << "    listen.address    - what is the ipaddress that this app should be using; usually localhost or null" << endl;
   cout << "    listen.address    - listen on which port" << endl;
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
	CommandLineParser    parser( argc, argv );

   string serverName = "Stat Server";

   string listenPort = "7800";    // this connection is for consistency, it may not be used at all
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

   parser.FindValue( "listen.port", listenPort );
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


   int listenPortAddress = 7800, dbPortAddress = 3306, listenS2SPort= 7802;
   try 
   {
      listenS2SPort = boost::lexical_cast<int>( listenForS2SPort );
      listenPortAddress = boost::lexical_cast<int>( listenPort );
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
   cout << "Server stack version " << ServerStackVersion << endl;
   cout << "ServerId " << serverId << endl;
   cout << "------------------------------------------------------------------" << endl << endl << endl;

   DiplodocusStat*    statServer = new DiplodocusStat( serverName, serverId );
   statServer->SetupListening( listenPortAddress );

   DiplodocusServerToServer* s2s = new DiplodocusServerToServer( serverName, serverId );
   s2s->SetupListening( listenS2SPort );
   
   //----------------------------------------------------------------
   
   statServer->Init();

   s2s->AddOutputChain( statServer );
   statServer->AddOutputChain( delta );

   s2s->Resume();
   statServer->Run();

   getch();
}

////////////////////////////////////////////////////////////////////////