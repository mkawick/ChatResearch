#pragma once

#include <string>
#include <map>
using namespace std;

#include "../NetworkCommon/DataTypes.h"
#include "../NetworkCommon/Packets/BasePacket.h"
#include "../NetworkCommon/Packets/AnalyticsPacket.h"

#include "GameCallbacks.h"

class GameFramework;
class DiplodocusGame;
class FruitadensServerToServer;
class CommandLineParser;

//////////////////////////////////////////////////////////

struct S2SConnectionSetupData
{
   FruitadensServerToServer*  s2sCommunication;
   string                     address;
   string                     serverName;
   U16                        port;
   ServerType                 serverType;
   vector< PacketType >       packetType;// used for filtering
   //vector< U16 > subpacketList;
};

//////////////////////////////////////////////////////////

class GameFramework
{
public:
   GameFramework( const char* gameName, const char* shortName, U8 gameProductId, const char* version = "0.05" );
   ~GameFramework();

   void  SetDatabaseIdentification( const string& uuid ) { m_gameUuid = uuid; }

   const string&  GetServerName() const { return m_serverName; }
   const string&  GetServerShortName() const { return m_shortName; }
   U32            GetServerId() const { return m_serverId; }
   U8             GetGameProductId() const { return m_gameProductId; }
   static GameFramework* Instance() { return m_instance; }

   //----------- configuration -----------------------

      //------------ defaults --------------------
      // DO NOT INVOKE THESE AFTER INVOKING THE RUN FUNCTION... they will have no effect
   void  SetupDefaultDatabaseConnection( const string& serverAddress, U16 port, const string& username,const string& password, const string& dbSchemaName );
   void  SetupDefaultListeningPort( U16 port );

   void  SetupDefaultAnalyticsConnection( const string& address, U16 port );
   void  SetupDefaultChatConnection( const string& address, U16 port );
   void  SetupDefaultPurchaseConnection( const string& address, U16 port );
   void  SetupDefaultNotificationConnection( const string& address, U16 port );
   void  SetupDefaultUserStatConnection( const string& address, U16 port );
   void  SetupDefaultS2S( const string& address, U16 port );
   void  SetupConnectionToAnotherServer( const string& address, const string& serverName, U16 port, ServerType serverType, PacketType packetType );
   void  AddPacketTypeToServer( const string& serverName, PacketType packetType );
      //------------ defaults --------------------

   void  UseCommandlineOverrides( int argc, const char* argv[] );

   //----------- end configuration -------------------


   void     AddTimer( U32 timerId, U32 callbackTimeMs = 100 ); // timers must be unique
   void     RegisterForIncomingData( GameCallbacks* basicGameServer ) { m_callbacksObject = basicGameServer; }
   bool     IsSetupProperly() const { if( m_callbacksObject == NULL || m_gameUuid.size() == 0 ) return false; return true; }

   //----------------------------------------------

   bool     SendErrorToClient( U32 connectionId, int errorCode, int errorSubCode );
   bool     SendGameData( U32 connectionId, U32 gatewayId, const MarshalledData* );
   bool     SendChatData( BasePacket* ); // we will own this data after
   bool     SendToAnotherServer( BasePacket* );// based on packet type... see SetupConnectionToAnotherServer
   bool     InformClientWhoThisServerIs( U32 connectionId );
   bool     SendPacketToGateway( BasePacket* packet, U32 connectionId );

   void     SendStat( const string& statName, U16 integerIdentifier, float value, PacketAnalytics::StatType type );
   void     SendNotification( const string& userUuid, U32 userId, int notificationType, const string& additionalText );
   void     SendPushNotification( U32 userId, int gameType, unsigned int gameId, int notificationType, const string& additionalText );
   void     UpdatePushNotificationCount( U32 userId, int gameType, int notificationCount );

   void     SendGameResultToStatServer( int gameType, U32 gameId, int playerCount,
                                 unsigned int *pResultOrder, unsigned int *pPlayerFactions, unsigned int forfeitFlags );

   DiplodocusGame*   GetGame() { return m_connectionManager; }
   void     LockGameMutex();
   void     UnlockGameMutex();

   //----------------------------------------------

   bool     Run();

   //----------------------------------------------
   FruitadensServerToServer*  GetChat() { return m_chatServer; }
   FruitadensServerToServer*  GetNotification() { return m_notificationServer; }
   FruitadensServerToServer*  GetUserStats() { return m_userStatsServer; }

private:
   void     SetupS2SConnections( const string& address, U16 port );
   
   GameCallbacks* m_callbacksObject;

   string         m_serverName;
   string         m_shortName;
   string         m_serverAddress;
   U16            m_serverPort;
   U32            m_serverId;
   U8             m_gameProductId;
   string         m_version;
   string         m_gameUuid;

   DiplodocusGame*m_connectionManager;

   static GameFramework* m_instance;


   // default connection values
   U16            m_listenPort;

   U16            m_analyticsServerPort;
   string         m_analyticsServerAddress;

   U16            m_purchaseServerPort;
   string         m_purchaseServerAddress;

   U16            m_chatServerPort;
   string         m_chatServerAddress;
   FruitadensServerToServer*  m_chatServer;

   U16            m_notificationServerPort;
   string         m_notificationServerAddress;
   FruitadensServerToServer*  m_notificationServer;

   U16            m_userStatsPort;
   string         m_userStatsIpAddress;
   FruitadensServerToServer*  m_userStatsServer;
   

   vector< S2SConnectionSetupData > m_serverConnections;


   U16            m_listenForS2SPort;
   string         m_listenForS2SAddress;

   U16            m_dbPort;
   string         m_dbIpAddress;
   string         m_dbUsername;
   string         m_dbPassword;
   string         m_dbSchema;

   CommandLineParser* m_parser;

   map< U32, TimerInfo > m_timers;
};

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////