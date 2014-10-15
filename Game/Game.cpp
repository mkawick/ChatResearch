// Game.cpp
//

#include <iostream>
#include <list>
#include <vector>

#if PLATFORM == PLATFORM_WINDOWS
#include <conio.h>
#pragma warning (disable:4996)
#endif


#include <conio.h>
#include <assert.h>

#include <boost/lexical_cast.hpp>

#include "DiplodocusGame.h"


#include "../NetworkCommon/Version.h"
#include "../NetworkCommon/Packets/ServerToServerPacket.h"
#include "../NetworkCommon/NetworkIn/DiplodocusServerToServer.h"

#include "../NetworkCommon/NetworkOut/FruitadensServerToServer.h"

#include "../NetworkCommon/Utils/StringUtils.h"

using namespace std;

//-----------------------------------------------------------------------------

int main(int argc, const char* argv[])
{
   CommandLineParser    parser( argc, argv );

   string serverName = "MFM Game Server";
   string listenPortString = "23995";
   string listenAddressString = "localhost";

   string chatServerPortForGames = "9602";
   string chatServerAddressForGames = "localhost";

   string listenForS2SPort = "23996";
   string listenForS2SAddress = "localHost";

   //---------------------------------------

   parser.FindValue( "server.name", serverName );

   parser.FindValue( "listen.port", listenPortString );
   parser.FindValue( "listen.address", listenAddressString );

   parser.FindValue( "s2s.port", listenForS2SPort );
   parser.FindValue( "s2s.address", listenForS2SAddress );

   string dbPort = "16384";
   string dbIpAddress = "localhost";
   string dbUsername = "root";
   string dbPassword = "password";
   string dbSchema = "playdek";

   parser.FindValue( "db.address", dbIpAddress );
   parser.FindValue( "db.port", dbPort );
   parser.FindValue( "db.username", dbUsername );
   parser.FindValue( "db.password", dbPassword );
   parser.FindValue( "db.schema", dbSchema );

   int listenPort = 23995, 
      dbPortAddress = 16384, 
      chatServerPort = 9602, 
      listenS2SPort = 23996;
   try 
   {
      listenS2SPort = boost::lexical_cast<int>( listenForS2SPort );
      listenPort = boost::lexical_cast<int>( listenPortString );
      dbPortAddress = boost::lexical_cast<int>( dbPort );
      chatServerPort = boost::lexical_cast<int>( chatServerPortForGames );
   } 
   catch( boost::bad_lexical_cast const& ) 
   {
      std::cout << "Error: input string was not valid" << std::endl;
   }

   Database::Deltadromeus* delta = new Database::Deltadromeus;
   delta->SetConnectionInfo( dbIpAddress, dbPortAddress, dbUsername, dbPassword, dbSchema );
   if( delta->IsConnected() == false )
   {
      cout << "Error: Database connection is invalid." << endl;
      getch();
      return 1;
   }

   //---------------------------------------------------------------------

   U64 serverUniqueHashValue = GenerateUniqueHash( serverName );
   U32 serverId = (U32)serverUniqueHashValue;

   LogMessage( LOG_PRIO_INFO, serverName.c_str() );
   LogMessage( LOG_PRIO_INFO, "Server stack version %s" , ServerStackVersion );
   LogMessage( LOG_PRIO_INFO, "ServerId %u" , serverId );
   LogMessage( LOG_PRIO_INFO, "Db %s : %d", dbIpAddress.c_str(), dbPortAddress );
   LogMessage( LOG_PRIO_INFO, "Network protocol version: " , (int)NetworkVersionMajor , ":" , (int)NetworkVersionMinor );
      
   LogMessage( LOG_PRIO_INFO, "------------------------------------------------------------------" );

   //----------------------------------------------------------------
   DiplodocusGame*    middleware = new DiplodocusGame( serverName, serverId, GameProductId_MONKEYS_FROM_MARS );
   middleware->AddOutputChain( delta );

   middleware->SetupListening( listenPort );
   middleware->SetAsGame();

   /*FruitadensServerToServer chatControl( "fruity to chat" );
   chatControl.SetConnectedServerType( ServerType_Chat );*/
   //chatControl.SetServerId( static_cast< U32>( serverUniqueHashValue ) );
   
  /* chatControl.Connect( chatServerAddressForGames.c_str(), chatServerPort );
   chatControl.Resume();
   chatControl.NotifyEndpointOfIdentification( serverName, listenAddressString, static_cast< U32 >( serverUniqueHashValue ), ServerType_GameInstance, listenPort, GameProductId_SUMMONWAR, true, false, true, false, "" );
*/
   FruitadensServerToServer* connection = PrepConnection< FruitadensServerToServer, DiplodocusGame > ( chatServerAddressForGames, chatServerPort,    "chat",       middleware, ServerType_Chat, false );
   connection = connection;// compiler warning

   DiplodocusServerToServer* s2s = new DiplodocusServerToServer( serverName, serverId, 0, ServerType_GameInstance );
   s2s->SetAsGame();
   s2s->SetupListening( listenS2SPort );

   //----------------------------------------------------------------
   s2s->AddOutputChain( middleware );
   //middleware->AddOutputChain( &chatControl );

   middleware->Init();
   middleware->Run();

   getch();

	return 0;
}

