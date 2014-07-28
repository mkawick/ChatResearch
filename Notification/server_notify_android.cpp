#include <map>
// MYSQL on Win32 requires winsock to be included first
#include "server_comms.h"

#include "server_notify.h"

#include <mysql/mysql.h> // MySQL Include File

//#include <openssl/ssl.h>
//#include <openssl/err.h>
#include <string>
#include <map>
#include <iostream>
#include <curl/curl.h>
using namespace std;

//#include "server_database.h"
//#include "server_gamedata.h"
//#include "server_log.h"
//#include "string_funcs.h"
#include "../NetworkCommon/ExternalApis/HTTPSender.h"


#include <boost/format.hpp>
using boost::format;

typedef map< string, string>  KeyValueMap;
typedef pair< string, string>  KeyValuePair;
//string   printResponseData; //will hold the url's contents

//static string  authKey              = "AIzaSyD-KJoLqlrgnGipT5lqFUeQu4YojgqWgTA";
static string  NOTIFY_ALERT_SOUND   = "PlaydekNotification.ogg";
HTTP::GoogleAndroidNotificationSender* googleAndroidSender = NULL;
HTTP::AmazonAndroidNotificationSender* amazonSender = NULL;
//const int maxAndroidDeviceStringId = 1200; 

/////////////////////////////////////////////////////////////////////////////////////

/*
void devicetoa( const unsigned char *device, unsigned char* outputBuffer )
{
   //static char devstr[maxAndroidDeviceStringId];
   int len = strlen( (const char*) device );

   for (int i = 0; i < len; ++i)
   {
      outputBuffer[ i*2 ] = "0123456789abcdef"[device[i] >> 4];
      outputBuffer[ i*2 +1] = "0123456789abcdef"[device[i] & 0xf];
   }
   outputBuffer[len*2]=0;
}
*/

/////////////////////////////////////////////////////////////////////////////////////
/*
size_t   WriteCallback(char* buf, size_t size, size_t nmemb, void* up)
{ 
   //callback must have this declaration
   //buf is a pointer to the data that curl has for us
   //size*nmemb is the size of the buffer

   size_t num =  size*nmemb;
   int numItems = static_cast<int> ( num );

   for (int c = 0; c<numItems; c++)
   {
     printResponseData.push_back( buf[c] );
   }
   return num; //tell curl how many bytes we handled
}
*/

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

void StandardMessageSetup( KeyValueMap& dataMap, unsigned int userId, int gameType, unsigned int gameId, int badge, const char *opponent, const char* messageTypeString, const char* extraText )
{
   if (!opponent || !*opponent)
   {
      opponent = "Someone";
   }

   dataMap.insert( KeyValuePair( "loc-key", messageTypeString ) );
   dataMap.insert( KeyValuePair( "loc-args", opponent ) );
   dataMap.insert( KeyValuePair( "badge", (format( "%1%" ) % badge ).str()) );
   dataMap.insert( KeyValuePair( "sound", NOTIFY_ALERT_SOUND ) );
   dataMap.insert( KeyValuePair( "game", (format( "%1%" ) % gameId  ).str()) );
   dataMap.insert( KeyValuePair( "user", (format( "%1%" ) % userId ).str()) );
   dataMap.insert( KeyValuePair( "text", extraText ) );
}

// notification type specific functions
// ------

/////////////////////////////////////////////////////////////////////////////////////

// ------

/*
// returns the device ID of the given user, or NULL if it cannot be found.
// the pointer returned is valid until the next call to getUserDevice.
bool getUserDeviceAndroid( unsigned int userId, int gameType, unsigned char* buffer, int buffersize )
{
   bool found = false;
   buffer[0] = 0;

#if USE_MYSQL_DATABASE

   const char *gameName = GameDatabaseName(gameType);

   char query[256];
   MYSQL *sql = GetMysqlConnection();

   STRPRINTF(query, sizeof(query), "SELECT device_id FROM devices_android_%s WHERE user_id = \'%u\'", gameName, userId);
   mysql_query(sql, query);
   MYSQL_RES *res = mysql_store_result(sql);

   MYSQL_ROW row;
   if ((row = mysql_fetch_row(res)))
   {
      //deviceId = row[0];
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
*/

void  NotifyAndroidInit()
{
   assert( googleAndroidSender == NULL && amazonSender == NULL );

   googleAndroidSender = new HTTP::GoogleAndroidNotificationSender;
   amazonSender = new HTTP::AmazonAndroidNotificationSender;
}

void NotifyAndroidUninit()
{
   delete googleAndroidSender;
   delete amazonSender;
}

/*
bool NotifyAndroidSetUserDevice(unsigned int userId, int gameType, const unsigned char *deviceId)
{
#if USE_MYSQL_DATABASE
   MYSQL *sql = GetMysqlConnection();

   const char *gameName = GameDatabaseName(gameType);

   cout << "********************************" << endl;
   cout << "Device id = " << (char*) deviceId << endl;
   cout << "********************************" << endl;
   
   char query[maxAndroidDeviceStringId];
   unsigned char buffer[maxAndroidDeviceStringId];
   devicetoa( deviceId, buffer );
   STRPRINTF(query, sizeof(query), "REPLACE INTO devices_android_%s VALUES (%d, x\'%s\')", gameName, userId, buffer );

   int ret = mysql_query(sql, query );
   if (ret != 0)
   {
      const char* errorText = mysql_error( sql );
      cout << "DB Error: " << errorText << " on query " << query << endl;

      unsigned long errorCode = mysql_errno( sql );
      cout << "DB Error code: " << errorCode << endl;
      //LogMessage(LOG_PRIO_ERR, "Error %s (code %d) executing DB query: \"%s\"\n", mysql_error(sql), ret, query);
      return false;
   }

   //GameNotification notification;
   //va_list args;
   //va_start(args, notification);
   //notifyUserAndroid( (const char* )deviceId, userId, gameType, 1002, 52, notification )
   //sendTurnNotification( (const char* )deviceId, userId, gameType, 1002, 52 );
#endif

   return true;
}
*/

bool notifyUserAndroid( const unsigned char* deviceIdStr, unsigned int userId, int gameType, unsigned int gameId, int badgeId, GameNotification notification, va_list args)
{   
   assert( googleAndroidSender != NULL && amazonSender != NULL );

   const char* deviceId = reinterpret_cast< const char* >( deviceIdStr );
   
   // this mechanism works but may not be the best design.
   const char* amazonId = "amzn1."; 
   bool isAmazon = false;
   if( strncmp( deviceId, amazonId, strlen( amazonId ) ) == 0 )
   {
      isAmazon = true;
   }

   KeyValueMap dataMap;
   const char* opponent = va_arg(args, char *);
   
   switch (notification)
   {
   case gnTurn:
      StandardMessageSetup( dataMap, userId, gameType, gameId, badgeId, opponent, "GAME_NOTIFY_TURN", "It's your turn" );
      break;
   case gnInvite:
      StandardMessageSetup( dataMap, userId, gameType, gameId, badgeId, opponent, "GAME_NOTIFY_INVITE", "You have been invited" );
      break;
   case gnReady:
      StandardMessageSetup( dataMap, userId, gameType, gameId, badgeId, opponent, "GAME_NOTIFY_READY", "Ready to play" );
      break;
   case gnEnded:
      StandardMessageSetup( dataMap, userId, gameType, gameId, badgeId, opponent, "GAME_NOTIFY_FINISHED", "Finished" );
      break;
   }


   if( isAmazon == true )
   {
      return amazonSender->SendNotification( deviceId, dataMap );
   }
   else
   {
      return googleAndroidSender->SendNotification( deviceId, dataMap );
   }

   return false;
}

bool NotifyUserDirect_Android( unsigned int userId, const unsigned char *deviceId, int gameType,
                          unsigned int gameId, int badge_count, GameNotification notification, ... )
{
   //LogMessage(LOG_PRIO_INFO, "NotifyUserDirect_iOS: %s\n", devicetoa(deviceId));

   //int badge_count = CalculateBadgeNumberFromPendingNotifications( userId, gameType );

   va_list args;
   va_start(args, notification);

   bool ret = notifyUserAndroid( deviceId, userId, gameType, gameId, badge_count, notification, args);

   va_end(args);

   return ret;
}
