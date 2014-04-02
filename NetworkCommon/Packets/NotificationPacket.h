// NotificationPacket.h
#pragma once

#include "BasePacket.h"

///////////////////////////////////////////////////////////////////

class NotificationPacket : public BasePacket 
{
public:
   enum NotificationType
   {
      NotificationType_Base,
      NotificationType_TestNotification,
      NotificationType_EchoToServer,
      NotificationType_EchoToClient,

      NotificationType_RegisterDevice,
      NotificationType_SendNotification,
      //NotificationType_RequestListOfSales,
      //NotificationType_RequestListOfSalesResponse

   };

public:
   NotificationPacket( int packet_type = PacketType_Notification, int packet_sub_type = NotificationType_Base ) : BasePacket( packet_type, packet_sub_type ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;
};

///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////

class NotificationPacket_TestNotification : public NotificationPacket
{
public:
   NotificationPacket_TestNotification() : NotificationPacket( PacketType_Notification, NotificationType_TestNotification ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   message;
   int      type;
};

///////////////////////////////////////////////////////////////

class NotificationPacket_EchoToServer : public BasePacket
{
public:
   NotificationPacket_EchoToServer(): BasePacket( PacketType_Notification, NotificationPacket::NotificationType_EchoToServer ) {}
};

///////////////////////////////////////////////////////////////

class NotificationPacket_EchoToClient : public BasePacket
{
public:
   NotificationPacket_EchoToClient(): BasePacket( PacketType_Notification, NotificationPacket::NotificationType_EchoToClient ) {}
};

///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////

class NotificationPacket_RegisterDevice : public NotificationPacket
{
public:
   NotificationPacket_RegisterDevice() : NotificationPacket( PacketType_Notification, NotificationType_RegisterDevice ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   userUuid;
   U32      userId;
   string   deviceId;
};

///////////////////////////////////////////////////////////////

class NotificationPacket_SendNotification: public NotificationPacket
{
public:
   NotificationPacket_SendNotification() : NotificationPacket( PacketType_Notification, NotificationType_SendNotification ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   userUuid;
   U32      userId;
   int      notificationType; // see the enum in server_notify.h
   //bool     success;
};


///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////