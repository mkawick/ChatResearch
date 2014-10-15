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
#include "../NetworkCommon/NetworkUtils.h"
#include "../NetworkCommon/Utils/StringUtils.h"

#include "KhaanConnector.h"
#include "DiplodocusLoadBalancer.h"
#include "../NetworkCommon/NetworkIn/DiplodocusServerToServer.h"
//#include "FruitadensGateway.h"

#include "../NetworkCommon/Logging/server_log.h"
using namespace std;

#if PLATFORM == PLATFORM_WINDOWS
#pragma warning (disable:4996)

#include <conio.h>
#endif
/*
#pragma pack(push, 1)
struct Test1 { U8 byte; long* ptr; };
#pragma pack(pop)

#pragma pack(push, 2)
struct Test2 { U8 byte; long* ptr; };
#pragma pack(pop)

#pragma pack(push, 4)
struct Test3 { U8 byte; long* ptr; };
#pragma pack(pop)

#pragma pack(push, 8)
struct Test4 { U8 byte; long* ptr; };
#pragma pack(pop)

struct Test5 { U8 byte; long* ptr; };*/

////////////////////////////////////////////////////////////////////////

//FruitadensGateway* PrepFruitadens( const string& ipaddress, U16 port, U32 serverId, DiplodocusGateway* loginServer, const string& name );

////////////////////////////////////////////////////////////////////////

int main( int argc, const char* argv[] )
{
 /*  cout << "BasePacket::GetSize() :" << BasePacket::GetSize() << endl;
   cout << "sizeof(BasePacket):" << sizeof(BasePacket) << endl;
   cout << "sizeof( long* ):" << sizeof( long* ) << endl;

   cout << "sizeof( Test1 ):" << sizeof( Test1 ) << endl;
   cout << "sizeof( Test2 ):" << sizeof( Test2 ) << endl;
   cout << "sizeof( Test3 ):" << sizeof( Test3 ) << endl;
   cout << "sizeof( Test4 ):" << sizeof( Test4 ) << endl;
   cout << "sizeof( Test5 ):" << sizeof( Test5 ) << endl;*/

   CommandLineParser    parser( argc, argv );

   string serverName = "LoadBalancer";
   string listenPortString = "9500";
   string listenAddressString = "localhost";
   string listenForS2SPort = "9502";
   string listenForS2SAddress = "localHost";

   string printFunctionsString = "false";

   //---------------------------------------
   
   parser.FindValue( "server.name", serverName );
   parser.FindValue( "listen.port", listenPortString );
   parser.FindValue( "listen.address", listenAddressString );
   parser.FindValue( "s2s.port", listenForS2SPort );
   parser.FindValue( "s2s.address", listenForS2SAddress );

   parser.FindValue( "print.functions", printFunctionsString );

   FileLogOpen( serverName.c_str() );

   vector< string > params;
   if( parser.FindValue( "gateways", params ) )
   {
      LogMessage( LOG_PRIO_INFO, "connections found = [ " ); 
      vector< string >::iterator it = params.begin();
      while( it != params.end() )
      {
         string str = *it++;
         vector< string > values;
         parser.SeparateStringIntoValues( str, values, 3 );
         LogMessage( LOG_PRIO_INFO, (boost::format("%15s ={ %15s - %-10s }")  % str % values[0] % values[1] ).str().c_str() );
      }
      LogMessage( LOG_PRIO_INFO, "]" );
   }

   //--------------------------------------------------------------

   int listenPort = 9500, listenS2SPort = 9502;
   bool  printFunctions = false;
   try 
   {
       listenPort = boost::lexical_cast<int>( listenPortString );
       listenS2SPort = boost::lexical_cast<int>( listenForS2SPort );
   } 
   catch( boost::bad_lexical_cast const& ) 
   {
       LogMessage( LOG_PRIO_ERR, "Error: input string was not valid" );
   }

   if( printFunctionsString == "1" || printFunctionsString == "true" || printFunctionsString == "TRUE" )
   {
      printFunctions = true;
   }
   //--------------------------------------------------------------

   U64 serverUniqueHashValue = GenerateUniqueHash( serverName );
   U32 serverId = (U32)serverUniqueHashValue;

   cout << serverName << endl;
   LogMessage( LOG_PRIO_INFO, ( serverName + ":" ).c_str() );
   LogMessage( LOG_PRIO_INFO, "Server stack version: %s", ServerStackVersion );
   LogMessage( LOG_PRIO_INFO, "ServerId: %u", serverId );
   LogMessage( LOG_PRIO_INFO, "Network protocol version: " , (int)NetworkVersionMajor , ":" , (int)NetworkVersionMinor );
   LogMessage( LOG_PRIO_INFO, "------------------------------------------------------------------" );

   //--------------------------------------------------------------

   InitializeSockets();
   bool isBusy = IsPortBusy( listenPort ) | IsPortBusy( listenS2SPort );
   ShutdownSockets();

   if( isBusy == false )
   {
      DiplodocusLoadBalancer* loadBalancer = new DiplodocusLoadBalancer( serverName, serverId );
      loadBalancer->SetGatewayType( PacketServerIdentifier::GatewayType_None );
      loadBalancer->SetAsGame( false );
      loadBalancer->PrintFunctionNames( printFunctions );


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
      
      
      LogMessage( LOG_PRIO_INFO, "---------------------------- finished connecting ----------------------------" );

      loadBalancer->SetupListening( listenPort );

      DiplodocusServerToServer* s2s = new DiplodocusServerToServer( serverName, serverId, 0, ServerType_LoadBalancer );
      s2s->SetupListening( listenS2SPort );
      s2s->AddOutputChain( loadBalancer );

      loadBalancer->Init();

      loadBalancer->Resume();   
      loadBalancer->Run();
   }
   else
   {
      LogMessage( LOG_PRIO_ERR, "***********************************************" );
      LogMessage( LOG_PRIO_ERR, " error: that server port is busy " );
      LogMessage( LOG_PRIO_ERR, "  port: %d", listenPort );
      LogMessage( LOG_PRIO_ERR, "  port: %d", listenS2SPort );
      LogMessage( LOG_PRIO_ERR, " Note: you may have an instance already running" );
      LogMessage( LOG_PRIO_ERR, "        we must exit now" );
      LogMessage( LOG_PRIO_ERR, "***********************************************" );
      LogMessage( LOG_PRIO_ERR, "\nPress any key to exit" );
      getch();
   }
   
	return 0;
}
////////////////////////////////////////////////////////////////////////

