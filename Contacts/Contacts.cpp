// Contacts.cpp : 
//

#include <iostream>
#include <list>
#include <vector>

using namespace std;

#include <assert.h>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
using boost::format;


#include "../NetworkCommon/Version.h"
#include "../NetworkCommon/Utils/CommandLineParser.h"
#include "../NetworkCommon/DataTypes.h"
#include "../NetworkCommon/Utils/Utils.h"
#include "../NetworkCommon/Database/Deltadromeus.h"
#include "../NetworkCommon/Daemon/Daemonizer.h"
#include "../NetworkCommon/NetworkIn/DiplodocusServerToServer.h"

#if PLATFORM == PLATFORM_WINDOWS
#pragma warning (disable:4996)
#include <conio.h>
#endif   

#include "DiplodocusContact.h"

////////////////////////////////////////////////////////////////////////

void  PrintInstructions()
{
   cout << "contact takes params as follows:" << endl;
   cout << "> contact_server listen.port=7500 db.address=10.16.4.44 db.port=3306" << endl;
   cout << "   db.username=incinerator db.password=Cm8235 db.schema=playdek s2s.port=7502" << endl;
   cout << " NOTE: contact is a server-to-login, mainly." << endl;

   cout << endl << endl;
   cout << ": params are as follows:" << endl;
   cout << "    server.name       - allows a new name reported in logs and connections" << endl;
   cout << "    listen.address    - what is the ipaddress that this app should be using;" << endl;
   cout << "                        usually localhost or null" << endl;
   cout << "    listen.port       - listen on which port to gateway connections" << endl;
   cout << "    s2s.address       - where is the load balancer" << endl;
   cout << "    s2s.port          - load balancer" << endl;
   
   cout << "    db.address        - database ipaddress" << endl;
   cout << "    db.port           - database port" << endl;
   cout << "    db.username       - database username" << endl;
   cout << "    db.password       - database password" << endl;
   cout << "    db.schema         - database schema-table collection" << endl;

   cout << " -h, -help, -? for help " << endl;
}

////////////////////////////////////////////////////////////////////////

int main( int argc, const char* argv[] )
{
   daemonize( "contact_serverd" );

	CommandLineParser    parser( argc, argv );
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
   //cout << "Version " << version << endl;
   cout << "Server stack version " << ServerStackVersion << endl;
   cout << "ServerId " << serverId << endl;
   cout << "Db " << dbIpAddress << ":" << dbPortAddress << endl;
   cout << "------------------------------------------------------------------" << endl << endl << endl;

   DiplodocusContact*    contactServer = new DiplodocusContact( serverName, serverId );
   contactServer->AddOutputChain( delta );

   contactServer->SetupListening( listenPortAddress );

   DiplodocusServerToServer* s2s = new DiplodocusServerToServer( serverName, serverId, 0, ServerType_Contact );
   s2s->SetupListening( listenS2SPort );
   
   //----------------------------------------------------------------
   
   contactServer->Init();

   s2s->AddOutputChain( contactServer );

   contactServer->Run();

   getch();
}

////////////////////////////////////////////////////////////////////////
