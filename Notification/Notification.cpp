// Notification.cpp : Defines the entry point for the console application.
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

#include "NotificationMainThread.h"

#if PLATFORM == PLATFORM_WINDOWS
#include <conio.h>
#endif   

using namespace std;

////////////////////////////////////////////////////////////////////////

void  PrintInstructions()
{
   cout << "Notification takes params as follows:" << endl;
   cout << "Notification server does not accept connections from the gateway, but it does have those settings for completeness." << endl;
   cout << "> Notification_server db.address=10.16.4.44 db.port=3306 db.username=incinerator db.password=Cm8235 db.schema=playdek ... " << endl;
   cout << " NOTE: Starter is a server-to-games, who connect to it." << endl;

   cout << endl << endl;
   cout << ": params are as follows:" << endl;
   cout << "    server.name       - allows a new name reported in logs and connections" << endl;
   cout << "    listen.address    - what is the ipaddress that this app should be using;" << endl;
   cout << "                        usually localhost or null" << endl;
   cout << "    listen.port       - listen on which port to gateway connections (do not use)" << endl;
   cout << "    s2s.address       - how do other servers connect to it?" << endl;
   cout << "    s2s.port          - which port is open for s2s" << endl;
   
   cout << "    stat.address      - stat server ipaddress" << endl;
   cout << "    stat.port         - stat server port" << endl;
   
   cout << "    db.address        - database ipaddress" << endl;
   cout << "    db.port           - database port" << endl;
   cout << "    db.username       - database username" << endl;
   cout << "    db.password       - database password" << endl;
   cout << "    db.schema         - database schema-table collection" << endl;

   cout << "    ios.certpath      - path to ios keyfile and certfile" << endl;
   //cout << "    ios.certfile      - file and path to ios cert" << endl;
/*   cout << "    android.certfile  - file and path to android cert" << endl;
   cout << "    amazon.certfile   - file and path to amazon cert" << endl;*/

   cout << " -h, -help, -? for help " << endl;
}

////////////////////////////////////////////////////////////////////////

int main( int argc, const char* argv[] )
{
   daemonize( "notification_serverd" );

	CommandLineParser    parser( argc, argv );

   string serverName = "Notification Server";

   string listenPortString = "7900";    // this connection is for consistency, it may not be used at all
   string listenAddress = "localhost";

   string listenForS2SPort = "7902";
   string listenForS2SAddress = "localHost";
   string iosCertPath;

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

   parser.FindValue( "ios.certpath", iosCertPath );
   //parser.FindValue( "ios.keyfile", iosKeyFile );
 /*  parser.FindValue( "android.certfile", dbPassword );
   //parser.FindValue( "android.certfile", dbPassword );
   parser.FindValue( "amazon.certfile", dbSchema );*/


   int listenPort = 7900, dbPortAddress = 3306, listenS2SPort= 7902;
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
      NotificationMainThread*    notificationServer = new NotificationMainThread( serverName, serverId );
      notificationServer->SetupListening( listenPort );

      DiplodocusServerToServer* s2s = new DiplodocusServerToServer( serverName, serverId, 0, ServerType_Notification );
      s2s->SetupListening( listenS2SPort );
      
      //----------------------------------------------------------------
      
      notificationServer->Init( iosCertPath );

      s2s->AddOutputChain( notificationServer );
      notificationServer->AddOutputChain( delta );

      s2s->Resume();
      notificationServer->Run();
   }
   else
   {
      cout << "***********************************************" << endl;
      cout << " error: that server port is busy " << endl;
      cout << "  port: " << listenPort << endl;
      //cout << "  port: " << listenS2SPort << endl;
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