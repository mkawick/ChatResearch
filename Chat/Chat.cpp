// Chat.cpp
//

#include <iostream>
#include <list>
#include <vector>

#include "../NetworkCommon/Platform.h"
#include "../NetworkCommon/Version.h"

#if PLATFORM == PLATFORM_WINDOWS
#pragma warning (disable:4996)
#endif

#include <assert.h>

#include <boost/lexical_cast.hpp>


#include "../NetworkCommon/NetworkIn/Diplodocus.h"
#include "../NetworkCommon/NetworkIn/DiplodocusServerToServer.h"

#include "DiplodocusChat.h"

#if PLATFORM == PLATFORM_WINDOWS
#include <conio.h>
#endif   

using namespace std;
//-----------------------------------------------------------------------------

int main( int argc, const char* argv[] )
{
   CommandLineParser    parser( argc, argv );

   string serverName = "Chat Server";
   string listenPort = "7400";
   string listenAddress = "localhost";

   string listenForS2SPort = "7402";
   string listenForS2SAddress = "localHost";

   //---------------------------------------

   parser.FindValue( "server.name", serverName );

   parser.FindValue( "listen.port", listenPort );
   parser.FindValue( "listen.address", listenAddress );

   parser.FindValue( "s2s.port", listenForS2SPort );
   parser.FindValue( "s2s.address", listenForS2SAddress );

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

   int listenPortAddress = 7400, dbPortAddress = 3306, listenS2SPort = 7402;
   try 
   {
      listenS2SPort = boost::lexical_cast<int>( listenForS2SPort );
      listenPortAddress = boost::lexical_cast<int>( listenPort );
      dbPortAddress = boost::lexical_cast<int>( dbPortString );
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

   //----------------------------------------------------------------
   
   U64 serverUniqueHashValue = GenerateUniqueHash( serverName );
   U32 serverId = (U32)serverUniqueHashValue;

   cout << serverName << ":" << endl;
   //cout << "Version " << ServerStackVersion << endl;
   cout << "Server stack version " << ServerStackVersion << endl;
   cout << "ServerId " << serverId << endl;
   cout << "Db " << dbIpAddress << ":" << dbPortAddress << endl;
   cout << "------------------------------------------------------------------" << endl << endl << endl;

   DiplodocusChat*    middleware = new DiplodocusChat( serverName, serverId );
   middleware->AddOutputChain( delta );

   middleware->SetupListening( listenPortAddress );

   DiplodocusServerToServer* s2s = new DiplodocusServerToServer( serverName, serverId, 0 );
   s2s->SetupListening( listenS2SPort );
   
   //----------------------------------------------------------------
   
   middleware->Init();

   s2s->AddOutputChain( middleware );

   middleware->Run();

   getch();
	return 0;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------