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

////////////////////////////////////////////////////////////////////////

int main( int argc, char* argv[] )
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

   string version = "0.04";
   cout << serverName << endl;
   cout << "Version " << version << endl;
   cout << "ServerId " << (U32)serverUniqueHashValue << endl;
   cout << "------------------------------------------------------------------" << endl << endl << endl;

   DiplodocusLogin* loginServer = new DiplodocusLogin();
   
   FruitadensLogin chatOut( "login to chat" );
   chatOut.SetConnectedServerType( ServerType_Chat );
   chatOut.SetServerUniqueId( (U32)serverUniqueHashValue );

   FruitadensLogin gameServerOut( "fruity to gameserver" );
   gameServerOut.SetConnectedServerType( ServerType_GameInstance );
   gameServerOut.SetServerUniqueId( (U32)serverUniqueHashValue );

   loginServer->AddOutputChain( &chatOut );
   loginServer->AddOutputChain( &gameServerOut );
   loginServer->AddOutputChain( delta );
   loginServer->SetupListening( listenPort );
   
   chatOut.Connect( chatIpAddressString.c_str(), chatPort );
   chatOut.Resume();
   gameServerOut.Connect( agricolaIpAddressString.c_str(), agricolaPort );
   gameServerOut.Resume();

   chatOut.NotifyEndpointOfIdentification( serverName, (U32)serverUniqueHashValue, false, false, true );
   gameServerOut.NotifyEndpointOfIdentification( serverName, (U32)serverUniqueHashValue, false, false, true  );
   
   loginServer->Resume();

   loginServer->Run();

   getch();
   
	return 0;
}
////////////////////////////////////////////////////////////////////////
