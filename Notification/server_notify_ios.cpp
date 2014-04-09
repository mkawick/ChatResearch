#include <map>
// MYSQL on Win32 requires winsock to be included first
#include "server_comms.h"
#include "server_notify.h"
#include <mysql.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

//#include "server_database.h"
//#include "server_gamedata.h"
#include "server_log.h"
#include "string_funcs.h"

using namespace std;

#define DEVICE_TOKEN_LEN   32
#define MAX_PAYLOAD_SIZE   256
#define MAX_MESSAGE_SIZE   (sizeof(char) + sizeof(short) + 2 * sizeof(int) + \
                           DEVICE_TOKEN_LEN + sizeof(short) + MAX_PAYLOAD_SIZE)

// GameInfo flags
#define GAME_INFO_SANDBOX  (0x01)      // Use sandbox notification server

#define NOTIFY_ALERT_SOUND "PN3.caf"

struct GameInfo
{
   const char *cert;
   const char *key;
   const SSL_METHOD *method;
   SSL_CTX *ctx;
   SSL *ssl;
   int flags;
};

struct DeviceId
{
   unsigned char id[DEVICE_TOKEN_LEN];
};

struct ApnError
{
   unsigned char command;  // always 8
   unsigned char status;   // error code
   unsigned int id;        // notification id
};

std::map<unsigned int, DeviceId> s_Devices;
 
// keeps a count of how many notifications are outstanding for a given player
std::map<UserNotifyKey, unsigned int> s_PendingNotifications;

static GameInfo s_GameInfos[] =
{
   { NULL, NULL, 0, 0, 0, GAME_INFO_SANDBOX },
   //{ "AscensionCert.pem", "AscensionKey.pem", 0, 0, 0, 0 },  // GAME_SELECT_ASCENSION
   { NULL, NULL, 0, 0, 0, GAME_INFO_SANDBOX },  // GAME_SELECT_ASCENSION
   { NULL, NULL, 0, 0, 0, GAME_INFO_SANDBOX },  // GAME_SELECT_DOMINION
   { NULL, NULL, 0, 0, 0, GAME_INFO_SANDBOX },  // GAME_SELECT_THUNDERSTONE
   { NULL, NULL, 0, 0, 0, GAME_INFO_SANDBOX },  // GAME_SELECT_WOWCMG
   //{ "SummonWarCert.pem", "SummonWarKey.pem", 0, 0, 0, 0 },  // GAME_SELECT_SUMMONWAR
   { "SummonWarPTCert.pem", "SummonWarPTKey.pem", 0, 0, 0, 0 },  // GAME_SELECT_SUMMONWAR
   { NULL, NULL, 0, 0, 0, GAME_INFO_SANDBOX },  // GAME_SELECT_FOODFIGHT
   { NULL, NULL, 0, 0, 0, GAME_INFO_SANDBOX },  // GAME_SELECT_NIGHTFALL
   { NULL, NULL, 0, 0, 0, GAME_INFO_SANDBOX },  // GAME_SELECT_PENNYARCADE
   { NULL, NULL, 0, 0, 0, GAME_INFO_SANDBOX },  // GAME_SELECT_INFINITECITY
};
static int s_GameInfoCount = sizeof(s_GameInfos) / sizeof(s_GameInfos[0]);

static bool s_ApnIsInitialized = false;
static bool s_ApnIsConnected = false;

// converts a device ID into an ASCII string suitable for
// use in an SQL query.
static const char *devicetoa(const unsigned char *device)
{
   static char devstr[DEVICE_TOKEN_LEN * 2 + 1];

   for (int i = 0; i < DEVICE_TOKEN_LEN; ++i)
   {
      devstr[i * 2] = "0123456789abcdef"[device[i] >> 4];
      devstr[i * 2 + 1] = "0123456789abcdef"[device[i] & 0xf];
   }

   devstr[DEVICE_TOKEN_LEN * 2] = '\0';

   return devstr;
}

static void logNotification(unsigned int id, const unsigned char *buf, int binaryLen, int payloadLen)
{
   char msg[512];
   char *pMsg = msg;

   for (int i = 0; i < binaryLen; ++i)
   {
      *pMsg++ = "0123456789abcdef"[buf[i] >> 4];
      *pMsg++ = "0123456789abcdef"[buf[i] & 0xf];
   }

   memcpy(pMsg, buf + binaryLen, payloadLen);
   pMsg[payloadLen] = '\0';

   LogMessage(LOG_PRIO_DEBUG, "Sent notification %d:\'%s\'\n", id, msg);
}

// Sends a notification message to the apn server
static bool sendNotification(SSL *ssl, const unsigned char *device,
                             const char *payload, size_t payloadLen)
{
   static unsigned int s_NextId = 1;

   unsigned char msgBuf[MAX_MESSAGE_SIZE];
   unsigned char *msgPtr = msgBuf;

   short tokenSize = htons(DEVICE_TOKEN_LEN);
   short payloadSize = htons(payloadLen);

   *msgPtr++ = 1; //command 1
   
   // identifier
   unsigned int id = s_NextId++;
   unsigned int hostId = htonl(id);
   memcpy(msgPtr, &hostId, 4);
   msgPtr += 4;

   // expiry
   //unsigned int expiry = 0;
   unsigned int expiry = htonl(((unsigned int)time(NULL))+(60*60*24*7));  // expires in 7 days
   memcpy(msgPtr, &expiry, 4);
   msgPtr += 4;

   // token size
   memcpy(msgPtr, &tokenSize, 2);
   msgPtr += 2;

   // token
   memcpy(msgPtr, device, DEVICE_TOKEN_LEN);
   msgPtr += DEVICE_TOKEN_LEN;

   //payload size
   memcpy(msgPtr, &payloadSize, 2);
   msgPtr += 2;

   int binaryLen = msgPtr - msgBuf;

   //payload
   memcpy(msgPtr, payload, payloadLen);
   msgPtr += payloadLen;

   //logNotification(id, msgBuf, binaryLen, payloadLen);

   int bytesOut = SSL_write(ssl, msgBuf, msgPtr - msgBuf);
   if (bytesOut != msgPtr - msgBuf)
   {
      return false;
   }

   return true;
}

static bool checkConnection(int gameType)
{
   if (!s_ApnIsConnected)
   {
      return false;
   }

   SOCKET s = BIO_get_fd(SSL_get_rbio(s_GameInfos[gameType].ssl), NULL);

   fd_set readFds, errFds;

   FD_ZERO(&readFds);
   FD_ZERO(&errFds);

   FD_SET(s, &readFds);
   FD_SET(s, &errFds);

   timeval timeout;
   timeout.tv_sec = 0;
   timeout.tv_usec = 0;

   int ret = select(s + 1, &readFds, NULL, &errFds, &timeout);
   if (ret == 0)
   {
      return true;
   }

   if (FD_ISSET(s, &readFds))
   {
      ApnError errPacket;
      int bytes = recv(s, (char*)&errPacket, sizeof(errPacket), 0);
      if (bytes == sizeof(errPacket))
      {
         LogMessage(LOG_PRIO_DEBUG, "Error %d sending notification %d\n", errPacket.status,
            htonl(errPacket.id)); 
      }
   }
   else if (FD_ISSET(s, &errFds))
   {
      LogMessage(LOG_PRIO_DEBUG, "Apn socket error.\n");
   }
   else
   {
      LogMessage(LOG_PRIO_DEBUG, "Apn select() error.\n");
   }

   return false;
}

static int pem_passwd_cb(char *buf, int size, int rwflag, void *u)
{
   STRCPY(buf, size, "11incinerate11");
   return(strlen(buf));
}

// Disconnects from the Apple push notification server, if connected
static void apnDisconnect()
{
   for (int i = 0; i < s_GameInfoCount; ++i)
   {
      if (!s_GameInfos[i].cert)
      {
         continue;
      }

      if (s_GameInfos[i].ssl)
      {
         SOCKET s = BIO_get_fd(SSL_get_rbio(s_GameInfos[i].ssl), NULL);

         SSL_shutdown(s_GameInfos[i].ssl);
         SSL_free(s_GameInfos[i].ssl);
         s_GameInfos[i].ssl = 0;

         SSL_CTX_free(s_GameInfos[i].ctx);
         s_GameInfos[i].ctx = 0;

         s_GameInfos[i].method = 0;

         closesocket(s);
      }
   }

   s_ApnIsConnected = false;
}

// Attempts to connect to the Apple push notification server
static bool apnConnect()
{
   s_ApnIsConnected = false;


   hostent *sandbox = gethostbyname("gateway.sandbox.push.apple.com");
   if (!sandbox)
   {
      return false;
   }
    
   sockaddr_in sandboxAddr = { 0 };
   sandboxAddr.sin_family = AF_INET;
   memcpy(&sandboxAddr.sin_addr, sandbox->h_addr_list[0], sandbox->h_length);
   sandboxAddr.sin_port = htons(2195);

   hostent *apn = gethostbyname("gateway.push.apple.com");
   if (!apn)
   {
      return false;
   }

   sockaddr_in apnAddr = { 0 };
   apnAddr.sin_family = AF_INET;
   memcpy(&apnAddr.sin_addr, apn->h_addr_list[0], apn->h_length);
   apnAddr.sin_port = htons(2195);

   bool err = false;
   for (int i = 0; i < s_GameInfoCount; ++i)
   {
      if (!s_GameInfos[i].cert)
      {
         continue;
      }

      // open TCP connection
      SOCKET s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
      if (s < 0)
      {
         err = true;
         break;
      }

      sockaddr_in *addr = (s_GameInfos[i].flags & GAME_INFO_SANDBOX) ? &sandboxAddr : &apnAddr;

      if (connect(s, (sockaddr *)addr, sizeof(*addr)) < 0)
      {
         closesocket(s);
         LogMessage(LOG_PRIO_ERR, "Can't connect to Apple push server.\n");
         err = true;
         break;
      }

      // open SSL connection
      s_GameInfos[i].method = SSLv23_method();

      // Some versions of OpenSSL want this arg to be non-const
      s_GameInfos[i].ctx = SSL_CTX_new(const_cast<SSL_METHOD *>(s_GameInfos[i].method));

      if( s_GameInfos[i].ctx == NULL )// this fails in new notification server
      {
         closesocket(s);
         LogMessage(LOG_PRIO_ERR, "Can't create open ssl certificate\n" );
         err = true;
         break;
      }
      SSL_CTX_set_default_passwd_cb(s_GameInfos[i].ctx, pem_passwd_cb);

      if (!SSL_CTX_use_certificate_file(s_GameInfos[i].ctx, s_GameInfos[i].cert, SSL_FILETYPE_PEM))
      {
         closesocket(s);
         LogMessage(LOG_PRIO_ERR, "Can't load certificate file %d (%s)\n", i, s_GameInfos[i].cert);
         //ERR_print_errors_fp(stderr);
         err = true;
         break;
      }

      if (!SSL_CTX_use_PrivateKey_file(s_GameInfos[i].ctx, s_GameInfos[i].key, SSL_FILETYPE_PEM))
      {
         closesocket(s);
         LogMessage(LOG_PRIO_ERR, "Can't load private key file %d (%s)\n", i, s_GameInfos[i].key);
         //ERR_print_errors_fp(stderr);
         err = true;
         break;
      }

      s_GameInfos[i].ssl = SSL_new(s_GameInfos[i].ctx);

      BIO *sbio = BIO_new_socket(s, BIO_NOCLOSE);
      SSL_set_bio(s_GameInfos[i].ssl, sbio, sbio);

      int ret = SSL_connect(s_GameInfos[i].ssl);
      if (ret != 1)
      {
         closesocket(s);
         LogMessage(LOG_PRIO_ERR, "Can't establish SSL connection:\n");
         //ERR_print_errors_fp(stderr);
         err = true;
         break;
      }
   }

   if (err)
   {
      apnDisconnect();
      return false;
   }

   s_ApnIsConnected = true;
   return true;
}

// notification type specific functions
// ------

static bool sendTurnNotification(const unsigned char *deviceId, unsigned int userId, int gameType, unsigned int gameId, int badge, const char *opponent)
{
   char payload[MAX_PAYLOAD_SIZE];
   int payloadLen;

   if (!opponent || !*opponent)
   {
      opponent = "Someone";
   }

   payloadLen = STRPRINTF(payload, sizeof(payload),
      "{\"aps\":{\"alert\":{\"loc-key\":\"GAME_NOTIFY_TURN\",\"loc-args\":[\"%s\"]},\"badge\":%d,\"sound\":\"" NOTIFY_ALERT_SOUND "\"},"
      "\"game\":%d,\"user\":%d}", opponent, badge, gameId, userId);

   if (!sendNotification(s_GameInfos[gameType].ssl, deviceId, payload, payloadLen))
   {
      apnDisconnect();
      return false;
   }

   return true;
}

static bool sendInviteNotification(const unsigned char *deviceId, unsigned int userId, int gameType, unsigned int gameId, int badge, const char *opponent)
{
   char payload[MAX_PAYLOAD_SIZE];
   int payloadLen;

   if (!opponent || !*opponent)
   {
      opponent = "Someone";
   }

   payloadLen = STRPRINTF(payload, sizeof(payload),
      "{\"aps\":{\"alert\":{\"loc-key\":\"GAME_NOTIFY_INVITE\",\"loc-args\":[\"%s\"]},\"badge\":%d,\"sound\":\"" NOTIFY_ALERT_SOUND "\"},"
      "\"game\":%d,\"user\":%d}", opponent, badge, gameId, userId);

   if (!sendNotification(s_GameInfos[gameType].ssl, deviceId, payload, payloadLen))
   {
      apnDisconnect();
      return false;
   }

   return true;
}

static bool sendGameReadyNotification(const unsigned char *deviceId, unsigned int userId, int gameType, unsigned int gameId, int badge, const char *opponent)
{
   char payload[MAX_PAYLOAD_SIZE];
   int payloadLen;

   if (!opponent || !*opponent)
   {
      opponent = "Someone";
   }

   payloadLen = STRPRINTF(payload, sizeof(payload),
      "{\"aps\":{\"alert\":{\"loc-key\":\"GAME_NOTIFY_READY\",\"loc-args\":[\"%s\"]},\"badge\":%d,\"sound\":\"" NOTIFY_ALERT_SOUND "\"},"
      "\"game\":%d,\"user\":%d}", opponent, badge, gameId, userId);

   if (!sendNotification(s_GameInfos[gameType].ssl, deviceId, payload, payloadLen))
   {
      apnDisconnect();
      return false;
   }

   return true;
}

static bool sendGameEndedNotification(const unsigned char *deviceId, unsigned int userId, int gameType, unsigned int gameId, int badge, const char *opponent)
{
   char payload[MAX_PAYLOAD_SIZE];
   int payloadLen;

   if (!opponent || !*opponent)
   {
      opponent = "Someone";
   }

   payloadLen = STRPRINTF(payload, sizeof(payload),
      "{\"aps\":{\"alert\":{\"loc-key\":\"GAME_NOTIFY_FINISHED\",\"loc-args\":[\"%s\"]},\"badge\":%d,\"sound\":\"" NOTIFY_ALERT_SOUND "\"},"
      "\"game\":%d,\"user\":%d}", opponent, badge, gameId, userId);

   if (!sendNotification(s_GameInfos[gameType].ssl, deviceId, payload, payloadLen))
   {
      apnDisconnect();
      return false;
   }

   return true;
}

// ------

// returns the device ID of the given user, or NULL if it cannot be found.
// the pointer returned is valid until the next call to getUserDevice.
bool  getUserDeviceIos( unsigned int userId, int gameType, unsigned char* buffer, int buffersize )
{
   bool found = false;
   buffer[0] = 0;

#if USE_MYSQL_DATABASE

   const char *gameName = GameDatabaseName(gameType);

   char query[256];
   MYSQL *sql = GetMysqlConnection();

   STRPRINTF(query, sizeof(query), "SELECT device_id FROM devices_ios_%s WHERE user_id = \'%u\'", gameName, userId);
   mysql_query(sql, query);
   MYSQL_RES *res = mysql_store_result(sql);

   MYSQL_ROW row;
   if ((row = mysql_fetch_row(res)))
   {
      //strlen( row[0] );
      //memcpy( buffer, row[0], DEVICE_TOKEN_LEN);
      strncpy( (char*)buffer, row[0], buffersize);
      found = true;

      //LogMessage(LOG_PRIO_ERR, "    getUserDevice(userId=%d,gameType=%s) = %s\n", userId, gameName, devicetoa(deviceId) );
   }
   else
   {
      LogMessage(LOG_PRIO_ERR, "    !!!! getUserDevice(userId=%d,gameType=%s) failed!\n", userId, gameName);
   }
   mysql_free_result(res);
#endif
   return found;
}

bool NotifyIosInit()
{
   s_ApnIsInitialized = true;

   apnConnect();

   return true;
}

void NotifyIosUninit()
{
   apnDisconnect();
   s_ApnIsInitialized = false;

   s_Devices.clear();
   s_PendingNotifications.clear();
}

bool NotifyIosSetUserDevice(unsigned int userId, int gameType, const unsigned char *deviceId)
{
#if USE_MYSQL_DATABASE
   MYSQL *sql = GetMysqlConnection();

   const char *gameName = GameDatabaseName(gameType);
   
   char query[256];
   STRPRINTF(query, sizeof(query), "REPLACE INTO devices_ios_%s VALUES (%d, x\'%s\')",
      gameName, userId, devicetoa(deviceId));
   //LogMessage(LOG_PRIO_ERR, "    NotifyIosSetUserDevice: %s\n", query );
   
   int ret = mysql_query(sql, query);
   if (ret != 0)
   {
      //LogMessage(LOG_PRIO_ERR, "Error %s (code %d) executing DB query: \"%s\"\n", mysql_error(sql), ret, query);
      return false;
   }
#endif

   return true;
}

bool notifyUserIos( const unsigned char* deviceId, unsigned int userId, int gameType, unsigned int gameId, int badgeId, GameNotification notification, va_list args)
{
   if (!s_ApnIsInitialized)
   {
      return false;
   }

   if (!checkConnection(gameType))
   {
      apnDisconnect();

      if (!apnConnect())
      {
         return false;
      }
   }
   
   switch (notification)
   {
   case gnTurn:
      return sendTurnNotification( deviceId, userId, gameType, gameId, badgeId, va_arg(args, char *));
   case gnInvite:
      return sendInviteNotification( deviceId, userId, gameType, gameId, badgeId, va_arg(args, char *));
   case gnReady:
      return sendGameReadyNotification( deviceId, userId, gameType, gameId, badgeId, va_arg(args, char *));
   case gnEnded:
      return sendGameEndedNotification( deviceId, userId, gameType, gameId, badgeId, va_arg(args, char *));
   }

   return false;
}

int CalculateBadgeNumberFromPendingNotifications( unsigned int userId, unsigned int gameId )
{
   // how many notifications are outstanding for this user?
   int badgeId = 1;
   UserNotifyKey key;
   key.userId = userId;
   key.gameId = gameId;

   std::map<UserNotifyKey, unsigned int>::iterator i = s_PendingNotifications.lower_bound(key);
   if (i == s_PendingNotifications.end() || i->first != key)
   {
      s_PendingNotifications.insert(i, std::pair<UserNotifyKey, unsigned int>(key, 1));
   }
   else
   {
      badgeId = ++i->second;
   }

   return badgeId;
}

////////////////////////////////////////////////////////////

bool NotifyUser( unsigned int userId, int gameType, unsigned int gameId, GameNotification notification, ...)
{
#if USE_MYSQL_DATABASE

   if (!s_ApnIsInitialized)
   {
      return false;
   }
   
   bool isAndroid = false;

   const int bufferSize = 512;
   unsigned char deviceId[ bufferSize ];
   bool found = getUserDeviceIos( userId, gameType, deviceId, bufferSize );
   if( found == false )
   {
      found = getUserDeviceAndroid( userId, gameType, deviceId, bufferSize );
      if( found == false )
      {
         return false;
      }
      isAndroid = true;
   }

   int badgeId = CalculateBadgeNumberFromPendingNotifications( userId, gameId );

   //-----------------------
   va_list args;
   va_start(args, notification);

   // NYI: determine what kind of devices the user is using (ios, android, etc)
   // and route to the appropriate notification system.
   bool ret = false;
   
   if( isAndroid == true )
   {
      ret = notifyUserAndroid( deviceId, userId, gameType, gameId, badgeId, notification, args );
   }
   else
   {
      ret = notifyUserIos( deviceId, userId, gameType, gameId, badgeId, notification, args);
   }

   va_end(args);

   return ret;

#else

   return false;

#endif
}

////////////////////////////////////////////////////////////

void NotifyUserSetPendingCount(unsigned int userId, int gameType, int count)
{
   if (!s_ApnIsInitialized)
   {
      return;
   }

   UserNotifyKey key;
   key.userId = userId;
   key.gameId = gameType;

   std::map<UserNotifyKey, unsigned int>::iterator i = s_PendingNotifications.lower_bound(key);
   if (i == s_PendingNotifications.end() || i->first != key)
   {
      s_PendingNotifications.insert(i, std::pair<UserNotifyKey, unsigned int>(key, count));
   }
   else
   {
      i->second = count;
   }
}

void NotifyUserClear(unsigned int userId, int gameType)
{
   if (!s_ApnIsInitialized)
   {
      return;
   }

   UserNotifyKey key;
   key.userId = userId;
   key.gameId = gameType;

   std::map<UserNotifyKey, unsigned int>::iterator i = s_PendingNotifications.lower_bound(key);
   if (i != s_PendingNotifications.end() && i->first == key)
   {
      s_PendingNotifications.erase(i);
   }
}
 