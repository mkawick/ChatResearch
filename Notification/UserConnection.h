#pragma once

#include "../NetworkCommon/Utils/Enigmosaurus.h"
#include "../NetworkCommon/Packets/LoginPacket.h"
#include "../NetworkCommon/Packets/NotificationPacket.h"
#include "../NetworkCommon/UserAccount/UserAccountCommon.h"

///////////////////////////////////////////////////////////////////

struct ExtendedRegisteredDevice : public RegisteredDevice
{
   U32      userDeviceId;
   string   deviceId;
};

struct UserDeviceNotifications
{
   int      id;
   int      userDeviceId; // refers to the user_device table 
   int      gameType;
   bool     isEnabled;
   string   deviceId;
   string   audioFile;
   U8       repeatFrequencyInHours;
};

class NotificationMainThread;
class PacketDbQueryResult;

///////////////////////////////////////////////////////////////////

class UserConnection : public UserLoginBase  // this class exists most to handle devices
{
public:
   UserConnection();// const PacketPrepareForUserLogin* info );
   ~UserConnection();

   bool        HandleDbQueryResult( const PacketDbQueryResult* result );

   bool        IsReadyToAcceptClientRequests() const;
   bool        HandleRequestFromClient( const BasePacket* packet, U32 connectionId );

   /*const PacketPrepareForUserLogin&
               GetUserInfo() const { return m_userInfo; }*/

 /*  void        SetConnectionId( U32 id ) { m_userInfo.connectionId = id; }
   void        SetGatewayId( U32 id ) { m_userInfo.gatewayId = id; }*/

   bool                 IsLoggedOut() const { if( GetFirstConnectedId() == 0 ) return true; return false; }

   //void        Relog();
   //int         SecondsExpiredSinceLoggedOut();
   //void        UserLogged() { m_isLoggedOut = false; }
   //void        UserLoggedOut();

   //bool        NeedsUpdate() { m_requiresUpdate = true; }
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

   void        SendErrorToclient( U32 error );
   void        RequestAllDevicesForUser();
   void        RequestAllDeviceNotification();

   void        StoreListOfDevices( const PacketDbQueryResult* result );
   void        StoreDevicesPerGameList( const PacketDbQueryResult* result );
   void        CreateEnabledNotificationEntry( const PacketDbQueryResult* dbResult );
   bool        FindDeviceAndUpdate( RegisteredDeviceList& deviceList, const PacketNotification_RegisterDevice* registerDevice, U32 connectionId );

   void        TestNotification( const char* text, U32 type, U32 connectionId );
   void        RegisterNewDevice( const PacketNotification_RegisterDevice* registerDevice, U32 connectionId );
   void        SendNewDeviceRegistrationResponse( const string& uuid, U32 connectionId );


   void        UpdateDevice( const PacketNotification_UpdateDevice* device, U32 connectionId );
   void        RequestDevicesList( const PacketNotification_RequestListOfDevices* device, U32 connectionId );
   void        RemoveDevice( const PacketNotification_RemoveDevice* removal, U32 connectionId );
   void        UpdateDbRecordForDevice( U32 userDeviceId, U32 connectionId );
   void        CreateNewDeviceNotificationEntry( U32 userDeviceId, U32 gameType, const string& deviceId );
   bool        SendMessageToClient( BasePacket* packet, U32 connectionId ) const;

   

   RegisteredDeviceIterator      FindDeviceByUuid( const string& uuid );
   RegisteredDeviceIterator      FindDeviceById( U32 id );
   DeviceNotificationsIterator   FindDeviceNotification( U32 userDeviceId, U8 gameProductId );
   DeviceNotificationsIterator   FindDeviceNotificationByDeviceId( const string& uuid, U8 gameProductId );

   void        GrandfatherInOldDevices( RegisteredDeviceIterator iter, const string& newName, U32 connectionId );

   ///---------------------------------------------

   //PacketPrepareForUserLogin           m_userInfo;
   bool                                m_isLoggedOut;
   bool                                m_hasRequestedDevices;
   bool                                m_finishedLoadingListOfDevices;
   bool                                m_finishedLoadingListOfDevicesPerGame;
   time_t                              m_timeLoggedOut;

   RegisteredDeviceList                m_deviceList;
   DeviceNotificationsList             m_deviceEnabledList;

   static NotificationMainThread* m_mainThread;
};

///////////////////////////////////////////////////////////////////
