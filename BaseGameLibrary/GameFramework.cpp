// GameFramework.cpp

#include <assert.h>
#include <iostream>
using namespace std;

#include <boost/lexical_cast.hpp>
#include "../NetworkCommon/Version.h"
#include "../NetworkCommon/Utils/CommandLineParser.h"
#include "../NetworkCommon/Utils/Utils.h"

#include "../NetworkCommon/Packets/GamePacket.h"
#include "../NetworkCommon/Packets/ServerToServerPacket.h"
#include "../NetworkCommon/Packets/PacketFactory.h"

#include "DiplodocusGame.h"
#include "../NetworkCommon/NetworkIn/DiplodocusServerToServer.h"
#include "../NetworkCommon/Database/Deltadromeus.h"
#include "GameFramework.h"
#include "FruitadensServerToServer.h"


//-----------------------------------------------------
//-----------------------------------------------------

GameFramework::GameFramework( const char* name, const char* shortName, U8 gameProductId, const char* version ) :
   m_callbacksObject( NULL ),
   m_serverName( name ),
   m_shortName( shortName ),
   m_serverId( 0 ),
   m_gameProductId( gameProductId ),
   m_version( version )
{
   U64 serverUniqueHashValue = GenerateUniqueHash( GetServerName() );
   m_serverId = static_cast< U32 >( serverUniqueHashValue );

   // all of the standard connections
   m_gatewayListenPort = 9600;

   m_chatServerPort = 9602;
   m_chatServerAddress = "localhost";

   m_listenForS2SPort = 23996;
   m_listenForS2SAddress = "localHost";

   m_dbPort = 16384;
   m_dbIpAddress = "localhost";

   m_dbUsername = "root";
   m_dbPassword = "password";
   m_dbSchema = "playdek";
}

//-----------------------------------------------------

GameFramework::~GameFramework()
{
   // we should cleanup, but the running app currently never exits.
}

//-----------------------------------------------------

void  GameFramework::SetupDefaultDatabaseConnection( const string& serverAddress, U16 port, const string& username, const string& password, const string& dbSchemaName )
{
   m_dbIpAddress = serverAddress;
   m_dbPort = port;
   
   m_dbUsername = username;
   m_dbPassword = password;

   m_dbSchema = dbSchemaName;
}

//-----------------------------------------------------

void  GameFramework::SetupDefaultGateway( U16 port )
{
   m_gatewayListenPort = port;
}

//-----------------------------------------------------

void  GameFramework::SetupDefaultChatConnection( const string& address, U16 port )
{
   m_chatServerPort = port;
   m_chatServerAddress = address;
}

//-----------------------------------------------------

void  GameFramework::SetupDefaultS2S( const string& address, U16 port )
{
   m_listenForS2SPort = port;
   m_listenForS2SAddress = address;
}

//-----------------------------------------------------

void  GameFramework::UseCommandlineOverrides( int argc, const char* argv[] )
{
   string gatewayListenPort;
   string chatPort;
   string listenForS2SPort;
   string dbPort;


   try // these really can't throw, but to be consistent
   {
      gatewayListenPort = boost::lexical_cast<string>( m_gatewayListenPort );
      chatPort = boost::lexical_cast<string>( m_chatServerPort );
      listenForS2SPort = boost::lexical_cast<string>( m_listenForS2SPort );
      dbPort = boost::lexical_cast<string>( m_dbPort );
   }
   catch( ... )
   {
      cout << " major problem with command line params" << endl;
      assert( 0 );
   }

   //----------------------------------------

   CommandLineParser    parser( argc, argv );

   parser.FindValue( "listen.port", gatewayListenPort );

   parser.FindValue( "chat.port", chatPort );
   parser.FindValue( "chat.address", m_chatServerAddress );

   parser.FindValue( "s2s.port", listenForS2SPort );
   parser.FindValue( "s2s.address", m_chatServerAddress );

   parser.FindValue( "db.port", dbPort );
   parser.FindValue( "db.address", m_dbIpAddress );
   parser.FindValue( "db.username", m_dbUsername );
   parser.FindValue( "db.password", m_dbPassword );
   parser.FindValue( "db.schema", m_dbSchema );

   try 
   {
      m_gatewayListenPort = boost::lexical_cast<int>( gatewayListenPort );
      m_chatServerPort = boost::lexical_cast<int>( chatPort );
      m_listenForS2SPort = boost::lexical_cast<int>( listenForS2SPort );
      
      m_dbPort = boost::lexical_cast<int>( dbPort );
      
   } 
   catch( ... )
   {
      cout << " major problem with command line params" << endl;
      assert( 0 );
   }
}

//-----------------------------------------------------

bool  GameFramework::SendGameData( U32 connectionId, const MarshalledData* data )
{
   //if( m_connectionManager->IsConnectionValid( connectionId ) == true )
  /* int size = data->m_sizeOfData;
   bool didSend = true;
   const U8* ptr = data->m_data;
   int packetIndex = size / PacketGameplayRawData::MaxBufferSize + 1; // always start at 1.
   assert( packetIndex <= 254 );// must fit into 1 byte
   while( size > 0 )
   {
      int workingSize = size;
      if( workingSize > PacketGameplayRawData::MaxBufferSize )
      {
         workingSize = PacketGameplayRawData::MaxBufferSize;
      }
      
      PacketGameplayRawData* packet = new PacketGameplayRawData;
      packet->Prep( workingSize, ptr, packetIndex-- );
      packet->gameInstanceId = GetServerId();
      packet->gameProductId = GetGameProductId();

      size -= workingSize;
      ptr += workingSize;

      PacketGatewayWrapper* wrapper = new PacketGatewayWrapper;
      wrapper->pPacket = packet;
      wrapper->connectionId = connectionId;
      wrapper->gameInstanceId = GetServerId();
      if( m_connectionManager->AddOutputChainData( wrapper, connectionId ) == false )
      {
         delete wrapper;
         delete packet;
         return false;
         didSend =  false;
      }
   }
   if( didSend )
      return true;

   return false;*/

   const int MaxSize = PacketGameplayRawData::MaxBufferSize  - sizeof( PacketGatewayWrapper );

   return SendRawData< PacketGameplayRawData, DiplodocusGame > 
      ( data->m_data, data->m_sizeOfData, PacketGameplayRawData::Game, MaxSize, GetServerId(), GetGameProductId(), connectionId, m_connectionManager );
}

//-----------------------------------------------------

bool  GameFramework::SendChatData( BasePacket* packet )
{
   // this will be packed at a lower level
   /*PacketServerToServerWrapper* wrapper = new PacketServerToServerWrapper;
   wrapper->serverId = GetServerId();
   wrapper->gameProductId = GetGameProductId();
   wrapper->pPacket = packet;*/

   packet->gameProductId = GetGameProductId();
   packet->gameInstanceId = GetServerId();

   m_chatServer->AddOutputChainData( packet, 0 );

   return false;
}

//-----------------------------------------------------

bool  GameFramework::InformClientWhoThisServerIs( U32 connectionId )
{
   PacketGameIdentification*  id = new PacketGameIdentification;
   id->gameId = GetServerId();
   id->name = GetServerName();
   id->shortName = GetServerShortName();
   id->gameProductId = GetGameProductId();

   PacketGatewayWrapper* wrapper = new PacketGatewayWrapper;
   wrapper->pPacket = id;
   wrapper->connectionId = connectionId;

   // it is likely that we don not have any information about this client.. that is, we do not have the connection id
   // stored in the connectionManager here.
   if( m_connectionManager->AddOutputChainData( wrapper, connectionId ) == false )
   {
      delete wrapper;
      delete id;
      return false;
   }

   return true;
}

//-----------------------------------------------------

void     GameFramework::AddTimer( U32 timerId, U32 callbackTimeMs ) // timers must be unique
{
   map< U32, TimerInfo >:: iterator it = m_timers.find( timerId );
   if( it != m_timers.end() )
   {
      assert( 0 ); // duplicate timer ids are not allowed
   }

   U32 FifteenMinutes =15*60*1000;
   if( callbackTimeMs < 10 || callbackTimeMs > FifteenMinutes ) 
   {
      assert( 0 ); // timings outside of a good range.
   }

   TimerInfo timer;
   timer.scheduleTimeMs = callbackTimeMs;
   timer.timerId = timerId;
   timer.lastTimeMs = 0;
   m_timers.insert( pair< U32, TimerInfo >( timerId, timer ) );
}

//-----------------------------------------------------

bool  GameFramework::Run()
{
   cout << "Game framework version 0.8" << endl;

   Database::Deltadromeus* delta = new Database::Deltadromeus;
   delta->SetConnectionInfo( m_dbIpAddress, m_dbPort, m_dbUsername, m_dbPassword, m_dbSchema );
   if( delta->IsConnected() == false )
   {
      cout << "Error: Database connection is invalid." << endl;
      return false;
   }

   cout << GetServerName() << ":" << endl;
   cout << "Version " << m_version << endl;
   cout << "ServerId " << GetServerId() << endl;
   cout << "Product Id " << GetGameProductId() << endl;
   cout << "Db " << m_dbIpAddress << ":" << m_dbPort << endl;
   cout << "------------------------------------------------------------------" << endl << endl << endl;

   //----------------------------------------------------------------
   m_connectionManager = new DiplodocusGame( GetServerName(), GetServerId(), GetGameProductId() );
   m_connectionManager->SetDatabaseIdentification( m_gameUuid );

   m_connectionManager->AddOutputChain( delta );

   m_connectionManager->SetupListening( m_gatewayListenPort );
   m_connectionManager->SetAsGame();
   m_connectionManager->RegisterCallbacks( m_callbacksObject );

   // copy all timers
   map< U32, TimerInfo >::const_iterator it = m_timers.begin();
   while( it != m_timers.end() )
   {
      const pair< U32, TimerInfo >& timer = *it++;
      m_connectionManager->AddTimer( timer.second.timerId, timer.second.scheduleTimeMs );
   }

   m_timers.clear();

   string nameOfChatServerConnection = GetServerName();
   nameOfChatServerConnection += " to chat";
   m_chatServer = new FruitadensServerToServer( nameOfChatServerConnection.c_str() );
   m_chatServer->SetConnectedServerType( ServerType_Chat );
   m_chatServer->SetServerId( GetServerId() );
   m_chatServer->SetGameProductId( GetGameProductId() );

   m_chatServer->Connect( m_chatServerAddress.c_str(), m_chatServerPort );
   m_chatServer->Resume();
   m_chatServer->NotifyEndpointOfIdentification( GetServerName(), GetServerId(), GetGameProductId(), true, false, false, false );

   DiplodocusServerToServer* s2s = new DiplodocusServerToServer( GetServerName(), GetServerId(), GetGameProductId() );
   s2s->SetAsGame();
   s2s->SetupListening( m_listenForS2SPort );

   //----------------------------------------------------------------
   s2s->AddOutputChain( m_connectionManager );
   m_connectionManager->AddOutputChain( m_chatServer );

   m_connectionManager->Init();
   m_connectionManager->Run();


   return false;
}
//-----------------------------------------------------
//-----------------------------------------------------
