// Gateway.cpp
//

#include <list>
#include <vector>
#include <iostream>
#pragma warning (disable:4996)
using namespace std;

#include <conio.h>
#include <assert.h>

#include <boost/lexical_cast.hpp>

#include <cstdio>
#include <conio.h>
#include "../NetworkCommon/Utils/utils.h"
#include "../NetworkCommon/Utils/CommandLineParser.h"
#include "../NetworkCommon/Packets/ServerToServerPacket.h"

#include "KhaanConnector.h"
#include "DiplodocusGateway.h"
#include "FruitadensGateway.h"

////////////////////////////////////////////////////////////////////////

int main( int argc, char* argv[] )
{
   CommandLineParser    parser( argc, argv );

   string listenPortString = "9600";
   string listenAddressString = "localhost";

   string chatPortString = "9601";
   string chatIpAddressString = "localhost";

   string agricolaPortString = "23995";
   string agricolaIpAddressString = "localhost";

   string loginPortString = "3072";
   string loginIpAddressString = "localhost";


   parser.FindValue( "listen.port", listenPortString );
   parser.FindValue( "listen.address", listenAddressString );

   parser.FindValue( "chat.port", chatPortString );
   parser.FindValue( "chat.address", chatIpAddressString );

   parser.FindValue( "agricola.address", agricolaIpAddressString );
   parser.FindValue( "agricola.port", agricolaPortString );

   parser.FindValue( "login.port", loginPortString );
   parser.FindValue( "login.address", loginIpAddressString );

   int listenPort = 9600, chatPort = 9601, agricolaPort = 23995, loginPort = 3072;
   try 
   {
       listenPort = boost::lexical_cast<int>( listenPortString );
       chatPort = boost::lexical_cast<int>( chatPortString );
       agricolaPort = boost::lexical_cast<int>( agricolaPortString );
       loginPort = boost::lexical_cast<int>( loginPortString );
   } 
   catch( boost::bad_lexical_cast const& ) 
   {
       std::cout << "Error: input string was not valid" << std::endl;
   }

   //--------------------------------------------------------------

   string serverName = "Gateway server";
   U64 serverUniqueHashValue = GenerateUniqueHash( serverName );

   string version = "0.04";
   cout << serverName << endl;
   cout << "Version " << version << endl;
   cout << "ServerId " << (U32)serverUniqueHashValue << endl;
   cout << "------------------------------------------------------------------" << endl << endl << endl;

   DiplodocusGateway* gateway = new DiplodocusGateway();

   FruitadensGateway chatOut( "fruity to chat" );
   chatOut.SetConnectedServerType( ServerType_Chat );

   FruitadensGateway gameServerOut( "fruity to gameserver" );
   gameServerOut.SetConnectedServerType( ServerType_GameInstance );

   FruitadensGateway loginServerOut( "fruity to login" );
   loginServerOut.SetConnectedServerType( ServerType_Login );

   gateway->AddOutputChain( &chatOut );
   gateway->AddOutputChain( &gameServerOut );
   gateway->AddOutputChain( &loginServerOut );
   gateway->SetupListening( listenPort );
   
   //--------------------------------------------------------------

   chatOut.Connect( chatIpAddressString.c_str(), chatPort );
   chatOut.Resume();
   gameServerOut.Connect( agricolaIpAddressString.c_str(), agricolaPort );
   gameServerOut.Resume();
   loginServerOut.Connect( loginIpAddressString.c_str(), loginPort );
   loginServerOut.Resume();

   chatOut.NotifyEndpointOfIdentification( serverName, (U32)serverUniqueHashValue, false, false, true );
   gameServerOut.NotifyEndpointOfIdentification( serverName, (U32)serverUniqueHashValue, false, false, true );
   loginServerOut.NotifyEndpointOfIdentification( serverName, (U32)serverUniqueHashValue, false, false, true );
   
   gateway->Resume();
   gateway->Run();

   getch();
   
	return 0;
}
////////////////////////////////////////////////////////////////////////
