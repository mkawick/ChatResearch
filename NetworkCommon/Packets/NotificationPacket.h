// NotificationPacket.h
#pragma once

#include "BasePacket.h"

///////////////////////////////////////////////////////////////////

struct RegisteredDevice
{
public:
   string   uuid;
   string   name;   
   bool     isEnabled;
   U32      iconId;
   U8       productId;
   U8       platformId;   // int   FindPlatformId( const char* value ).. const char*   FindPlatformName( int platformId )

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;
};

///////////////////////////////////////////////////////////////////

class PacketNotification : public BasePacket 
{
public:
   enum NotificationType
   {
      NotificationType_Base,
      NotificationType_TestNotification,
      NotificationType_EchoToServer,
      NotificationType_EchoToClient,

      NotificationType_RegisterDevice,
      NotificationType_RegisterDeviceResponse,
      NotificationType_UpdateDevice,
      NotificationType_SendNotification,
      NotificationType_UpdateNotificationCount,

      NotificationType_RequestListOfDevices,  // based on product.. 0 means all devices
      NotificationType_RequestListOfDevicesResponse,

      NotificationType_RemoveDevice,
      NotificationType_RemoveDeviceResponse

     /* NotificationType_EnableDevice,
      NotificationType_EnableDeviceResponse,*/
   };

public:
   PacketNotification( int packet_type = PacketType_Notification, int packet_sub_type = NotificationType_Base ) : BasePacket( packet_type, packet_sub_type ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;
};

///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////

class PacketNotification_TestNotification : public PacketNotification
{
public:
   PacketNotification_TestNotification() : PacketNotification( PacketType_Notification, NotificationType_TestNotification ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   message;
   int      type;
};

///////////////////////////////////////////////////////////////

class PacketNotification_EchoToServer : public BasePacket
{
public:
   PacketNotification_EchoToServer(): BasePacket( PacketType_Notification, PacketNotification::NotificationType_EchoToServer ) {}
};

///////////////////////////////////////////////////////////////

class PacketNotification_EchoToClient : public BasePacket
{
public:
   PacketNotification_EchoToClient(): BasePacket( PacketType_Notification, PacketNotification::NotificationType_EchoToClient ) {}
};

///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////

class PacketNotification_RegisterDevice : public PacketNotification
{
public:
   PacketNotification_RegisterDevice() : PacketNotification( PacketType_Notification, NotificationType_RegisterDevice ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   deviceName; 
   string   deviceId;
   string   assignedUuid;
   U8       platformId;
};

///////////////////////////////////////////////////////////////

class PacketNotification_RegisterDeviceResponse : public PacketNotification
{
public:
   PacketNotification_RegisterDeviceResponse() : PacketNotification( PacketType_Notification, NotificationType_RegisterDeviceResponse ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   deviceUuid;
};

///////////////////////////////////////////////////////////////

class PacketNotification_UpdateDevice : public PacketNotification
{
public:
   PacketNotification_UpdateDevice() : PacketNotification( PacketType_Notification, NotificationType_UpdateDevice ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   deviceUuid;
   string   deviceName;
   bool     isEnabled;
   int      iconId;
   int      gameType;
};

///////////////////////////////////////////////////////////////

class PacketNotification_SendNotification: public PacketNotification
{
public:
   PacketNotification_SendNotification() : PacketNotification( PacketType_Notification, NotificationType_SendNotification ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string         userUuid;
   U32            userId;
   int            gameType;
   unsigned int   gameId;
   int            notificationType; // see the enum in server_notify.h
   string         additionalText;
};

///////////////////////////////////////////////////////////////

class PacketNotification_UpdateNotificationCount: public PacketNotification
{
public:
   PacketNotification_UpdateNotificationCount() : PacketNotification( PacketType_Notification, NotificationType_UpdateNotificationCount ) {  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   U32            userId;
   int            gameType;
   int            notificationCount;
};


///////////////////////////////////////////////////////////////

class PacketNotification_RequestListOfDevices: public PacketNotification
{
public:
   PacketNotification_RequestListOfDevices() : PacketNotification( PacketType_Notification, NotificationType_RequestListOfDevices ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   U32      platformId;
};

///////////////////////////////////////////////////////////////

class PacketNotification_RequestListOfDevicesResponse: public PacketNotification
{
public:
   PacketNotification_RequestListOfDevicesResponse() : PacketNotification( PacketType_Notification, NotificationType_RequestListOfDevicesResponse ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   SerializedVector< RegisteredDevice > devices;
};

///////////////////////////////////////////////////////////////

class PacketNotification_RemoveDevice: public PacketNotification
{
public:
   PacketNotification_RemoveDevice() : PacketNotification( PacketType_Notification, NotificationType_RemoveDevice ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string      deviceUuid;
};

///////////////////////////////////////////////////////////////

class PacketNotification_RemoveDeviceResponse: public PacketNotification
{
public:
   PacketNotification_RemoveDeviceResponse() : PacketNotification( PacketType_Notification, NotificationType_RemoveDeviceResponse ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string      deviceUuid;
   bool        success;
};



///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////