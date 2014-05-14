// PacketNotification.cpp

#include "NotificationPacket.h"


///////////////////////////////////////////////////////////////

bool  RegisteredDevice::SerializeIn( const U8* data, int& bufferOffset )
{
   Serialize::In( data, bufferOffset, uuid );
   Serialize::In( data, bufferOffset, name );
   Serialize::In( data, bufferOffset, isEnabled );
   Serialize::In( data, bufferOffset, iconId );
   Serialize::In( data, bufferOffset, productId );
   Serialize::In( data, bufferOffset, platformId );
   return true;
}

bool  RegisteredDevice::SerializeOut( U8* data, int& bufferOffset ) const
{
   Serialize::Out( data, bufferOffset, uuid );
   Serialize::Out( data, bufferOffset, name );
   Serialize::Out( data, bufferOffset, isEnabled );
   Serialize::Out( data, bufferOffset, iconId );
   Serialize::Out( data, bufferOffset, productId );
   Serialize::Out( data, bufferOffset, platformId );
   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketNotification::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );

   return true;
}

bool  PacketNotification::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketNotification_TestNotification::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketNotification::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, message );
   Serialize::In( data, bufferOffset, type );

   return true;
}

bool  PacketNotification_TestNotification::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketNotification::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, message );
   Serialize::Out( data, bufferOffset, type );

   return true;
}

///////////////////////////////////////////////////////////////
/*
bool  PacketNotification_EchoToServer::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, name );
   Serialize::In( data, bufferOffset, uuid );

   return true;
}

bool  PacketNotification_EchoToServer::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, name );
   Serialize::Out( data, bufferOffset, uuid );

   return true;
}*/

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
/*
bool  PacketNotification_EchoToClient::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, name );
   Serialize::In( data, bufferOffset, uuid );

   return true;
}

bool  PacketNotification_EchoToClient::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, name );
   Serialize::Out( data, bufferOffset, uuid );

   return true;
}*/

///////////////////////////////////////////////////////////////

bool  PacketNotification_RegisterDevice::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketNotification::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, deviceName );
   //Serialize::In( data, bufferOffset, userId );
   Serialize::In( data, bufferOffset, deviceId );
   Serialize::In( data, bufferOffset, platformId );

   return true;
}

bool  PacketNotification_RegisterDevice::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketNotification::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, deviceName );
   //Serialize::Out( data, bufferOffset, userId );
   Serialize::Out( data, bufferOffset, deviceId );
   Serialize::Out( data, bufferOffset, platformId );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketNotification_RegisterDeviceResponse::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketNotification::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, deviceUuid );

   return true;
}

bool  PacketNotification_RegisterDeviceResponse::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketNotification::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, deviceUuid );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketNotification_UpdateDevice::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketNotification::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, deviceUuid );
   Serialize::In( data, bufferOffset, deviceName );
   Serialize::In( data, bufferOffset, isEnabled );
   Serialize::In( data, bufferOffset, iconId );
   Serialize::In( data, bufferOffset, gameType );

   return true;
}

bool  PacketNotification_UpdateDevice::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketNotification::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, deviceUuid );
   Serialize::Out( data, bufferOffset, deviceName );
   Serialize::Out( data, bufferOffset, isEnabled );
   Serialize::Out( data, bufferOffset, iconId );
   Serialize::Out( data, bufferOffset, gameType );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketNotification_SendNotification::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketNotification::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, userUuid );
   Serialize::In( data, bufferOffset, userId );
   Serialize::In( data, bufferOffset, gameType );
   Serialize::In( data, bufferOffset, gameId );
   Serialize::In( data, bufferOffset, notificationType );
   Serialize::In( data, bufferOffset, additionalText );

   return true;
}

bool  PacketNotification_SendNotification::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketNotification::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, userUuid );
   Serialize::Out( data, bufferOffset, userId );
   Serialize::Out( data, bufferOffset, gameType );
   Serialize::Out( data, bufferOffset, gameId );
   Serialize::Out( data, bufferOffset, notificationType );
   Serialize::Out( data, bufferOffset, additionalText );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketNotification_UpdateNotificationCount::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketNotification::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, userId );
   Serialize::In( data, bufferOffset, gameType );
   Serialize::In( data, bufferOffset, notificationCount );

   return true;
}

bool  PacketNotification_UpdateNotificationCount::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketNotification::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, userId );
   Serialize::Out( data, bufferOffset, gameType );
   Serialize::Out( data, bufferOffset, notificationCount );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketNotification_RequestListOfDevices::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketNotification::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, platformId );

   return true;
}

bool  PacketNotification_RequestListOfDevices::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketNotification::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, platformId );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketNotification_RequestListOfDevicesResponse::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketNotification::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, devices );

   return true;
}

bool  PacketNotification_RequestListOfDevicesResponse::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketNotification::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, devices );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketNotification_RemoveDevice::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketNotification::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, deviceUuid );

   return true;
}

bool  PacketNotification_RemoveDevice::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketNotification::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, deviceUuid );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketNotification_RemoveDeviceResponse::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketNotification::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, deviceUuid );
   Serialize::In( data, bufferOffset, success );

   return true;
}

bool  PacketNotification_RemoveDeviceResponse::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketNotification::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, deviceUuid );
   Serialize::Out( data, bufferOffset, success );

   return true;
}

///////////////////////////////////////////////////////////////