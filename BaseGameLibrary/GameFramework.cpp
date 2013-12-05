// GameFramework.cpp

#include <assert.h>
#include <iostream>


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
#include "../NetworkCommon/NetworkOut/FruitadensServerToServer.h"

using namespace std;

GameFramework* GameFramework::m_instance = NULL;

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
   m_listenForGatewayPort = 9600;

   m_chatServerPort = 7402;
   m_chatServerAddress = "localhost";

   m_listenForS2SPort = 21002;
   m_listenForS2SAddress = "localHost";

   m_dbPort = 16384;
   m_dbIpAddress = "localhost";

   m_dbUsername = "root";
   m_dbPassword = "password";
   m_dbSchema = "playdek";

   assert( m_instance == NULL );
   m_instance = this;
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
   m_listenForGatewayPort = port;
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

void  GameFramework::SetupConnectionToAnotherServer( const string& address, const string& serverName, U16 port, ServerType serverType, PacketType packetType  )
{
   S2SConnectionSetupData s2sConnection;
   s2sConnection.s2sCommunication =    NULL;
   s2sConnection.address =             address;
   s2sConnection.serverName =          serverName;
   s2sConnection.port =                port;
   s2sConnection.serverType =          serverType;
   s2sConnection.packetType.push_back( packetType );

   m_serverConnections.push_back( s2sConnection );
}

//-----------------------------------------------------

void  GameFramework::AddPacketTypeToServer( const string& serverName, PacketType packetType  )
{
   vector< S2SConnectionSetupData >::iterator it = m_serverConnections.begin();
   while( it != m_serverConnections.end() )
   {
      if( it->serverName == serverName )
      {
         it->packetType.push_back( packetType );
         return;
      }
      it++;
   }
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
      gatewayListenPort = boost::lexical_cast<string>( m_listenForGatewayPort );
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

   parser.FindValue( "listen.address", m_serverAddress );
   parser.FindValue( "listen.port", gatewayListenPort );

   parser.FindValue( "chat.port", chatPort );
   parser.FindValue( "chat.address", m_chatServerAddress );

   parser.FindValue( "s2s.port", listenForS2SPort );
   parser.FindValue( "s2s.address", m_listenForS2SAddress );

   parser.FindValue( "db.port", dbPort );
   parser.FindValue( "db.address", m_dbIpAddress );
   parser.FindValue( "db.username", m_dbUsername );
   parser.FindValue( "db.password", m_dbPassword );
   parser.FindValue( "db.schema", m_dbSchema );

   try 
   {
      m_listenForGatewayPort = boost::lexical_cast<int>( gatewayListenPort );
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
   const int MaxSize = PacketGameplayRawData::MaxBufferSize  - sizeof( PacketGatewayWrapper );

   return SendRawData< PacketGameplayRawData, DiplodocusGame > 
      ( data->m_data, data->m_sizeOfData, PacketGameplayRawData::Game, MaxSize, GetServerId(), GetGameProductId(), "raw", connectionId, m_connectionManager );
}

//-----------------------------------------------------

bool     GameFramework::SendErrorToClient( U32 connectionId, int errorCode, int errorSubCode )
{
   return SendPacketToGateway( new PacketErrorReport( errorCode, errorSubCode ), connectionId );
}

//-----------------------------------------------------

bool     GameFramework::SendChatData( BasePacket* packet )
{
   // this will be packed at a lower level
   //PacketServerToServerWrapper* wrapper = new PacketServerToServerWrapper;

   packet->gameProductId = GetGameProductId();
   packet->gameInstanceId = GetServerId();

   m_chatServer->AddOutputChainData( packet, 0 );

   return false;
}

//-----------------------------------------------------

bool     GameFramework::SendToAnotherServer( BasePacket* packet )  // this could notmally go through the Diplodocus Game. We are bypassing that and simplifying as a result.
{
   //int packetType = packet->packetType;

   vector< S2SConnectionSetupData >::iterator it = m_serverConnections.begin();
   while( it != m_serverConnections.end() )
   {
      if( it->s2sCommunication && 
         it->s2sCommunication->AddOutputChainData( packet, 0 ) == true )
      {
         return true;
      }
      it++;
   }
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

   return SendPacketToGateway( id, connectionId );
}

//-----------------------------------------------------

bool  GameFramework::SendPacketToGateway( BasePacket* packet, U32 connectionId )
{
   PacketFactory factory;
   PacketGatewayWrapper* wrapper = new PacketGatewayWrapper();
   wrapper->SetupPacket( packet, connectionId );

   if( m_connectionManager->AddOutputChainData( wrapper, connectionId ) == false )
   {
      //factory.CleanupPacket( static_cast< BasePacket >( wrapper ) );
      delete wrapper;
      delete packet;

      return false;
   }

   return false;
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
   cout << "Version: " << m_version << endl;
   cout << "ServerId: " << GetServerId() << endl;
   cout << "Product Id: " << (int) GetGameProductId() << endl;
   cout << "Db: " << m_dbIpAddress << ":" << m_dbPort << endl;
   cout << "------------------------------------------------------------------" << endl << endl << endl;

   //----------------------------------------------------------------
   m_connectionManager = new DiplodocusGame( GetServerName(), GetServerId(), GetGameProductId() );
   m_connectionManager->SetDatabaseIdentification( m_gameUuid );

   m_connectionManager->AddOutputChain( delta );

   m_connectionManager->SetupListening( m_listenForGatewayPort );
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
   m_chatServer->NotifyEndpointOfIdentification( GetServerName(), "localhost", GetServerId(), m_listenForGatewayPort, GetGameProductId(), true, false, false, false );
   m_chatServer->AddToOutwardFilters( PacketType_Chat );

   DiplodocusServerToServer* s2s = new DiplodocusServerToServer( GetServerName(), GetServerId(), GetGameProductId() );
   s2s->SetAsGame();
   s2s->SetupListening( m_listenForS2SPort );

   //----------------------------------------------------------------
   s2s->AddOutputChain( m_connectionManager );
   m_connectionManager->AddOutputChain( m_chatServer );

   SetupS2SConnections( m_serverAddress, 0 );

   m_connectionManager->Init();
   m_connectionManager->Run();


   return false;
}

//-----------------------------------------------------

void     GameFramework::SetupS2SConnections( const string& address, U16 port )
{
   vector< S2SConnectionSetupData >::iterator it = m_serverConnections.begin();
   while( it != m_serverConnections.end() )
   {
      S2SConnectionSetupData& setup = *it++;
      string nameOfServerConnection = GetServerName();
      nameOfServerConnection += " to ";
      nameOfServerConnection += setup.serverName;

      setup.s2sCommunication;
      FruitadensServerToServer* serverComm = new FruitadensServerToServer( nameOfServerConnection.c_str() );
      serverComm->SetConnectedServerType( setup.serverType );
      serverComm->SetServerId( GetServerId() );
      serverComm->SetGameProductId( GetGameProductId() );

      serverComm->Connect( setup.address.c_str(), setup.port );
      serverComm->Resume();
      serverComm->NotifyEndpointOfIdentification( GetServerName(), address, GetServerId(), port, GetGameProductId(), true, false, false, false );

      vector< PacketType >::iterator packetTypeIt = setup.packetType.begin();
      while( packetTypeIt != setup.packetType.end() )
      {
         serverComm->AddToOutwardFilters( *packetTypeIt++);
      }

      m_connectionManager->AddOutputChain( serverComm );

      setup.s2sCommunication = serverComm;
   }
}


//-----------------------------------------------------
//-----------------------------------------------------
