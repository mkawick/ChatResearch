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
#include "../NetworkCommon/Utils/StringUtils.h"
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
   
   cout << " - for simple single DB connection - " << endl;
   cout << "    db.address        - database ipaddress" << endl;
   cout << "    db.port           - database port" << endl;
   cout << "    db.username       - database username" << endl;
   cout << "    db.password       - database password" << endl;
   cout << "    db.schema         - database schema-table collection" << endl;
   cout << "    keepalive         - send keep alive packets to clients of this server" << endl;

   cout << " - for multiple DB connections " << endl;
   cout << " A single db connection would look like this... note the 'all' designation" << endl;
   cout << " databaselist=[all:192.168.1.0:21000:root:password:playdek]" << endl;
   cout << " Multiple connections look like this" << endl;
   cout << " databaselist=[user:192.168.1.0:21000:root:password:playdek, game:192.168.1.0:21000:root:password:playdek ]" << endl;

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

#if PLATFORM == PLATFORM_WINDOWS // default
   string enableKeepAliveString = "false";
#else
   string enableKeepAliveString = "true";
#endif

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
   parser.FindValue( "keepalive", enableKeepAliveString );
   //parser.FindValue( "ios.keyfile", iosKeyFile );
 /*  parser.FindValue( "android.certfile", dbPassword );
   //parser.FindValue( "android.certfile", dbPassword );
   parser.FindValue( "amazon.certfile", dbSchema );*/

   FileLogOpen( serverName.c_str() );

   int listenPort = 7900, dbPortAddress = 3306, listenS2SPort= 7902;
   bool enableKeepAlive = ConvertToTrueFalse( enableKeepAliveString );
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
      NotificationMainThread*    notificationServer = new NotificationMainThread( serverName, serverId );
      notificationServer->SetupListening( listenPort );
      notificationServer->RequireKeepAlive( enableKeepAlive );

      DiplodocusServerToServer* s2s = new DiplodocusServerToServer( serverName, serverId, 0, ServerType_Notification );
      s2s->SetupListening( listenS2SPort );
      
      //----------------------------------------------------------------
      
      notificationServer->Init( iosCertPath );

      s2s->AddOutputChain( notificationServer );
      //----------------------------------------------------------------

      if( Database::ConnectToMultipleDatabases< NotificationMainThread > ( parser, notificationServer ) == false )
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
         notificationServer->AddOutputChain( delta );
      }

      //----------------------------------------------------------------

      s2s->Resume();
      
      s2s->QueueInboundRequest( PacketType_Login, PacketLogin::LoginType_RequestLoginStatusOfAllUsers, ServerType_Login );
      notificationServer->Run();
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

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------