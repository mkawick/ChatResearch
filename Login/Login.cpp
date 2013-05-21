// Login.cpp
//

#include <iostream>
#include <list>
#include <vector>
#pragma warning (disable:4996)
using namespace std;

#include <conio.h>
#include <assert.h>

#include <boost/lexical_cast.hpp>

#include <cstdio>
#include <conio.h>
#include "../NetworkCommon/Utils/utils.h"
#include "../NetworkCommon/Utils/CommandLineParser.h"

#include "../NetworkCommon/Database/Deltadromeus.h"
#include "../NetworkCommon/Packets/ServerToServerPacket.h"

#include "KhaanLogin.h"
#include "DiplodocusLogin.h"
#include "FruitadensLogin.h"

// db.address=10.16.4.44 db.port=3306 db.username=admin db.password=Pz5328!@ db.schema=pleiades
////////////////////////////////////////////////////////////////////////

FruitadensLogin* PrepFruitadensLogin( const string& ipaddress, U16 port, U32 serverId, DiplodocusLogin* loginServer );

////////////////////////////////////////////////////////////////////////

int main( int argc, const char* argv[] )
{
   CommandLineParser    parser( argc, argv );

   string listenPortString = "3072";
   string listenAddressString = "localhost";

   string chatPortString = "9602";
   string chatIpAddressString = "localhost";

   string agricolaPortString = "23996";
   string agricolaIpAddressString = "localhost";

   //---------------------------------------

   parser.FindValue( "listen.port", listenPortString );
   parser.FindValue( "listen.address", listenAddressString );

   parser.FindValue( "chat.port", chatPortString );
   parser.FindValue( "chat.address", chatIpAddressString );

   parser.FindValue( "agricola.address", agricolaPortString );
   parser.FindValue( "agricola.port", agricolaIpAddressString );


   string dbPortString = "16384";
   string dbIpAddress = "localhost";
   string dbUsername = "root";
   string dbPassword = "password";
   string dbSchema = "pleiades";

   parser.FindValue( "db.address", dbIpAddress );
   parser.FindValue( "db.port", dbPortString );
   parser.FindValue( "db.username", dbUsername );
   parser.FindValue( "db.password", dbPassword );
   parser.FindValue( "db.schema", dbSchema );


   int listenPort = 3072, chatPort = 9602, dbPortAddress = 16384, agricolaPort = 23996;
   try 
   {
       listenPort = boost::lexical_cast<int>( listenPortString );
       chatPort = boost::lexical_cast<int>( chatPortString );
       agricolaPort = boost::lexical_cast<int>( agricolaPortString );
       dbPortAddress = boost::lexical_cast<int>( dbPortString );
   } 
   catch( boost::bad_lexical_cast const& ) 
   {
       std::cout << "Error: input string was not valid" << std::endl;
   }

   //--------------------------------------------------------------

   Deltadromeus* delta = new Deltadromeus;
   delta->SetConnectionInfo( dbIpAddress, dbPortAddress, dbUsername, dbPassword, dbSchema );
   if( delta->IsConnected() == false )
   {
      cout << "Error: Database connection is invalid." << endl;
      getch();
      return 1;
   }

   //----------------------------------------------------------------


   string serverName = "Login server";
   U64 serverUniqueHashValue = GenerateUniqueHash( serverName );
   U32 serverId = (U32)serverUniqueHashValue;

   string version = "0.04";
   cout << serverName << endl;
   cout << "Version " << version << endl;
   cout << "ServerId " << serverId << endl;
   cout << "------------------------------------------------------------------" << endl << endl << endl;

   DiplodocusLogin* loginServer = new DiplodocusLogin( serverName, serverId );
   
   FruitadensLogin chatOut( "login to chat" );
   chatOut.SetConnectedServerType( ServerType_Chat );
   chatOut.SetServerUniqueId( serverId );

  /* FruitadensLogin gameServerOut( "fruity to gameserver" );
   gameServerOut.SetConnectedServerType( ServerType_GameInstance );
   gameServerOut.SetServerUniqueId( serverId );*/

   loginServer->AddOutputChain( &chatOut );
   //loginServer->AddOutputChain( &gameServerOut );
   loginServer->AddOutputChain( delta );
   loginServer->SetupListening( listenPort );
   
   chatOut.Connect( chatIpAddressString.c_str(), chatPort );
   chatOut.Resume();

   // various games. We will need to deal with allowing a dynamic number of games in future
   FruitadensLogin* game1 = PrepFruitadensLogin( agricolaIpAddressString, agricolaPort, serverId, loginServer );
   FruitadensLogin* game2 = PrepFruitadensLogin( "localhost", 24600, serverId, loginServer );
   FruitadensLogin* game3 = PrepFruitadensLogin( "localhost", 24602, serverId, loginServer );

   game1->NotifyEndpointOfIdentification( serverName, serverId, false, false, true  );
   game2->NotifyEndpointOfIdentification( serverName, serverId, false, false, true  );
   game3->NotifyEndpointOfIdentification( serverName, serverId, false, false, true  );

   loginServer->Resume();
   loginServer->Run();

   getch();
   
	return 0;
}

////////////////////////////////////////////////////////////////////////

FruitadensLogin* PrepFruitadensLogin( const string& ipaddress, U16 port, U32 serverId, DiplodocusLogin* loginServer )
{
   FruitadensLogin* gameServerOut = new FruitadensLogin( "fruity to gameserver" );
   gameServerOut->SetConnectedServerType( ServerType_GameInstance );
   gameServerOut->SetServerUniqueId( serverId );

   loginServer->AddOutputChain( gameServerOut );

   gameServerOut->Connect( ipaddress.c_str(), port );
   gameServerOut->Resume();

   return gameServerOut;
}

////////////////////////////////////////////////////////////////////////
