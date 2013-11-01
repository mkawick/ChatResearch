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

using namespace std;

#if PLATFORM == PLATFORM_WINDOWS

#include <conio.h>
#endif

////////////////////////////////////////////////////////////////////////

FruitadensGateway* PrepFruitadens( const string& ipaddress, U16 port, U32 serverId, DiplodocusGateway* loginServer );

////////////////////////////////////////////////////////////////////////

int main( int argc, const char* argv[] )
{
   CommandLineParser    parser( argc, argv );

   string listenPortString = "9600";
   string listenAddressString = "localhost";

   string chatPortString = "9601";
   string chatIpAddressString = "localhost";

   string game1PortString = "23550";
   string game1Address = "localhost";

   string loginPortString = "3072";
   string loginIpAddressString = "localhost";

   string assetDeliveryPortString = "9700";
   string assetDeliveryIpAddressString = "localhost";

   string contactPortString = "9800";
   string contactIpAddressString = "localhost";

   string purchasePortString = "9900";
   string purchaseIpAddressString = "localhost";

   //--------------------------------------------------------------

   parser.FindValue( "listen.port", listenPortString );
   parser.FindValue( "listen.address", listenAddressString );

   parser.FindValue( "chat.port", chatPortString );
   parser.FindValue( "chat.address", chatIpAddressString );

   parser.FindValue( "game.port", game1PortString );
   parser.FindValue( "game.address", game1Address );

   parser.FindValue( "login.port", loginPortString );
   parser.FindValue( "login.address", loginIpAddressString );

   parser.FindValue( "asset.port", assetDeliveryPortString );
   parser.FindValue( "asset.address", assetDeliveryIpAddressString );

   parser.FindValue( "contact.port", contactPortString );
   parser.FindValue( "contact.address", contactIpAddressString );

   parser.FindValue( "purchase.port", purchasePortString );
   parser.FindValue( "purchase.address", purchaseIpAddressString );

   int listenPort = 9600, chatPort = 9601, game1Port = 23550, loginPort = 3072, assetPort = 9700, contactPort = 9800, purchasePort = 9900;
   try 
   {
       listenPort = boost::lexical_cast<int>( listenPortString );
       chatPort = boost::lexical_cast<int>( chatPortString );
       loginPort = boost::lexical_cast<int>( loginPortString );
       game1Port = boost::lexical_cast<int>( game1PortString );
       assetPort = boost::lexical_cast<int>( assetDeliveryPortString );
       contactPort = boost::lexical_cast<int>( contactPortString );
       purchasePort = boost::lexical_cast<int>( purchasePortString );
   } 
   catch( boost::bad_lexical_cast const& ) 
   {
       std::cout << "Error: input string was not valid" << std::endl;
   }

   //--------------------------------------------------------------

   string serverName = "Gateway server";
   U64 serverUniqueHashValue = GenerateUniqueHash( serverName );
   U32 serverId = (U32)serverUniqueHashValue;

   cout << serverName << endl;
   cout << "Version " << version << endl;
   cout << "ServerId " << serverId << endl;
   cout << "------------------------------------------------------------------" << endl << endl << endl;

   DiplodocusGateway* gateway = new DiplodocusGateway( serverName, serverId );
   gateway->SetAsGateway( true );

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
   
   //--------------------------------------------------------------

#ifndef _TRACK_MEMORY_LEAK_
   FruitadensGateway* game1 = PrepFruitadens( game1Address, game1Port, serverId, gateway );
   FruitadensGateway* mfm = PrepFruitadens( "localhost", 23995, serverId, gateway );
   //FruitadensGateway* game3 = PrepFruitadens( "localhost", 23402, serverId, gateway );
   //FruitadensGateway* game4 = PrepFruitadens( "localhost", 23550, serverId, gateway );// summoner wars

   U8 gameProductId = 0;
   game1->NotifyEndpointOfIdentification( serverName, serverId, gameProductId, false, false, true, true  );
   mfm->NotifyEndpointOfIdentification( serverName, serverId, gameProductId, false, false, true, true  );
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

   gateway->Init();

   gateway->Resume();   
   gateway->Run();

   getch();
   
	return 0;
}
////////////////////////////////////////////////////////////////////////

FruitadensGateway* PrepFruitadens( const string& ipaddress, U16 port, U32 serverId, DiplodocusGateway* loginServer )
{
   FruitadensGateway* gameServerOut = new FruitadensGateway( "fruity to gameserver" );
   gameServerOut->SetConnectedServerType( ServerType_GameInstance );
   gameServerOut->SetServerUniqueId( serverId );

   loginServer->AddOutputChain( gameServerOut );

   cout << "Game server: " << ipaddress << ":" << port << endl;
   gameServerOut->Connect( ipaddress.c_str(), port );
   gameServerOut->Resume();

   return gameServerOut;
}

////////////////////////////////////////////////////////////////////////
