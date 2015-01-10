#include "../NetworkCommon/ServerConstants.h"

#if PLATFORM == PLATFORM_WINDOWS
#pragma warning( disable:4996 )
#endif

#include "UserConnection.h"

#include <string>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
using boost::format;

#include "../NetworkCommon/Packets/PacketFactory.h"
#include "../NetworkCommon/Packets/DbPacket.h"
#include "../NetworkCommon/Utils/Utils.h"
#include "../NetworkCommon/Utils/StringUtils.h"

#include "NotificationMainThread.h"
#include "NotificationCommon.h"

#include "server_notify.h"

//////////////////////////////////////////////////////////////


NotificationMainThread* UserConnection::m_mainThread = NULL;

//////////////////////////////////////////////////////////////

UserConnection::UserConnection( ) :
                                 m_isLoggedOut( false ), 
                                 m_hasRequestedDevices( false ),
                                 m_finishedLoadingListOfDevices( false ),
                                 m_finishedLoadingListOfDevicesPerGame( false )
{
}

UserConnection::~UserConnection()
{
}
/*
void     UserConnection::UserLoggedOut()
{
   m_isLoggedOut = true; 
   time( &m_timeLoggedOut );
}


int      UserConnection::SecondsExpiredSinceLoggedOut()
{
   if( m_timeLoggedOut == 0 )
      return 0;

   time_t currentTime;
   time( &currentTime );

   return  static_cast<int> ( difftime( currentTime, m_timeLoggedOut ) );
}

void     UserConnection::Relog()
{
   m_timeLoggedOut = 0;
   m_isLoggedOut = false;
}*/

bool     UserConnection::IsReadyToAcceptClientRequests() const
{
   if( m_hasRequestedDevices == true && 
       m_finishedLoadingListOfDevices == true && 
       m_finishedLoadingListOfDevicesPerGame == true )
      return true;
   return false;
}

//------------------------------------------------------------

void     UserConnection::RequestAllDevicesForUser()
{
   cout << "UserConnection::RequestAllDevicesForUser" << endl;
   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =           m_userId;//m_userInfo.connectionId;
   dbQuery->meta =         "";
   dbQuery->lookup =       QueryType_DeviceList;
   dbQuery->serverLookup = 0;
   dbQuery->query = "SELECT * FROM user_device WHERE user_uuid='";
   dbQuery->query += m_userUuid;
   dbQuery->query += "'";

   //cout, dbQuery->query );

   m_mainThread->AddQueryToOutput( dbQuery );
}

//------------------------------------------------------------

void  UserConnection::RequestAllDeviceNotification()
{
   cout << "UserConnection::RequestAllDevicesForUser" << endl;
   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =           m_userId;//m_userInfo.connectionId;
   dbQuery->meta =         "";
   dbQuery->lookup =       QueryType_DevicePerGameList;
   dbQuery->serverLookup = 0;
   dbQuery->query = "SELECT * FROM user_device_notification WHERE user_device_id IN ( select id from user_device where user_uuid='";
   dbQuery->query += m_userUuid;
   dbQuery->query += "')";

   //cout, dbQuery->query );

   m_mainThread->AddQueryToOutput( dbQuery );
}

//------------------------------------------------------------

void  UserConnection::Update()
{
   if( m_hasRequestedDevices == false &&
         m_deviceList.size() == 0 )
   {
      RequestAllDevicesForUser();

      RequestAllDeviceNotification();

      m_hasRequestedDevices = true;
      

      /*select * from user_device_notification where id in 
( select id from user_device where user_uuid="user8")*/
   }
}

//////////////////////////////////////////////////////////////

bool  UserConnection::HandleDbQueryResult( const PacketDbQueryResult* result )
{
   cout << "UserConnection::RequestAllDeviceNotification" << endl;
   PacketFactory factory;
   switch( result->lookup )
   {
   case QueryType_DeviceList:
      {
         StoreListOfDevices( result );
      }
      return true;
   case QueryType_DevicePerGameList:
      {
         StoreDevicesPerGameList( result );
      }
      return true;
   case QueryType_InsertDevice:
      {
         CreateEnabledNotificationEntry( result );
      }
      return true;
   }
   return false;
}

//------------------------------------------------------------

void  UserConnection::StoreListOfDevices( const PacketDbQueryResult* dbResult )
{
   cout << "UserConnection::StoreListOfDevices" << endl;
   m_finishedLoadingListOfDevices = true;
   if( dbResult->successfulQuery == false )
   {
      UserConnectionList connectionList;
      m_connectionDetails.AssembleAllConnections( connectionList );
      m_mainThread->SendErrorToClient( connectionList, PacketErrorReport::ErrorType_Notification_NoDevicesListed );
      return;
   }
   m_deviceList.clear();

   UserDeviceTable            enigma( dbResult->bucket );
   UserDeviceTable::iterator  it = enigma.begin();
   while( it != enigma.end() )
   {
      UserDeviceTable::row       row = *it++;

      ExtendedRegisteredDevice rd;
      rd.userDeviceId =                      boost::lexical_cast< int  >( row[ TableUserDevice::Column_id ] );
      rd.name =                              row[ TableUserDevice::Column_name ];
      rd.uuid =                              row[ TableUserDevice::Column_device_uuid ];
      rd.iconId =                            boost::lexical_cast< int  >( row[ TableUserDevice::Column_icon_id ] );
      rd.deviceId =                          row[ TableUserDevice::Column_device_id ];
      rd.platformId =                        boost::lexical_cast< int  >( row[ TableUserDevice::Column_platformId ] );
      rd.isEnabled =                         boost::lexical_cast< bool  >( row[ TableUserDevice::Column_is_enabled ] );
   
      m_deviceList.push_back( rd );
   }
}

//------------------------------------------------------------

void  UserConnection::StoreDevicesPerGameList( const PacketDbQueryResult* dbResult )
{
   cout << "UserConnection::StoreDevicesPerGameList" << endl;
   m_finishedLoadingListOfDevicesPerGame = true;
   if( dbResult->successfulQuery == false )
   {
      UserConnectionList connectionList;
      m_connectionDetails.AssembleAllConnections( connectionList );
      m_mainThread->SendErrorToClient( connectionList, PacketErrorReport::ErrorType_Notification_NoDevicesEnabledForThisGame );
      return;
   }

   m_deviceEnabledList.clear();

   UserDeviceNotificationsTable            enigma( dbResult->bucket );
   UserDeviceNotificationsTable::iterator  it = enigma.begin();
   while( it != enigma.end() )
   {
      UserDeviceNotificationsTable::row       row = *it++;

      UserDeviceNotifications dn;
      dn.id =                                boost::lexical_cast< int  >( row[ TableUserDeviceNotifications::Column_id ] );
      dn.userDeviceId =                      boost::lexical_cast< int  >( row[ TableUserDeviceNotifications::Column_user_device_id ] );
      dn.gameType =                          boost::lexical_cast< int  >( row[ TableUserDeviceNotifications::Column_game_type ] );
      dn.isEnabled =                         boost::lexical_cast< bool >( row[ TableUserDeviceNotifications::Column_is_enabled ] );
      dn.deviceId =                          row[ TableUserDeviceNotifications::Column_device_id ];

      dn.audioFile =                         row[ TableUserDeviceNotifications::Column_audio_file ];
      dn.repeatFrequencyInHours =            boost::lexical_cast< int  >( row[ TableUserDeviceNotifications::Column_repeat_frequency_in_hours ] );
      if( dn.deviceId == "null" )
         dn.deviceId.clear();

      m_deviceEnabledList.push_back( dn );
   }
}

//------------------------------------------------------------

bool  UserConnection::HandleRequestFromClient( const BasePacket* packet, U32 connectionId )
{
   cout << "UserConnection::HandleRequestFromClient" << endl;
   U8 packetType = packet->packetType;
   PacketFactory factory;
   switch( packetType )
   {
   case PacketType_Notification:
      {
         switch( packet->packetSubType )
         {
         case PacketNotification::NotificationType_TestNotification:
            {
               const PacketNotification_TestNotification* test = static_cast< const PacketNotification_TestNotification* >( packet );
               TestNotification( test->message, test->type, connectionId );
            }
            break;
         case PacketNotification::NotificationType_RegisterDevice:
            {
               const PacketNotification_RegisterDevice* registerDevice = static_cast< const PacketNotification_RegisterDevice* >( packet );
               RegisterNewDevice( registerDevice, connectionId );
            }
            break;
        /* case PacketNotification::NotificationType_RegisterDevice:
            {
            }
            break;*/
         case PacketNotification::NotificationType_UpdateDevice:
            {
               const PacketNotification_UpdateDevice* device = static_cast< const PacketNotification_UpdateDevice* >( packet );
               UpdateDevice( device, connectionId );
            }
            break;
         case PacketNotification::NotificationType_RequestListOfDevices:  // based on product.. 0 means all devices
            {
               const PacketNotification_RequestListOfDevices* request = static_cast< const PacketNotification_RequestListOfDevices* >( packet );
               RequestDevicesList( request, connectionId );
            }
            break;
         case PacketNotification::NotificationType_RemoveDevice: 
            {
               const PacketNotification_RemoveDevice* removal = static_cast< const PacketNotification_RemoveDevice* >( packet );
               RemoveDevice( removal, connectionId );
            }
            break;
       /*  case PacketNotification::GamePacketType_Notification: 
            {
               const PacketGame_Notification* removal = static_cast< const PacketGame_Notification* >( packet );
               UserRequestedNotification( removal );
            }
            break;*/
         }
      }
      break;
   
   }
   return false;
}

//------------------------------------------------------------
#define DEVICE_TOKEN_LEN   1024

static const char *DeviceToAscii(const unsigned char *device, int device_len)
{
   const int WideCharSize = 2;
   const int TerminatingCharSize = 2;

   static char devstr[DEVICE_TOKEN_LEN * WideCharSize + TerminatingCharSize ];

   for (int i = 0; i < device_len; ++i)
   {
      devstr[i * WideCharSize] = "0123456789abcdef"[device[i] >> 4];
      devstr[i * WideCharSize + 1] = "0123456789abcdef"[device[i] & 0xf];
   }

   devstr[device_len * WideCharSize] = '\0';
   devstr[device_len * WideCharSize + 1] = '\0';

   return devstr;
}

//------------------------------------------------------------

bool     UserConnection::FindDeviceAndUpdate( RegisteredDeviceList& deviceList, const PacketNotification_RegisterDevice* potentialDevice, U32 connectionId )
{
   string printableDeviceId = BufferToHexString( potentialDevice->deviceId, 16 );
   LogMessage( LOG_PRIO_INFO, "Device stats: assignedUuid:%s, deviceId[%s], %s", potentialDevice->assignedUuid.c_str(), printableDeviceId.c_str(), FindPlatformName( potentialDevice->platformId ) );
   LogMessage( LOG_PRIO_INFO, "NumUserDevices in storage: %u", deviceList.size() );
   
   const string& vendorProvidedDeviceId = potentialDevice->deviceId;
   const string& assignedUuid = potentialDevice->assignedUuid;
   U8 gameProductId = potentialDevice->gameProductId;
   // potentialDevice
   int count = 0;
   if( assignedUuid.size() > 0 )// do we have a previously assignedUuid, this device may have been reset, so...
   {
      RegisteredDeviceIterator   deviceIt = deviceList.begin();
      while( deviceIt != deviceList.end() )
      {
         count++;
         LogMessage( LOG_PRIO_INFO, " %d) Device: %s", count, assignedUuid.c_str() );
         if( deviceIt->uuid == assignedUuid )
         {
            LogMessage( LOG_PRIO_INFO, "FindDeviceAndUpdate: device matched by assignedUuid, update info beginning" );
            // look up the  device notification and if it exists, report this error.
            U32 userDeviceId = deviceIt->userDeviceId;
            DeviceNotificationsIterator   deviceEnabledIt = FindDeviceNotification( userDeviceId, gameProductId );
            if( deviceEnabledIt != m_deviceEnabledList.end() )
            {
               UserConnectionList connectionList;
               m_connectionDetails.AssembleAllConnections( connectionList );
               m_mainThread->SendErrorToClient( connectionList, PacketErrorReport::ErrorType_Notification_DeviceAlreadyRegistered );
               SendNewDeviceRegistrationResponse( deviceIt->uuid, connectionId );
            }
            else
            {
               LogMessage( LOG_PRIO_INFO, "FindDeviceAndUpdate: CreateNewDeviceNotificationEntry because we did find the notification id" );
               //otherwise, we are logging in from a new game and we need to create a new entry... you are logging in from the same game but a different device
               CreateNewDeviceNotificationEntry( userDeviceId, gameProductId, deviceIt->deviceId );
            }

            if( potentialDevice->deviceName.size() && deviceIt->name.size() == 0 )
            {
               LogMessage( LOG_PRIO_INFO, "FindDeviceAndUpdate: GrandfatherInOldDevices. Existing device name is empty" );
               GrandfatherInOldDevices( deviceIt, potentialDevice->deviceName, connectionId );
            }
            LogMessage( LOG_PRIO_INFO, "FindDeviceAndUpdate: return true" );
            return true;
         }
         deviceIt++;
      }
   }
   else
   {
      LogMessage( LOG_PRIO_INFO, " Device lookup by vendor provided id because we have no assigned Uuid" );
      // the device may have been wiped or something so let's look through the existing vendor ids and see if we have a match
      RegisteredDeviceIterator   deviceIt = deviceList.begin();
      while( deviceIt != deviceList.end() )
      {
         count++;
         const string& previouslyRegisteredDeviceId = deviceIt->deviceId;
         
         if( previouslyRegisteredDeviceId == vendorProvidedDeviceId && 
            deviceIt->platformId == potentialDevice->platformId) // we already have this device
         {
            LogMessage( LOG_PRIO_INFO, "FindDeviceAndUpdate: device matched by vendor provided device uuid" );
            U32 userDeviceId = deviceIt->userDeviceId;
            DeviceNotificationsIterator   deviceEnabledIt = FindDeviceNotification( userDeviceId, gameProductId );
            if( deviceEnabledIt != m_deviceEnabledList.end() )
            {
               UserConnectionList connectionList;
               m_connectionDetails.AssembleAllConnections( connectionList );
               m_mainThread->SendErrorToClient( connectionList, PacketErrorReport::ErrorType_Notification_DeviceAlreadyRegistered );
               SendNewDeviceRegistrationResponse( deviceIt->uuid, connectionId );
            }
            else
            {
               LogMessage( LOG_PRIO_INFO, "FindDeviceAndUpdate: CreateNewDeviceNotificationEntry because we did find the notification id" );
               //otherwise, we are logging in from a new game and we need to create a new entry... you are logging in from the same game but a different device
               CreateNewDeviceNotificationEntry( userDeviceId, gameProductId, deviceIt->deviceId );
            }
            LogMessage( LOG_PRIO_INFO, "FindDeviceAndUpdate: return true" );
      
            return true;
         }
         deviceIt++;
         
      }
   }

   LogMessage( LOG_PRIO_INFO, "FindDeviceAndUpdate: return false... assignedUuid %s not found", potentialDevice->assignedUuid.c_str() );
   return false;
}

//------------------------------------------------------------

void RemoveSpecialCharacters(const string& str, string& outString)
{
   int len = str.size();
   if( len < 2 )
   {
      outString = str;
      return;
   }
   char * buffer = new char[len];
   int idx = 0;

   for(int i=0; i<len; i++)
   {
      char c = str[i];
      if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z')
         || (c >= 'a' && c <= 'z') || (c == '.') || (c == '_'))
      {
         buffer[idx] = c;
         idx++;
      }
   }

   outString.copy( buffer, idx, 0 );
   delete buffer;
}

//------------------------------------------------------------

void PrintDetailsOfDeviceRegistrationToConsole( const string& deviceId, U32 userId, const string& userUuid )
{
   LogMessage( LOG_PRIO_INFO, "Device id len = %u", deviceId.size() );
   const U32 maxLen = 20;
   char  tempDeviceId[ maxLen + 1 ];

   U32 usableLen = maxLen;
   if( deviceId.size() < usableLen )
      usableLen = deviceId.size();

   // clean up the string before printing... not too much and remove beeps.
   
 /*  string outString;
   RemoveSpecialCharacters( temp, outString );*/

   LogMessage( LOG_PRIO_INFO, "----------------------------------------------------------" );
   if( usableLen )
   {
      deviceId.copy( tempDeviceId, usableLen );
      string temp = BufferToHexString( reinterpret_cast< const U8* >( tempDeviceId ), usableLen );
      LogMessage( LOG_PRIO_INFO, " RegisterNewDevice: [%s]", temp.c_str() );
   }
   LogMessage( LOG_PRIO_INFO, " Useruuid: %s", userUuid.c_str() );
   LogMessage( LOG_PRIO_INFO, " UserId: %d", userId );
}

//------------------------------------------------------------

void     UserConnection::TestNotification( const char* text, U32 type, U32 connectionId )
{
   U8 gameProductId = m_connectionDetails.GetGameProductId( connectionId );
   m_mainThread->StoreLastUserNotification( m_userId, gameProductId, 5,
      type, text );

   m_mainThread->SetupNotificationsToSendImmediately();

}

//------------------------------------------------------------
//------------------------------------------------------------

void     UserConnection::RegisterNewDevice( const PacketNotification_RegisterDevice* potentialDevice, U32 connectionId )
{
   U32 gatewayId = GetGatewayId( connectionId );
   U8 gameProductId = m_connectionDetails.GetGameProductId( connectionId );
   LogMessage( LOG_PRIO_INFO, "RegisterNewDevice: user: %s, userId: %u, connectionId: %u, gatewayId: %u", m_userName.c_str(), m_userId, connectionId, gatewayId );
   PrintDetailsOfDeviceRegistrationToConsole( potentialDevice->deviceId, m_userId, m_userUuid );

   if( potentialDevice->deviceId.size() < 8 )
   {
      UserConnectionList connectionList;
      m_connectionDetails.AssembleAllConnections( connectionList );
      LogMessage( LOG_PRIO_ERR, "Error: device has no identifier" );
      m_mainThread->SendErrorToClient( connectionList, PacketErrorReport::ErrorType_Notification_DeviceIdIncorrect );
      return;
   }

   if( FindDeviceAndUpdate( m_deviceList, potentialDevice, connectionId ) == true )
   {
      UserConnectionList connectionList;
      m_connectionDetails.AssembleAllConnections( connectionList );
      LogMessage( LOG_PRIO_ERR, "Error: device is already in user's list" );
      m_mainThread->SendErrorToClient( connectionList, PacketErrorReport::ErrorType_Notification_DeviceAlreadyRegistered );
      return;
   }

   U32 xorValue = GetCurrentMilliseconds();
   string newDeviceUuid = GenerateUUID( xorValue );

   string query( "INSERT INTO user_device ( user_uuid, device_uuid, device_id, name, icon_id, platformId, user_id, is_enabled )" );
   query += " VALUES( '";
   query += m_userUuid;
   query += "', '";
   query += newDeviceUuid;
   query += "', x'";
   query += DeviceToAscii( (const unsigned char*)potentialDevice->deviceId.c_str(), potentialDevice->deviceId.size() );
   query += "', '%s', '1', '"; // device name, icon id
   query += boost::lexical_cast< string  >( (int) potentialDevice->platformId );
   query += "', '";
   query += boost::lexical_cast< string  >( m_userId );
   query += "', '1' )";


   PrintDetailsOfDeviceRegistrationToConsole( potentialDevice->deviceId, m_userId, m_userUuid );
   LogMessage( LOG_PRIO_INFO, " New device uuid: %s", newDeviceUuid.c_str() );
  /* LogMessage( LOG_PRIO_INFO, "-------------------------------------------------------" );
   LogMessage( LOG_PRIO_INFO, " RegisterNewDevice: [%s]", BufferToHexString( potentialDevice->deviceId, 16 ).c_str() );
   LogMessage( LOG_PRIO_INFO, " Useruuid: %s", m_userUuid );
   LogMessage( LOG_PRIO_INFO, " UserId: %d", m_userId );*/


   // we're going to assume that this new entry works fine. There is the potential for a uuid conflict, so we'll need to build that later.
   ExtendedRegisteredDevice* rd = new ExtendedRegisteredDevice;
      rd->userDeviceId =  0;// we need to look this up
      rd->name =      potentialDevice->deviceName;
      rd->uuid =      newDeviceUuid;
      rd->iconId =    1;
      rd->deviceId =  potentialDevice->deviceId.c_str();
      rd->platformId = potentialDevice->platformId;
      rd->isEnabled =  true;

   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =           m_userId;//m_userInfo.connectionId;
   dbQuery->meta =         newDeviceUuid;
   dbQuery->lookup =       QueryType_InsertDevice;
   dbQuery->serverLookup = gameProductId;
   dbQuery->query =        query;
   dbQuery->customData  =  rd;

   dbQuery->escapedStrings.insert( potentialDevice->deviceName );
   m_mainThread->AddQueryToOutput( dbQuery );


   RequestAllDevicesForUser();
   //m_deviceList.push_back( *rd );// this is costly

   //RequestAllDeviceNotification();// this is the best way. There is an id that the db must generate for this to be a coherent value.

   SendNewDeviceRegistrationResponse( newDeviceUuid, connectionId );
}

//------------------------------------------------------------

void  UserConnection::SendNewDeviceRegistrationResponse( const string& uuid, U32 connectionId )
{
   PacketNotification_RegisterDeviceResponse* response = new PacketNotification_RegisterDeviceResponse;
   response->deviceUuid = uuid;
   SendMessageToClient( response, connectionId );
}

//------------------------------------------------------------

void  UserConnection::CreateEnabledNotificationEntry( const PacketDbQueryResult* dbResult )
{
   ExtendedRegisteredDevice* customData = reinterpret_cast<ExtendedRegisteredDevice*>( dbResult->customData );
   if( dbResult->successfulQuery == false )
   {
      delete customData;
      UserConnectionList connectionList;
      m_connectionDetails.AssembleAllConnections( connectionList );
      m_mainThread->SendErrorToClient( connectionList, PacketErrorReport::ErrorType_Notification_CannotInsertNewDevice );
      return;
   }

   U32 userDeviceId = dbResult->insertId;
   U32 gameType = static_cast< U32>( dbResult->serverLookup );

   CreateNewDeviceNotificationEntry( userDeviceId, gameType, customData->deviceId );   

   //----------------------------
   customData->deviceId = userDeviceId;
   m_deviceList.push_back( *customData );
   delete customData;
   //----------------------------
}

//------------------------------------------------------------

// we do not make all of the same error checks here... it's too costly and assumed to have already have been performed
void  UserConnection::CreateNewDeviceNotificationEntry( U32 userDeviceId, U32 gameType, const string& deviceId )
{
   string printableDeviceId = BufferToHexString( reinterpret_cast< const U8* >( deviceId.c_str() ), 20 );
   LogMessage( LOG_PRIO_INFO, "CreateNewDeviceNotificationEntry: user: %s, userId: %u, userDeviceId: %u, gameType: %d, deviceId: [%s]", m_userName.c_str(), m_userId, userDeviceId, gameType, printableDeviceId.c_str() );

   string query( "INSERT INTO user_device_notification ( user_device_id, game_type, is_enabled, device_id) " );
   query += " VALUES( ";
   query += boost::lexical_cast< string  >( userDeviceId );
   query += ", ";
   query += boost::lexical_cast< string  >( gameType );
   query += ", 1, x'";
   query += DeviceToAscii( (const unsigned char*)deviceId.c_str(), deviceId.size() );
   query += "')";

   LogMessage( LOG_PRIO_INFO, "---------------------------------------------------------" );
   LogMessage( LOG_PRIO_INFO, " CreateNewDeviceNotificationEntry: [%s]", printableDeviceId.c_str() );
   LogMessage( LOG_PRIO_INFO, " user_device_id: %d", userDeviceId );
   LogMessage( LOG_PRIO_INFO, " Useruuid: %s", m_userUuid );
   LogMessage( LOG_PRIO_INFO, " UserId: %d", m_userId );
   //cout, query );

   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =           m_userId;//m_userInfo.connectionId;
   dbQuery->meta =         "";
   dbQuery->lookup =       QueryType_InsertDeviceNotification;
   dbQuery->serverLookup = gameType;
   dbQuery->query =        query;
   dbQuery->isFireAndForget = true;  // MAJOR importance... we are requesting an update of all the data below

   m_mainThread->AddQueryToOutput( dbQuery );

   RequestAllDeviceNotification();// restore all of our data.
}

//------------------------------------------------------------

void  UserConnection::GrandfatherInOldDevices( RegisteredDeviceIterator iter, const string& newName, U32 connectionId )
{
   iter->name = newName;
   UpdateDbRecordForDevice( iter->userDeviceId, connectionId );
}

//------------------------------------------------------------

void  UserConnection::UpdateDevice( const PacketNotification_UpdateDevice* updateDevicePacket, U32 connectionId )
{
   if( updateDevicePacket->deviceUuid.size() < 1 )
   {
      UserConnectionList connectionList;
      m_connectionDetails.AssembleAllConnections( connectionList );
      m_mainThread->SendErrorToClient( connectionList, PacketErrorReport::ErrorType_Notification_DeviceIdIncorrect );
      return;
   }

   //bool deviceFound = false;
   U32   userDeviceId = 0;
   const string uuid = updateDevicePacket->deviceUuid.c_str();

   RegisteredDeviceIterator deviceIt = FindDeviceByUuid( uuid );
   if( deviceIt == m_deviceList.end() )
   {
      string text = "UpdateDevice:: User ";
      text += m_userName;
      text += " tried to update a device but it could not be found ";
      text += updateDevicePacket->deviceName.c_str();
      text += ", ";
      text += updateDevicePacket->deviceUuid.c_str();
      text += " but the device does not exist in UserConnection";
      m_mainThread->Log( text, 1 );

      UserConnectionList connectionList;
      m_connectionDetails.AssembleAllConnections( connectionList );
      m_mainThread->SendErrorToClient( connectionList, PacketErrorReport::ErrorType_Notification_DeviceIdIncorrect );
      return;
   }
   
   userDeviceId = deviceIt->userDeviceId;
   U8 gameProductId = GetGameProductId( connectionId );

   if( userDeviceId == 0 )
   {
      string text = "UpdateDevice:: User ";
      text += m_userName;
      text += " tried to update a device but it could not be found ";
      text += updateDevicePacket->deviceName.c_str();
      text += ", ";
      text += updateDevicePacket->deviceUuid.c_str();
      text += " but the device does not exist in UserConnection";
      m_mainThread->Log( text, 1 );
      return;
   }

   DeviceNotificationsIterator deviceEnabledIt = FindDeviceNotification( userDeviceId, gameProductId );
   if( deviceEnabledIt == m_deviceEnabledList.end() )
   {
      string text = "UpdateDevice:: User ";
      text += m_userName;
      text += " tried to update a device but the enabled record could not be found ";
      text += updateDevicePacket->deviceName.c_str();
      text += ", ";
      text += updateDevicePacket->deviceUuid.c_str();
      text += " but the device does not exist in UserConnection";
      m_mainThread->Log( text, 1 );
      UserConnectionList connectionList;
      m_connectionDetails.AssembleAllConnections( connectionList );
      m_mainThread->SendErrorToClient( connectionList, PacketErrorReport::ErrorType_Notification_DeviceIdIncorrect );
      return;
   }

   // update
   deviceIt->iconId = updateDevicePacket->iconId;
   deviceIt->name = updateDevicePacket->deviceName;
   deviceEnabledIt->audioFile = updateDevicePacket->audioFile.c_str();
   deviceEnabledIt->repeatFrequencyInHours = updateDevicePacket->repeatFrequencyInHours;
   
   //deviceIt->isEnabled = updateDevicePacket->isEnabled; << this is the "general" isEnabled.

   deviceEnabledIt->gameType = updateDevicePacket->gameType;
   deviceEnabledIt->isEnabled = updateDevicePacket->isEnabled;

   UpdateDbRecordForDevice( userDeviceId, connectionId );
}

//------------------------------------------------------------

void  UserConnection::UpdateDbRecordForDevice( U32 userDeviceId, U32 connectionId )
{
   RegisteredDeviceIterator deviceIt = FindDeviceById( userDeviceId );
   if( deviceIt == m_deviceList.end() )
   {
      string text = "UpdateDbRecordForDevice:: User ";
      text += m_userName;
      text += " tried to update a device but it could not be found ";
      text += " but the device does not exist in UserConnection";
      m_mainThread->Log( text, 1 );
      return;
   }

   //U32 userDeviceId = deviceIt->userDeviceId;
   U8 gameProductId = GetGameProductId( connectionId );

   DeviceNotificationsIterator   deviceEnabledIt = FindDeviceNotification( userDeviceId, gameProductId );
   if( deviceEnabledIt == m_deviceEnabledList.end() )
   {
      return;
   }

   string query( "UPDATE user_device SET name='%s', icon_id=" );
   query += boost::lexical_cast< string  >( deviceIt->iconId );
   query += ", is_enabled=";
   query += boost::lexical_cast< string  >( deviceEnabledIt->isEnabled );
   query += " WHERE id=";
   query += boost::lexical_cast< string  >( deviceIt->userDeviceId );

   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =           m_userId;//m_userInfo.connectionId;
   dbQuery->meta =         "";
   dbQuery->lookup =       QueryType_UpdateDevice;
   dbQuery->serverLookup = userDeviceId;
   dbQuery->query =        query;

   dbQuery->escapedStrings.insert( deviceIt->name );

   m_mainThread->AddQueryToOutput( dbQuery );



   string query2( "UPDATE user_device_notification SET is_enabled=");
   query2 += boost::lexical_cast< string  >( deviceEnabledIt->isEnabled );
   query2 += ", audio_file='%s', repeat_frequency_in_hours=";
   query2 += boost::lexical_cast< string  >( (int) (deviceEnabledIt->repeatFrequencyInHours) );

   query2 += " WHERE id=";// is enabled, timestamp
   query2 += boost::lexical_cast< string  >( deviceEnabledIt->id );
   
   //----------------------------

   dbQuery = new PacketDbQuery;
   dbQuery->id =           m_userId;//m_userInfo.connectionId;
   dbQuery->meta =         "";
   dbQuery->lookup =       QueryType_InsertDeviceNotification;
   dbQuery->serverLookup = userDeviceId;
   dbQuery->query =        query2;
   dbQuery->isFireAndForget = true;

   dbQuery->escapedStrings.insert( deviceEnabledIt->audioFile );

   m_mainThread->AddQueryToOutput( dbQuery );
}

void     DumpDevice( const string& userName, const string&  userUuid, const ExtendedRegisteredDevice& rd )
{
   int numToCopy = 20;
   if( rd.deviceId.size() > static_cast< U32 >( numToCopy ) )
      numToCopy = rd.deviceId.size();
   string temp = rd.deviceId.substr( 0,numToCopy );
   string deviceOutStringId;
   RemoveSpecialCharacters( temp, deviceOutStringId );
   LogMessage( LOG_PRIO_INFO, "User Name: %s", userName.c_str() );
   LogMessage( LOG_PRIO_INFO, "User Uuid: %s", userUuid.c_str() );
   LogMessage( LOG_PRIO_INFO, "   device id: %s", deviceOutStringId.c_str() );
   LogMessage( LOG_PRIO_INFO, "   device uuid: %s", rd.uuid.c_str() );
   LogMessage( LOG_PRIO_INFO, "   device iconId: ", rd.iconId );
   LogMessage( LOG_PRIO_INFO, "   device isEnabled: %s", ConvertToTrueFalseString( rd.isEnabled ) );
   LogMessage( LOG_PRIO_INFO, "   device idname %s", rd.name.c_str() );
   LogMessage( LOG_PRIO_INFO, "   device platformId: %d", rd.platformId );
   LogMessage( LOG_PRIO_INFO, "   device productId: %d", rd.productId );
   LogMessage( LOG_PRIO_INFO, "   device userDeviceId: %d", rd.userDeviceId );
   LogMessage( LOG_PRIO_INFO, "---------------------------------------" );
}
//------------------------------------------------------------

void        UserConnection::RequestDevicesList( const PacketNotification_RequestListOfDevices* device, U32 connectionId )
{
   PacketNotification_RequestListOfDevicesResponse* response = new PacketNotification_RequestListOfDevicesResponse;

   U8 gameProductId = GetGameProductId( connectionId );
   // todo, divide this into multiple packets to deal with overflow.
   RegisteredDeviceIterator deviceIt = m_deviceList.begin();
   while( deviceIt != m_deviceList.end() )
   {
      DumpDevice( m_userName, m_userUuid, *deviceIt );

      U32 userDeviceId = deviceIt->userDeviceId;
      DeviceNotificationsIterator deviceNotificationIter = FindDeviceNotification( userDeviceId, gameProductId );
      if( deviceNotificationIter != m_deviceEnabledList.end() )
      {
         RegisteredDevice rd;
         rd.iconId =                   deviceIt->iconId;
         rd.isEnabled =                deviceNotificationIter->isEnabled;
         rd.name =                     deviceIt->name;
         rd.platformId =               deviceIt->platformId;
         rd.productId =                deviceNotificationIter->gameType;
         rd.audioFile =                deviceNotificationIter->audioFile;
         rd.repeatFrequencyInHours =   deviceNotificationIter->repeatFrequencyInHours;
         rd.uuid =                     deviceIt->uuid;
         response->devices.push_back( rd );
      }
      deviceIt++;
   }

   SendMessageToClient( response, connectionId );
}

//------------------------------------------------------------

void        UserConnection::RemoveDevice( const PacketNotification_RemoveDevice* removal, U32 connectionId )
{
   LogMessage( LOG_PRIO_INFO, "Remove device" );
   PacketNotification_RemoveDeviceResponse* response = new PacketNotification_RemoveDeviceResponse;
   response->success = false;
   const string lookupUuid = removal->deviceUuid.c_str();
   response->deviceUuid = lookupUuid;
   U8 gameProductId = GetGameProductId( connectionId );

   RegisteredDeviceIterator deviceIt = m_deviceList.begin();
   while( deviceIt != m_deviceList.end() )
   {
      if( deviceIt->uuid == lookupUuid )
      {
         U32 userDeviceId = deviceIt->userDeviceId;  
         DeviceNotificationsIterator deviceNotificationIter = FindDeviceNotification( userDeviceId, gameProductId );

         PacketDbQuery* dbQuery = new PacketDbQuery;
         dbQuery->id =           m_userId;//m_userInfo.connectionId;
         dbQuery->meta =         "";
         dbQuery->lookup =       QueryType_DeleteDevice;
         dbQuery->serverLookup = m_userId;
         dbQuery->query = "DELETE FROM user_device WHERE user_uuid='";
         dbQuery->query += m_userUuid; // a little extra validation
         dbQuery->query += "' AND id='";
         dbQuery->query += boost::lexical_cast<string> ( userDeviceId );
         dbQuery->query += "'";
         dbQuery->isFireAndForget = true;

         LogMessage( LOG_PRIO_INFO, dbQuery->query.c_str() );

         m_mainThread->AddQueryToOutput( dbQuery );

         dbQuery = new PacketDbQuery;
         dbQuery->id =           m_userId;//m_userInfo.connectionId;
         dbQuery->meta =         "";
         dbQuery->lookup =       QueryType_DeleteDeviceNotification;
         dbQuery->serverLookup = m_userId;
         dbQuery->query = "DELETE FROM user_device_notification WHERE user_device_id='";
         dbQuery->query += boost::lexical_cast<string> ( userDeviceId );
         dbQuery->query += "'";
         dbQuery->isFireAndForget = true;

         LogMessage( LOG_PRIO_INFO, dbQuery->query.c_str() );

         m_mainThread->AddQueryToOutput( dbQuery );

         m_deviceList.erase( deviceIt );
         m_deviceEnabledList.erase( deviceNotificationIter );

         response->success = true;

         break; // iterator will be bad at this point
      }
      deviceIt++;
   }

   SendMessageToClient( response, connectionId );
   RequestDevicesList( NULL, connectionId );
}
/*
//------------------------------------------------------------

void        UserConnection::UserRequestedNotification( const PacketGame_Notification* notification )
{
}
*/
//------------------------------------------------------------
//------------------------------------------------------------

UserConnection::RegisteredDeviceIterator      
   UserConnection::FindDeviceByUuid( const string& uuid )
{
   RegisteredDeviceIterator deviceIt = m_deviceList.begin();
   while( deviceIt != m_deviceList.end() )
   {
      if( deviceIt->uuid == uuid )
      {
         return deviceIt;
      }
      deviceIt++;
   }
   return deviceIt;
}

//------------------------------------------------------------

UserConnection::RegisteredDeviceIterator      
   UserConnection::FindDeviceById( U32 id )
{
   RegisteredDeviceIterator deviceIt = m_deviceList.begin();
   while( deviceIt != m_deviceList.end() )
   {
      if( deviceIt->userDeviceId == id )
      {
         return deviceIt;
      }
      deviceIt++;
   }
   return deviceIt;
}

//------------------------------------------------------------

// if you pass in a 0, you get all devices for all platforms.
UserConnection::DeviceNotificationsIterator   
   UserConnection::FindDeviceNotification( U32 id, U8 gameProductId )
{
   DeviceNotificationsIterator  deviceEnabledIt = m_deviceEnabledList.begin();
   while( deviceEnabledIt != m_deviceEnabledList.end() )
   {
      if( gameProductId == deviceEnabledIt->gameType )// we should match both fields.
      {
         if( deviceEnabledIt->userDeviceId == id )
         {
            return deviceEnabledIt;
         }
      }
      deviceEnabledIt++;
   }
   return deviceEnabledIt;
}

//------------------------------------------------------------

// if you pass in a 0, you get all devices for all platforms.
UserConnection::DeviceNotificationsIterator   
   UserConnection::FindDeviceNotificationByDeviceId( const string& id, U8 gameProductId )
{
   DeviceNotificationsIterator  deviceEnabledIt = m_deviceEnabledList.begin();
   while( deviceEnabledIt != m_deviceEnabledList.end() )
   {
      if( gameProductId == deviceEnabledIt->gameType )// got to match both fields.
      {
         if( deviceEnabledIt->deviceId == id )
         {
            return deviceEnabledIt;
         }
      }
      deviceEnabledIt++;
   }
   return deviceEnabledIt;
}

//---------------------------------------------------------------

bool     UserConnection::SendMessageToClient( BasePacket* packet, U32 connectionId ) const
{
   U32 gatewayId = GetGatewayId( connectionId );
   //U32 connectionId = m_userInfo.connectionId;
   //U32 gatewayId = m_userInfo.gatewayId;
   PacketGatewayWrapper* wrapper = new PacketGatewayWrapper();
   wrapper->SetupPacket( packet, connectionId );

   m_mainThread->SendMessageToClient( wrapper, connectionId, gatewayId );
   return true;
}

//////////////////////////////////////////////////////////////
