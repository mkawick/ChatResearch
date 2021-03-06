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

#include "../NetworkCommon/Daemon/Daemonizer.h"

#include "KhaanGateway.h"
#include "DiplodocusGateway.h"
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

   cout << "    stat.address      - stat server ipaddress" << endl;
   cout << "    stat.port         - stat server port" << endl;

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

   string printPacketTypes = "false";

   string rerouteAddressString = "";
   string reroutePortString = "";

   string statPortString = "7802";
   string statIpAddressString = "localhost";
   

   //--------------------------------------------------------------

   if( parser.IsRequestingInstructions() == true )
   {
      PrintInstructions();
      return 0;
   }

   parser.FindValue( "server.name", serverName );

   parser.FindValue( "balancer.port", loadBalancerPortString );
   parser.FindValue( "balancer.address", loadBalancerAddressString );

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

   parser.FindValue( "print.packets", printPacketTypes );

   parser.FindValue( "reroute.port", reroutePortString );
   parser.FindValue( "reroute.address", rerouteAddressString );

   parser.FindValue( "stat.port", statPortString );
   parser.FindValue( "stat.address", statIpAddressString );

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
         statPort = 7802,
         listenPort = 9600;

   U16 reroutePort = 0;
   bool printPackets = false;
   try 
   {
      assetPort = boost::lexical_cast<int>( assetDeliveryPortString );
      balancerPort = boost::lexical_cast<U16>( loadBalancerPortString );
      chatPort = boost::lexical_cast<int>( chatPortString );
      contactPort = boost::lexical_cast<int>( contactPortString );

      loginPort = boost::lexical_cast<int>( loginPortString );

      statPort = boost::lexical_cast<int>( statPortString );

      purchasePort = boost::lexical_cast<int>( purchasePortString );
      reroutePort = boost::lexical_cast<U16>( reroutePortString );
      listenPort = boost::lexical_cast<int>( listenPortString );
   } 
   catch( boost::bad_lexical_cast const& ) 
   {
       std::cout << "Error: input string was not valid" << std::endl;
   }

   
   if( printPacketTypes == "1" || printPacketTypes == "true" || printPacketTypes == "TRUE" )
   {
      printPackets = true;
   }
   //--------------------------------------------------------------

   U64 serverUniqueHashValue = GenerateUniqueHash( serverName );
   U32 serverId = (U32)serverUniqueHashValue;

   cout << serverName << endl;
   cout << "Server stack version " << ServerStackVersion << endl;
   cout << "ServerId " << serverId << endl;
   cout << "------------------------------------------------------------------" << endl << endl << endl;

   DiplodocusGateway* gatewayServer = new DiplodocusGateway( serverName, serverId );
   gatewayServer->SetAsGateway( true );
   gatewayServer->SetAsGame( false );
   gatewayServer->PrintPacketTypes( printPackets );
   gatewayServer->SetupListening( listenPort );
   gatewayServer->SetupReroute( rerouteAddressString, reroutePort );
   
   
   //--------------------------------------------------------------

   ConnectToMultipleGames< FruitadensGateway, DiplodocusGateway > ( parser, gatewayServer, true );

   PrepConnection< FruitadensGateway, DiplodocusGateway > ( chatIpAddressString, chatPort, "chat", gatewayServer, ServerType_Chat, true );

   PrepConnection< FruitadensGateway, DiplodocusGateway > ( assetDeliveryIpAddressString, assetPort, "asset", gatewayServer, ServerType_Asset, true );

   PrepConnection< FruitadensGateway, DiplodocusGateway > ( loginIpAddressString, loginPort, "logon", gatewayServer, ServerType_Login, true );

   PrepConnection< FruitadensGateway, DiplodocusGateway > ( contactIpAddressString, contactPort, "contact", gatewayServer, ServerType_Contact, true );
   
   PrepConnection< FruitadensGateway, DiplodocusGateway > ( purchaseIpAddressString, purchasePort, "purchase", gatewayServer, ServerType_Purchase, true );

   PrepConnection< FruitadensGateway, DiplodocusGateway > ( statIpAddressString, statPort, "stat", gatewayServer, ServerType_Stat, true );
   
   cout << "---------------------------- finished connecting ----------------------------" << endl;

   PrepConnection< FruitadensServerToServer, DiplodocusGateway > ( loadBalancerAddressString, balancerPort, "balancer", gatewayServer, ServerType_LoadBalancer, false );

   gatewayServer->Init();
   gatewayServer->Resume();   
   gatewayServer->Run();

   getch();
   
	return 0;
}


////////////////////////////////////////////////////////////////////////
