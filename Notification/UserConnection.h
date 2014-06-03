#pragma once

#include "../NetworkCommon/Utils/Enigmosaurus.h"
#include "../NetworkCommon/Packets/LoginPacket.h"
#include "../NetworkCommon/Packets/NotificationPacket.h"


///////////////////////////////////////////////////////////////////

struct ExtendedRegisteredDevice : public RegisteredDevice
{
   int      userDeviceId;
   string   deviceId;
};

struct UserDeviceNotifications
{
   int      id;
   int      userDeviceId; // refers to the user_device table 
   int      gameType;
   bool     isEnabled;
   string   deviceId;
};

class NotificationMainThread;
class PacketDbQueryResult;

///////////////////////////////////////////////////////////////////

class UserConnection  // this class exists most to handle devices
{
public:
   UserConnection( const PacketPrepareForUserLogin* info );
   ~UserConnection();

   bool        HandleDbQueryResult( const PacketDbQueryResult* result );
   bool        HandleRequestFromClient( const BasePacket* packet );

   const PacketPrepareForUserLogin&
               GetUserInfo() const { return m_userInfo; }

   void        SetConnectionId( U32 id ) { m_userInfo.connectionId = id; }

   bool        IsLoggedOut() const { return m_isLoggedOut; }
   void        Relog();
   int         SecondsExpiredSinceLoggedOut();
   void        UserLogged() { m_isLoggedOut = false; }
   void        UserLoggedOut();

   bool        NeedsUpdate() { m_requiresUpdate = true; }
   void        Update();


   static void SetServer( NotificationMainThread* main ) { m_mainThread = main; }
private:

   typedef  list< ExtendedRegisteredDevice >    RegisteredDeviceList;
   typedef  RegisteredDeviceList::iterator      RegisteredDeviceIterator;

   typedef  list< UserDeviceNotifications >     DeviceNotificationsList;
   typedef  DeviceNotificationsList::iterator   DeviceNotificationsIterator;

   enum 
   {
      QueryType_DeviceList,
      QueryType_DevicePerGameList,
      QueryType_InsertDevice,
      QueryType_InsertDeviceNotification,
      QueryType_UpdateDevice,
      QueryType_UpdateDeviceNotification,
      QueryType_Request,
      QueryType_DeleteDevice,
      QueryType_DeleteDeviceNotification
   };

private:

   void        RequestAllDevicesForUser();
   void        RequestAllDeviceNotification();

   void        StoreListOfDevices( const PacketDbQueryResult* result );
   void        StoreDevicesPerGameList( const PacketDbQueryResult* result );
   void        CreateEnabledNotificationEntry( const PacketDbQueryResult* dbResult );
   bool        FindDeviceAndUpdate( RegisteredDeviceList deviceList, const PacketNotification_RegisterDevice* registerDevice );
   void        RegisterNewDevice( const PacketNotification_RegisterDevice* registerDevice );
   void        SendNewDeviceRegistrationResponse( const string& uuid );


   void        UpdateDevice( const PacketNotification_UpdateDevice* device );
   void        RequestDevicesList( const PacketNotification_RequestListOfDevices* device );
   void        RemoveDevice( const PacketNotification_RemoveDevice* removal );
   void        UpdateDbRecordForDevice( U32 id );
   void        CreateNewDeviceNotificationEntry( U32 userDeviceId, U32 gameType, const string& deviceId );
   bool        SendMessageToClient( BasePacket* packet ) const;

   

   RegisteredDeviceIterator      FindDeviceByUuid( const string& uuid );
   RegisteredDeviceIterator      FindDeviceById( U32 id );
   DeviceNotificationsIterator   FindDeviceNotificationByUserDeviceId( U32 id );
   DeviceNotificationsIterator   FindDeviceNotificationByDeviceId( const string& uuid );

   void        GrandfatherInOldDevices( RegisteredDeviceIterator iter, const string& newName );

   ///---------------------------------------------

   PacketPrepareForUserLogin           m_userInfo;
   bool                                m_isLoggedOut;
   bool                                m_hasRequestedDevices;
   time_t                              m_timeLoggedOut;

   RegisteredDeviceList                m_deviceList;
   DeviceNotificationsList             m_deviceEnabledList;
   bool                                m_requiresUpdate;

   static NotificationMainThread* m_mainThread;
};

///////////////////////////////////////////////////////////////////
