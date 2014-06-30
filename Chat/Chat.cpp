// Chat2.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <list>
#include <vector>


#include "../NetworkCommon/ServerConstants.h"

#if PLATFORM == PLATFORM_WINDOWS
#pragma warning( disable:4996 )
#endif

#include "../NetworkCommon/Platform.h"
#include "../NetworkCommon/Version.h"
#include "../NetworkCommon/Utils/CommandLineParser.h"

#include <assert.h>

#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
using boost::format;

#include "../NetworkCommon/NetworkIn/Diplodocus.h"
#include "../NetworkCommon/NetworkIn/DiplodocusServerToServer.h"
#include "../NetworkCommon/Daemon/Daemonizer.h"
#include "../NetworkCommon/NetworkUtils.h"

#include "DiplodocusChat.h"

#if PLATFORM == PLATFORM_WINDOWS
#include <conio.h>
#endif   

using namespace std;

////////////////////////////////////////////////////////////////////////

void  PrintInstructions()
{
   cout << "Chat takes params as follows:" << endl;
   cout << "> chat_server listen.port=7400 db.address=10.16.4.44 db.port=3306 db.username=incinerator db.password=Cm8235 db.schema=playdek ... " << endl;
   cout << " NOTE: chat is a server-to-games, who connect to it." << endl;

   cout << endl << endl;
   cout << ": params are as follows:" << endl;
   cout << "    server.name       - allows a new name reported in logs and connections" << endl;
   cout << "    listen.address    - what is the ipaddress that this app should be using;" << endl;
   cout << "                        usually localhost or null" << endl;
   cout << "    listen.port       - listen on which port to gateway connections" << endl;
   cout << "    s2s.address       - where is the load balancer" << endl;
   cout << "    s2s.port          - load balancer" << endl;
   
   cout << "    stat.address      - stat server ipaddress" << endl;
   cout << "    stat.port         - stat server port" << endl;
   
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
   daemonize( "chat_serverd" );

   CommandLineParser    parser( argc, argv );

   string serverName = "Chat Server";
   string listenPortString = "7400";
   string listenAddress = "localhost";

   string listenForS2SPort = "7402";
   string listenForS2SAddress = "localHost";

   string dbPortString = "16384";
   string dbIpAddress = "localhost";
   string dbUsername = "root";
   string dbPassword = "password";
   string dbSchema = "playdek";

   string statPortString = "7802";
   string statIpAddressString = "localhost";

   //--------------------------------------------------------------

   if( parser.IsRequestingInstructions() == true )
   {
      PrintInstructions();
      return 0;
   }

   parser.FindValue( "db.address", dbIpAddress );
   parser.FindValue( "db.port", dbPortString );
   parser.FindValue( "db.username", dbUsername );
   parser.FindValue( "db.password", dbPassword );
   parser.FindValue( "db.schema", dbSchema );

   parser.FindValue( "server.name", serverName );

   parser.FindValue( "listen.port", listenPortString );
   parser.FindValue( "listen.address", listenAddress );

   parser.FindValue( "s2s.port", listenForS2SPort );
   parser.FindValue( "s2s.address", listenForS2SAddress );

   parser.FindValue( "stat.port", statPortString );
   parser.FindValue( "stat.address", statIpAddressString );

   int   listenPort = 7400, 
         dbPortAddress = 3306,
         statPort = 7802, 
         listenS2SPort = 7402;
   try 
   {
      listenPort = boost::lexical_cast<int>( listenPortString );
      statPort = boost::lexical_cast<int>( statPortString );
      dbPortAddress = boost::lexical_cast<int>( dbPortString );
      listenS2SPort = boost::lexical_cast<int>( listenForS2SPort );
   } 
   catch( boost::bad_lexical_cast const& ) 
   {
       std::cout << "Error: input string was not valid" << std::endl;
   }

   Database::Deltadromeus* delta = new Database::Deltadromeus;
   delta->SetConnectionInfo( dbIpAddress, dbPortAddress, dbUsername, dbPassword, dbSchema );
   if( delta->IsConnected() == false )
   {
      cout << "Error: Database connection is invalid." << endl;
      getch();
      return 1;
   }

   //----------------------------------------------------------------
   
   U64 serverUniqueHashValue = GenerateUniqueHash( serverName );
   U32 serverId = (U32)serverUniqueHashValue;

   cout << serverName << ":" << endl;
   cout << "Server stack version " << ServerStackVersion << endl;
   cout << "ServerId " << serverId << endl;
   cout << "Db " << dbIpAddress << ":" << dbPortAddress << endl;
   cout << "------------------------------------------------------------------" << endl << endl << endl;

   //--------------------------------------------------------------

   InitializeSockets();
   bool isBusy = IsPortBusy( listenPort ) | IsPortBusy( listenS2SPort );
   ShutdownSockets();

   if( isBusy == false )
   {
      DiplodocusChat*    middleware = new DiplodocusChat( serverName, serverId );
      middleware->SetupListening( listenPort );

      DiplodocusServerToServer* s2s = new DiplodocusServerToServer( serverName, serverId, 0, ServerType_Chat );
      s2s->SetupListening( listenS2SPort );

      PrepConnection< FruitadensServer, DiplodocusChat > ( statIpAddressString, statPort, "stat", middleware, ServerType_Stat, true );
      
      //----------------------------------------------------------------
      
      middleware->AddOutputChain( delta );
      s2s->AddOutputChain( middleware );

      middleware->Init();
      middleware->Run();
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

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------