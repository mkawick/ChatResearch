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

#include <cstdio>
#include "../NetworkCommon/Version.h"
#include "../NetworkCommon/Utils/utils.h"
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

   string game1PortString = "21000";
   string game1Address = "localhost";
   string game2PortString = "21100";
   string game2Address = "localhost";

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
   

   //--------------------------------------------------------------

   parser.FindValue( "server.name", serverName );

   parser.FindValue( "balancer.port", loadBalancerPortString );
   parser.FindValue( "balancer.address", loadBalancerAddressString );

   parser.FindValue( "listen.port", listenPortString );
   parser.FindValue( "listen.address", listenAddressString );

   parser.FindValue( "chat.port", chatPortString );
   parser.FindValue( "chat.address", chatIpAddressString );

   parser.FindValue( "game.port", game1PortString );
   parser.FindValue( "game.address", game1Address );
   parser.FindValue( "game2.port", game2PortString );
   parser.FindValue( "game2.address", game2Address );

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

   int   listenPort = 9600, 
         chatPort = 7400, 
         game1Port = 21000, 
         game2Port = 21100, 
         loginPort = 7600, 
         assetPort = 7300, 
         contactPort = 7500, 
         purchasePort = 7700,
         balancerPort = 9502;

   U16 reroutePort = 0;
   bool printPackets = false;
   try 
   {
       listenPort = boost::lexical_cast<int>( listenPortString );
       chatPort = boost::lexical_cast<int>( chatPortString );
       loginPort = boost::lexical_cast<int>( loginPortString );
       game1Port = boost::lexical_cast<int>( game1PortString );
       game2Port = boost::lexical_cast<int>( game2PortString );
       assetPort = boost::lexical_cast<int>( assetDeliveryPortString );
       contactPort = boost::lexical_cast<int>( contactPortString );
       purchasePort = boost::lexical_cast<int>( purchasePortString );
       reroutePort = boost::lexical_cast<U16>( reroutePortString );
       balancerPort = boost::lexical_cast<U16>( loadBalancerPortString );
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
   cout << "Version " << version << endl;
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

   gateway->AddOutputChain( &chatOut );
   gateway->AddOutputChain( &gameServerOut );
   gateway->AddOutputChain( &loginServerOut );
   gateway->AddOutputChain( &assetServer );
   gateway->AddOutputChain( &contactServer );
   gateway->AddOutputChain( &purchaseServer );

   gateway->SetupListening( listenPort );
   gateway->SetupReroute( rerouteAddressString, reroutePort );
   
   //--------------------------------------------------------------

#ifndef _TRACK_MEMORY_LEAK_
   FruitadensGateway* game1 = PrepFruitadens( game1Address, game1Port, serverId, gateway, "game1" );
   FruitadensGateway* mfm = PrepFruitadens( "localhost", game2Port, serverId, gateway, "MFM" );
   //FruitadensGateway* game3 = PrepFruitadens( "localhost", 23402, serverId, gateway );
   //FruitadensGateway* game4 = PrepFruitadens( "localhost", 23550, serverId, gateway );// summoner wars

   U8 gameProductId = 0;
   game1->NotifyEndpointOfIdentification( serverName, gateway->GetIpAddress(), serverId, gateway->GetPort(), gameProductId, false, false, true, true  );
   mfm->NotifyEndpointOfIdentification( serverName, gateway->GetIpAddress(), serverId, gateway->GetPort(), gameProductId, false, false, true, true  );
#endif // _TRACK_MEMORY_LEAK_

   
   
   //game3->NotifyEndpointOfIdentification( serverName, serverId, gameProductId, false, false, true, true  );
   //game4->NotifyEndpointOfIdentification( serverName, serverId, gameProductId, false, false, true, true  );

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
   serverComm->NotifyEndpointOfIdentification( 
                                       gateway->GetServerName(), 
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
