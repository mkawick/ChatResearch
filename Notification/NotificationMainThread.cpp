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
#include "../NetworkCommon/Utils/StringUtils.h"

#include "../NetworkCommon/Packets/ServerToServerPacket.h"
#include "../NetworkCommon/Packets/PacketFactory.h"
#include "../NetworkCommon/Packets/NotificationPacket.h"

#include "../NetworkCommon/Database/Deltadromeus.h"// breaking rules to make things easier for GARY
#include <mysql/mysql.h> // MySQL Include File


#include "NotificationMainThread.h"

#include "../NetworkCommon/Database/StringLookup.h"
#include "../NetworkCommon/NetworkIn/DiplodocusTools.h"

#include "server_notify.h"

#include <openssl/ssl.h>
#include <openssl/err.h>

//extern MYSQL *g_NotificationMYSQL;

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

NotificationMainThread::NotificationMainThread( const string& serverName, U32 serverId ): Queryer(), ChainedType( serverName, serverId, 0,  ServerType_Notification ), m_database( NULL )
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

   ChainedType::Init();
   
   NotifyIosInit( iosPathToCertAndKeyFile.c_str() ); 
   NotifyAndroidInit(); 
}
///////////////////////////////////////////////////////////////////

void     NotificationMainThread::ServerWasIdentified( IChainedInterface* khaan )
{
   BasePacket* packet = NULL;
   PackageForServerIdentification( m_serverName, m_localIpAddress, m_externalIpAddress, m_serverId, m_serverType, m_listeningPort, m_gameProductId, m_isGame, m_isControllerApp, true, m_gatewayType, &packet );
   Khaan* localKhaan = static_cast< Khaan* >( khaan );
   localKhaan->AddOutputChainDataNoLock( packet );
   // this is not thread safe, but will be invoked from within the same thread.
   m_clientsNeedingUpdate.push_back( localKhaan->GetChainedId() );
}

///////////////////////////////////////////////////////////////////

bool     NotificationMainThread::AddInputChainData( BasePacket* packet, U32 gatewayId )
{
   m_mutex.lock();
   m_inputPacketsToBeProcessed.push_back( PacketStorage( packet, gatewayId ) );
   m_mutex.unlock();

   return true;
}

//---------------------------------------------------------------

bool     NotificationMainThread:: ProcessPacket( PacketStorage& storage )
{
   cout << "NotificationMainThread::ProcessPacket enter" << endl;

   BasePacket* packet = storage.packet;
   U32 gatewayId = storage.gatewayId;

   U8 packetType = packet->packetType;
   
   if( packetType == PacketType_GatewayInformation )
   {
      PacketCleaner cleaner( packet );// we do not accept any data from the gateway
      HandleCommandFromGateway( packet, gatewayId );
      return true;
   }

   if( packetType == PacketType_ServerJobWrapper )// login and such
   {
      cout << "NotificationMainThread::ProcessPacket ServerJob" << endl;
      PacketCleaner cleaner( packet );
      HandlePacketFromOtherServer( packet, gatewayId );
      return true;
   }

   if( packetType == PacketType_GatewayWrapper ) 
   {
      cout << "NotificationMainThread::ProcessPacket HandlePacketFromGateway" << endl;
      if( HandlePacketFromGateway( packet, gatewayId ) == true )
      {
         PacketCleaner cleaner( packet );
      }
      else
      {
         cout << "NotificationMainThread::HandlePacketFromGateway HandleRequestFromClient" << endl;
         m_listOfDelayedPackets.push_back( PacketStorage( storage ) );
      }
      return true;
   }
   
   return false;
}

//---------------------------------------------------------------

void  NotificationMainThread::ProcessDelayedPackets()
{
   if( m_listOfDelayedPackets.size() == 0 )
      return;

   LogMessage( LOG_PRIO_INFO, "NotificationMainThread::ProcessDelayedPackets: ", m_listOfDelayedPackets.size() );

   deque< PacketStorage > storage = m_listOfDelayedPackets;
   m_listOfDelayedPackets.clear();

   deque< PacketStorage >:: iterator it = storage.begin();
   while( it != storage.end() )
   {
      PacketStorage& ps = *it;
      PacketServerJobWrapper* wrapper = static_cast< PacketServerJobWrapper* >( ps.packet );
      BasePacket* unwrappedPacket = wrapper->pPacket;
      U32  connectionId = wrapper->serverId;

      LogMessage( LOG_PRIO_INFO, "ProcessDelayedPackets:FindUserConnection( %u )", connectionId );
      UserConnectionIterator item = m_userConnectionMap.find( connectionId );
      if( item != m_userConnectionMap.end() )
      {
         LogMessage( LOG_PRIO_INFO, "ProcessDelayedPackets:FindUserConnection success" );
         UserConnection& user = item->second;
         if( user.IsReadyToAcceptClientRequests() == true )
         {
            LogMessage( LOG_PRIO_INFO, "ProcessDelayedPackets:handling the packet" );
            user.HandleRequestFromClient( unwrappedPacket );
            it = storage.erase( it );
         }
         else
         {
            LogMessage( LOG_PRIO_INFO, "ProcessDelayedPackets:restoring the packet" );
            it ++; // advance without removing this packet
         }
      }
      else //user does not exist
      {
         LogMessage( LOG_PRIO_INFO, "ProcessDelayedPackets:FindUserConnection failed" );
      
         BasePacket* packet = ps.packet;
         PacketCleaner cleaner( packet );
         it = storage.erase( it );
      }
   }

   LogMessage( LOG_PRIO_INFO, "ProcessDelayedPackets: pushing packets back into storage: %u", storage.size() );
   // save any remaining packets
   m_listOfDelayedPackets = storage;
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
            cout << "AddOutputChainData: Non-null custom data " );*/
      }
      return true;
   }
   return false;
}
//---------------------------------------------------------------

bool     NotificationMainThread::SendMessageToClient( BasePacket* packet, U32 connectionId, U32 gatewayId )
{
   if( packet->packetType == PacketType_GatewayWrapper )// this is already wrapped up and ready for the gateway... send it on.
   {
      Threading::MutexLock locker( m_mutex );

      ClientMapIterator itInputs = m_connectedClients.begin();
      while( itInputs != m_connectedClients.end() )// only one output currently supported.
      {
         KhaanServerToServer* khaan = static_cast< KhaanServerToServer* >( itInputs->second );
         if( khaan->GetChainedType() == ChainedType_InboundSocketConnector && 
            khaan->GetServerId() == gatewayId )
         {
            khaan->AddOutputChainData( packet );
            m_clientsNeedingUpdate.push_back( khaan->GetChainedId() );
            return true;
         }
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


//---------------------------------------------------------------

bool     NotificationMainThread::HandlePacketFromGateway( BasePacket* packet, U32 gatewayId )
{
   cout << "NotificationMainThread::HandlePacketFromGateway enter" << endl;
   U8 packetType = packet->packetType;
   if( packetType != PacketType_GatewayWrapper )
   {
      return true;
   }

   PacketServerJobWrapper* wrapper = static_cast< PacketServerJobWrapper* >( packet );
   BasePacket* unwrappedPacket = wrapper->pPacket;
   U32  connectionId = wrapper->serverId;
   packetType = unwrappedPacket->packetType;

   
   {// local scope
      //Threading::MutexLock locker( m_mutex );
      UserConnectionIterator item = m_userConnectionMap.find( connectionId );
      if( item != m_userConnectionMap.end() )
      {
         UserConnection& user = item->second;
         if( user.IsReadyToAcceptClientRequests() == true )
         {
            cout << "NotificationMainThread::HandlePacketFromGateway HandleRequestFromClient" << endl;
            user.HandleRequestFromClient( unwrappedPacket );
            return true;
         }
         else
         {
            cout << "NotificationMainThread::HandlePacketFromGateway nope" << endl;
            return false;// save this packet for later consumption
         }
      }
   }
   // user does not exist... simply cleanup
   return true;
}

//---------------------------------------------------------------

bool     NotificationMainThread::ConnectUser( const PacketPrepareForUserLogin* loginPacket )
{
   U32 connectionId = loginPacket->connectionId;
   string uuid = loginPacket->uuid;
   U32 gatewayId = loginPacket->gatewayId;
   //LogMessage( LOG_PRIO_INFO, "Prep for logon: %d, %s, %s, %s", connectionId, loginPacket->userName.c_str(), uuid.c_str(), loginPacket->password.c_str() );
   LogMessage_LoginPacket( loginPacket );
   
   UserConnectionIterator it = m_userConnectionMap.find( connectionId );// don't do anything if this user is already logged in.
   if( it != m_userConnectionMap.end() )
      return false;

   bool found = false;
   // if the user is already here but relogged, simply add a new entry using the same basic setup
   //m_mutex.lock();
      it = m_userConnectionMap.begin();
      while( it != m_userConnectionMap.end() )
      {
         if( it->second.GetUserInfo().uuid == loginPacket->uuid ) 
         {
            found = true;
            it->second.SetConnectionId( connectionId );
            it->second.SetGatewayId( gatewayId );
            
            it->second.Relog();
            m_userConnectionMap.insert( UserConnectionPair( connectionId, it->second ) );
            m_userConnectionMap.erase( it );
            break;
         }
         it++;
      }
   //m_mutex.unlock();

   if( found == false )
   {
      UserConnection user( loginPacket );
      user.SetServer( this );

      //m_mutex.lock();
      m_userConnectionMap.insert( UserConnectionPair( connectionId, user ) );
      //m_mutex.unlock();
   }
   return true;
}


//---------------------------------------------------------------

bool     NotificationMainThread::DisconnectUser( const PacketPrepareForUserLogout* logoutPacket )
{
   //LogMessage( LOG_PRIO_INFO, "Prep for logout: %d, %s", logoutPacket->connectionId, logoutPacket->uuid.c_str() );
   LogMessage_LogoutPacket( logoutPacket );

   U32 connectionId = logoutPacket->connectionId;

   //m_mutex.lock();
   UserConnectionIterator it = m_userConnectionMap.find( connectionId );
   //m_mutex.unlock();
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
      // the following query is simply for validation of the user account and for printing results.
      MYSQL *mysql = database->GetDbHandle();
      string userQuery( "SELECT user_name, active, last_login_timestamp FROM users WHERE user_id=" );
      userQuery += boost::lexical_cast< string  >( user_id );

      string   userName;
      //bool     isActive = false;
      string   lastTimeLoggedIn;

      int ret = mysql_query( mysql, userQuery.c_str() );
      if (ret != 0)
      {
         LogMessage( LOG_PRIO_ERR, "Error %s (code %d) executing DB query: \"%s\"", mysql_error(mysql), ret, userQuery.c_str() );
         return;
      }
      {
         MYSQL_RES *res = mysql_store_result(mysql);
         if( res != NULL )
         {
            if( res->row_count == 0 )
            {
               LogMessage( LOG_PRIO_ERR, " User does not exist for DB query: user_id=%d\"%s\"", user_id, userQuery.c_str() );
               return;
            }
            
            MYSQL_ROW row = mysql_fetch_row( res );
            userName = row[0];
            string tempIsActive = ( row[1] != NULL ) ? row[1]:"0";
            //isActive = (tempIsActive == "1")? true:false;
            lastTimeLoggedIn = row[2];

            mysql_free_result(res);
         }
      }

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

      //cout << query );


      ret = mysql_query( mysql, query.c_str() );
      if (ret != 0)
      {
         LogMessage( LOG_PRIO_ERR, "Error %s (code %d) executing DB query: \"%s\"", mysql_error(mysql), ret, query.c_str() );
         //cout << "        error: " << mysql_error(mysql) );
         return;
      }

      MYSQL_RES *res = mysql_store_result(mysql);
      if( res != NULL && res->row_count > 0 )
      {
         LogMessage( LOG_PRIO_INFO, "---------------------------------------------------------" );
         LogMessage( LOG_PRIO_INFO, "RunQueryAndNotification: user_id=%d, name=\"%s\"", user_id, userName.c_str() );
         LogMessage( LOG_PRIO_INFO, "  num devices to send notifications: %d", res->row_count );
         //LogMessage( LOG_PRIO_INFO, "RunQueryAndNotification: user_id=%d, name=\"%s\"", user_id, userName.c_str() );

         int count = 0;
         MYSQL_ROW row;
         for( row = mysql_fetch_row(res); row; row = mysql_fetch_row(res) )
         {
            ++count;
            if( row[0] == NULL )
            {
               continue;
            }
            U32 deviceIdLen = strlen( row[0] );
            if( deviceIdLen > 32 )
               deviceIdLen= 32;
            if( deviceIdLen == 0 )
            {
               continue;
            }
            string deviceIdHexDump;
            deviceIdHexDump.reserve( 3*deviceIdLen + 10 );
            for( U32 i=0; i<deviceIdLen; i++ )
            {
               deviceIdHexDump += ToHexString( (U8) (row[0][i]) ) + " ";
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
               LogMessage( LOG_PRIO_INFO, "  %d) NotifyUserDirect_iOS .. deviceId = [ %s ]", count, deviceIdHexDump.c_str() );
               NotifyUserDirect_iOS( user_id, 
                  (const unsigned char*)row[0], 
                  game_type, 
                  game_id, 
                  badge_count,
                  audioFile, 
                  (GameNotification)notificationType,
                  additionalText );
            }
            else if( device_platform == 2 ) // Android
            {
               LogMessage( LOG_PRIO_INFO, "  %d) NotifyUserDirect_Android .. deviceId = [ %s ]", count, deviceIdHexDump.c_str() );
               NotifyUserDirect_Android( user_id,
                  (const unsigned char*)row[0],
                  game_type,
                  game_id,
                  badge_count,
                  (GameNotification)notificationType,
                  additionalText );
            }

            if( shouldSetupResendNotifications == true && repeatFrequencyInHours > 0 )
            {
               SetupUserNotificationResend( user_id, game_type, device_id, 60 * 60 * repeatFrequencyInHours );
            }
         }
         mysql_free_result(res);
         
         LogMessage( LOG_PRIO_INFO, "RunQueryAndNotification complete" );
         LogMessage( LOG_PRIO_INFO, "---------------------------------------------------------" );
         
      }


      //NotifyUser(unwrappedPacket->userId, unwrappedPacket->gameType, unwrappedPacket->gameId,
      //            (GameNotification)unwrappedPacket->notificationType, unwrappedPacket->additionalText.c_str());
   }
}

//---------------------------------------------------------------

bool     NotificationMainThread::HandleNotification( const PacketNotification_SendNotification* unwrappedPacket )
{
   cout << "NotificationMainThread::HandleNotification enter" << endl;
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
   
   cout << "NotificationMainThread::HandleNotification exit" << endl;
   return false;
}

//---------------------------------------------------------------
//---------------------------------------------------------------

void     NotificationMainThread::SetupNotificationsToSendImmediately()
{
   m_lastNotificationCheck_TimeStamp = 0;// simplest way to guarantee that we fire again immediately
}

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

      std::map< UserNotificationKey, UserNotificationRecord >::iterator itt = m_PendingNotifications.begin();
      for( ; itt != m_PendingNotifications.end(); ++itt )
      {
         if( itt->second.resendNotificationDelaySeconds == 0 )
         {
            continue;
         }

         if( (unsigned int)(itt->second.lastNotificationTime-currentTime) < itt->second.resendNotificationDelaySeconds )
         {
            continue;
         }

         itt->second.lastNotificationTime = currentTime;
         itt->second.resendNotificationDelaySeconds = 0;

         cout << "NotificationMainThread::SetupNotificationsToSendImmediately enter" << endl;
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
         cout << "NotificationMainThread::SetupNotificationsToSendImmediately exit" << endl;

      }

   }
}

//---------------------------------------------------------------
//---------------------------------------------------------------

bool     NotificationMainThread::AddQueryToOutput( PacketDbQuery* query )
{
   cout << "NotificationMainThread::AddQueryToOutput enter" << endl;
   ChainLinkIteratorType itOutputs = m_listOfOutputs.begin();
   while( itOutputs != m_listOfOutputs.end() )// only one output currently supported.
   {
      ChainType* outputPtr = static_cast< ChainType*> ( (*itOutputs).m_interface );
      if( outputPtr->AddInputChainData( query, m_chainId ) == true )
      {
         cout << "NotificationMainThread::AddQueryToOutput success" << endl;
         return true;
      }
      itOutputs++;
   }

   cout << "NotificationMainThread::AddQueryToOutput fail" << endl;
   BasePacket* packet = static_cast<BasePacket*>( query );
   PacketFactory factory;
   factory.CleanupPacket( packet );/// normally, we'd leave this up to the invoker to cleanup. 
   return false;
}

//---------------------------------------------------------------

void     NotificationMainThread::RemoveExpiredConnections()
{
   UserConnectionIterator it = m_userConnectionMap.begin();
   while( it != m_userConnectionMap.end() )
   {
      UserConnection& contact = it->second;
      UserConnectionIterator temp = it++;
      if( contact.IsLoggedOut() )
      {
         cout << "NotificationMainThread::RemoveExpiredConnections enter" << endl;
         if( contact.SecondsExpiredSinceLoggedOut() > SecondsBeforeRemovingLoggedOutUser )
         {
            //m_userLookupById.erase( contact.GetUserInfo().id );
            
            m_userConnectionMap.erase( temp );
         }
         cout << "NotificationMainThread::RemoveExpiredConnections exit" << endl;
      }
      else 
      {
         contact.Update();
      }
   }
}

//---------------------------------------------------------------
//---------------------------------------------------------------


void     NotificationMainThread::UpdateDbResults()
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
}

//---------------------------------------------------------------

void     NotificationMainThread::FindDatabaseAmongOutgoingConnections()
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

//---------------------------------------------------------------

int      NotificationMainThread::CallbackFunction()
{
   UpdateDbResults();

   CleanupOldClientConnections( "KhaanServerToServer" );
   UpdateAllConnections( "KhaanServerToServer" );
   UpdateInputPacketToBeProcessed();

   CommonUpdate();

   ProcessDelayedPackets();
   //time_t currentTime;
   //time( &currentTime );

   //int numClients = static_cast< int >( m_connectedClients.size() );
   //UpdateConsoleWindow( m_timeOfLastTitleUpdate, m_uptime, m_numTotalConnections, numClients, m_listeningPort, m_serverName );

   PeriodicCheckForNewNotifications();
   RemoveExpiredConnections();

   if( m_database == NULL )
   {
      FindDatabaseAmongOutgoingConnections();
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


   std::map<UserNotificationKey, UserNotificationRecord>::iterator it = m_PendingNotifications.lower_bound(key);
   if( it == m_PendingNotifications.end() || it->first != key )
   {
      if( notification_count > 0 )
      {
         UserNotificationRecord record;
         record.notificationCount = notification_count;
         record.lastNotificationGameId = 0;
         record.lastNotificationType = 0;
         record.lastNotificationText.clear();
         record.resendNotificationDelaySeconds = 0;
         record.lastNotificationTime = 0;

         m_PendingNotifications.insert(it, std::pair<UserNotificationKey,UserNotificationRecord>(key,record) );
      }
   }
   else
   {
      if( notification_count > 0 )
      {
         it->second.notificationCount = notification_count;

         it->second.lastNotificationGameId = 0;
         it->second.lastNotificationType = 0;
         it->second.lastNotificationText.clear();
         it->second.resendNotificationDelaySeconds = 0;
         it->second.lastNotificationTime = 0;
      }
      else
      {
         m_PendingNotifications.erase(it);
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

   std::map< UserNotificationKey, UserNotificationRecord >::iterator it = m_PendingNotifications.lower_bound(key);
   if (it == m_PendingNotifications.end() || it->first != key)
   {
      UserNotificationRecord record;
      record.notificationCount = notification_count;
      record.lastNotificationGameId = 0;
      record.lastNotificationType = 0;
      record.lastNotificationText.clear();
      record.resendNotificationDelaySeconds = 0;
      record.lastNotificationTime = 0;

      m_PendingNotifications.insert(it, std::pair<UserNotificationKey,UserNotificationRecord>(key,record));
   }
   else
   {
      notification_count = ++it->second.notificationCount;
   }

   return notification_count;
}

//---------------------------------------------------------------

bool NotificationMainThread::StoreLastUserNotification( unsigned int userId, unsigned int gameType, int gameId,
                                                      unsigned int notificationType, string additionalText )
{
   UserNotificationKey key;
   key.userId = userId;
   key.gameType = gameType;

   std::map< UserNotificationKey, UserNotificationRecord >::iterator i = m_PendingNotifications.lower_bound(key);
   if (i == m_PendingNotifications.end() || i->first != key)
   {
      UserNotificationRecord record;
      record.notificationCount = 1;
      record.lastNotificationGameId = gameId;
      record.lastNotificationType = notificationType;
      record.lastNotificationText = additionalText;
      record.resendNotificationDelaySeconds = 0;
      record.lastNotificationTime = 0;

      m_PendingNotifications.insert(i, std::pair<UserNotificationKey,UserNotificationRecord>(key,record));
   }
   else
   {
      i->second.lastNotificationGameId = gameId;
      i->second.lastNotificationType = notificationType;
      i->second.lastNotificationText = additionalText;
      i->second.resendNotificationDelaySeconds = 0;
      i->second.lastNotificationTime = 0;
   }

   return true;
}

//---------------------------------------------------------------

bool NotificationMainThread::SetupUserNotificationResend( unsigned int userId, unsigned int gameType,
                                                         unsigned int deviceId, unsigned delayTimeSeconds )
{
   UserNotificationKey key;
   key.userId = userId;
   key.gameType = gameType;

   std::map< UserNotificationKey, UserNotificationRecord >::iterator i = m_PendingNotifications.lower_bound(key);
   if (i == m_PendingNotifications.end() || i->first != key)
   {
      return false;
   }
   else
   {
      i->second.resendNotificationDelaySeconds = delayTimeSeconds;
      time( &i->second.lastNotificationTime );
   }

   return true;
 }

//---------------------------------------------------------------
//---------------------------------------------------------------
