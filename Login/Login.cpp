// Login.cpp
//

#include <iostream>
#include <list>
#include <vector>

//#include <conio.h>
#include <assert.h>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
using boost::format;

#include <cstdio>
#include <memory.h>

#include "../NetworkCommon/Version.h"
#include "../NetworkCommon/Utils/Utils.h"

#include "../NetworkCommon/Utils/CommandLineParser.h"

#include "../NetworkCommon/Database/Deltadromeus.h"
#include "../NetworkCommon/Packets/ServerToServerPacket.h"
#include "../NetworkCommon/NetworkUtils.h"

#include "../NetworkCommon/Daemon/Daemonizer.h"

#include "KhaanLogin.h"
#include "DiplodocusLogin.h"
#include "FruitadensLogin.h"

#if PLATFORM == PLATFORM_WINDOWS
#include <conio.h>
#pragma warning (disable:4996)
#endif

using namespace std;

////////////////////////////////////////////////////////////////////////

void  PrintInstructions()
{
   cout << "login Server:" << endl;
   cout << "* This may be the most complex server that we have. It accepts" << endl;
   cout << "* user connections from the gateway, looks up the user account"<< endl;
   cout << "* info, and if all goes well, sends the successful login info"<< endl;
   cout << "* to the other servers. If the login fails, the login server tells"<< endl; 
   cout << "* the gateway to block that user." << endl;

   cout << endl;

   cout << "  Relogin also happens here which makes a minor difference in " << endl;   
   cout << "  the login process because of the db and user-account latency." << endl;
   cout << "  The user account info is kept around for retries... especially" << endl;
   cout << "  sice the app may log the user out frequently." << endl << endl;

   cout << "Login takes the following parameters using this format:" << endl;
   cout << "> login_server db.address=10.16.4.44 db.port=3306 db.username=incinerator" << endl;
   cout << "  db.password=Cm8235 db.schema=playdek listen.address=localhost" << endl;
   cout << "  listen.port=7600 contact.address=localhost chat.address=localhost" << endl;
   cout << "  contact.port=7502 asset.port=7302 autoAddLoginProduct=false" << endl;
   cout << "  games=[localhost:21000:summon_war,192.168.1.1:21100:MFM]" << endl;

   cout << endl << endl;
   cout << ": params are as follows:" << endl;
   cout << "    server.name       - allows a new name reported in logs and connections" << endl;
   cout << "    listen.address    - what is the ipaddress that this app should be using;" << endl;
   cout << "                        usually localhost or null" << endl;
   cout << "    listen.port       - listen on which port" << endl;
  // cout << "    s2s.address       - where is the for gateway, login and others" << endl;
   //cout << "    s2s.port          - where is the port for gateway, login and others" << endl;
   
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


   cout << " ------ all of the following parameters are optional. --------" << endl;
   
   cout << "    balancer.address  - where is the load balancer" << endl;
   cout << "    balancer.port     - load balancer" << endl;
   
   cout << "    asset.address     - asset server ipaddress" << endl;
   cout << "    asset.port        - asset server port" << endl;

   cout << "    chat.address      - chat server ipaddress" << endl;
   cout << "    chat.port         - chat server port" << endl;

   cout << "    contact.address   - contact server ipaddress" << endl;
   cout << "    contact.port      - contact server port" << endl;

   cout << "    purchase.address  - purchase server ipaddress" << endl;
   cout << "    purchase.port     - purchase server port" << endl;

   cout << "    analytics.address - analytics server ipaddress" << endl;
   cout << "    analytics.port    - analytics server port" << endl;

   cout << "    notification.address - when a user registers a device..." << endl;
   cout << "    notification.port - when a user registers a device..." << endl;

   cout << "    userstats.address - user stats server ipaddress" << endl;
   cout << "    userstats.port    - user stats server port" << endl;

   cout << "    print.functions   - print each function for debugging" << endl;
   cout << "    print.packets     - print each packet for debugging" << endl;

   cout << "    autoAddLoginProduct - automatically add the product from which you login" << endl;
   cout << "                          to the list of user purchases [true, false]" << endl;

   cout << "    games = [192.168.1.0:21000:MFM,localhost:21100:game1]     - game list" << endl;

   cout << " -h, -help, -? for help " << endl;
}

////////////////////////////////////////////////////////////////////////

int main( int argc, const char* argv[] )
{
   daemonize( "login_serverd" );

   CommandLineParser    parser( argc, argv );

   string serverName = "Login server";
   string listenPortString = "7600";
   string listenAddressString = "localhost";

   string assetPortString = "7302";
   string assetIpAddressString = "localhost";

   string chatPortString = "7402";
   string chatIpAddressString = "localhost";

   string contactPortString = "7502";
   string contactIpAddressString = "localhost";

   string purchasePortString = "7702";
   string purchaseIpAddressString = "localhost";

   string analyticsPortString = "7802";
   string analyticsIpAddressString = "localhost";

   string notificationPortString = "7902";
   string notificationIpAddressString = "localhost";

   string userStatsPortString = "12002";
   string userStatsIpAddressString = "localhost";

   string autoAddLoginProductString = "true";

   string printFunctionsString = "false";
   string printPacketTypes = "false";

   
   //--------------------------------------------------------------

   if( parser.IsRequestingInstructions() == true )
   {
      PrintInstructions();
      return 0;
   }

   parser.FindValue( "server.name", serverName );

   parser.FindValue( "listen.port", listenPortString );
   parser.FindValue( "listen.address", listenAddressString );

   parser.FindValue( "chat.port", chatPortString );
   parser.FindValue( "chat.address", chatIpAddressString );

   parser.FindValue( "contact.port", contactPortString );
   parser.FindValue( "contact.address", contactIpAddressString );

   parser.FindValue( "asset.port", assetPortString );
   parser.FindValue( "asset.address", assetIpAddressString );

   parser.FindValue( "purchase.port", assetPortString );
   parser.FindValue( "purchase.address", assetIpAddressString );

   parser.FindValue( "analytics.port", analyticsPortString );
   parser.FindValue( "analytics.address", analyticsIpAddressString );

   parser.FindValue( "notification.port", notificationPortString );
   parser.FindValue( "notification.address", notificationIpAddressString );

   parser.FindValue( "userstat.port", userStatsPortString );
   parser.FindValue( "userstat.address", userStatsIpAddressString );

   parser.FindValue( "autoAddLoginProduct", autoAddLoginProductString );

   parser.FindValue( "print.functions", printFunctionsString );
   parser.FindValue( "print.packets", printPacketTypes );

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


   int   listenPort = 7600, 
         dbPortAddress = 3306, 
         chatPort = 7402, 
         contactPort=7502, 
         assetPort=7302, 
         purchasePort=7702, 
         analyticsPort = 7802, 
         notificationPort = 7902,
         userStatsPort = 12000;
   bool autoAddLoginProduct = true,
         printPackets = false, 
         printFunctions = false;

   try 
   {
       listenPort = boost::lexical_cast<int>( listenPortString );
       dbPortAddress = boost::lexical_cast<int>( dbPortString );

       assetPort = boost::lexical_cast<int>( assetPortString );
       chatPort = boost::lexical_cast<int>( chatPortString );
       contactPort = boost::lexical_cast<int>( contactPortString );
       analyticsPort = boost::lexical_cast<int>( analyticsPortString );
       notificationPort = boost::lexical_cast<int>( notificationPortString );
       userStatsPort = boost::lexical_cast<int>( userStatsPortString );

       std::transform( autoAddLoginProductString.begin(), autoAddLoginProductString.end(), autoAddLoginProductString.begin(), ::tolower );
       if( autoAddLoginProductString == "1" || autoAddLoginProductString == "true" )
          autoAddLoginProduct = true;
       else
          autoAddLoginProduct = false;
   } 
   catch( boost::bad_lexical_cast const& ) 
   {
       std::cout << "Error: input string was not valid" << std::endl;
   }

   if( printPacketTypes == "1" || printPacketTypes == "true" || printPacketTypes == "TRUE" )
   {
      printPackets = true;
   }
   if( printFunctionsString == "1" || printFunctionsString == "true" || printFunctionsString == "TRUE" )
   {
      printFunctions = true;
   }

   //----------------------------------------------------------------

   U64 serverUniqueHashValue = GenerateUniqueHash( serverName );
   U32 serverId = (U32)serverUniqueHashValue;

   cout << serverName << endl;
   cout << "Server stack version " << ServerStackVersion << endl;
   cout << "ServerId " << serverId << endl;
   cout << "Db " << dbIpAddress << ":" << dbPortAddress << endl;
   cout << "Network protocol version: " << (int)NetworkVersionMajor << ":" << (int)NetworkVersionMinor << endl;
   
   cout << "   flag: autoAddLoginProduct = " << std::boolalpha << autoAddLoginProduct << endl;
   cout << "------------------------------------------------------------------" << endl << endl << endl;

   InitializeSockets();
   bool isBusy = IsPortBusy( listenPort );
   ShutdownSockets();

   if( isBusy == false )
   {
      DiplodocusLogin* loginServer = new DiplodocusLogin( serverName, serverId );
      loginServer->SetAsControllerApp( false );
      loginServer->SetGatewayType( PacketServerIdentifier::GatewayType_None );
      loginServer->SetAsGame( false );

      //loginServer->AddOutputChain( delta );
      loginServer->SetupListening( listenPort );
      loginServer->AutoAddTheProductFromWhichYouLogin( autoAddLoginProduct );
      loginServer->PrintPacketTypes( printPackets );
      loginServer->PrintFunctionNames( printFunctions );

      //----------------------------------------------------------------

      if( Database::ConnectToMultipleDatabases< DiplodocusLogin > ( parser, loginServer ) == false )
      {
         Database::Deltadromeus* delta = new Database::Deltadromeus;
         delta->SetConnectionInfo( dbIpAddress, dbPortAddress, dbUsername, dbPassword, dbSchema );
         delta->SetConnectionType( Database::Deltadromeus::DbConnectionType_All );
         if( delta->IsConnected() == false )
         {
            cout << "Error: Database connection is invalid." << endl;
            getch();
            return 1;
         }
         loginServer->AddOutputChain( delta );
      }
      
      //----------------------------------

      PrepConnection< FruitadensLogin, DiplodocusLogin > ( chatIpAddressString, chatPort,                   "chat",           loginServer, ServerType_Chat, true );
      PrepConnection< FruitadensLogin, DiplodocusLogin > ( contactIpAddressString, contactPort,             "contact",        loginServer, ServerType_Contact, true );
      PrepConnection< FruitadensLogin, DiplodocusLogin > ( assetIpAddressString, assetPort,                 "asset",          loginServer, ServerType_Asset, true );
      PrepConnection< FruitadensLogin, DiplodocusLogin > ( purchaseIpAddressString, purchasePort,           "purchase",       loginServer, ServerType_Purchase, true );
      PrepConnection< FruitadensLogin, DiplodocusLogin > ( analyticsIpAddressString, analyticsPort,         "analytics",      loginServer, ServerType_Analytics, true );
      PrepConnection< FruitadensLogin, DiplodocusLogin > ( notificationIpAddressString, notificationPort,   "notification",   loginServer, ServerType_Notification, true );
      PrepConnection< FruitadensLogin, DiplodocusLogin > ( userStatsIpAddressString, userStatsPort,         "userstat",       loginServer, ServerType_UserStats, true );
      
      ConnectToMultipleGames< FruitadensLogin, DiplodocusLogin > ( parser, loginServer, true );

      loginServer->Init();
      loginServer->Resume();
      loginServer->Run();
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

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
