// Gateway.cpp
//

#include <list>
#include <vector>
#include <iostream>

#if PLATFORM == PLATFORM_WINDOWS
#pragma warning (disable:4996)
#endif

#include <assert.h>

#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

#include <cstdio>
#include "../NetworkCommon/Version.h"
#include "../NetworkCommon/Utils/Utils.h"
#include "../NetworkCommon/Utils/CommandLineParser.h"
#include "../NetworkCommon/Packets/ServerToServerPacket.h"

#include "KhaanGateway.h"
#include "DiplodocusGateway.h"
#include "FruitadensGateway.h"
#include "../NetworkCommon/NetworkOut/FruitadensServerToServer.h"

using namespace std;

#if PLATFORM == PLATFORM_WINDOWS

#include <conio.h>
#endif

////////////////////////////////////////////////////////////////////////

FruitadensGateway* PrepGameConnection( const string& ipaddress, U16 port, U32 serverId, DiplodocusGateway* gatewayServer, const string& gameName, const string& serverName, const string& listenAddressString, U16 serverPort );
void  PrepS2SServer( const string& ipaddress, U16 port, U32 serverId, Diplodocus<Khaan>* gatewayServer, const string& serverName, const string& listenAddressString, U16 serverPort, ServerType serverType );
void  SetupLoadBalancerConnection( DiplodocusGateway* gateway, string address, U16 port );

////////////////////////////////////////////////////////////////////////

void  PrintInstructions()
{
   cout << "Gateway takes the following parameters using this format:" << endl;
   cout << "> gateway_server param1=localhost param1.extension='path' ... " << endl;
   cout << " NOTE: the gateway does not talk to the DB in any way" << endl;

   cout << endl << endl;
   cout << ": params are as follows:" << endl;
   cout << "    server.name       - allows a new name reported in logs and connections" << endl;
   cout << "    listen.address    - what is the ipaddress that this app should be using; usually localhost or null" << endl;
   cout << "    listen.address    - listen on which port" << endl;

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

   string statsPortString = "7802";
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

   parser.FindValue( "stat.port", statsPortString );
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
         statsPort = 7802,
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

      statsPort = boost::lexical_cast<int>( statsPortString );

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

#ifndef _TRACK_MEMORY_LEAK_

   U8    gameProductId = 0;
   cout << "games found = " << endl << "[ " << endl; 
   vector< string >::iterator it = params.begin();
   while( it != params.end() )
   {
      string str = *it++;
      vector< string > values;
      if( parser.SeparateStringIntoValues( str, values, 3 ) == true )
      {
         cout << boost::format("%15s ={ %6s - %-6s }")  % values[0] % values[1] % values[2] << endl;
         //192.168.1.0:21000:MFM
         string gameAddress = values[0];
         string gameName = values[2];
         int port = 0;
         bool success = false;
         try
         {
            port = boost::lexical_cast<int>( values[1] );
            success = true;
         } 
         catch( boost::bad_lexical_cast const& ) 
         {
             cout << "Error: input string was not valid" << endl;
         }
         if( success )
         {
            FruitadensGateway* game = PrepGameConnection( gameAddress, port, serverId, gatewayServer, gameName, serverName, gatewayServer->GetIpAddress(), gatewayServer->GetPort() );
         }
      }
      else
      {
         cout << "Not enough params for this game:" << str << endl;
      }
   }
   cout << "]" << endl;
#endif // _TRACK_MEMORY_LEAK_

   PrepConnection< FruitadensGateway, DiplodocusGateway > ( chatIpAddressString, chatPort, "chat", gatewayServer, ServerType_Chat, true );

   PrepConnection< FruitadensGateway, DiplodocusGateway > ( assetDeliveryIpAddressString, assetPort, "asset", gatewayServer, ServerType_Asset, true );

   PrepConnection< FruitadensGateway, DiplodocusGateway > ( loginIpAddressString, loginPort, "logon", gatewayServer, ServerType_Login, true );

   PrepConnection< FruitadensGateway, DiplodocusGateway > ( contactIpAddressString, contactPort, "contact", gatewayServer, ServerType_Contact, true );
   
   PrepConnection< FruitadensGateway, DiplodocusGateway > ( purchaseIpAddressString, purchasePort, "purchase", gatewayServer, ServerType_Purchase, true );


   cout << "Stat server: " << statIpAddressString << ":" << statsPort << endl;

   FruitadensServerToServer* ptr =  PrepS2SOutwardConnection( statIpAddressString, statsPort, serverId, serverName, ServerType_Stat, gatewayServer, gatewayServer->GetIpAddress(), gatewayServer->GetPort() );
   
   cout << "---------------------------- finished connecting ----------------------------" << endl;

   SetupLoadBalancerConnection( gatewayServer, loadBalancerAddressString, balancerPort );

   gatewayServer->Init();
   gatewayServer->Resume();   
   gatewayServer->Run();

   getch();
   
	return 0;
}


////////////////////////////////////////////////////////////////////////

void PrepS2SServer( const string& ipaddress, U16 port, U32 serverId, DiplodocusGateway* gatewayServer, const string& serverName, const string& listenAddressString, U16 serverPort, ServerType serverType )
{
   string connectionText = "fruity to ";
   if( serverName.size() )
   {
      connectionText += serverName;
   }
   else
   {
      connectionText += "stats";
   }
   FruitadensServerToServer* remoteServer = new FruitadensServerToServer( connectionText.c_str() );
   remoteServer->SetConnectedServerType( serverType );
   remoteServer->SetServerUniqueId( serverId );

   U32 gameProductId = 0;
   remoteServer->NotifyEndpointOfIdentification( serverName, listenAddressString, serverId, serverPort, gameProductId, false, false, true, true );
   cout << "Remote server: " << ipaddress << ":" << port << endl;
   remoteServer->Connect( ipaddress.c_str(), port );
   remoteServer->Resume();

   remoteServer->AddInputChain( gatewayServer );
   //gatewayServer->AddOutputChain( remoteServer );
}

////////////////////////////////////////////////////////////////////////

FruitadensGateway* PrepGameConnection( const string& ipaddress, U16 port, U32 serverId, DiplodocusGateway* gatewayServer, const string& gameName, const string& serverName, const string& listenAddressString, U16 serverPort )
{
   string gameServerText = "fruity to ";
   if( gameName.size() )
   {
      gameServerText += gameName;
   }
   else
   {
      gameServerText += "gameserver";
   }
   FruitadensGateway* gameServerOut = new FruitadensGateway( gameServerText.c_str() );
   gameServerOut->SetConnectedServerType( ServerType_GameInstance );
   gameServerOut->SetServerUniqueId( serverId );

   gatewayServer->AddOutputChain( gameServerOut );

   U32 gameProductId = 0;
   gameServerOut->NotifyEndpointOfIdentification( serverName, listenAddressString, serverId, serverPort, gameProductId, false, false, true, true );
   cout << "Game server: " << ipaddress << ":" << port << endl;
   gameServerOut->Connect( ipaddress.c_str(), port );
   gameServerOut->Resume();

   return gameServerOut;
}

////////////////////////////////////////////////////////////////////////

void  SetupLoadBalancerConnection( DiplodocusGateway* gateway, string address, U16 port )
{
   string nameOfServerConnection = gateway->GetServerName();
   nameOfServerConnection += " to LoadBalancer";

   FruitadensServerToServer* serverComm = new FruitadensServerToServer( nameOfServerConnection.c_str() );
   serverComm->SetConnectedServerType( ServerType_LoadBalancer );
   serverComm->SetServerId( gateway->GetServerId() );
   //serverComm->SetGameProductId( GetGameProductId() );

   serverComm->NotifyEndpointOfIdentification(gateway->GetServerName(), 
                                       gateway->GetIpAddress(), 
                                       gateway->GetServerId(), 
                                       gateway->GetPort(), 
                                       0, 
                                       gateway->IsGameServer(), 
                                       false, 
                                       false, 
                                       gateway->IsGateway() );

   serverComm->Connect( address.c_str(), port );
   serverComm->Resume();

   gateway->AddOutputChain( serverComm );
}

////////////////////////////////////////////////////////////////////////
