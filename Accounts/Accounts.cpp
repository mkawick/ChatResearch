// Accounts.cpp : Defines the entry point for the console application.
//

// Login.cpp
//

#include <iostream>
#include <list>
#include <vector>

//#include <conio.h>
#include <assert.h>

#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
using boost::format;

#include <cstdio>
#include <memory.h>

#include "../NetworkCommon/Version.h"
#include "../NetworkCommon/Utils/Utils.h"

#include "../NetworkCommon/Utils/CommandLineParser.h"

#include "../NetworkCommon/Database/Deltadromeus.h"
#include "../NetworkCommon/Packets/ServerToServerPacket.h"

#include "../NetworkCommon/Daemon/Daemonizer.h"
#include "../NetworkCommon/Logging/server_log.h"

#include "StatusUpdate.h"


#if PLATFORM == PLATFORM_WINDOWS
#include <conio.h>
#pragma warning (disable:4996)
#endif


using namespace std;

// db.address=10.16.4.44 db.port=3306 db.username=admin db.password=Pz5328!@ db.schema=pleiades
////////////////////////////////////////////////////////////////////////

//FruitadensLogin* PrepFruitadensLogin( const string& ipaddress, U16 port, U32 serverId, DiplodocusLogin* loginServer );

////////////////////////////////////////////////////////////////////////

int main( int argc, const char* argv[] )
{
   daemonize( "account_serverd" );

   U32 beginTime = GetCurrentMilliseconds();
   Sleep( 1000 );
   U32 endTime = GetCurrentMilliseconds();

   cout<< "Time for 1000 ms sleep was : " << endTime - beginTime << endl;

   CommandLineParser    parser( argc, argv );

   string serverName = "Account server";

   string chatPortString = "9602";
   string chatIpAddressString = "localhost";

   string agricolaPortString = "23996";
   string agricolaIpAddressString = "localhost";

   string enableUserProducts = "false";
   string userUuidFixOnly = "false";

   //---------------------------------------

   parser.FindValue( "server.name", serverName );

   parser.FindValue( "chat.port", chatPortString );
   parser.FindValue( "chat.address", chatIpAddressString );

   parser.FindValue( "agricola.address", agricolaPortString );
   parser.FindValue( "agricola.port", agricolaIpAddressString );

   parser.FindValue( "user.products_update", enableUserProducts );
   parser.FindValue( "user.uuid_fix_only", userUuidFixOnly );


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


   bool enableAddingUserProducts = false;
   bool onlyUpdatesUuid = false;
   int chatPort = 9602, dbPortAddress = 3306, agricolaPort = 23996;
   try 
   {
       //listenPort = boost::lexical_cast<int>( listenPortString );
       chatPort = boost::lexical_cast<int>( chatPortString );
       agricolaPort = boost::lexical_cast<int>( agricolaPortString );
       dbPortAddress = boost::lexical_cast<int>( dbPortString );

       if( enableUserProducts.size() )
       {
          enableAddingUserProducts = ( enableUserProducts == "true" || enableUserProducts == "1" );
       }
       if( userUuidFixOnly.size() )
       {
          onlyUpdatesUuid = ( userUuidFixOnly == "true" || userUuidFixOnly == "1" );
       }
   } 
   catch( boost::bad_lexical_cast const& ) 
   {
       std::cout << "Error: input string was not valid" << std::endl;
       LogMessage(LOG_PRIO_ERR, "Error: input string was not valid\n");
   }

   //--------------------------------------------------------------


   U64 serverUniqueHashValue = GenerateUniqueHash( serverName );
   U32 serverId = (U32)serverUniqueHashValue;

   cout << serverName << endl;
   cout << "Server stack version " << ServerStackVersion << endl;
   cout << "ServerId " << serverId << endl;
   cout << "Network protocol version: " << (int)NetworkVersionMajor << ":" << (int)NetworkVersionMinor << endl;
   
   cout << "------------------------------------------------------------------" << endl << endl << endl;
   LogMessage(LOG_PRIO_ERR, "serverName\n");
   LogMessage(LOG_PRIO_ERR, "Server stack version %s\n", ServerStackVersion );
   LogMessage(LOG_PRIO_ERR, "ServerId %d\n", serverId);
   LogMessage(LOG_PRIO_ERR, "------------------------------------------------------------------\n\n\n");

   StatusUpdate* server = new StatusUpdate( serverName, serverId );

   //----------------------------------------------------------------
   if( Database::ConnectToMultipleDatabases< StatusUpdate > ( parser, server ) == false )
   {
      Database::Deltadromeus* delta = new Database::Deltadromeus;
      delta->SetConnectionInfo( dbIpAddress, dbPortAddress, dbUsername, dbPassword, dbSchema );
      delta->SetConnectionType( Database::Deltadromeus::DbConnectionType_All );
      if( delta->IsConnected() == false )
      {
         cout << "Error: Database connection is invalid." << endl;
         getch();
         return 1;
      }
      server->AddOutputChain( delta );
   }

   //----------------------------------------------------------------

   server->EnableAddingUserProducts( enableAddingUserProducts );
   server->SetAsServicingUuidOnly( onlyUpdatesUuid );
   
   //server->Init();
   server->Resume();

   //getch();
   while( 1 ) // infinite loop
   {
      Sleep( 1000 );
   }
   
	return 0;
}


////////////////////////////////////////////////////////////////////////
