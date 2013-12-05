// Contacts.cpp : 
//

#include <iostream>
#include <list>
#include <vector>
#pragma warning (disable:4996)
using namespace std;

#include <assert.h>

#include <boost/lexical_cast.hpp>

#include "DiplodocusContact.h"
#include "../NetworkCommon/NetworkIn/DiplodocusServerToServer.h"
#include "../Networkcommon/Version.h"
#include "../NetworkCommon/Utils/CommandLineParser.h"
#include "../NetworkCommon/DataTypes.h"
#include "../NetworkCommon/Utils/Utils.h"
#include "../NetworkCommon/Database/Deltadromeus.h"


#if PLATFORM == PLATFORM_WINDOWS
#include <conio.h>
#endif   

int main( int argc, const char* argv[] )
{
	CommandLineParser    parser( argc, argv );

   // listen.address=127.0.0.1 listen.port=9800 
   // db.address=10.16.4.44 db.port=3306 db.username=incinerator db.password=Cm8235 db.schema=playdek 
   // s2s.port=9802

   // listen.address=127.0.0.1 listen.port=9800 db.address=10.16.4.44 db.port=3306 db.username=admin db.password=Pz5328!@ db.schema=playdek s2s.port=9802
   string serverName = "Contact Server";
   
   string listenPort = "7500";
   string listenAddress = "localhost";

   string listenForS2SPort = "7502";
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

   int listenPortAddress = 7500, dbPortAddress = 3306, listenS2SPort = 7502;
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
   cout << "Version " << version << endl;
   cout << "ServerId " << serverId << endl;
   cout << "Db " << dbIpAddress << ":" << dbPortAddress << endl;
   cout << "------------------------------------------------------------------" << endl << endl << endl;

   DiplodocusContact*    contactServer = new DiplodocusContact( serverName, serverId );
   contactServer->AddOutputChain( delta );

   contactServer->SetupListening( listenPortAddress );

   DiplodocusServerToServer* s2s = new DiplodocusServerToServer( serverName, serverId );
   s2s->SetupListening( listenS2SPort );
   
   //----------------------------------------------------------------
   
   contactServer->Init();

   s2s->AddOutputChain( contactServer );

   contactServer->Run();

   getch();
}

