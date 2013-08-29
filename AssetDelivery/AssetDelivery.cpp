// AssetDelivery.cpp : Defines the entry point for the console application.
//


#include <iostream>
#include <list>
#include <vector>
#pragma warning (disable:4996)
using namespace std;

#include <assert.h>

#include <boost/lexical_cast.hpp>

#include "DiplodocusAsset.h"
#include "../NetworkCommon/NetworkIn/DiplodocusServerToServer.h"
#include "../Networkcommon/Version.h"
#include "../NetworkCommon/Utils/CommandLineParser.h"
#include "../NetworkCommon/DataTypes.h"
#include "../NetworkCommon/Utils/Utils.h"


#if PLATFORM == PLATFORM_WINDOWS
#include <conio.h>
#endif   

int main( int argc, const char* argv[] )
{
	CommandLineParser    parser( argc, argv );

   string listenPort = "9700";
   string listenAddress = "localhost";

   string listenForS2SPort = "9702";
   string listenForS2SAddress = "localHost";

   string assetDictionary = "assets.ini";
   string assetPath = "C:/projects/Mber/ServerStack/X_testFiles_X";

   //---------------------------------------
   parser.FindValue( "listen.port", listenPort );
   parser.FindValue( "listen.address", listenAddress );

   parser.FindValue( "s2s.port", listenForS2SPort );
   parser.FindValue( "s2s.address", listenForS2SAddress );

   parser.FindValue( "asset.dictionary", assetDictionary );
   parser.FindValue( "asset.path", assetPath );

   int listenPortAddress = 9800, listenS2SPort = 9802;
   try 
   {
      listenS2SPort = boost::lexical_cast<int>( listenForS2SPort );
      listenPortAddress = boost::lexical_cast<int>( listenPort );
   } 
   catch( boost::bad_lexical_cast const& ) 
   {
       std::cout << "Error: input string was not valid" << std::endl;
   }


   //----------------------------------------------------------------
   
   string serverName = "Asset Server";
   U64 serverUniqueHashValue = GenerateUniqueHash( serverName );
   U32 serverId = (U32)serverUniqueHashValue;

   cout << serverName << ":" << endl;
   cout << "Version " << version << endl;
   cout << "ServerId " << serverId << endl;
   cout << "Asset file " << assetDictionary << endl;
   cout << "Asset path " << assetPath << endl;
   cout << "------------------------------------------------------------------" << endl << endl << endl;

   DiplodocusAsset*    assetServer = new DiplodocusAsset( serverName, serverId );
   assetServer->SetupListening( listenPortAddress );
   assetServer->SetIniFilePath( assetPath, assetDictionary );

   DiplodocusServerToServer* s2s = new DiplodocusServerToServer( serverName, serverId );
   s2s->SetupListening( listenS2SPort );
   
   //----------------------------------------------------------------
   
   assetServer->Init();

   s2s->AddOutputChain( assetServer );

   assetServer->Run();

   getch();
}

