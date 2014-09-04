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

#include "NotificationMainThread.h"
#include "NotificationCommon.h"

//////////////////////////////////////////////////////////////


NotificationMainThread* UserConnection::m_mainThread = NULL;

//////////////////////////////////////////////////////////////

UserConnection::UserConnection( const PacketPrepareForUserLogin* info ) : m_userInfo( *info ), 
                                 m_isLoggedOut( false ), 
                                 m_requiresUpdate( true ),
                                 m_hasRequestedDevices( false )
{
}

UserConnection::~UserConnection()
{
}

void  UserConnection::UserLoggedOut()
{
   m_isLoggedOut = true; 
   time( &m_timeLoggedOut );
}


int   UserConnection::SecondsExpiredSinceLoggedOut()
{
   if( m_timeLoggedOut == 0 )
      return 0;

   time_t currentTime;
   time( &currentTime );

   return  static_cast<int> ( difftime( currentTime, m_timeLoggedOut ) );
}

void  UserConnection::Relog()
{
   m_timeLoggedOut = 0;
   m_isLoggedOut = false;
}

//------------------------------------------------------------

void  UserConnection::RequestAllDevicesForUser()
{
   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =           m_userInfo.connectionId;
   dbQuery->meta =         "";
   dbQuery->lookup =       QueryType_DeviceList;
   dbQuery->serverLookup = m_userInfo.userId;
   dbQuery->query = "SELECT * FROM user_device WHERE user_uuid='";
   dbQuery->query += m_userInfo.uuid.c_str();
   dbQuery->query += "'";

   //cout << dbQuery->query << endl;

   m_mainThread->AddQueryToOutput( dbQuery );
}

//------------------------------------------------------------

void  UserConnection::RequestAllDeviceNotification()
{
   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =           m_userInfo.connectionId;
   dbQuery->meta =         "";
   dbQuery->lookup =       QueryType_DevicePerGameList;
   dbQuery->serverLookup = m_userInfo.userId;
   dbQuery->query = "SELECT * FROM user_device_notification WHERE user_device_id IN ( select id from user_device where user_uuid='";
   dbQuery->query += m_userInfo.uuid.c_str();
   dbQuery->query += "')";

   //cout << dbQuery->query << endl;

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
   if( dbResult->successfulQuery == false )
   {
      m_mainThread->SendErrorToClient( m_userInfo.connectionId, m_userInfo.gatewayId, PacketErrorReport::ErrorType_Notification_NoDevicesListed );
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
   if( dbResult->successfulQuery == false )
   {
      m_mainThread->SendErrorToClient( m_userInfo.connectionId, m_userInfo.gatewayId, PacketErrorReport::ErrorType_Notification_NoDevicesEnabledForThisGame );
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

bool  UserConnection::HandleRequestFromClient( const BasePacket* packet )
{
   U8 packetType = packet->packetType;
   PacketFactory factory;
   switch( packetType )
   {
   case PacketType_Notification:
      {
         switch( packet->packetSubType )
         {
         case PacketNotification::NotificationType_RegisterDevice:
            {
               const PacketNotification_RegisterDevice* registerDevice = static_cast< const PacketNotification_RegisterDevice* >( packet );
               RegisterNewDevice( registerDevice );
            }
            break;
        /* case PacketNotification::NotificationType_RegisterDevice:
            {
            }
            break;*/
         case PacketNotification::NotificationType_UpdateDevice:
            {
               const PacketNotification_UpdateDevice* device = static_cast< const PacketNotification_UpdateDevice* >( packet );
               UpdateDevice( device );
            }
            break;
         case PacketNotification::NotificationType_RequestListOfDevices:  // based on product.. 0 means all devices
            {
               const PacketNotification_RequestListOfDevices* request = static_cast< const PacketNotification_RequestListOfDevices* >( packet );
               RequestDevicesList( request );
            }
            break;
         case PacketNotification::NotificationType_RemoveDevice: 
            {
               const PacketNotification_RemoveDevice* removal = static_cast< const PacketNotification_RemoveDevice* >( packet );
               RemoveDevice( removal );
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

static const char *devicetoa(const unsigned char *device, int device_len)
{
   static char devstr[DEVICE_TOKEN_LEN * 2 + 1];

   for (int i = 0; i < device_len; ++i)
   {
      devstr[i * 2] = "0123456789abcdef"[device[i] >> 4];
      devstr[i * 2 + 1] = "0123456789abcdef"[device[i] & 0xf];
   }

   devstr[device_len * 2] = '\0';

   return devstr;
}

//------------------------------------------------------------

bool     UserConnection::FindDeviceAndUpdate( RegisteredDeviceList deviceList, const PacketNotification_RegisterDevice* registerDevice )
{
   RegisteredDeviceIterator   deviceIt = deviceList.begin();
   while( deviceIt != deviceList.end() )
   {
      //if( deviceIt->deviceId == registerDevice->deviceId )
      if( deviceIt->uuid == registerDevice->assignedUuid )
      {
         // look up the  device notification and if it exists, report this error.
         U32 userDeviceId = deviceIt->userDeviceId;
         DeviceNotificationsIterator   deviceEnabledIt = FindDeviceNotificationByUserDeviceId( userDeviceId );
         if( deviceEnabledIt != m_deviceEnabledList.end() )
         {
            m_mainThread->SendErrorToClient( m_userInfo.connectionId, m_userInfo.gatewayId, PacketErrorReport::ErrorType_Notification_DeviceAlreadyRegistered );
            SendNewDeviceRegistrationResponse( deviceIt->uuid );
         }
         else
         {
            //otherwise, we are logging in from a new game and we need to create a new entry... you are logging in from the same game but a different device
            CreateNewDeviceNotificationEntry( userDeviceId, m_userInfo.gameProductId, deviceIt->deviceId );
         }

         if( registerDevice->deviceName.size() && deviceIt->name.size() == 0 )
         {
            GrandfatherInOldDevices( deviceIt, registerDevice->deviceName );
         }
         return true;
      }
      deviceIt++;
   }

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
   // clean up the string before printing... not too much and remove beeps.
   string temp = deviceId.substr(0,20);
   string outString;
   RemoveSpecialCharacters( temp, outString );
   cout << "/////////////////////////////////////////////////////////" << endl;
   cout << " RegisterNewDevice: " << outString << endl;
   cout << " Useruuid: " << userUuid << endl;
   cout << " UserId: " << userId << endl;
}

//------------------------------------------------------------
//------------------------------------------------------------

void     UserConnection::RegisterNewDevice( const PacketNotification_RegisterDevice* registerDevice )
{
   if( registerDevice->deviceId.size() < 8 )
   {
      m_mainThread->SendErrorToClient( m_userInfo.connectionId, m_userInfo.gatewayId, PacketErrorReport::ErrorType_Notification_DeviceIdIncorrect );
      return;
   }

   if( FindDeviceAndUpdate( m_deviceList, registerDevice ) == true )
   {
      m_mainThread->SendErrorToClient( m_userInfo.connectionId, m_userInfo.gatewayId, PacketErrorReport::ErrorType_Notification_DeviceAlreadyRegistered );
      return;
   }

   U32 xorValue = GetCurrentMilliseconds();
   string newDeviceUuid = GenerateUUID( xorValue );

   string query( "INSERT INTO user_device ( user_uuid, device_uuid, device_id, name, icon_id, platformId, user_id, is_enabled )" );
   query += " VALUES( '";
   query += m_userInfo.uuid.c_str();
   query += "', '";
   query += newDeviceUuid;
   query += "', x'";
   query += devicetoa( (const unsigned char*)registerDevice->deviceId.c_str(), registerDevice->deviceId.size() );
   query += "', '%s', '1', '"; // device name, icon id
   query += boost::lexical_cast< string  >( (int) registerDevice->platformId );
   query += "', '";
   query += boost::lexical_cast< string  >( m_userInfo.userId );
   query += "', '1' )";

   cout << "/////////////////////////////////////////////////////////" << endl;
   cout << " RegisterNewDevice: " << registerDevice->deviceId << endl;
   cout << " Useruuid: " << m_userInfo.uuid << endl;
   cout << " UserId: " << m_userInfo.userId << endl;


   // we're going to assume that this new entry works fine. There is the potential for a uuid conflict, so we'll need to build that later.
   ExtendedRegisteredDevice* rd = new ExtendedRegisteredDevice;
      rd->userDeviceId =  0;// we need to look this up
      rd->name =      registerDevice->deviceName;
      rd->uuid =      newDeviceUuid;
      rd->iconId =    1;
      rd->deviceId =  registerDevice->deviceId.c_str();
      rd->platformId = registerDevice->platformId;
      rd->isEnabled =  true;

   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =           m_userInfo.connectionId;
   dbQuery->meta =         newDeviceUuid;
   dbQuery->lookup =       QueryType_InsertDevice;
   dbQuery->serverLookup = m_userInfo.gameProductId;
   dbQuery->query =        query;
   dbQuery->customData  =  rd;

   dbQuery->escapedStrings.insert( registerDevice->deviceName );
   m_mainThread->AddQueryToOutput( dbQuery );


   RequestAllDevicesForUser();
   //m_deviceList.push_back( *rd );// this is costly

   //RequestAllDeviceNotification();// this is the best way. There is an id that the db must generate for this to be a coherent value.

   SendNewDeviceRegistrationResponse( newDeviceUuid );
}

//------------------------------------------------------------

void  UserConnection::SendNewDeviceRegistrationResponse( const string& uuid )
{
   PacketNotification_RegisterDeviceResponse* response = new PacketNotification_RegisterDeviceResponse;
   response->deviceUuid = uuid;
   SendMessageToClient( response );
}

//------------------------------------------------------------

void  UserConnection::CreateEnabledNotificationEntry( const PacketDbQueryResult* dbResult )
{
   ExtendedRegisteredDevice* rd = reinterpret_cast<ExtendedRegisteredDevice*>( dbResult->customData );
   if( dbResult->successfulQuery == false )
   {
      delete rd;
      m_mainThread->SendErrorToClient( m_userInfo.connectionId, m_userInfo.gatewayId, PacketErrorReport::ErrorType_Notification_CannotInsertNewDevice );
      return;
   }

   U32 userDeviceId = dbResult->insertId;
   U32 gameType = static_cast< U32>( dbResult->serverLookup );

   CreateNewDeviceNotificationEntry( userDeviceId, gameType, rd->deviceId );   

   //----------------------------
   rd->deviceId = userDeviceId;
   m_deviceList.push_back( *rd );
   delete rd;
   //----------------------------
}

//------------------------------------------------------------

// we do not make all of the same error checks here... it's too costly and assumed to have already have been performed
void  UserConnection::CreateNewDeviceNotificationEntry( U32 userDeviceId, U32 gameType, const string& deviceId )
{
   string query( "INSERT INTO user_device_notification ( user_device_id, game_type, is_enabled, device_id) " );
   query += " VALUES( ";
   query += boost::lexical_cast< string  >( userDeviceId );
   query += ", ";
   query += boost::lexical_cast< string  >( gameType );
   query += ", 1, x'";
   query += devicetoa( (const unsigned char*)deviceId.c_str(), deviceId.size() );
   query += "')";

   cout << "/////////////////////////////////////////////////////////" << endl;
   cout << " CreateNewDeviceNotificationEntry: " << deviceId << endl;
   cout << " user_device_id: " << userDeviceId << endl;
   cout << " Useruuid: " << m_userInfo.uuid << endl;
   cout << " UserId: " << m_userInfo.userId << endl;
   //cout << query << endl;

   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =           m_userInfo.connectionId;
   dbQuery->meta =         "";
   dbQuery->lookup =       QueryType_InsertDeviceNotification;
   dbQuery->serverLookup = gameType;
   dbQuery->query =        query;
   dbQuery->isFireAndForget = true;  // MAJOR importance... we are requesting an update of all the data below

   m_mainThread->AddQueryToOutput( dbQuery );

   RequestAllDeviceNotification();// restore all of our data.
}

//------------------------------------------------------------

void  UserConnection::GrandfatherInOldDevices( RegisteredDeviceIterator iter, const string& newName )
{
   iter->name = newName;
   UpdateDbRecordForDevice( iter->userDeviceId );
}

//------------------------------------------------------------

void  UserConnection::UpdateDevice( const PacketNotification_UpdateDevice* updateDevicePacket )
{
   if( updateDevicePacket->deviceUuid.size() < 1 )
   {
      m_mainThread->SendErrorToClient( m_userInfo.connectionId, m_userInfo.gatewayId, PacketErrorReport::ErrorType_Notification_DeviceIdIncorrect );
      return;
   }

   bool deviceFound = false;
   U32   userDeviceId = 0;
   const string uuid = updateDevicePacket->deviceUuid.c_str();

   RegisteredDeviceIterator deviceIt = FindDeviceByUuid( uuid );
   if( deviceIt == m_deviceList.end() )
   {
      string text = "UpdateDevice:: User ";
      text += m_userInfo.userName.c_str();
      text += " tried to update a device but it could not be found ";
      text += updateDevicePacket->deviceName.c_str();
      text += ", ";
      text += updateDevicePacket->deviceUuid.c_str();
      text += " but the device does not exist in UserConnection";
      m_mainThread->Log( text, 1 );
      m_mainThread->SendErrorToClient( m_userInfo.connectionId, m_userInfo.gatewayId, PacketErrorReport::ErrorType_Notification_DeviceIdIncorrect );
      return;
   }
   
   userDeviceId = deviceIt->userDeviceId;

   if( userDeviceId == 0 )
   {
      string text = "UpdateDevice:: User ";
      text += m_userInfo.userName.c_str();
      text += " tried to update a device but it could not be found ";
      text += updateDevicePacket->deviceName.c_str();
      text += ", ";
      text += updateDevicePacket->deviceUuid.c_str();
      text += " but the device does not exist in UserConnection";
      m_mainThread->Log( text, 1 );
      return;
   }

   DeviceNotificationsIterator deviceEnabledIt = FindDeviceNotificationByUserDeviceId( userDeviceId );
   if( deviceEnabledIt == m_deviceEnabledList.end() )
   {
      string text = "UpdateDevice:: User ";
      text += m_userInfo.userName.c_str();
      text += " tried to update a device but the enabled record could not be found ";
      text += updateDevicePacket->deviceName.c_str();
      text += ", ";
      text += updateDevicePacket->deviceUuid.c_str();
      text += " but the device does not exist in UserConnection";
      m_mainThread->Log( text, 1 );
      m_mainThread->SendErrorToClient( m_userInfo.connectionId, m_userInfo.gatewayId, PacketErrorReport::ErrorType_Notification_DeviceIdIncorrect );
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

   UpdateDbRecordForDevice( userDeviceId );
}

//------------------------------------------------------------

void  UserConnection::UpdateDbRecordForDevice( U32 id )
{
   RegisteredDeviceIterator deviceIt = FindDeviceById( id );
   if( deviceIt == m_deviceList.end() )
   {
      string text = "UpdateDbRecordForDevice:: User ";
      text += m_userInfo.userName.c_str();
      text += " tried to update a device but it could not be found ";
      text += " but the device does not exist in UserConnection";
      m_mainThread->Log( text, 1 );
      return;
   }

   U32 userDeviceId = deviceIt->userDeviceId;
   
   DeviceNotificationsIterator   deviceEnabledIt = FindDeviceNotificationByUserDeviceId( userDeviceId );
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
   dbQuery->id =           m_userInfo.connectionId;
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
   dbQuery->id =           m_userInfo.connectionId;
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
   cout << "User Name: " << userName << endl;
   cout << "User Uuid: " << userUuid << endl;
   cout << "   device id: " << deviceOutStringId << endl;
   cout << "   device uuid: " << rd.uuid << endl;
   cout << "   device iconId: " << rd.iconId << endl;
   cout << "   device isEnabled: " << rd.isEnabled << endl;
   cout << "   device idname " << rd.name << endl;
   cout << "   device platformId: " << rd.platformId << endl;
   cout << "   device productId: " << rd.productId << endl;
   cout << "   device userDeviceId: " << rd.userDeviceId << endl;
   cout << " --------------------------- " << endl;
}
//------------------------------------------------------------

void        UserConnection::RequestDevicesList( const PacketNotification_RequestListOfDevices* device )
{
   PacketNotification_RequestListOfDevicesResponse* response = new PacketNotification_RequestListOfDevicesResponse;

   // todo, divide this into multiple packets to deal with overflow.
   RegisteredDeviceIterator deviceIt = m_deviceList.begin();
   while( deviceIt != m_deviceList.end() )
   {
      DumpDevice( m_userInfo.userName, m_userInfo.uuid, *deviceIt );

      U32 userDeviceId = deviceIt->userDeviceId;
      DeviceNotificationsIterator deviceNotificationIter = FindDeviceNotificationByUserDeviceId( userDeviceId );
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

   SendMessageToClient( response );
}

//------------------------------------------------------------

void        UserConnection::RemoveDevice( const PacketNotification_RemoveDevice* removal )
{
   cout << "Remove device" << endl;
   PacketNotification_RemoveDeviceResponse* response = new PacketNotification_RemoveDeviceResponse;
   response->success = false;
   const string lookupUuid = removal->deviceUuid.c_str();
   response->deviceUuid = lookupUuid;

   RegisteredDeviceIterator deviceIt = m_deviceList.begin();
   while( deviceIt != m_deviceList.end() )
   {
      if( deviceIt->uuid == lookupUuid )
      {
         U32 userDeviceId = deviceIt->userDeviceId;  
         DeviceNotificationsIterator deviceNotificationIter = FindDeviceNotificationByUserDeviceId( userDeviceId );

         PacketDbQuery* dbQuery = new PacketDbQuery;
         dbQuery->id =           m_userInfo.connectionId;
         dbQuery->meta =         "";
         dbQuery->lookup =       QueryType_DeleteDevice;
         dbQuery->serverLookup = m_userInfo.userId;
         dbQuery->query = "DELETE FROM user_device WHERE user_uuid='";
         dbQuery->query += m_userInfo.uuid.c_str(); // a little extra validation
         dbQuery->query += "' AND id='";
         dbQuery->query += boost::lexical_cast<string> ( userDeviceId );
         dbQuery->query += "'";
         dbQuery->isFireAndForget = true;

         cout << dbQuery->query << endl;

         m_mainThread->AddQueryToOutput( dbQuery );

         dbQuery = new PacketDbQuery;
         dbQuery->id =           m_userInfo.connectionId;
         dbQuery->meta =         "";
         dbQuery->lookup =       QueryType_DeleteDeviceNotification;
         dbQuery->serverLookup = m_userInfo.userId;
         dbQuery->query = "DELETE FROM user_device_notification WHERE user_device_id='";
         dbQuery->query += boost::lexical_cast<string> ( userDeviceId );
         dbQuery->query += "'";
         dbQuery->isFireAndForget = true;

         cout << dbQuery->query << endl;

         m_mainThread->AddQueryToOutput( dbQuery );

         m_deviceList.erase( deviceIt );
         m_deviceEnabledList.erase( deviceNotificationIter );

         response->success = true;

         break; // iterator will be bad at this point
      }
      deviceIt++;
   }

   SendMessageToClient( response );
   RequestDevicesList( NULL );
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
   UserConnection::FindDeviceNotificationByUserDeviceId( U32 id )
{
   DeviceNotificationsIterator  deviceEnabledIt = m_deviceEnabledList.begin();
   while( deviceEnabledIt != m_deviceEnabledList.end() )
   {
      if( m_userInfo.gameProductId == deviceEnabledIt->gameType )// we should match both fields.
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
   UserConnection::FindDeviceNotificationByDeviceId( const string& id )
{
   DeviceNotificationsIterator  deviceEnabledIt = m_deviceEnabledList.begin();
   while( deviceEnabledIt != m_deviceEnabledList.end() )
   {
      if( m_userInfo.gameProductId == deviceEnabledIt->gameType )// got to match both fields.
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

bool     UserConnection::SendMessageToClient( BasePacket* packet ) const
{
   U32 connectionId = m_userInfo.connectionId;
   U32 gatewayId = m_userInfo.gatewayId;
   PacketGatewayWrapper* wrapper = new PacketGatewayWrapper();
   wrapper->SetupPacket( packet, connectionId );

   m_mainThread->SendMessageToClient( wrapper, connectionId, gatewayId );
   return true;
}

//////////////////////////////////////////////////////////////
