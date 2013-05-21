// GameFramework.cpp

#include "GameFramework.h"
#include <assert.h>

#include <boost/lexical_cast.hpp>
#include "../../NetworkCommon/Utils/CommandLineParser.h"
#include "../../NetworkCommon/Utils/Utils.h"

#include "../../NetworkCommon/Database/Deltadromeus.h"

#include "DiplodocusGame.h"
#include "FruitadensServerToServer.h"
#include "../../NetworkCommon/NetworkIn/DiplodocusServerToServer.h"
#include "../../NetworkCommon/Packets/GamePacket.h"

//-----------------------------------------------------
//-----------------------------------------------------

GameFramework::GameFramework( const char* name, const char* version ) :
   m_callbacksObject( NULL ),
   m_serverName( name ),
   m_version( version )
{
   U64 serverUniqueHashValue = GenerateUniqueHash( m_serverName );
   m_serverId = static_cast< U32 >( serverUniqueHashValue );

   // all of the standard connections
   m_gatewayListenPort = 23995;

   m_chatServerPort = 9602;
   m_chatServerAddress = "localhost";

   m_listenForS2SPort = 23996;
   m_listenForS2SAddress = "localHost";

   m_dbPort = 16384;
   m_dbIpAddress = "localhost";

   m_dbUsername = "root";
   m_dbPassword = "password";
   m_dbSchema = "pleiades";
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
   {
      PacketGameplayRawData* packet = new PacketGameplayRawData;
      packet->size = data->m_sizeOfData;
      packet->data = data->m_data;

      PacketGatewayWrapper* wrapper = new PacketGatewayWrapper;
      wrapper->pPacket = packet;
      wrapper->connectionId = connectionId;
      return m_connectionManager->AddOutputChainData( packet, connectionId );
   }

   return false;
}

//-----------------------------------------------------

bool  GameFramework::Run()
{
   Deltadromeus* delta = new Deltadromeus;
   delta->SetConnectionInfo( m_dbIpAddress, m_dbPort, m_dbUsername, m_dbPassword, m_dbSchema );
   if( delta->IsConnected() == false )
   {
      cout << "Error: Database connection is invalid." << endl;
      return false;
   }

   cout << m_serverName << ":" << endl;
   cout << "Version " << m_version << endl;
   cout << "ServerId " << m_serverId << endl;
   cout << "------------------------------------------------------------------" << endl << endl << endl;

   //----------------------------------------------------------------
   m_connectionManager = new DiplodocusGame( m_serverName, m_serverId );
   m_connectionManager->SetDatabaseIdentification( m_gameUuid );

   m_connectionManager->AddOutputChain( delta );

   m_connectionManager->SetupListening( m_gatewayListenPort );
   m_connectionManager->SetAsGame();
   m_connectionManager->RegisterCallbacks( m_callbacksObject );

   string nameOfChatServerConnection = m_serverName;
   nameOfChatServerConnection += " to chat";
   FruitadensServerToServer chatControl( nameOfChatServerConnection.c_str() );
   chatControl.SetConnectedServerType( ServerType_Chat );
   chatControl.SetServerId( m_serverId );   
   chatControl.Connect( m_chatServerAddress.c_str(), m_chatServerPort );
   chatControl.Resume();
   chatControl.NotifyEndpointOfIdentification( m_serverName, m_serverId, true, false, false );

   DiplodocusServerToServer* s2s = new DiplodocusServerToServer( m_serverName, m_serverId );
   s2s->SetAsGame();
   s2s->SetupListening( m_listenForS2SPort );

   //----------------------------------------------------------------
   s2s->AddOutputChain( m_connectionManager );
   m_connectionManager->AddOutputChain( &chatControl );

   m_connectionManager->Run();

   return false;
}
//-----------------------------------------------------
//-----------------------------------------------------
