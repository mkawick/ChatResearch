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

#include "KhaanConnector.h"
#include "DiplodocusGateway.h"
#include "FruitadensGateway.h"
#include "../NetworkCommon/NetworkOut/FruitadensServerToServer.h"

using namespace std;

#if PLATFORM == PLATFORM_WINDOWS

#include <conio.h>
#endif

////////////////////////////////////////////////////////////////////////

FruitadensGateway* PrepFruitadens( const string& ipaddress, U16 port, U32 serverId, DiplodocusGateway* loginServer, const string& name );
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

  /* string game1PortString = "21000";
   string game1Address = "localhost";
   string game2PortString = "21100";
   string game2Address = "localhost";*/

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

 /*  parser.FindValue( "game.port", game1PortString );
   parser.FindValue( "game.address", game1Address );
   parser.FindValue( "game2.port", game2PortString );
   parser.FindValue( "game2.address", game2Address );*/

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
   if( parser.FindValue( "games", params ) )
   {
      cout << "No games were listed. No connections will be made with any games" << endl;
   }

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

   DiplodocusGateway* gateway = new DiplodocusGateway( serverName, serverId );
   gateway->SetAsGateway( true );
   gateway->SetAsGame( false );
   gateway->PrintPacketTypes( printPackets );

   FruitadensGateway chatOut( "fruity to chat" );
   chatOut.SetConnectedServerType( ServerType_Chat );

   FruitadensGateway gameServerOut( "fruity to gameserver" );
   gameServerOut.SetConnectedServerType( ServerType_GameInstance );

   FruitadensGateway loginServerOut( "fruity to login" );
   loginServerOut.SetConnectedServerType( ServerType_Login );

   FruitadensGateway assetServer( "fruity to asset" );
   assetServer.SetConnectedServerType( ServerType_Asset );

   FruitadensGateway contactServer( "fruity to contact" );
   contactServer.SetConnectedServerType( ServerType_Contact );

   FruitadensGateway purchaseServer( "fruity to purchase" );
   purchaseServer.SetConnectedServerType( ServerType_Purchase );

   FruitadensGateway statOut( "fruity to stat" );
   statOut.SetConnectedServerType( ServerType_Stat );

   gateway->AddOutputChain( &chatOut );
   gateway->AddOutputChain( &gameServerOut );
   gateway->AddOutputChain( &loginServerOut );
   gateway->AddOutputChain( &assetServer );
   gateway->AddOutputChain( &contactServer );
   gateway->AddOutputChain( &purchaseServer );
   gateway->AddOutputChain( &statOut );

   gateway->SetupListening( listenPort );
   gateway->SetupReroute( rerouteAddressString, reroutePort );
   
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
            FruitadensGateway* game = PrepFruitadens( gameAddress, port, serverId, gateway, gameName );
            game->NotifyEndpointOfIdentification( serverName, gateway->GetIpAddress(), serverId, gateway->GetPort(), gameProductId, false, false, true, true  );
         }
      }
      else
      {
         cout << "Not enough params for this game:" << str << endl;
      }
   }
   cout << "]" << endl;
#endif // _TRACK_MEMORY_LEAK_

   cout << "Chat server: " << chatIpAddressString << ":" << chatPort << endl;
   chatOut.Connect( chatIpAddressString.c_str(), chatPort );
   chatOut.Resume();

   cout << "Login server: " << loginIpAddressString << ":" << loginPort << endl;
   loginServerOut.Connect( loginIpAddressString.c_str(), loginPort );
   loginServerOut.Resume();

   cout << "Asset server: " << assetDeliveryIpAddressString << ":" << assetPort << endl;
   assetServer.Connect( assetDeliveryIpAddressString.c_str(), assetPort );
   assetServer.Resume();

   cout << "Contact server: " << contactIpAddressString << ":" << contactPort << endl;
   contactServer.Connect( contactIpAddressString.c_str(), contactPort );
   contactServer.Resume();

   cout << "Purchase server: " << purchaseIpAddressString << ":" << purchasePort << endl;
   purchaseServer.Connect( purchaseIpAddressString.c_str(), purchasePort );
   purchaseServer.Resume();
   
   cout << "---------------------------- finished connecting ----------------------------" << endl;

   SetupLoadBalancerConnection( gateway, loadBalancerAddressString, balancerPort );

   gateway->Init();

   gateway->Resume();   
   gateway->Run();

   getch();
   
	return 0;
}
////////////////////////////////////////////////////////////////////////

FruitadensGateway* PrepFruitadens( const string& ipaddress, U16 port, U32 serverId, DiplodocusGateway* loginServer, const string& name )
{
   string servername = "fruity to ";
   if( name.size() )
   {
      servername += name;
   }
   else
   {
      servername += "gameserver";
   }
   FruitadensGateway* gameServerOut = new FruitadensGateway( servername.c_str() );
   gameServerOut->SetConnectedServerType( ServerType_GameInstance );
   gameServerOut->SetServerUniqueId( serverId );

   loginServer->AddOutputChain( gameServerOut );

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

   serverComm->Connect( address.c_str(), port );
   serverComm->Resume();
   serverComm->NotifyEndpointOfIdentification(gateway->GetServerName(), 
                                       gateway->GetIpAddress(), 
                                       gateway->GetServerId(), 
                                       gateway->GetPort(), 
                                       0, 
                                       gateway->IsGameServer(), 
                                       false, 
                                       false, 
                                       gateway->IsGateway() );

  /* vector< PacketType >::iterator packetTypeIt = setup.packetType.begin();
   while( packetTypeIt != setup.packetType.end() )
   {
      serverComm->AddToOutwardFilters( *packetTypeIt++);
   }*/

   gateway->AddOutputChain( serverComm );
}

////////////////////////////////////////////////////////////////////////
