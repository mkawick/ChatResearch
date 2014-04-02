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

// Associates a user with an iOS device. Any previous association for a user
// is removed.
bool NotifyIosSetUserDevice(unsigned int userId, int gameType, const unsigned char *deviceId);

// Sends a notification to a user's client.
bool NotifyUser(unsigned int userId, int gameType, unsigned int gameId, GameNotification notification, ...);

// Set pending notification count for the given user
void NotifyUserSetPendingCount(unsigned int userId, int gameType, int count);

// Resets pending notifications for the given user
void NotifyUserClear(unsigned int userId, int gameType);

// Looks up the user's db device info
bool  getUserDeviceIos( unsigned int userId, int gameType, unsigned char* buffer );

//-----------------------------------------------------

void NotifyAndroidInit();
void NotifyAndroidUninit();
bool NotifyAndroidSetUserDevice(unsigned int userId, int gameType, const unsigned char *deviceId);
bool  getUserDeviceAndroid( unsigned int userId, int gameType, unsigned char* buffer, int bufferSize );

// for android, there is nothing to initialize
bool notifyUserAndroid( const unsigned char* deviceId, unsigned int userId, int gameType, unsigned int gameId, int badgeId, GameNotification notification, va_list args);

//-----------------------------------------------------

struct UserNotifyKey
{
   unsigned int userId;
   unsigned int gameId;

   inline bool operator<(const UserNotifyKey &rhs) const
   {
      if( rhs.userId >= userId ) 
      {
         return gameId < rhs.gameId;
      }
      return userId < rhs.userId;
   }

   inline bool operator==(const UserNotifyKey &rhs) const
   {
      return userId == rhs.userId && gameId == rhs.gameId;
   }

   inline bool operator!=(const UserNotifyKey &rhs) const
   {
      return userId != rhs.userId || gameId != rhs.gameId;
   }
};

//-----------------------------------------------------

#endif   //H_SERVER_NOTIFY_H

