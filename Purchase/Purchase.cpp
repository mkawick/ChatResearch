// Purchase.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <list>
#include <vector>
#pragma warning (disable:4996)
using namespace std;

#include <assert.h>

#include <boost/lexical_cast.hpp>

#include "DiplodocusPurchase.h"
#include "../NetworkCommon/NetworkIn/DiplodocusServerToServer.h"
#include "../NetworkCommon/Database/Deltadromeus.h"
#include "../Networkcommon/Version.h"
#include "../NetworkCommon/Utils/CommandLineParser.h"
#include "../NetworkCommon/DataTypes.h"
#include "../NetworkCommon/Utils/Utils.h"

#include "../NetworkCommon/Daemon/Daemonizer.h"
#include "../NetworkCommon/Logging/server_log.h"


#if PLATFORM == PLATFORM_WINDOWS
#include <conio.h>
#endif   

int main( int argc, const char* argv[] )
{
	CommandLineParser    parser( argc, argv );

   string listenPort = "9900";
   string listenAddress = "localhost";

   string listenForS2SPort = "9902";
   string listenForS2SAddress = "localHost";

   //---------------------------------------
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


   int listenPortAddress = 9900, dbPortAddress = 3306, listenS2SPort = 9902;
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

   //--------------------------------------------------------------

   Database::Deltadromeus* delta = new Database::Deltadromeus;
   delta->SetConnectionInfo( dbIpAddress, dbPortAddress, dbUsername, dbPassword, dbSchema );
   if( delta->IsConnected() == false )
   {
      cout << "Error: Database connection is invalid." << endl;
      LogMessage(LOG_PRIO_ERR, "Error: Database connection is invalid.\n");
      getch();
      return 1;
   }

   //----------------------------------------------------------------
   
   string serverName = "Purchase Server";
   U64 serverUniqueHashValue = GenerateUniqueHash( serverName );
   U32 serverId = (U32)serverUniqueHashValue;

   cout << serverName << ":" << endl;
   cout << "Version " << version << endl;
   cout << "ServerId " << serverId << endl;
   cout << "------------------------------------------------------------------" << endl << endl << endl;

   DiplodocusPurchase*    purchaseServer = new DiplodocusPurchase( serverName, serverId );
   purchaseServer->SetupListening( listenPortAddress );

   DiplodocusServerToServer* s2s = new DiplodocusServerToServer( serverName, serverId );
   s2s->SetupListening( listenS2SPort );
   
   //----------------------------------------------------------------
   
   purchaseServer->Init();

   s2s->AddOutputChain( purchaseServer );

   purchaseServer->AddOutputChain( delta );

   purchaseServer->Run();

   getch();
}

