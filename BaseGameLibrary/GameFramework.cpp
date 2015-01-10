// GameFramework.cpp

#include <assert.h>
#include <iostream>
#include <boost/format.hpp>


#include <boost/lexical_cast.hpp>
#include "../NetworkCommon/Version.h"
#include "../NetworkCommon/Utils/CommandLineParser.h"
#include "../NetworkCommon/Utils/Utils.h"
#include "../NetworkCommon/Utils/StringUtils.h"
#include "../NetworkCommon/Packets/GamePacket.h"
#include "../NetworkCommon/Packets/NotificationPacket.h"
#include "../NetworkCommon/Packets/UserStatsPacket.h"
#include "../NetworkCommon/Packets/ServerToServerPacket.h"
#include "../NetworkCommon/Packets/PacketFactory.h"

#include "../NetworkCommon/Packets/LoginPacket.h"
#include "DiplodocusGame.h"
#include "../NetworkCommon/NetworkIn/DiplodocusServerToServer.h"
#include "../NetworkCommon/Database/Deltadromeus.h"
#include "GameFramework.h"
#include "../NetworkCommon/NetworkOut/FruitadensServerToServer.h"

#include "../NetworkCommon/Logging/server_log.h"

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
   m_version( version ),
   m_enableKeepAlive( false )
{
   U64 serverUniqueHashValue = GenerateUniqueHash( GetServerName() );
   m_serverId = static_cast< U32 >( serverUniqueHashValue );

   // all of the standard connections
   m_listenPort = 9600;

   m_chatServerPort = 7402;
   m_chatServerAddress = "localhost";

   m_analyticsServerPort = 7802;
   m_analyticsServerAddress = "localhost";

   m_purchaseServerPort = 7702;
   m_purchaseServerAddress = "localhost";

   m_notificationServerPort = 7902;
   m_notificationServerAddress = "localhost";

   m_userStatsPort = 12002;
   m_userStatsIpAddress = "localhost";

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

void  GameFramework::SetupDefaultListeningPort( U16 port )
{
   m_listenPort = port;
}

//-----------------------------------------------------

void  GameFramework::SetupDefaultAnalyticsConnection( const string& address, U16 port )
{
   m_analyticsServerPort = port;
   m_analyticsServerAddress = address;
}

//-----------------------------------------------------

void  GameFramework::SetupDefaultChatConnection( const string& address, U16 port )
{
   m_chatServerPort = port;
   m_chatServerAddress = address;
}

//-----------------------------------------------------

void  GameFramework::SetupDefaultPurchaseConnection( const string& address, U16 port )
{
   m_purchaseServerPort = port;
   m_purchaseServerAddress = address;
}


//-----------------------------------------------------

void  GameFramework::SetupDefaultNotificationConnection( const string& address, U16 port )
{
   m_notificationServerPort = port;
   m_notificationServerAddress = address;
}

//-----------------------------------------------------

void  GameFramework::SetupDefaultUserStatConnection( const string& address, U16 port )
{
   m_userStatsPort = port;
   m_userStatsIpAddress = address;
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
   string   gatewayListenPort;
   string   analyticsPort;
   string   chatPort;
   string   notificationPort;
   string   userStatsPort;
   string   purchasePort;
   string   listenForS2SPort;
   string   dbPort;

#if PLATFORM == PLATFORM_WINDOWS // default
   string enableKeepAliveString = "false";
#else
   string enableKeepAliveString = "true";
#endif


   try // these really can't throw, but to be consistent
   {
      gatewayListenPort = boost::lexical_cast<string>( m_listenPort );
      analyticsPort = boost::lexical_cast<string>( m_analyticsServerPort );
      chatPort = boost::lexical_cast<string>( m_chatServerPort );
      notificationPort = boost::lexical_cast<string>( m_notificationServerPort );
      userStatsPort = boost::lexical_cast<string>( m_userStatsPort );
      purchasePort = boost::lexical_cast<string>( m_purchaseServerPort );
      listenForS2SPort = boost::lexical_cast<string>( m_listenForS2SPort );
      dbPort = boost::lexical_cast<string>( m_dbPort );
   }
   catch( ... )
   {
      cout << " major problem with command line params" << endl;
      assert( 0 );
   }

   //----------------------------------------

   m_parser = new CommandLineParser( argc, argv );

   m_parser->FindValue( "listen.port", gatewayListenPort );
   m_parser->FindValue( "listen.address", m_serverAddress );   

   m_parser->FindValue( "analytics.port", analyticsPort );
   m_parser->FindValue( "analytics.address", m_analyticsServerAddress );

   m_parser->FindValue( "chat.port", chatPort );
   m_parser->FindValue( "chat.address", m_chatServerAddress );

   m_parser->FindValue( "notification.port", notificationPort );
   m_parser->FindValue( "notification.address", m_notificationServerAddress );

   m_parser->FindValue( "notification.port", notificationPort );
   m_parser->FindValue( "notification.address", m_notificationServerAddress );

   m_parser->FindValue( "userstats.port", userStatsPort );
   m_parser->FindValue( "userstats.address", m_userStatsIpAddress );

   m_parser->FindValue( "purchase.port", purchasePort );
   m_parser->FindValue( "purchase.address", m_purchaseServerAddress );

   m_parser->FindValue( "s2s.port", listenForS2SPort );
   m_parser->FindValue( "s2s.address", m_listenForS2SAddress );   

   m_parser->FindValue( "db.port", dbPort );
   m_parser->FindValue( "db.address", m_dbIpAddress );
   m_parser->FindValue( "db.username", m_dbUsername );
   m_parser->FindValue( "db.password", m_dbPassword );
   m_parser->FindValue( "db.schema", m_dbSchema );
   m_parser->FindValue( "keepalive", enableKeepAliveString );

   try 
   {
      m_listenPort = boost::lexical_cast<int>( gatewayListenPort );
      m_analyticsServerPort = boost::lexical_cast<int>( analyticsPort );
      m_chatServerPort = boost::lexical_cast<int>( chatPort );
      m_notificationServerPort = boost::lexical_cast<int>( notificationPort );
      m_userStatsPort = boost::lexical_cast<int>( userStatsPort );
      m_purchaseServerPort = boost::lexical_cast<int>( purchasePort );
      m_listenForS2SPort = boost::lexical_cast<int>( listenForS2SPort );
      
      m_dbPort = boost::lexical_cast<int>( dbPort );

      m_enableKeepAlive = ConvertToTrueFalse( enableKeepAliveString );
      
   } 
   catch( ... )
   {
      cout << " major problem with command line params" << endl;
      assert( 0 );
   }
}

//-----------------------------------------------------

bool  GameFramework::SendGameData( U32 connectionId, U32 gatewayId, const MarshalledData* data )
{
   const int MaxSize = PacketGameplayRawData::MaxBufferSize  - sizeof( PacketGatewayWrapper );

   return SendRawData< PacketGameplayRawData, DiplodocusGame > 
      ( data->m_data, data->m_sizeOfData, PacketGameplayRawData::Game, MaxSize, GetServerId(), GetGameProductId(), "raw", connectionId, gatewayId, m_connectionManager );
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
   int packetType = packet->packetType;

   vector< S2SConnectionSetupData >::iterator it = m_serverConnections.begin();
   while( it != m_serverConnections.end() )
   {
      bool found = false;
      vector< PacketType >::iterator findTypeIt =  it->packetType.begin();
      while( findTypeIt !=  it->packetType.end() )
      {
         if( *findTypeIt++ == packetType )
         {
            found = true;
            break;
         }
      }
      FruitadensServerToServer*   fruity = it->s2sCommunication;
      if( found==true && fruity )
      {
         PacketServerToServerWrapper* wrapper = new PacketServerToServerWrapper;
         wrapper->gameInstanceId = GetServerId();
         wrapper->gameProductId = GetGameProductId();
         wrapper->serverId = m_serverId;
         wrapper->pPacket = packet;

         if( fruity->AddOutputChainData( wrapper, 0 ) == true )
         {
            return true;
         }
         else
         {
            PacketFactory factory;
            packet = wrapper;
            factory.CleanupPacket( packet );
         }
      }
      it++;
   }
   return false;
}

//-----------------------------------------------------
/*
bool  GameFramework::InformClientWhoThisServerIs( U32 connectionId )
{
   PacketGameIdentification*  id = new PacketGameIdentification;
   id->gameId = GetServerId();
   id->name = GetServerName();
   id->shortName = GetServerShortName();
   id->gameProductId = GetGameProductId();

   return SendPacketToGateway( id, connectionId );
}
*/
void  GameFramework::PackGameIdentificationPack( PacketGameIdentification*& packet )
{
   packet = NULL;
   PacketGameIdentification*  id = new PacketGameIdentification;
   id->gameId = GetServerId();
   id->name = GetServerName();
   id->shortName = GetServerShortName();
   id->gameProductId = GetGameProductId();

   packet = id;
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

void  GameFramework::SendStat( const string& statName, U16 integerIdentifier, float value, PacketAnalytics::StatType type )
{
   GetGame()->TrackStats( m_serverName, m_serverId, statName, integerIdentifier, value, type );
}

//-----------------------------------------------------

void  GameFramework::SendNotification( const string& userUuid, U32 userId, int notificationType, const string& additionalText  )
{
   if( m_notificationServer )
   {
      PacketNotification_SendNotification* packet = new PacketNotification_SendNotification;
      packet->userId = userId;
      packet->userUuid = userUuid;
      packet->notificationType = notificationType;
      packet->additionalText = additionalText;

      PacketServerToServerWrapper* wrapper = new PacketServerToServerWrapper;
      wrapper->gameInstanceId = GetServerId();
      wrapper->gameProductId = GetGameProductId();
      wrapper->serverId = GetServerId();
      wrapper->pPacket = packet;

      m_notificationServer->AddOutputChainData( wrapper, -1 );
   }
}


void GameFramework::SendPushNotification( U32 userId, int gameType, unsigned int gameId, int notificationType, const string& additionalText  )
{
   if( m_notificationServer )
   {
      PacketNotification_SendNotification* packet = new PacketNotification_SendNotification;
      packet->userUuid.clear();
      packet->userId = userId;
      packet->gameType = gameType;
      packet->gameId = gameId;
      packet->notificationType = notificationType;
      packet->additionalText = additionalText;

      PacketServerToServerWrapper* wrapper = new PacketServerToServerWrapper;
      wrapper->gameInstanceId = GetServerId();
      wrapper->gameProductId = GetGameProductId();
      wrapper->serverId = GetServerId();
      wrapper->pPacket = packet;

      m_notificationServer->AddOutputChainData( wrapper, -1 );
   }
}

void GameFramework::UpdatePushNotificationCount( U32 userId, int gameType, int notificationCount )
{
   if( m_notificationServer )
   {
      PacketNotification_UpdateNotificationCount* packet = new PacketNotification_UpdateNotificationCount;
      packet->userId = userId;
      packet->gameType = gameType;
      packet->notificationCount = notificationCount;

      PacketServerToServerWrapper* wrapper = new PacketServerToServerWrapper;
      wrapper->gameInstanceId = GetServerId();
      wrapper->gameProductId = GetGameProductId();
      wrapper->serverId = GetServerId();
      wrapper->pPacket = packet;

      m_notificationServer->AddOutputChainData( wrapper, -1 );
   }
}

void GameFramework::SendGameResultToStatServer( int gameType, U32 gameId, int playerCount,
                                 unsigned int *pResultOrder, unsigned int *pPlayerFactions, unsigned int forfeitFlags )
{
   if( playerCount < 2 || playerCount > PacketUserStats_ReportGameResult::k_maxPlayerCount )
   {
      return;
   }

   if( m_userStatsServer )
   {
      PacketUserStats_ReportGameResult* packet = new PacketUserStats_ReportGameResult;
      packet->gameType = gameType;
      packet->gameId = gameId;
      packet->playerCount = playerCount;
      memcpy( packet->resultOrder, pResultOrder, playerCount * sizeof(unsigned int) );
      memcpy( packet->playerFactions, pPlayerFactions, playerCount * sizeof(unsigned int) );
      packet->forfeitFlags = forfeitFlags;

      PacketServerToServerWrapper* wrapper = new PacketServerToServerWrapper;
      wrapper->gameInstanceId = m_serverId;
      wrapper->gameProductId = m_gameProductId;
      wrapper->serverId = m_serverId;
      wrapper->pPacket = packet;

      m_userStatsServer->AddOutputChainData( wrapper, -1 );
   }
}

void GameFramework::SendUserForfeitToStatServer( int gameType, U32 userId, U32 gameId )
{
   if( m_userStatsServer )
   {
      PacketUserStats_ReportUserForfeit* packet = new PacketUserStats_ReportUserForfeit;
      packet->gameType = gameType;
      packet->userId = userId;
      packet->gameId = gameId;

      PacketServerToServerWrapper* wrapper = new PacketServerToServerWrapper;
      wrapper->gameInstanceId = m_serverId;
      wrapper->gameProductId = m_gameProductId;
      wrapper->serverId = m_serverId;
      wrapper->pPacket = packet;

      m_userStatsServer->AddOutputChainData( wrapper, -1 );
   }
}


void     GameFramework::LockGameMutex()
{
   m_connectionManager->LockMutex();
}

void     GameFramework::UnlockGameMutex()
{
   m_connectionManager->UnlockMutex();
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
   FileLogOpen( GetServerName().c_str() );

   LogMessage( LOG_PRIO_INFO, "Game framework version 1.01" );

  /* Database::Deltadromeus* delta = new Database::Deltadromeus;
   delta->SetConnectionInfo( m_dbIpAddress, m_dbPort, m_dbUsername, m_dbPassword, m_dbSchema );
   if( delta->IsConnected() == false )
   {
      LogMessage( 1, "Error: Database connection is invalid." );
      return false;
   }*/

   LogMessage( LOG_PRIO_INFO, ( GetServerName() + ":" ).c_str() );
   LogMessage( LOG_PRIO_INFO, "Server stack version: %s", ServerStackVersion );
   LogMessage( LOG_PRIO_INFO, "Version: %s", m_version.c_str() );
   LogMessage( LOG_PRIO_INFO, "ServerId: %d", GetServerId() );
   LogMessage( LOG_PRIO_INFO, "Product Id: %d", (int) GetGameProductId() );
   LogMessage( LOG_PRIO_INFO, "Db: %s:%d", m_dbIpAddress.c_str(), m_dbPort );
   LogMessage( LOG_PRIO_INFO, "Network protocol version: %d:%d", (int)NetworkVersionMajor, (int)NetworkVersionMinor );
   
   LogMessage( LOG_PRIO_INFO, "------------------------------------------------------------------\n\n\n" );

   

   //----------------------------------------------------------------
   m_connectionManager = new DiplodocusGame( GetServerName(), GetServerId(), GetGameProductId() );
   m_connectionManager->SetDatabaseIdentification( m_gameUuid );
   m_connectionManager->SetGatewayType( PacketServerIdentifier::GatewayType_None );
   m_connectionManager->SetAsControllerApp( false );
   m_connectionManager->SetAsGame();
   m_connectionManager->RequireKeepAlive( m_enableKeepAlive );

   //----------------------------------------------------------------
   if( Database::ConnectToMultipleDatabases< DiplodocusGame > ( *m_parser, m_connectionManager ) == false )
   {
      Database::Deltadromeus* delta = new Database::Deltadromeus;
      delta->SetConnectionInfo( m_dbIpAddress, m_dbPort, m_dbUsername, m_dbPassword, m_dbSchema );
      delta->SetConnectionType( Database::Deltadromeus::DbConnectionType_All );
      if( delta->IsConnected() == false )
      {
         LogMessage( LOG_PRIO_ERR, "Error: Database connection is invalid." );
         return 1;
      }
      m_connectionManager->AddOutputChain( delta );
   }

   delete m_parser;
   //----------------------------------------------------------------
   
   m_connectionManager->SetupListening( m_listenPort );   
   m_connectionManager->RegisterCallbacks( m_callbacksObject );

   // copy all timers
   map< U32, TimerInfo >::const_iterator it = m_timers.begin();
   while( it != m_timers.end() )
   {
      const pair< U32, TimerInfo >& timer = *it++;
      m_connectionManager->AddTimer( timer.second.timerId, timer.second.scheduleTimeMs );
   }

   m_timers.clear();

   DiplodocusServerToServer* s2s = new DiplodocusServerToServer( GetServerName(), GetServerId(), GetGameProductId(), ServerType_GameInstance );
   s2s->SetAsGame();
   s2s->SetupListening( m_listenForS2SPort );
   s2s->AddOutputChain( m_connectionManager );

   //m_serverConnections.push_back( s2sConnection );
   PrepConnection< FruitadensServerToServer, DiplodocusGame > ( m_analyticsServerAddress, m_analyticsServerPort, "analytics", m_connectionManager, ServerType_Analytics, true, GetGameProductId() );

   FruitadensServerToServer* purchase = PrepConnection< FruitadensServerToServer, DiplodocusGame > ( m_purchaseServerAddress, m_purchaseServerPort, "purchase", m_connectionManager, ServerType_Purchase, true );
   S2SConnectionSetupData serverConn;
   serverConn.s2sCommunication =    purchase;
   serverConn.address =             m_purchaseServerAddress;
   serverConn.serverName =          purchase->GetName();
   serverConn.port =                m_purchaseServerPort;
   serverConn.serverType =          ServerType_Purchase;
   serverConn.packetType.push_back( PacketType_Purchase );
   serverConn.packetType.push_back( PacketType_Tournament );
   purchase->AddToOutwardFilters( PacketType_Purchase );
   purchase->AddToOutwardFilters( PacketType_Tournament );
   m_serverConnections.push_back( serverConn );

   m_chatServer = PrepConnection< FruitadensServerToServer, DiplodocusGame > ( m_chatServerAddress, m_chatServerPort, "chat", m_connectionManager, ServerType_Chat, true );
   m_chatServer->AddToOutwardFilters( PacketType_Chat );

   m_notificationServer = PrepConnection< FruitadensServerToServer, DiplodocusGame > ( m_notificationServerAddress, m_notificationServerPort, "notification", m_connectionManager, ServerType_Notification, true );
   m_notificationServer->AddToOutwardFilters( PacketType_Notification );

   m_userStatsServer = PrepConnection< FruitadensServerToServer, DiplodocusGame > ( m_userStatsIpAddress, m_userStatsPort, "userstat", m_connectionManager, ServerType_UserStats, true );
   m_userStatsServer->AddToOutwardFilters( PacketType_UserStats );

   //----------------------------------------------------------------
   

   SetupS2SConnections( m_serverAddress, 0 );

   m_connectionManager->Init();

   s2s->QueueInboundRequest( PacketType_Login, PacketLogin::LoginType_RequestToLogoutAllUsersForGame, ServerType_Login );
   
   m_connectionManager->Run();

   return false;
}

//-----------------------------------------------------

void     GameFramework::SetupS2SConnections( const string& address, U16 port )
{
   vector< S2SConnectionSetupData >::iterator it = m_serverConnections.begin();
   while( it != m_serverConnections.end() )
   {
      FruitadensServerToServer* serverComm = NULL;
      S2SConnectionSetupData& setup = *it++;
      if( setup.s2sCommunication == NULL )
      {
         serverComm = PrepConnection< FruitadensServerToServer, DiplodocusGame > ( address, port, "game", m_connectionManager, ServerType_GameInstance, true );
      }
      else 
      {
         serverComm = setup.s2sCommunication;
      }

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
