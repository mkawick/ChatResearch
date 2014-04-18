#include "UserConnection.h"

#include <string>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
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

//------------------------------------------------------------

void  UserConnection::RequestAllDevicesForUser()
{
   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =           m_userInfo.connectionId;
   dbQuery->meta =         "";
   dbQuery->lookup =       QueryType_DeviceList;
   dbQuery->serverLookup = m_userInfo.userId;
   dbQuery->query = "SELECT * FROM user_device WHERE user_uuid='";
   dbQuery->query += m_userInfo.uuid;
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
   dbQuery->query += m_userInfo.uuid;
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
      m_mainThread->SendErrorToClient( m_userInfo.connectionId, PacketErrorReport::ErrorType_Notification_NoDevicesListed );
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
      m_mainThread->SendErrorToClient( m_userInfo.connectionId, PacketErrorReport::ErrorType_Notification_NoDevicesEnabledForThisGame );
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
      dn.deviceId =                          boost::lexical_cast< int  >( row[ TableUserDeviceNotifications::Column_device_id ] );
      dn.gameType =                          boost::lexical_cast< int  >( row[ TableUserDeviceNotifications::Column_game_type ] );
      dn.isEnabled=                          boost::lexical_cast< bool  >( row[ TableUserDeviceNotifications::Column_is_enabled ] );

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
        /* case PacketNotification::NotificationType_EnableDevice:
            {
            }
            break;
         case PacketNotification::
         //   NotificationType_EnableDeviceResponse,*/
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

void     UserConnection::RegisterNewDevice( const PacketNotification_RegisterDevice* registerDevice )
{
   if( registerDevice->deviceId.size() < 8 )
   {
      m_mainThread->SendErrorToClient( m_userInfo.connectionId, PacketErrorReport::ErrorType_Notification_DeviceIdIncorrect );
      return;
   }

   RegisteredDeviceIterator   deviceIt = m_deviceList.begin();
   while( deviceIt != m_deviceList.end() )
   {
      if( deviceIt->deviceId == registerDevice->deviceId )
      {
         // look up the  device notification and if it exists, report this error.
         U32 userDeviceId = deviceIt->userDeviceId;
         DeviceNotificationsIterator   deviceEnabledIt = FindDeviceNotificationByDeviceId( userDeviceId );
         if( deviceEnabledIt != m_deviceEnabledList.end() )
         {
            m_mainThread->SendErrorToClient( m_userInfo.connectionId, PacketErrorReport::ErrorType_Notification_DeviceAlreadyRegistered );
            SendNewDeviceRegistrationResponse( deviceIt->uuid );
         }
         else
         {
            //otherwise, we are logging in from a new game and we need to create a new entry... you are logging in from the same game but a different device
            CreateNewDeviceNotificationEntry( userDeviceId, m_userInfo.gameProductId );
         }

         if( registerDevice->deviceName.size() && deviceIt->name.size() == 0 )
         {
            GrandfatherInOldDevices( deviceIt, registerDevice->deviceName );
         }
         return;
      }
      deviceIt++;
   }

   string newDeviceUuid = GenerateUUID ();

   string query( "INSERT INTO user_device VALUES( DEFAULT, '");
   query += m_userInfo.uuid;
   query += "', '";
   query += newDeviceUuid;
   query += "', x'";
   query += devicetoa( (const unsigned char*)registerDevice->deviceId.c_str(), registerDevice->deviceId.size() );
   query += "', '";
   query += registerDevice->deviceName;
   query += "', '1', '"; // icon id
   query += boost::lexical_cast< string  >( (int) registerDevice->platformId );
   query += "', '";
   query += boost::lexical_cast< string  >( m_userInfo.userId );
   query += "', '1', DEFAULT )";// is_enabled, date

   //cout << query << endl;

   // we're going to assume that this new entry works fine. There is the potential for a uuid conflict, so we'll need to build that later.
   ExtendedRegisteredDevice* rd = new ExtendedRegisteredDevice;
      rd->userDeviceId =  0;// we need to look this up
      rd->name =      registerDevice->deviceName;
      rd->uuid =      newDeviceUuid;
      rd->iconId =    1;
      rd->deviceId =  registerDevice->deviceId;
      rd->platformId = registerDevice->platformId;
      rd->isEnabled =  true;

   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =           m_userInfo.connectionId;
   dbQuery->meta =         newDeviceUuid;
   dbQuery->lookup =       QueryType_InsertDevice;
   dbQuery->serverLookup = m_userInfo.gameProductId;
   dbQuery->query =        query;
   dbQuery->customData  = rd;

   m_mainThread->AddQueryToOutput( dbQuery );


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
      m_mainThread->SendErrorToClient( m_userInfo.connectionId, PacketErrorReport::ErrorType_Notification_CannotInsertNewDevice );
      return;
   }

   U32 userDeviceId = dbResult->insertId;
   U32 gameType = static_cast< U32>( dbResult->serverLookup );

   //----------------------------
   rd->deviceId = userDeviceId;
   m_deviceList.push_back( *rd );
   delete rd;
   //----------------------------

   CreateNewDeviceNotificationEntry( userDeviceId, gameType );   
}

//------------------------------------------------------------

// we do not make all of the same error checks here... it's too costly and assumed to have already have been performed
void  UserConnection::CreateNewDeviceNotificationEntry( U32 userDeviceId, U32 gameType )
{
   string query( "INSERT INTO user_device_notification VALUES( DEFAULT, ");
   query += boost::lexical_cast< string  >( userDeviceId );
   query += ", ";
   query += boost::lexical_cast< string  >( gameType );
   query += ", 1, DEFAULT )";// is enabled, timestamp

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
      m_mainThread->SendErrorToClient( m_userInfo.connectionId, PacketErrorReport::ErrorType_Notification_DeviceIdIncorrect );
      return;
   }

   bool deviceFound = false;
   U32   userDeviceId = 0;
   const string& uuid = updateDevicePacket->deviceUuid;

   RegisteredDeviceIterator deviceIt = FindDeviceByUuid( uuid );
   if( deviceIt == m_deviceList.end() )
   {
      string text = "UpdateDevice:: User ";
      text += m_userInfo.userName;
      text += " tried to update a device but it could not be found ";
      text += updateDevicePacket->deviceName;
      text += ", ";
      text += updateDevicePacket->deviceUuid;
      text += " but the device does not exist in UserConnection";
      m_mainThread->Log( text, 1 );
      m_mainThread->SendErrorToClient( m_userInfo.connectionId, PacketErrorReport::ErrorType_Notification_DeviceIdIncorrect );
      return;
   }
   
   userDeviceId = deviceIt->userDeviceId;

   if( userDeviceId == 0 )
   {
      string text = "UpdateDevice:: User ";
      text += m_userInfo.userName;
      text += " tried to update a device but it could not be found ";
      text += updateDevicePacket->deviceName;
      text += ", ";
      text += updateDevicePacket->deviceUuid;
      text += " but the device does not exist in UserConnection";
      m_mainThread->Log( text, 1 );
      return;
   }

   DeviceNotificationsIterator deviceEnabledIt = FindDeviceNotificationByDeviceId( userDeviceId );
   if( deviceEnabledIt == m_deviceEnabledList.end() )
   {
      string text = "UpdateDevice:: User ";
      text += m_userInfo.userName;
      text += " tried to update a device but the enabled record could not be found ";
      text += updateDevicePacket->deviceName;
      text += ", ";
      text += updateDevicePacket->deviceUuid;
      text += " but the device does not exist in UserConnection";
      m_mainThread->Log( text, 1 );
      m_mainThread->SendErrorToClient( m_userInfo.connectionId, PacketErrorReport::ErrorType_Notification_DeviceIdIncorrect );
      return;
   }

   // update
   deviceIt->iconId = updateDevicePacket->iconId;
   deviceIt->name = updateDevicePacket->deviceName;
   
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
      text += m_userInfo.userName;
      text += " tried to update a device but it could not be found ";
      text += " but the device does not exist in UserConnection";
      m_mainThread->Log( text, 1 );
      return;
   }

   U32 userDeviceId = deviceIt->userDeviceId;
   
   DeviceNotificationsIterator deviceEnabledIt = FindDeviceNotificationByDeviceId( userDeviceId );
   if( deviceEnabledIt == m_deviceEnabledList.end() )
   {
      return;
   }


   string query( "UPDATE user_device SET name='");
   query += deviceIt->name;
   query += "', icon_id=";
   query += boost::lexical_cast< string  >( deviceIt->iconId );
   query += " WHERE id=";
   query += boost::lexical_cast< string  >( deviceIt->userDeviceId );

   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =           m_userInfo.connectionId;
   dbQuery->meta =         "";
   dbQuery->lookup =       QueryType_UpdateDevice;
   dbQuery->serverLookup = userDeviceId;
   dbQuery->query =        query;

   m_mainThread->AddQueryToOutput( dbQuery );



   string query2( "UPDATE user_device_notification SET is_enabled=");
   query2 += boost::lexical_cast< string  >( deviceEnabledIt->isEnabled );

   query2 += " where id=";// is enabled, timestamp
   query2 += boost::lexical_cast< string  >( deviceEnabledIt->id );
   
   //----------------------------

   dbQuery = new PacketDbQuery;
   dbQuery->id =           m_userInfo.connectionId;
   dbQuery->meta =         "";
   dbQuery->lookup =       QueryType_InsertDeviceNotification;
   dbQuery->serverLookup = userDeviceId;
   dbQuery->query =        query2;
   dbQuery->isFireAndForget = true;

   m_mainThread->AddQueryToOutput( dbQuery );
}

//------------------------------------------------------------

void        UserConnection::RequestDevicesList( const PacketNotification_RequestListOfDevices* device )
{
   PacketNotification_RequestListOfDevicesResponse* response = new PacketNotification_RequestListOfDevicesResponse;

   RegisteredDeviceIterator deviceIt = m_deviceList.begin();
   while( deviceIt != m_deviceList.end() )
   {
      U32 userDeviceId = deviceIt->userDeviceId;
      DeviceNotificationsIterator deviceNotificationIter = FindDeviceNotificationByDeviceId( userDeviceId );
      if( deviceNotificationIter != m_deviceEnabledList.end() )
      {
         RegisteredDevice rd;
         rd.iconId = deviceIt->iconId;
         rd.isEnabled = deviceNotificationIter->isEnabled;
         rd.name = deviceIt->name;
         rd.platformId = deviceIt->platformId;
         rd.productId = deviceNotificationIter->gameType;
         rd.uuid = deviceIt->uuid;
         response->devices.push_back( rd );
      }
      deviceIt++;
   }

   SendMessageToClient( response );
}

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
   UserConnection::FindDeviceNotificationByDeviceId( U32 id )
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
   PacketGatewayWrapper* wrapper = new PacketGatewayWrapper();
   wrapper->SetupPacket( packet, connectionId );

   m_mainThread->SendMessageToClient( wrapper, connectionId );
   return true;
}

//////////////////////////////////////////////////////////////
