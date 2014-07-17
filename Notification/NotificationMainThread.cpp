// NotificationMainThread.cpp

#include <iostream>
#include <time.h>
#include <string>
using namespace std;

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/replace.hpp>

#include "NotificationMainThread.h"

#include "../NetworkCommon/Utils/CommandLineParser.h"

#include "../NetworkCommon/Packets/ServerToServerPacket.h"
#include "../NetworkCommon/Packets/PacketFactory.h"
#include "../NetworkCommon/Packets/NotificationPacket.h"

#include "../NetworkCommon/Database/Deltadromeus.h"// breaking rules to make things easier for GARY
#include <mysql/mysql.h> // MySQL Include File


#include "NotificationMainThread.h"

#include "../NetworkCommon/Database/StringLookup.h"

#include "server_notify.h"

#include <openssl/ssl.h>
#include <openssl/err.h>

//extern MYSQL *g_NotificationMYSQL;

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

NotificationMainThread::NotificationMainThread( const string& serverName, U32 serverId ): Queryer(), Diplodocus< KhaanServerToServer >( serverName, serverId, 0,  ServerType_Notification ), m_database( NULL )
{
   time( &m_lastNotificationCheck_TimeStamp );
   m_lastNotificationCheck_TimeStamp = ZeroOutMinutes( m_lastNotificationCheck_TimeStamp );
   SetSleepTime( 100 ); 
}

NotificationMainThread :: ~NotificationMainThread()
{
   NotifyIosUninit();
   NotifyAndroidUninit();
}

void     NotificationMainThread :: Init( const string& iosPathToCertAndKeyFile )
{
   SSL_library_init();
   SSL_load_error_strings();
   OpenSSL_add_all_algorithms();

   Diplodocus::Init();
   
   NotifyIosInit( iosPathToCertAndKeyFile.c_str() ); 
   NotifyAndroidInit(); 
}
///////////////////////////////////////////////////////////////////

void     NotificationMainThread::ServerWasIdentified( IChainedInterface* khaan )
{
   BasePacket* packet = NULL;
   PackageForServerIdentification( m_serverName, m_localIpAddress, m_externalIpAddress, m_serverId, m_serverType, m_listeningPort, m_gameProductId, m_isGame, m_isControllerApp, true, m_gatewayType, &packet );
   ChainedType* localKhaan = static_cast< ChainedType* >( khaan );
   localKhaan->AddOutputChainData( packet, 0 );
   m_clientsNeedingUpdate.push_back( localKhaan->GetServerId() );
}

///////////////////////////////////////////////////////////////////

bool     NotificationMainThread::AddInputChainData( BasePacket* packet, U32 connectionId )
{
   if( packet->packetType == PacketType_GatewayInformation )
   {
      PacketCleaner cleaner( packet );// we do not accept any data from the gateway
      HandleCommandFromGateway( packet, connectionId );
      return true;
   }

   if( packet->packetType == PacketType_ServerJobWrapper )// login and such
   {
      PacketCleaner cleaner( packet );
      HandlePacketFromOtherServer( packet, connectionId );
      return true;
   }

   if( packet->packetType == PacketType_GatewayWrapper ) 
   {
      PacketCleaner cleaner( packet );
      HandlePacketFromGateway( packet, connectionId );
      return true;
   }
   
   return false;
}

//---------------------------------------------------------------

bool   NotificationMainThread::AddOutputChainData( BasePacket* packet, U32 connectionId ) // coming from downsteam in
{
   if( packet->packetType == PacketType_DbQuery )
   {
      if( packet->packetSubType == BasePacketDbQuery::QueryType_Result )
      {
         PacketDbQueryResult* result = static_cast<PacketDbQueryResult*>( packet );
         Threading::MutexLock locker( m_mutex );
         m_dbQueries.push_back( result );
        /* if( result->customData != NULL )
            cout << "AddOutputChainData: Non-null custom data " << endl;*/
      }
      return true;
   }
   return false;
}
//---------------------------------------------------------------

bool     NotificationMainThread::SendMessageToClient( BasePacket* packet, U32 connectionId )
{
   if( packet->packetType == PacketType_GatewayWrapper )// this is already wrapped up and ready for the gateway... send it on.
   {
      Threading::MutexLock locker( m_mutex );

      ClientMapIterator itInputs = m_connectedClients.begin();
      if( itInputs != m_connectedClients.end() )// only one output currently supported.
      {
         KhaanServerToServer* khaan = static_cast< KhaanServerToServer* >( itInputs->second );
         khaan->AddOutputChainData( packet );
         m_clientsNeedingUpdate.push_back( khaan->GetChainedId() );
         itInputs++;
      }
      return true;
   }

   return false;
}

//---------------------------------------------------------------
//---------------------------------------------------------------

bool  NotificationMainThread::HandlePacketFromOtherServer( BasePacket* packet, U32 connectionId )// not thread safe
{
   if( packet->packetType != PacketType_ServerJobWrapper )
   {
      return false;
   }

   PacketServerJobWrapper* wrapper = static_cast< PacketServerJobWrapper* >( packet );
   BasePacket* unwrappedPacket = wrapper->pPacket;
   U32  serverIdLookup = wrapper->serverId;
   serverIdLookup = serverIdLookup;

   U8 packetType = unwrappedPacket->packetType;

   if( packetType == PacketType_Login )
   {
      switch( unwrappedPacket->packetSubType )
      {
      case PacketLogin::LoginType_PrepareForUserLogin:
         ConnectUser( static_cast< PacketPrepareForUserLogin* >( unwrappedPacket ) );
         return true;

      case PacketLogin::LoginType_PrepareForUserLogout:
         DisconnectUser( static_cast< PacketPrepareForUserLogout* >( unwrappedPacket ) );
         return true;
      }
   }
   else if( packetType == PacketType_Notification )
   {
      if( unwrappedPacket->packetSubType == PacketNotification::NotificationType_SendNotification )
      {
         HandleNotification( static_cast< PacketNotification_SendNotification* >( unwrappedPacket ) );
      }
      else if( unwrappedPacket->packetSubType == PacketNotification::NotificationType_UpdateNotificationCount )
      {
         HandleUpdateNotificationCount( static_cast< PacketNotification_UpdateNotificationCount* >( unwrappedPacket ) );
      }
   }

   return false;
}

//---------------------------------------------------------------

bool     NotificationMainThread::HandlePacketFromGateway( BasePacket* packet, U32 connectionId )
{
   if( packet->packetType != PacketType_GatewayWrapper )
   {
      return false;
   }

   PacketServerJobWrapper* wrapper = static_cast< PacketServerJobWrapper* >( packet );
   BasePacket* unwrappedPacket = wrapper->pPacket;
   U32  serverIdLookup = wrapper->serverId;
   serverIdLookup = serverIdLookup;

   U8 packetType = unwrappedPacket->packetType;
   
   {// local scope
      Threading::MutexLock locker( m_mutex );
      UserConnectionIterator item = m_userConnectionMap.find( connectionId );
      if( item != m_userConnectionMap.end() )
      {
         UserConnection& user = item->second;
         user.HandleRequestFromClient( unwrappedPacket );
      }
   }
   return false;
}

//---------------------------------------------------------------

bool     NotificationMainThread::ConnectUser( const PacketPrepareForUserLogin* loginPacket )
{
   U32 connectionId = loginPacket->connectionId;

   UserConnectionIterator it = m_userConnectionMap.find( connectionId );// don't do anything if this user is already logged in.
   if( it != m_userConnectionMap.end() )
      return false;

   bool found = false;
   // if the user is already here but relogged, simply add a new entry using the same basic setup
   m_mutex.lock();
      it = m_userConnectionMap.begin();
      while( it != m_userConnectionMap.end() )
      {
         if( it->second.GetUserInfo().uuid == loginPacket->uuid ) 
         {
            found = true;
            it->second.SetConnectionId( connectionId );
            
            it->second.Relog();
            m_userConnectionMap.insert( UserConnectionPair( connectionId, it->second ) );
            m_userConnectionMap.erase( it );
            break;
         }
         it++;
      }
   m_mutex.unlock();

   if( found == false )
   {
      UserConnection user( loginPacket );
      user.SetServer( this );

      m_mutex.lock();
      m_userConnectionMap.insert( UserConnectionPair( connectionId, user ) );
      m_mutex.unlock();
   }
   return true;
}


//---------------------------------------------------------------

bool     NotificationMainThread::DisconnectUser( const PacketPrepareForUserLogout* loginPacket )
{
   U32 connectionId = loginPacket->connectionId;

   m_mutex.lock();
   UserConnectionIterator it = m_userConnectionMap.find( connectionId );
   m_mutex.unlock();
   if( it == m_userConnectionMap.end() )
      return false;

   //it->second.NeedsUpdate();
   it->second.UserLoggedOut();

   return true;
}

//---------------------------------------------------------------

void  NotificationMainThread::RunQueryAndNotification( Database::Deltadromeus* database, 
                              unsigned int user_id,
                              unsigned int game_type,
                              unsigned int game_id, 
                              int   badge_count,
                              int   notificationType,
                              const char*   additionalText,
                              bool  shouldSetupResendNotifications )
{
   if( database != NULL )
   {
      /*
      SELECT user_device.device_id, user_device.platformId, user_device.id, user_device_notification.audio_file, user_device_notification.repeat_frequency_in_hours  
      FROM user_device JOIN user_device_notification ON user_device.id=user_device_notification.user_device_id WHERE user_device.user_id='16464' AND 
      user_device_notification.game_type='5' AND user_device_notification.is_enabled='1' AND user_device.is_enabled='1'
      */
      string query( "SELECT user_device.device_id, user_device.platformId, user_device.id, user_device_notification.audio_file, user_device_notification.repeat_frequency_in_hours ");
      query += " FROM user_device JOIN user_device_notification "
                     "ON user_device.id=user_device_notification.user_device_id WHERE user_device.user_id='";
      query += boost::lexical_cast< string  >( user_id );
      query += "' AND user_device_notification.game_type='";
      query += boost::lexical_cast< string  >( game_type );
      query += "' AND user_device_notification.is_enabled='1' ";
      query += "AND user_device.is_enabled='1'";

      //cout << query << endl;


      MYSQL *mysql = database->GetDbHandle();

      int ret = mysql_query( mysql, query.c_str() );
      if (ret != 0)
      {
         cout << "Error " << mysql_error(mysql)
              << " (code " << ret << ") executing DB query: \""
              << query << "\"" << endl;
         cout << "        error: " << mysql_error(mysql) << endl;
         return;
      }

      MYSQL_RES *res = mysql_store_result(mysql);
      if( res != NULL )
      {
         MYSQL_ROW row;
         for( row = mysql_fetch_row(res); row; row = mysql_fetch_row(res) )
         {
            if( row[0] == NULL )
            {
               continue;
            }

            unsigned int device_platform;
            sscanf( row[1], "%u", &device_platform );

            unsigned int device_id;
            sscanf( row[2], "%u", &device_id );

            char audioFile[60];
            audioFile[0] = 0;
            if( row[3] != NULL )
            {
               sscanf( row[3], "%s", audioFile );
            }

            unsigned int repeatFrequencyInHours;
            sscanf( row[4], "%u", &repeatFrequencyInHours );


            if( device_platform == 1 ) // iOS
            {
               NotifyUserDirect_iOS( user_id, 
                  (const unsigned char*)row[0], 
                  game_type, 
                  game_id, 
                  badge_count,
                  audioFile, 
                  (GameNotification)notificationType,
                  additionalText );

               if( shouldSetupResendNotifications == true &&
                  repeatFrequencyInHours > 0 )
               {
                  SetupUserNotificationResend( user_id, game_type, device_id, 60 * repeatFrequencyInHours );
               }
            }
         }
         mysql_free_result(res);
      }

      //NotifyUser(unwrappedPacket->userId, unwrappedPacket->gameType, unwrappedPacket->gameId,
      //            (GameNotification)unwrappedPacket->notificationType, unwrappedPacket->additionalText.c_str());
   }
}

//---------------------------------------------------------------

bool     NotificationMainThread::HandleNotification( const PacketNotification_SendNotification* unwrappedPacket )
{
   // it's very unlikely that this user is loaded already. It's probably best to just look up the user's devices and send notifications.

   unsigned int user_id = unwrappedPacket->userId;
   unsigned int game_type = unwrappedPacket->gameType;
   unsigned int game_id = unwrappedPacket->gameId;

   int badge_count = CalculateBadgeNumberFromPendingNotifications( user_id, game_type, game_id );

   StoreLastUserNotification( user_id, game_type, game_id, unwrappedPacket->notificationType, unwrappedPacket->additionalText );

   RunQueryAndNotification( m_database, 
                              user_id,
                              game_type,
                              game_id, 
                              badge_count,
                              unwrappedPacket->notificationType,
                              unwrappedPacket->additionalText.c_str(),
                              true );
 /*  if( m_database != NULL )
   {
      string query( "SELECT user_device.device_id, user_device.platformId, user_device.id, user_device_notification.audio_file, user_device_notification.repeat_frequency_in_hours ");
      query += " FROM user_device JOIN user_device_notification "
                     "ON user_device.id=user_device_notification.user_device_id WHERE user_device.user_id='";
      query += boost::lexical_cast< string  >( user_id );
      query += "' AND user_device_notification.game_type='";
      query += boost::lexical_cast< string  >( game_type );
      query += "' AND user_device_notification.is_enabled='1' ";
      query += "AND user_device.is_enabled='1'";

      //cout << query << endl;


      MYSQL *mysql = m_database->GetDbHandle();

      int ret = mysql_query( mysql, query.c_str() );
      if (ret != 0)
      {
         cout << "Error " << mysql_error(mysql)
              << " (code " << ret << ") executing DB query: \""
              << query << "\"" << endl;
         cout << "        error: " << mysql_error(mysql) << endl;
         return false;
      }

      MYSQL_RES *res = mysql_store_result(mysql);
      if( res != NULL )
      {
         MYSQL_ROW row;
         for( row = mysql_fetch_row(res); row; row = mysql_fetch_row(res) )
         {
            if( row[0] == NULL )
            {
               continue;
            }

            unsigned int device_platform;
            sscanf( row[1], "%u", &device_platform );

            unsigned int device_id;
            sscanf( row[2], "%u", &device_id );

            char audioFile[60];
            sscanf( row[3], "%s", audioFile );

            unsigned int repeatFrequencyInHours;
            sscanf( row[4], "%u", &repeatFrequencyInHours );


            if( device_platform == 1 ) // iOS
            {
               NotifyUserDirect_iOS( user_id, 
                  (const unsigned char*)row[0], 
                  game_type, 
                  game_id, 
                  badge_count,
                  audioFile, 
                  (GameNotification)unwrappedPacket->notificationType,
                  unwrappedPacket->additionalText.c_str() );

            }
            else if( device_platform == 2 ) // Android
            {
               NotifyUserDirect_Android( user_id, (const unsigned char*)row[0], game_type, game_id, badge_count,
                  (GameNotification)unwrappedPacket->notificationType, unwrappedPacket->additionalText.c_str() );
            }


            if( repeatFrequencyInHours )
            {
               SetupUserNotificationResend( user_id, game_type, device_id, 60 * repeatFrequencyInHours );
            }
            //SetupUserNotificationResend( user_id, game_type, device_id, 60 );
         }
         mysql_free_result(res);
      }

      //NotifyUser(unwrappedPacket->userId, unwrappedPacket->gameType, unwrappedPacket->gameId,
      //            (GameNotification)unwrappedPacket->notificationType, unwrappedPacket->additionalText.c_str());
   }*/

   //
   return false;
}

//---------------------------------------------------------------
//---------------------------------------------------------------

void     NotificationMainThread::PeriodicCheckForNewNotifications()
{
   time_t currentTime;
   time( &currentTime );

   double elapsed = difftime( currentTime, m_lastNotificationCheck_TimeStamp );
   if( elapsed >= timeoutNotificationSend ) 
   {
      //m_lastNotificationCheck_TimeStamp = ZeroOutMinutes( currentTime );// advance the hour.
      m_lastNotificationCheck_TimeStamp = currentTime;
   
      // hooks for sending notifications

      std::map<UserNotificationKey,UserNotificationRecord>::iterator itt = m_PendingNotifications.begin();
      for( ; itt != m_PendingNotifications.end(); ++itt )
      {
         if( itt->second.resendNotificationDelay == 0 )
         {
            continue;
         }

         if( (unsigned int)(itt->second.lastNotificationTime-currentTime) < itt->second.resendNotificationDelay )
         {
            continue;
         }

         itt->second.lastNotificationTime = currentTime;
         itt->second.resendNotificationDelay = 0;

         
         unsigned int user_id = itt->first.userId;
         unsigned int game_type = itt->first.gameType;
         unsigned int game_id = itt->second.lastNotificationGameId;

         int badge_count = itt->second.notificationCount;

         RunQueryAndNotification( m_database, 
                              user_id,
                              game_type,
                              game_id, 
                              badge_count,
                              itt->second.lastNotificationType, 
                              itt->second.lastNotificationText.c_str(),
                              false );

         /*if( m_database != NULL )
         {
            string query( "SELECT user_device.device_id, user_device.platformId, user_device.id FROM user_device JOIN user_device_notification "
                           "ON user_device.id=user_device_notification.user_device_id WHERE user_device.user_id='");
            query += boost::lexical_cast< string  >( user_id );
            query += "' AND user_device_notification.game_type='";
            query += boost::lexical_cast< string  >( game_type );
            query += "' AND user_device_notification.is_enabled='1' ";
            query += "AND user_device.is_enabled='1'";

            //cout << query << endl;

            MYSQL *mysql = m_database->GetDbHandle();

            int ret = mysql_query( mysql, query.c_str() );
            if (ret != 0)
            {
               cout << "Error " << mysql_error(mysql)
                    << " (code " << ret << ") executing DB query: \""
                    << query << "\"" << endl;
               cout << "        error: " << mysql_error(mysql) << endl;
               break;
            }

            MYSQL_RES *res = mysql_store_result(mysql);
            if( res != NULL )
            {
               MYSQL_ROW row;
               for( row = mysql_fetch_row(res); row; row = mysql_fetch_row(res) )
               {
                  if( row[0] == NULL )
                  {
                     continue;
                  }

                  unsigned int device_platform;
                  sscanf( row[1], "%u", &device_platform );

                  unsigned int device_id;
                  sscanf( row[2], "%u", &device_id );

                  if( device_platform == 1 ) // iOS
                  {
                     NotifyUserDirect_iOS( user_id, (const unsigned char*)row[0], game_type, game_id, badge_count,
                        (GameNotification)itt->second.lastNotificationType, itt->second.lastNotificationText );
                  }
                  else if( device_platform == 2 ) // Android
                  {
                     NotifyUserDirect_Android( user_id, (const unsigned char*)row[0], game_type, game_id, badge_count,
                        (GameNotification)itt->second.lastNotificationType, itt->second.lastNotificationText );
                  }

               }
               mysql_free_result(res);
            }
         }*/
      }

   }
}

//---------------------------------------------------------------
//---------------------------------------------------------------

bool     NotificationMainThread::AddQueryToOutput( PacketDbQuery* query )
{
   ChainLinkIteratorType itOutputs = m_listOfOutputs.begin();
   while( itOutputs != m_listOfOutputs.end() )// only one output currently supported.
   {
      ChainType* outputPtr = static_cast< ChainType*> ( (*itOutputs).m_interface );
      if( outputPtr->AddInputChainData( query, m_chainId ) == true )
      {
         return true;
      }
      itOutputs++;
   }

   BasePacket* packet = static_cast<BasePacket*>( query );
   PacketFactory factory;
   factory.CleanupPacket( packet );/// normally, we'd leave this up to the invoker to cleanup. 
   return false;
}

//---------------------------------------------------------------

void     NotificationMainThread::RemoveExpiredConnections()
{
   m_mutex.lock();
   UserConnectionIterator it = m_userConnectionMap.begin();
   while( it != m_userConnectionMap.end() )
   {
      UserConnection& contact = it->second;
      UserConnectionIterator temp = it++;
      if( contact.IsLoggedOut() )
      {
         if( contact.SecondsExpiredSinceLoggedOut() > SecondsBeforeRemovingLoggedOutUser )
         {
            //m_userLookupById.erase( contact.GetUserInfo().id );
            
            m_userConnectionMap.erase( temp );
         }
      }
      else 
      {
         contact.Update();
      }
   }
   m_mutex.unlock();
}

//---------------------------------------------------------------
//---------------------------------------------------------------

int     NotificationMainThread::MainLoop_InputProcessing()
{
   PacketFactory factory;

   m_mutex.lock();
   deque< PacketDbQueryResult* > tempQueue = m_dbQueries;
   m_dbQueries.clear();
   m_mutex.unlock();

   deque< PacketDbQueryResult* >::iterator it = tempQueue.begin();
   while( it != tempQueue.end() )
   {
      PacketDbQueryResult* dbResult = *it++;

      U32 connectionId = dbResult->id;
      if( connectionId != 0 )
      {
         UserConnectionIterator it = m_userConnectionMap.find( connectionId );
         if( it != m_userConnectionMap.end() )
         {
            it->second.HandleDbQueryResult( dbResult );
         }
         else // user must have disconnected
         {
            BasePacket* packet = static_cast<BasePacket*>( dbResult );
            factory.CleanupPacket( packet );
         }
      }
      else
      {
         // locally handled query
         // LOOK HERE GARY FOR async queries
      }
   }
   return 1;
}

//---------------------------------------------------------------

int      NotificationMainThread::MainLoop_OutputProcessing()
{
   UpdateAllConnections();

   time_t currentTime;
   time( &currentTime );

   int numClients = static_cast< int >( m_connectedClients.size() );
   UpdateConsoleWindow( m_timeOfLastTitleUpdate, m_uptime, m_numTotalConnections, numClients, m_listeningPort, m_serverName );

   PeriodicCheckForNewNotifications();
   RemoveExpiredConnections();

   if( m_database == NULL )
   {
      ChainLinkIteratorType itOutputs = m_listOfOutputs.begin();
      while( itOutputs != m_listOfOutputs.end() )// only one output currently supported.
      {
         ChainType* outputPtr = static_cast< ChainType*> ( (*itOutputs).m_interface );
         Database::Deltadromeus* db = dynamic_cast< Database::Deltadromeus*> ( outputPtr );
         if( db != NULL )
         {
            m_database = db;
            break;
         }
         itOutputs++;
      }
   }

   return 1;
}

//---------------------------------------------------------------

bool NotificationMainThread::HandleUpdateNotificationCount( const PacketNotification_UpdateNotificationCount* unwrappedPacket )
{
   UserNotificationKey key;
   key.userId = unwrappedPacket->userId;
   key.gameType = unwrappedPacket->gameType;

   int notification_count = unwrappedPacket->notificationCount;


   std::map<UserNotificationKey, UserNotificationRecord>::iterator i = m_PendingNotifications.lower_bound(key);
   if( i == m_PendingNotifications.end() || i->first != key )
   {
      if( notification_count > 0 )
      {
         UserNotificationRecord record;
         record.notificationCount = notification_count;
         record.lastNotificationGameId = 0;
         record.lastNotificationType = 0;
         record.lastNotificationText.clear();
         record.resendNotificationDelay = 0;
         record.lastNotificationTime = 0;

         m_PendingNotifications.insert(i, std::pair<UserNotificationKey,UserNotificationRecord>(key,record) );
      }
   }
   else
   {
      if( notification_count > 0 )
      {
         i->second.notificationCount = notification_count;

         i->second.lastNotificationGameId = 0;
         i->second.lastNotificationType = 0;
         i->second.lastNotificationText.clear();
         i->second.resendNotificationDelay = 0;
         i->second.lastNotificationTime = 0;
      }
      else
      {
         m_PendingNotifications.erase(i);
      }
   }

   return true;
}

//---------------------------------------------------------------

int NotificationMainThread::CalculateBadgeNumberFromPendingNotifications( unsigned int userId, unsigned int gameType, int gameId )
{
   // how many notifications are outstanding for this user?
   int notification_count = 1;
   UserNotificationKey key;
   key.userId = userId;
   key.gameType = gameType;

   std::map<UserNotificationKey,UserNotificationRecord>::iterator i = m_PendingNotifications.lower_bound(key);
   if (i == m_PendingNotifications.end() || i->first != key)
   {
      UserNotificationRecord record;
      record.notificationCount = notification_count;
      record.lastNotificationGameId = 0;
      record.lastNotificationType = 0;
      record.lastNotificationText.clear();
      record.resendNotificationDelay = 0;
      record.lastNotificationTime = 0;

      m_PendingNotifications.insert(i, std::pair<UserNotificationKey,UserNotificationRecord>(key,record));
   }
   else
   {
      notification_count = ++i->second.notificationCount;
   }

   return notification_count;
}


bool NotificationMainThread::StoreLastUserNotification( unsigned int userId, unsigned int gameType, int gameId,
                                                      unsigned int notificationType, string additionalText )
{
   UserNotificationKey key;
   key.userId = userId;
   key.gameType = gameType;

   std::map<UserNotificationKey,UserNotificationRecord>::iterator i = m_PendingNotifications.lower_bound(key);
   if (i == m_PendingNotifications.end() || i->first != key)
   {
      UserNotificationRecord record;
      record.notificationCount = 1;
      record.lastNotificationGameId = gameId;
      record.lastNotificationType = notificationType;
      record.lastNotificationText = additionalText;
      record.resendNotificationDelay = 0;
      record.lastNotificationTime = 0;

      m_PendingNotifications.insert(i, std::pair<UserNotificationKey,UserNotificationRecord>(key,record));
   }
   else
   {
      i->second.lastNotificationGameId = gameId;
      i->second.lastNotificationType = notificationType;
      i->second.lastNotificationText = additionalText;
      i->second.resendNotificationDelay = 0;
      i->second.lastNotificationTime = 0;
   }

   return true;
}

 bool NotificationMainThread::SetupUserNotificationResend( unsigned int userId, unsigned int gameType,
                                                         unsigned int deviceId, unsigned delayTime )
 {
   UserNotificationKey key;
   key.userId = userId;
   key.gameType = gameType;

   std::map<UserNotificationKey,UserNotificationRecord>::iterator i = m_PendingNotifications.lower_bound(key);
   if (i == m_PendingNotifications.end() || i->first != key)
   {
      return false;
   }
   else
   {
      i->second.resendNotificationDelay = delayTime;
      time( &i->second.lastNotificationTime );
   }

   return true;
 }

//---------------------------------------------------------------
//---------------------------------------------------------------
