// Login.cpp
//

#include <iostream>
#include <list>
#include <vector>

//#include <conio.h>
#include <assert.h>

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

// db.address=10.16.4.44 db.port=3306 db.username=admin db.password=Pz5328!@ db.schema=pleiades
////////////////////////////////////////////////////////////////////////

FruitadensLogin* PrepFruitadensLogin( const string& ipaddress, U16 port, U32 serverId, DiplodocusLogin* loginServer, const string& name );
void  SendServerNotification( const string& serverName, const string& serverAddress, U32 serverId, U16 serverPort, FruitadensLogin* fruity )
{
   fruity->NotifyEndpointOfIdentification( serverName, serverAddress, serverId, serverPort, 0, false, false, true, false  );
}

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
   
   parser.FindValue( "game.port", gamePortString );
   parser.FindValue( "game.address", gameIpAddressString );

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


   int listenPort = 7600, dbPortAddress = 3306, chatPort = 7402, contactPort=7502, assetPort=7302, purchasePort=7702;
   int gamePort = 23996;
   bool autoAddLoginProduct = true;

   try 
   {
       listenPort = boost::lexical_cast<int>( listenPortString );
       dbPortAddress = boost::lexical_cast<int>( dbPortString );

       chatPort = boost::lexical_cast<int>( chatPortString );
       contactPort = boost::lexical_cast<int>( contactPortString );
       assetPort = boost::lexical_cast<int>( assetPortString );

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
   loginServer->AddOutputChain( delta );
   loginServer->SetupListening( listenPort );
   loginServer->AutoAddTheProductFromWhichYouLogin( autoAddLoginProduct );
   
   //----------------------------------

   FruitadensLogin chatOut( "login to chat" );
   chatOut.SetConnectedServerType( ServerType_Chat );
   chatOut.SetServerUniqueId( serverId );
   loginServer->AddOutputChain( &chatOut );
   chatOut.Connect( chatIpAddressString.c_str(), chatPort );
   chatOut.Resume();
   SendServerNotification( serverName, listenAddressString, serverId, listenPort, &chatOut );

   FruitadensLogin contactOut( "login to contact" );
   contactOut.SetConnectedServerType( ServerType_Contact );
   contactOut.SetServerUniqueId( serverId );
   loginServer->AddOutputChain( &contactOut );
   contactOut.Connect( contactIpAddressString.c_str(), contactPort );
   contactOut.Resume();
   SendServerNotification( serverName, listenAddressString, serverId, listenPort, &contactOut );

   FruitadensLogin assetOut( "login to asset" );
   assetOut.SetConnectedServerType( ServerType_Asset );
   assetOut.SetServerUniqueId( serverId );
   loginServer->AddOutputChain( &assetOut );
   assetOut.Connect( assetIpAddressString.c_str(), assetPort );
   assetOut.Resume();
   SendServerNotification( serverName, listenAddressString, serverId, listenPort, &assetOut );

   FruitadensLogin purchaseOut( "login to purchase" );
   purchaseOut.SetConnectedServerType( ServerType_Purchase );
   purchaseOut.SetServerUniqueId( serverId );
   loginServer->AddOutputChain( &purchaseOut );
   purchaseOut.Connect( purchaseIpAddressString.c_str(), purchasePort );
   purchaseOut.Resume();
   SendServerNotification( serverName, listenAddressString, serverId, listenPort, &purchaseOut );
   

   // various games. We will need to deal with allowing a dynamic number of games in future
   FruitadensLogin* game1 = PrepFruitadensLogin( gameIpAddressString, gamePort, serverId, loginServer, "Game1" );
   FruitadensLogin* game2 = PrepFruitadensLogin( "localhost", 23996, serverId, loginServer, "MFM" );
  /* FruitadensLogin* game3 = PrepFruitadensLogin( "localhost", 24602, serverId, loginServer );
   FruitadensLogin* game4 = PrepFruitadensLogin( "localhost", 24604, serverId, loginServer );*/

   SendServerNotification( serverName, listenAddressString, serverId, listenPort, game1 );// serverName, , serverId, , &chatOut
   SendServerNotification( serverName, listenAddressString, serverId, listenPort, game2 );
   /*SendServerNotification( serverName, serverId, game2 );
   SendServerNotification( serverName, serverId, game3 );
   SendServerNotification( serverName, serverId, game4 );*/

   loginServer->Init();
   loginServer->Resume();
   loginServer->Run();

   getch();
   
	return 0;
}

////////////////////////////////////////////////////////////////////////

FruitadensLogin* PrepFruitadensLogin( const string& ipaddress, U16 port, U32 serverId, DiplodocusLogin* loginServer, const string& name )
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
   FruitadensLogin* gameServerOut = new FruitadensLogin( servername.c_str() );
   gameServerOut->SetConnectedServerType( ServerType_GameInstance );
   gameServerOut->SetServerUniqueId( serverId );

   loginServer->AddOutputChain( gameServerOut );

   gameServerOut->Connect( ipaddress.c_str(), port );
   gameServerOut->Resume();

   return gameServerOut;
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
