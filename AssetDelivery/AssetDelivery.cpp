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

   //---------------------------------------

   parser.FindValue( "server.name", serverName );

   parser.FindValue( "listen.port", listenPortString );
   parser.FindValue( "listen.address", listenAddress );

   parser.FindValue( "s2s.port", listenForS2SPort );
   parser.FindValue( "s2s.address", listenForS2SAddress );

   parser.FindValue( "asset.dictionary", assetDictionary );
   parser.FindValue( "asset.path", assetPath );

   int listenPort = 7300, listenS2SPort = 7302;
   try 
   {
      listenS2SPort = boost::lexical_cast<int>( listenForS2SPort );
      listenPort = boost::lexical_cast<int>( listenPortString );
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
   cout << "Network protocol version: " << (int)NetworkVersionMajor << ":" << (int)NetworkVersionMinor << endl;
   
   cout << "Asset file " << assetDictionary << endl;
   cout << "Asset path " << assetPath << endl;
   cout << "------------------------------------------------------------------" << endl << endl << endl;

   InitializeSockets();
   bool isBusy = IsPortBusy( listenPort ) | IsPortBusy( listenS2SPort );
   ShutdownSockets();

   if( isBusy == false )
   {
      AssetMainThread*    assetServer = new AssetMainThread( serverName, serverId );
      assetServer->SetupListening( listenPort );
      assetServer->SetIniFilePath( assetPath, assetDictionary );

      DiplodocusServerToServer* s2s = new DiplodocusServerToServer( serverName, serverId, 0, ServerType_Asset );
      s2s->SetupListening( listenS2SPort );
      
      //----------------------------------------------------------------
      
      assetServer->Init();

      s2s->AddOutputChain( assetServer );

      assetServer->Run();
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
