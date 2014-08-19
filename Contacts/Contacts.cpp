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
#include "../NetworkCommon/NetworkUtils.h"

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
      
   cout << "    analytics.address - analytics server ipaddress" << endl;
   cout << "    analytics.port    - analytics server port" << endl;
   
   cout << " - for simple single DB connection - " << endl;
   cout << "    db.address        - database ipaddress" << endl;
   cout << "    db.port           - database port" << endl;
   cout << "    db.username       - database username" << endl;
   cout << "    db.password       - database password" << endl;
   cout << "    db.schema         - database schema-table collection" << endl;

   cout << " - for multiple DB connections " << endl;
   cout << " A single db connection would look like this... note the 'all' designation" << endl;
   cout << " databaselist=[all:192.168.1.0:21000:root:password:playdek]" << endl;
   cout << " Multiple connections look like this" << endl;
   cout << " databaselist=[user:192.168.1.0:21000:root:password:playdek, game:192.168.1.0:21000:root:password:playdek ]" << endl;


   cout << " -h, -help, -? for help " << endl;
}

////////////////////////////////////////////////////////////////////////

int main( int argc, const char* argv[] )
{
   daemonize( "contact_serverd" );

	CommandLineParser    parser( argc, argv );
   string serverName = "Contact Server";
   
   string listenPortString = "7500";
   string listenAddress = "localhost";

   string listenForS2SPort = "7502";
   string listenForS2SAddress = "localHost";

   string dbPortString = "16384";
   string dbIpAddress = "localhost";
   string dbUsername = "root";
   string dbPassword = "password";
   string dbSchema = "playdek";

   string analyticsPortString = "7802";
   string analyticsIpAddressString = "localhost";

   //---------------------------------------
   
   parser.FindValue( "server.name", serverName );

   parser.FindValue( "listen.port", listenPortString );
   parser.FindValue( "listen.address", listenAddress );

   parser.FindValue( "s2s.port", listenForS2SPort );
   parser.FindValue( "s2s.address", listenForS2SAddress );

   parser.FindValue( "db.address", dbIpAddress );
   parser.FindValue( "db.port", dbPortString );
   parser.FindValue( "db.username", dbUsername );
   parser.FindValue( "db.password", dbPassword );
   parser.FindValue( "db.schema", dbSchema );

   parser.FindValue( "analytics.port", analyticsPortString );
   parser.FindValue( "analytics.address", analyticsIpAddressString );

   int   listenPort = 7500, 
         dbPortAddress = 3306, 
         analyticsPort = 7802, 
         listenS2SPort = 7502;
   try 
   {
      listenPort = boost::lexical_cast<int>( listenPortString );
      analyticsPort = boost::lexical_cast<int>( analyticsPortString );
      dbPortAddress = boost::lexical_cast<int>( dbPortString );
      listenS2SPort = boost::lexical_cast<int>( listenForS2SPort );
   } 
   catch( boost::bad_lexical_cast const& ) 
   {
       std::cout << "Error: input string was not valid" << std::endl;
   }

   //----------------------------------------------------------------
   
   U64 serverUniqueHashValue = GenerateUniqueHash( serverName );
   U32 serverId = (U32)serverUniqueHashValue;

   cout << serverName << ":" << endl;
   //cout << "Version " << version << endl;
   cout << "Server stack version " << ServerStackVersion << endl;
   cout << "ServerId " << serverId << endl;
   cout << "Db " << dbIpAddress << ":" << dbPortAddress << endl;
   cout << "Network protocol version: " << (int)NetworkVersionMajor << ":" << (int)NetworkVersionMinor << endl;
   
   cout << "------------------------------------------------------------------" << endl << endl << endl;
   //--------------------------------------------------------------

   InitializeSockets();
   bool isBusy = IsPortBusy( listenPort ) | IsPortBusy( listenS2SPort );
   ShutdownSockets();

   if( isBusy == false )
   {
      DiplodocusContact*    contactServer = new DiplodocusContact( serverName, serverId );

      contactServer->SetupListening( listenPort );

      DiplodocusServerToServer* s2s = new DiplodocusServerToServer( serverName, serverId, 0, ServerType_Contact );
      s2s->SetupListening( listenS2SPort );

      PrepConnection< FruitadensServer, DiplodocusContact > ( analyticsIpAddressString, analyticsPort, "analytics", contactServer, ServerType_Analytics, true );
      
      //----------------------------------------------------------------

      if( Database::ConnectToMultipleDatabases< DiplodocusContact > ( parser, contactServer ) == false )
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
         contactServer->AddOutputChain( delta );
      }

      //----------------------------------------------------------------

      s2s->AddOutputChain( contactServer );
      contactServer->Init();
      contactServer->Run();
   }
   else
   {
      cout << "***********************************************" << endl;
      cout << " error: that server port is busy " << endl;
      cout << "  port: " << listenPort << endl;
      cout << "  port: " << listenS2SPort << endl;
      cout << " Note: you may have an instance already running" << endl;
      cout << "        we must exit now" << endl;
      cout << "***********************************************" << endl;
      cout << endl << "Press any key to exit" << endl;
      getch();
   }

   return 0;
}

////////////////////////////////////////////////////////////////////////
