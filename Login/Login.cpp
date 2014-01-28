// Login.cpp
//

#include <iostream>
#include <list>
#include <vector>

//#include <conio.h>
#include <assert.h>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include <cstdio>
#include <memory.h>

#include "../NetworkCommon/Version.h"
#include "../NetworkCommon/Utils/BoostExtensions.h"
#include "../NetworkCommon/Utils/Utils.h"

#include "../NetworkCommon/Utils/CommandLineParser.h"

#include "../NetworkCommon/Database/Deltadromeus.h"
#include "../NetworkCommon/Packets/ServerToServerPacket.h"

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

int main( int argc, const char* argv[] )
{
   daemonize( "login_serverd" );

   CommandLineParser    parser( argc, argv );

   string serverName = "Login server";
   string listenPortString = "7600";
   string listenAddressString = "localhost";

   string chatPortString = "7402";
   string chatIpAddressString = "localhost";

   string contactPortString = "7502";
   string contactIpAddressString = "localhost";

   string assetPortString = "7302";
   string assetIpAddressString = "localhost";

   string purchasePortString = "7702";
   string purchaseIpAddressString = "localhost";

   string statPortString = "7802";
   string statIpAddressString = "localhost";

   string gamePortString = "21002";
   string gameIpAddressString = "localhost";

   string autoAddLoginProductString = "true";

   //---------------------------------------

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

   parser.FindValue( "stat.port", statPortString );
   parser.FindValue( "stat.address", statIpAddressString );
   
  /* parser.FindValue( "game.port", gamePortString );
   parser.FindValue( "game.address", gameIpAddressString );*/

   parser.FindValue( "autoAddLoginProduct", autoAddLoginProductString );

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


   int listenPort = 7600, dbPortAddress = 3306, chatPort = 7402, contactPort=7502, assetPort=7302, purchasePort=7702, statPort = 7802 ;
   int gamePort = 23996;
   bool autoAddLoginProduct = true;

   try 
   {
       listenPort = boost::lexical_cast<int>( listenPortString );
       dbPortAddress = boost::lexical_cast<int>( dbPortString );

       chatPort = boost::lexical_cast<int>( chatPortString );
       contactPort = boost::lexical_cast<int>( contactPortString );
       assetPort = boost::lexical_cast<int>( assetPortString );

       statPort = boost::lexical_cast<int>( statPortString );

       gamePort = boost::lexical_cast<int>( gamePortString );

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

   

   //--------------------------------------------------------------

   Database::Deltadromeus* delta = new Database::Deltadromeus;
   delta->SetConnectionInfo( dbIpAddress, dbPortAddress, dbUsername, dbPassword, dbSchema );
   if( delta->IsConnected() == false )
   {
      cout << "Error: Database connection is invalid." << endl;
      getch();
      return 1;
   }

   //----------------------------------------------------------------

   U64 serverUniqueHashValue = GenerateUniqueHash( serverName );
   U32 serverId = (U32)serverUniqueHashValue;

   cout << serverName << endl;
   //cout << "Version " << ServerStackVersion << endl;
   cout << "Server stack version " << ServerStackVersion << endl;
   cout << "ServerId " << serverId << endl;
   cout << "Db " << dbIpAddress << ":" << dbPortAddress << endl;
   cout << "   flag: autoAddLoginProduct = " << std::boolalpha << autoAddLoginProduct << endl;
   cout << "------------------------------------------------------------------" << endl << endl << endl;

   DiplodocusLogin* loginServer = new DiplodocusLogin( serverName, serverId );
   loginServer->SetAsControllerApp( false );
   loginServer->SetAsGateway( false );
   loginServer->SetAsGame( false );

   loginServer->AddOutputChain( delta );
   loginServer->SetupListening( listenPort );
   loginServer->AutoAddTheProductFromWhichYouLogin( autoAddLoginProduct );
   
   //----------------------------------

   PrepConnection< FruitadensLogin, DiplodocusLogin > ( chatIpAddressString, chatPort, "chat", loginServer, ServerType_Chat, true );
   PrepConnection< FruitadensLogin, DiplodocusLogin > ( contactIpAddressString, contactPort, "contact", loginServer, ServerType_Contact, true );
   PrepConnection< FruitadensLogin, DiplodocusLogin > ( assetIpAddressString, assetPort, "asset", loginServer, ServerType_Asset, true );
   PrepConnection< FruitadensLogin, DiplodocusLogin > ( purchaseIpAddressString, purchasePort, "purchase", loginServer, ServerType_Purchase, true );
   PrepConnection< FruitadensLogin, DiplodocusLogin > ( statIpAddressString, statPort, "stat", loginServer, ServerType_Stat, true );
   
   ConnectToMultipleGames< FruitadensLogin, DiplodocusLogin > ( parser, loginServer, true );

   loginServer->Init();
   loginServer->Resume();
   loginServer->Run();

   getch();
   
	return 0;
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
