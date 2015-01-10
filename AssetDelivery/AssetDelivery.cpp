// AssetDelivery.cpp : Defines the entry point for the console application.
//


#include <iostream>
#include <list>
#include <vector>

using namespace std;

#include <assert.h>

#include "AssetCommon.h"
#include "AssetMainThread.h"
#include "../NetworkCommon/NetworkIn/DiplodocusServerToServer.h"
#include "../NetworkCommon/NetworkUtils.h"
#include "../NetworkCommon/Utils/StringUtils.h"

#include "../NetworkCommon/Packets/LoginPacket.h"

#if PLATFORM == PLATFORM_WINDOWS
#include <conio.h>
#pragma warning (disable:4996)
#endif 



////////////////////////////////////////////////////////////////////////

void  PrintInstructions()
{
   cout << "asset takes params as follows:" << endl;
   cout << "> asset_server listen.port=7300 s2s.port=7302 game.port=21002" << endl;
   cout << "  asset.path='C:/projects/Mber/ServerStack/X_testFiles_X'" << endl;
   cout << "  asset.dictionary=\"assets_of_assets.ini\"" << endl;
   cout << " NOTE: asset is a server-to-games, who connect to it." << endl;

   cout << endl << endl;
   cout << ": params are as follows:" << endl;
   cout << "    server.name       - allows a new name reported in logs and connections" << endl;
   cout << "    listen.address    - what is the ipaddress that this app should be using;" << endl;
   cout << "                        usually localhost or null" << endl;
   cout << "    listen.port       - listen on which port to gateway connections" << endl;
   cout << "    s2s.address       - where is the load balancer" << endl;
   cout << "    s2s.port          - load balancer" << endl;

   cout << "    asset.dictionary  - name of the dictionary/project file" << endl;
   cout << "    asset.path        - base directory for asset server assets" << endl;
   cout << "    keepalive         - send keep alive packets to clients of this server" << endl;

   cout << "    asset server does not have db access" << endl;


   cout << " -h, -help, -? for help " << endl;
}

////////////////////////////////////////////////////////////////////////

int main( int argc, const char* argv[] )
{
   daemonize( "asset_serverd" );

	CommandLineParser    parser( argc, argv );

   string serverName = "Asset Server";
   string listenPortString = "7300";
   string listenAddress = "localhost";

   string listenForS2SPort = "7302";
   string listenForS2SAddress = "localHost";

   string assetDictionary = "assets_of_assets.ini";
   string assetPath = "C:/projects/Mber/ServerStack/X_testFiles_X";

#if PLATFORM == PLATFORM_WINDOWS // default
   string enableKeepAliveString = "false";
#else
   string enableKeepAliveString = "true";
#endif

   //---------------------------------------

   parser.FindValue( "server.name", serverName );

   parser.FindValue( "listen.port", listenPortString );
   parser.FindValue( "listen.address", listenAddress );

   parser.FindValue( "s2s.port", listenForS2SPort );
   parser.FindValue( "s2s.address", listenForS2SAddress );

   parser.FindValue( "asset.dictionary", assetDictionary );
   parser.FindValue( "asset.path", assetPath );
   parser.FindValue( "keepalive", enableKeepAliveString );

   FileLogOpen( serverName.c_str() );
   int listenPort = 7300, listenS2SPort = 7302;
   bool enableKeepAlive = ConvertToTrueFalse( enableKeepAliveString );
   try 
   {
      listenS2SPort = boost::lexical_cast<int>( listenForS2SPort );
      listenPort = boost::lexical_cast<int>( listenPortString );
   } 
   catch( boost::bad_lexical_cast const& ) 
   {
       LogMessage( LOG_PRIO_ERR, "Error: input string was not valid" );
   }

   //----------------------------------------------------------------
 
   U64 serverUniqueHashValue = GenerateUniqueHash( serverName );
   U32 serverId = (U32)serverUniqueHashValue;

   LogMessage( LOG_PRIO_INFO, serverName.c_str() );
   LogMessage( LOG_PRIO_INFO, "Server stack version %s" , ServerStackVersion );
   LogMessage( LOG_PRIO_INFO, "ServerId %u" , serverId );
   //LogMessage( LOG_PRIO_INFO, "Db %s : %d", dbIpAddress, dbPortAddress );
   LogMessage( LOG_PRIO_INFO, "Network protocol version: %d:%d" , (int)NetworkVersionMajor , ":" , (int)NetworkVersionMinor );
      
   LogMessage( LOG_PRIO_INFO, "------------------------------------------------------------------" );

   InitializeSockets();
   bool isBusy = IsPortBusy( listenPort ) | IsPortBusy( listenS2SPort );
   ShutdownSockets();

   if( isBusy == false )
   {
      AssetMainThread*    assetServer = new AssetMainThread( serverName, serverId );
      assetServer->SetupListening( listenPort );
      assetServer->SetIniFilePath( assetPath, assetDictionary );
      assetServer->RequireKeepAlive( enableKeepAlive );

      DiplodocusServerToServer* s2s = new DiplodocusServerToServer( serverName, serverId, 0, ServerType_Asset );
      s2s->SetupListening( listenS2SPort );
      
      //----------------------------------------------------------------
      
      assetServer->Init();

      s2s->AddOutputChain( assetServer );

      s2s->QueueInboundRequest( PacketType_Login, PacketLogin::LoginType_RequestLoginStatusOfAllUsers, ServerType_Login );
      
      assetServer->Run();
   }
   else
   {
      LogMessage( LOG_PRIO_ERR, "***********************************************" );
      LogMessage( LOG_PRIO_ERR, " error: that server port is busy " );
      LogMessage( LOG_PRIO_ERR, "  port: %d", listenPort );
      LogMessage( LOG_PRIO_ERR, " Note: you may have an instance already running" );
      LogMessage( LOG_PRIO_ERR, "        we must exit now" );
      LogMessage( LOG_PRIO_ERR, "***********************************************" );
      LogMessage( LOG_PRIO_ERR, "\nPress any key to exit" );
      getch();
   }

   return 0;
}

////////////////////////////////////////////////////////////////////////
