#pragma once

#include <deque>
#include <map>
using namespace std;

#include "../NetworkCommon/DataTypes.h"
#include "../NetworkCommon/Utils/CommandLineParser.h"
#include "../NetworkCommon/Utils/Utils.h"
#include "../NetworkCommon/NetworkIn/Khaan.h"
#include "../NetworkCommon/Database/Deltadromeus.h"

#include "../NetworkCommon/NetworkIn/Diplodocus.h"
#include "../NetworkCommon/Database/QueryHandler.h"
#include "../NetworkCommon/NetworkIn/KhaanServerToServer.h"

#include "UserConnection.h"

namespace Database
{
   class Deltadromeus;
}

///////////////////////////////////////////////////////////////////

class NotificationMainThread : public Queryer, public Diplodocus< KhaanServerToServer >
{
public: 
   typedef Diplodocus< KhaanServerToServer > ChainedType;

public:
   NotificationMainThread( const string& serverName, U32 serverId );
   ~NotificationMainThread();
   const char*             GetClassName() const { return "NotificationMainThread"; }

   void     Init( const string& iosPathToCertAndKeyFile );

   bool     AddInputChainData( BasePacket* packet, U32 connectionId );
   bool     AddOutputChainData( BasePacket* packet, U32 connectionId );
   bool     SendMessageToClient( BasePacket* packet, U32 connectionId, U32 gatewayId );

   bool     AddQueryToOutput( PacketDbQuery* packet );
   void     ServerWasIdentified( IChainedInterface* khaan );

private:

   bool     HandlePacketFromOtherServer( BasePacket* packet, U32 connectionId );
   bool     HandlePacketFromGateway( BasePacket* packet, U32 connectionId );
   bool     ConnectUser( const PacketPrepareForUserLogin* loginPacket );
   bool     DisconnectUser( const PacketPrepareForUserLogout* unwrappedPacket );
   bool     HandleNotification( const PacketNotification_SendNotification* unwrappedPacket );
   void     RunQueryAndNotification( Database::Deltadromeus* database, 
                              unsigned int user_id,
                              unsigned int game_type,
                              unsigned int game_id, 
                              int   badge_count,
                              int   notificationType,
                              const char*   additionalText,
                              bool  shouldSetupResendNotifications );
   
   void     PeriodicCheckForNewNotifications();

   void     RemoveExpiredConnections();

   int      MainLoop_InputProcessing();
   int      MainLoop_OutputProcessing();

   bool     HandleUpdateNotificationCount( const PacketNotification_UpdateNotificationCount* unwrappedPacket );
   int      CalculateBadgeNumberFromPendingNotifications( unsigned int userId, unsigned int gameType, int gameId );
   bool     StoreLastUserNotification( unsigned int userId, unsigned int gameType, int gameId,
                                       unsigned int notificationType, string additionalText );
   bool     SetupUserNotificationResend( unsigned int userId, unsigned int gameType, unsigned int deviceId, unsigned delayTimeSeconds );

   typedef  map< U32, UserConnection >    UserConnectionMap;
   typedef  pair< U32, UserConnection >   UserConnectionPair;
   typedef  UserConnectionMap::iterator   UserConnectionIterator;

   typedef  deque< PacketDbQueryResult* > QueryResultDeque;
   typedef  QueryResultDeque::iterator    QueryResultDequeIterator;

   UserConnectionMap       m_userConnectionMap;
   QueryResultDeque        m_dbQueries;

   Database::Deltadromeus* m_database; // only one for now

   time_t                  m_lastNotificationCheck_TimeStamp;
   //static const int        timeoutNotificationSend = 2 * 60;// two minutes
   static const int        timeoutNotificationSend = 10; // ten seconds
   static const int        SecondsBeforeRemovingLoggedOutUser = 3;

   
   struct UserNotificationKey
   {
      unsigned int userId;
      unsigned int gameType;

      inline bool operator<(const UserNotificationKey &rhs) const
      {
         if( rhs.userId >= userId ) 
         {
            return gameType < rhs.gameType;
         }
         return userId < rhs.userId;
      }

      inline bool operator==(const UserNotificationKey &rhs) const
      {
         return userId == rhs.userId && gameType == rhs.gameType;
      }

      inline bool operator!=(const UserNotificationKey &rhs) const
      {
         return userId != rhs.userId || gameType != rhs.gameType;
      }
   };

   struct UserNotificationRecord
   {
      unsigned int   notificationCount;

      unsigned int   lastNotificationGameId;
      unsigned int   lastNotificationType;
      string         lastNotificationText;

      unsigned int   resendNotificationDelaySeconds;
      time_t         lastNotificationTime;
   };

   // keeps a count of how many notifications are outstanding for a given player
   std::map<UserNotificationKey, UserNotificationRecord> m_PendingNotifications;
};

///////////////////////////////////////////////////////////////////