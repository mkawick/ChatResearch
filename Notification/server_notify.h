#ifndef H_SERVER_NOTIFY_H
#define H_SERVER_NOTIFY_H

enum GameNotification
{
   gnTurn,
   gnInvite,
   gnReady,
   gnEnded
};

// initialize the iOS push notification system
bool NotifyIosInit();

// uninitialize the iOS push notification system
void NotifyIosUninit();

// Sends a notification to a user's client.
//bool NotifyUser(unsigned int userId, int gameType, unsigned int gameId, GameNotification notification, ...);

// Set pending notification count for the given user
//void NotifyUserSetPendingCount(unsigned int userId, int gameType, int count);

// Resets pending notifications for the given user
//void NotifyUserClear(unsigned int userId, int gameType);

// Looks up the user's db device info
//bool  getUserDeviceIos( unsigned int userId, int gameType, unsigned char* buffer );

//-----------------------------------------------------

void NotifyAndroidInit();
void NotifyAndroidUninit();
bool NotifyAndroidSetUserDevice(unsigned int userId, int gameType, const unsigned char *deviceId);
bool  getUserDeviceAndroid( unsigned int userId, int gameType, unsigned char* buffer, int bufferSize );

// for android, there is nothing to initialize
bool notifyUserAndroid( const unsigned char* deviceId, unsigned int userId, int gameType, unsigned int gameId, int badgeId, GameNotification notification, va_list args);

bool NotifyUserDirect_iOS( unsigned int userId, const unsigned char *deviceId,
                          int gameType, unsigned int gameId,
                          int badge_count, GameNotification notification, ... );

//-----------------------------------------------------

//struct UserNotifyKey
//{
//   unsigned int userId;
//   unsigned int gameType;
//
//   inline bool operator<(const UserNotifyKey &rhs) const
//   {
//      if( rhs.userId >= userId ) 
//      {
//         return gameType < rhs.gameType;
//      }
//      return userId < rhs.userId;
//   }
//
//   inline bool operator==(const UserNotifyKey &rhs) const
//   {
//      return userId == rhs.userId && gameType == rhs.gameType;
//   }
//
//   inline bool operator!=(const UserNotifyKey &rhs) const
//   {
//      return userId != rhs.userId || gameType != rhs.gameType;
//   }
//};

//-----------------------------------------------------

#endif   //H_SERVER_NOTIFY_H

