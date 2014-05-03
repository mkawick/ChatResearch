// LoadBalancer.cpp : Defines the entry point for the console application.
//

#include <list>
#include <vector>
#include <iostream>

#include <assert.h>

#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
using boost::format;

#include <cstdio>
#include "../NetworkCommon/Version.h"
#include "../NetworkCommon/Utils/Utils.h"
#include "../NetworkCommon/Utils/CommandLineParser.h"
#include "../NetworkCommon/Packets/ServerToServerPacket.h"

#include "KhaanConnector.h"
#include "DiplodocusLoadBalancer.h"
#include "../NetworkCommon/NetworkIn/DiplodocusServerToServer.h"
//#include "FruitadensGateway.h"

using namespace std;

#if PLATFORM == PLATFORM_WINDOWS
#pragma warning (disable:4996)

#include <conio.h>
#endif

////////////////////////////////////////////////////////////////////////

//FruitadensGateway* PrepFruitadens( const string& ipaddress, U16 port, U32 serverId, DiplodocusGateway* loginServer, const string& name );

////////////////////////////////////////////////////////////////////////

int main( int argc, const char* argv[] )
{
   CommandLineParser    parser( argc, argv );

   string serverName = "LoadBalancer";
   string listenPortString = "9500";
   string listenAddressString = "localhost";
   string listenForS2SPort = "9502";
   string listenForS2SAddress = "localHost";

   //---------------------------------------
   
   parser.FindValue( "server.name", serverName );
   parser.FindValue( "listen.port", listenPortString );
   parser.FindValue( "listen.address", listenAddressString );
   parser.FindValue( "s2s.port", listenForS2SPort );
   parser.FindValue( "s2s.address", listenForS2SAddress );


   vector< string > params;
   if( parser.FindValue( "gateways", params ) )
   {
      cout << "connections found = " << endl << "[ " << endl; 
      vector< string >::iterator it = params.begin();
      while( it != params.end() )
      {
         string str = *it++;
         vector< string > values;
         parser.SeparateStringIntoValues( str, values, 3 );
         cout << boost::format("%15s ={ %15s - %-10s }")  % str % values[0] % values[1] << endl; // values[2]
      }
      cout << "]" << endl;
   }
  /* else
   {
      assert(0);
   }   */

   //--------------------------------------------------------------

   int listenPort = 9500, listenS2SPort = 9502;
   try 
   {
       listenPort = boost::lexical_cast<int>( listenPortString );
       listenS2SPort = boost::lexical_cast<int>( listenForS2SPort );
   } 
   catch( boost::bad_lexical_cast const& ) 
   {
       std::cout << "Error: input string was not valid" << std::endl;
   }


   //--------------------------------------------------------------

   U64 serverUniqueHashValue = GenerateUniqueHash( serverName );
   U32 serverId = (U32)serverUniqueHashValue;

   cout << serverName << endl;
   //cout << "Version " << version << endl;
   cout << "Server stack version " << ServerStackVersion << endl;
   cout << "ServerId " << serverId << endl;
   cout << "------------------------------------------------------------------" << endl << endl << endl;

   DiplodocusLoadBalancer* loadBalancer = new DiplodocusLoadBalancer( serverName, serverId );
   loadBalancer->SetGatewayType( PacketServerIdentifier::GatewayType_None );
   loadBalancer->SetAsGame( false );


   vector< string >::iterator it = params.begin();
   while( it != params.end() )
   {
      string str = *it++;
      string address, value;
      //parser.SeparateStringIntoKeyValue( str, address, value );
      vector< string > values;
      parser.SeparateStringIntoValues( str, values, 3 );

      address = values[0];
      string type = values[2];
      U16 port = boost::lexical_cast< U16 >( values[1] );
      loadBalancer->AddGatewayAddress( address, port );
   }
   
   
   cout << "---------------------------- finished connecting ----------------------------" << endl;

   loadBalancer->SetupListening( listenPort );

   DiplodocusServerToServer* s2s = new DiplodocusServerToServer( serverName, serverId, 0, ServerType_LoadBalancer );
   s2s->SetupListening( listenS2SPort );
   s2s->AddOutputChain( loadBalancer );

   loadBalancer->Init();

   loadBalancer->Resume();   
   loadBalancer->Run();

   getch();
   
	return 0;
}
////////////////////////////////////////////////////////////////////////

