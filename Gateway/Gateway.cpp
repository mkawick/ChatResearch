// Gateway.cpp
//

#include <list>
#include <vector>
#include <iostream>
#include <assert.h>

#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
using boost::format;

#include <cstdio>
#include "../NetworkCommon/Version.h"
#include "../NetworkCommon/Utils/Utils.h"
#include "../NetworkCommon/Utils/CommandLineParser.h"
#include "../NetworkCommon/Packets/ServerToServerPacket.h"
#include "../NetworkCommon/NetworkUtils.h"

#include "../NetworkCommon/Daemon/Daemonizer.h"

#include "KhaanGateway.h"
#include "MainGatewayThread.h"
#include "FruitadensGateway.h"
#include "../NetworkCommon/NetworkOut/FruitadensServerToServer.h"

using namespace std;

#if PLATFORM == PLATFORM_WINDOWS
#pragma warning (disable:4996)
#include <conio.h>
#endif

////////////////////////////////////////////////////////////////////////

void  PrintInstructions()
{
   cout << "Gateway takes the following parameters using this format:" << endl;
   cout << "> gateway_server param1=localhost param1.extension='path' ... " << endl;
   cout << " NOTE: the gateway does not talk to the DB in any way" << endl;

   cout << endl << endl;
   cout << ": params are as follows:" << endl;
   cout << "    server.name       - allows a new name reported in logs and connections" << endl;
   cout << "    listen.address    - what is the ipaddress that this app should be using;" << endl;
   cout << "                        usually localhost or null" << endl;
   cout << "    listen.port       - listen on which port" << endl;
   cout << "    external.ip.address - the public facing ipaddress. Leave blank for" << endl;
   cout << "                          internal-only networks" << endl;

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

   cout << "    notification.address- notification server ipaddress" << endl;
   cout << "    notification.port - notification server port" << endl;

   cout << "    asset.only        - connect to only the asset server" << endl;
   cout << "    asset.block       - do not connect to the asset server" << endl;

   cout << "    print.functions   - print each function for debugging" << endl;
   cout << "    print.packets     - print each packet for debugging" << endl;

   cout << "    games = [192.168.1.0:21000:MFM,localhost:21100:game1]     - game list" << endl;

   cout << " -h, -help, -? for help " << endl;
}

////////////////////////////////////////////////////////////////////////

int main( int argc, const char* argv[] )
{
   daemonize( "gateway_serverd" );

   CommandLineParser    parser( argc, argv );

   string serverName = "Gateway server";

   string loadBalancerPortString = "9502";
   string loadBalancerAddressString = "localhost";
   string externalIpAddressString = "localhost";

   string listenPortString = "9500";
   string listenAddressString = "localhost";

   string chatPortString = "7400";
   string chatIpAddressString = "localhost";

   string loginPortString = "7600";
   string loginIpAddressString = "localhost";

   string assetDeliveryPortString = "7300";
   string assetDeliveryIpAddressString = "localhost";

   string contactPortString = "7500";
   string contactIpAddressString = "localhost";

   string purchasePortString = "7700";
   string purchaseIpAddressString = "localhost";

   //string rerouteAddressString = "";
   //string reroutePortString = "";

   string analyticsPortString = "7802";
   string analyticsIpAddressString = "localhost";

   string notificationPortString = "7900";
   string notificationIpAddressString = "localhost";

   string printFunctionsString = "false";
   string printPacketTypes = "false";

   string assetOnlyString = "false";
   string assetBlockString = "false";

   //--------------------------------------------------------------

   if( parser.IsRequestingInstructions() == true )
   {
      PrintInstructions();
      return 0;
   }

   parser.FindValue( "server.name", serverName );

   parser.FindValue( "balancer.port", loadBalancerPortString );
   parser.FindValue( "balancer.address", loadBalancerAddressString );

   parser.FindValue( "external.ip.address", externalIpAddressString );

   parser.FindValue( "listen.port", listenPortString );
   parser.FindValue( "listen.address", listenAddressString );

   parser.FindValue( "chat.port", chatPortString );
   parser.FindValue( "chat.address", chatIpAddressString );

   parser.FindValue( "login.port", loginPortString );
   parser.FindValue( "login.address", loginIpAddressString );

   parser.FindValue( "asset.port", assetDeliveryPortString );
   parser.FindValue( "asset.address", assetDeliveryIpAddressString );

   parser.FindValue( "contact.port", contactPortString );
   parser.FindValue( "contact.address", contactIpAddressString );

   parser.FindValue( "purchase.port", purchasePortString );
   parser.FindValue( "purchase.address", purchaseIpAddressString );

   parser.FindValue( "notification.port", notificationPortString );
   parser.FindValue( "notification.address", notificationIpAddressString );

   //parser.FindValue( "reroute.port", reroutePortString );
   //parser.FindValue( "reroute.address", rerouteAddressString );

   parser.FindValue( "analytics.port", analyticsPortString );
   parser.FindValue( "analytics.address", analyticsIpAddressString );

   parser.FindValue( "print.functions", printFunctionsString );
   parser.FindValue( "print.packets", printPacketTypes );

   parser.FindValue( "asset.only", assetOnlyString );
   parser.FindValue( "asset.block", assetBlockString );

   vector< string > params;
  /* if( parser.FindValue( "games", params ) )
   {
      cout << "No games were listed. No connections will be made with any games" << endl;
   }*/

   int   assetPort = 7300,  
         balancerPort = 9502,
         chatPort = 7400,
         contactPort = 7500, 
         loginPort = 7600,
         purchasePort = 7700,
         notificationPort = 7900,
         analyticsPort = 7802,
         listenPort = 9600;

   U16 reroutePort = 0;
   bool printPackets = false, 
         printFunctions = false,
         assetOnly = false, 
         assetBlock = false; 

   try 
   {
      assetPort =          boost::lexical_cast<int>( assetDeliveryPortString );
      balancerPort =       boost::lexical_cast<U16>( loadBalancerPortString );
      chatPort =           boost::lexical_cast<int>( chatPortString );
      contactPort =        boost::lexical_cast<int>( contactPortString );
      loginPort =          boost::lexical_cast<int>( loginPortString );
      analyticsPort =      boost::lexical_cast<int>( analyticsPortString );
      purchasePort =       boost::lexical_cast<int>( purchasePortString );
      notificationPort =   boost::lexical_cast<U16>( notificationPort );      
      listenPort =         boost::lexical_cast<int>( listenPortString );
      ///reroutePort =        boost::lexical_cast<U16>( reroutePortString );

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
   if( assetOnlyString == "1" || assetOnlyString == "true" || assetOnlyString == "TRUE" )
   {
      cout << " ----------------------------------------------- " << endl;
      cout << " Asset only server" << endl;
      
      assetOnly = true;
   }
   if( assetBlockString == "1" || assetBlockString == "true" || assetBlockString == "TRUE" )
   {
      cout << " ----------------------------------------------- " << endl;
      cout << " prevents talking to asset server" << endl;
      assetBlock = true;
   }

   if( assetOnly && assetBlock )
   {
      cout << " ----------------------------------------------- " << endl;
      cout << " CONFIGURATION PROBLEM: you cannot have both asset.only and asset.block flags set" << endl;
      return 1;
   }

   //--------------------------------------------------------------

   U64 serverUniqueHashValue = GenerateUniqueHash( serverName );
   U32 serverId = (U32)serverUniqueHashValue;

   cout << serverName << endl;
   cout << "Server stack version " << ServerStackVersion << endl;
   cout << "ServerId " << serverId << endl;
   cout << "External ip address: " << externalIpAddressString << endl;
   cout << "Network protocol version: " << (int)GlobalNetworkProtocolVersion << endl;
   cout << "------------------------------------------------------------------" << endl << endl << endl;

   InitializeSockets();
   bool isBusy = IsPortBusy( listenPort );
   ShutdownSockets();

   if( isBusy == false )
   {
      MainGatewayThread* gatewayServer = new MainGatewayThread( serverName, serverId );
      if( externalIpAddressString.size() )
      {
         gatewayServer->SetExternalIpAddress( externalIpAddressString );
      }
      gatewayServer->SetGatewayType( PacketServerIdentifier::GatewayType_Normal );
      if( assetOnly == true || assetBlock == false )
      {
         gatewayServer->SetGatewayType( PacketServerIdentifier::GatewayType_Asset );
      }
      gatewayServer->SetAsGame( false );
      gatewayServer->PrintPacketTypes( printPackets );
      gatewayServer->PrintFunctionNames( printFunctions );
      gatewayServer->SetupListening( listenPort );
      //gatewayServer->SetServerType( ServerType_Gateway );

      //ServerType serverType = gatewayServer->GetServerType();
      //U8 type = static_cast< U8 >( serverType );

      if( assetOnly == true )
      {
         gatewayServer->AllowUnauthenticatedConnections();
         cout << " Asset only server does not require authentication " << endl;
      }
      //gatewayServer->SetupReroute( rerouteAddressString, reroutePort );
      
      
      //--------------------------------------------------------------

      if( assetOnly == false )
      {
         ConnectToMultipleGames< FruitadensGateway, MainGatewayThread > ( parser, gatewayServer, true );

         PrepConnection< FruitadensGateway, MainGatewayThread > ( chatIpAddressString,          chatPort,         "chat",           gatewayServer, ServerType_Chat, true );
         PrepConnection< FruitadensGateway, MainGatewayThread > ( loginIpAddressString,         loginPort,        "logon",          gatewayServer, ServerType_Login, true );
         PrepConnection< FruitadensGateway, MainGatewayThread > ( contactIpAddressString,       contactPort,      "contact",        gatewayServer, ServerType_Contact, true );
         PrepConnection< FruitadensGateway, MainGatewayThread > ( purchaseIpAddressString,      purchasePort,     "purchase",       gatewayServer, ServerType_Purchase, true );
         PrepConnection< FruitadensGateway, MainGatewayThread > ( analyticsIpAddressString,     analyticsPort,         "analytics",      gatewayServer, ServerType_Analytics, true );
         PrepConnection< FruitadensGateway, MainGatewayThread > ( notificationIpAddressString,  notificationPort, "notification",   gatewayServer, ServerType_Notification, true );
      
      }
      if( assetBlock == false )
      {
         PrepConnection< FruitadensGateway, MainGatewayThread > ( assetDeliveryIpAddressString, assetPort,        "asset",          gatewayServer, ServerType_Asset, true );
      }

      cout << "---------------------------- finished connecting ----------------------------" << endl;

      FruitadensServerToServer* connection = PrepConnection< FruitadensServerToServer, MainGatewayThread > ( loadBalancerAddressString, balancerPort,    "balancer",       gatewayServer, ServerType_LoadBalancer, false );
      
      gatewayServer->Init();
      gatewayServer->Resume();   
      gatewayServer->Run();
   }
   else
   {
      cout << "***********************************************" << endl;
      cout << " error: that server port is busy " << endl;
      cout << "  port: " << listenPort << endl;
      cout << " Note: you may have an instance already running" << endl;
      cout << "        we must exit now" << endl;
      cout << "***********************************************" << endl;
      cout << endl << "Press any key to exit" << endl;
      getch();
   }
   
	return 0;
}


////////////////////////////////////////////////////////////////////////
