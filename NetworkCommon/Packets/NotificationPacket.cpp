// PacketNotification.cpp

#include "NotificationPacket.h"


///////////////////////////////////////////////////////////////

bool  RegisteredDevice::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   Serialize::In( data, bufferOffset, uuid, minorVersion );
   Serialize::In( data, bufferOffset, name, minorVersion );
   Serialize::In( data, bufferOffset, isEnabled, minorVersion );
   Serialize::In( data, bufferOffset, iconId, minorVersion );
   Serialize::In( data, bufferOffset, productId, minorVersion );
   Serialize::In( data, bufferOffset, platformId, minorVersion );
   Serialize::In( data, bufferOffset, audioFile, minorVersion );
   Serialize::In( data, bufferOffset, repeatFrequencyInHours, minorVersion );
   return true;
}

bool  RegisteredDevice::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   Serialize::Out( data, bufferOffset, uuid, minorVersion );
   Serialize::Out( data, bufferOffset, name, minorVersion );
   Serialize::Out( data, bufferOffset, isEnabled, minorVersion );
   Serialize::Out( data, bufferOffset, iconId, minorVersion );
   Serialize::Out( data, bufferOffset, productId, minorVersion );
   Serialize::Out( data, bufferOffset, platformId, minorVersion );
   Serialize::Out( data, bufferOffset, audioFile, minorVersion );
   Serialize::Out( data, bufferOffset, repeatFrequencyInHours, minorVersion );
   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketNotification::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );

   return true;
}

bool  PacketNotification::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketNotification_TestNotification::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketNotification::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, message, minorVersion );
   Serialize::In( data, bufferOffset, type, minorVersion );

   return true;
}

bool  PacketNotification_TestNotification::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketNotification::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, message, minorVersion );
   Serialize::Out( data, bufferOffset, type, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////
/*
bool  PacketNotification_EchoToServer::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, name );
   Serialize::In( data, bufferOffset, uuid );

   return true;
}

bool  PacketNotification_EchoToServer::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, name );
   Serialize::Out( data, bufferOffset, uuid );

   return true;
}*/

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
/*
bool  PacketNotification_EchoToClient::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, name );
   Serialize::In( data, bufferOffset, uuid );

   return true;
}

bool  PacketNotification_EchoToClient::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, name );
   Serialize::Out( data, bufferOffset, uuid );

   return true;
}*/

///////////////////////////////////////////////////////////////

bool  PacketNotification_RegisterDevice::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketNotification::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, deviceName, minorVersion );
   Serialize::In( data, bufferOffset, deviceId, minorVersion );
   Serialize::In( data, bufferOffset, assignedUuid, minorVersion );
   Serialize::In( data, bufferOffset, platformId, minorVersion );

   return true;
}

bool  PacketNotification_RegisterDevice::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{ 
   PacketNotification::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, deviceName, minorVersion );
   Serialize::Out( data, bufferOffset, deviceId, minorVersion );
   Serialize::Out( data, bufferOffset, assignedUuid, minorVersion );
   Serialize::Out( data, bufferOffset, platformId, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketNotification_RegisterDeviceResponse::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketNotification::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, deviceUuid, minorVersion );

   return true;
}

bool  PacketNotification_RegisterDeviceResponse::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketNotification::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, deviceUuid, minorVersion );
   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketNotification_UpdateDevice::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketNotification::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, deviceUuid, minorVersion );
   Serialize::In( data, bufferOffset, deviceName, minorVersion );
   Serialize::In( data, bufferOffset, isEnabled, minorVersion );
   Serialize::In( data, bufferOffset, iconId, minorVersion );
   Serialize::In( data, bufferOffset, gameType, minorVersion );
   Serialize::In( data, bufferOffset, audioFile, minorVersion );
   Serialize::In( data, bufferOffset, repeatFrequencyInHours, minorVersion );

   return true;
}

bool  PacketNotification_UpdateDevice::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketNotification::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, deviceUuid, minorVersion );
   Serialize::Out( data, bufferOffset, deviceName, minorVersion );;
   Serialize::Out( data, bufferOffset, isEnabled, minorVersion );
   Serialize::Out( data, bufferOffset, iconId, minorVersion );
   Serialize::Out( data, bufferOffset, gameType, minorVersion );
   Serialize::Out( data, bufferOffset, audioFile, minorVersion );
   Serialize::Out( data, bufferOffset, repeatFrequencyInHours, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketNotification_SendNotification::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketNotification::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, userUuid, minorVersion );
   Serialize::In( data, bufferOffset, userId, minorVersion );
   Serialize::In( data, bufferOffset, gameType, minorVersion );
   Serialize::In( data, bufferOffset, gameId, minorVersion );
   Serialize::In( data, bufferOffset, notificationType, minorVersion );
   Serialize::In( data, bufferOffset, additionalText, minorVersion );

   return true;
}

bool  PacketNotification_SendNotification::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketNotification::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, userUuid, minorVersion );
   Serialize::Out( data, bufferOffset, userId, minorVersion );
   Serialize::Out( data, bufferOffset, gameType, minorVersion );
   Serialize::Out( data, bufferOffset, gameId, minorVersion );
   Serialize::Out( data, bufferOffset, notificationType, minorVersion );
   Serialize::Out( data, bufferOffset, additionalText, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketNotification_UpdateNotificationCount::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketNotification::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, userId, minorVersion );
   Serialize::In( data, bufferOffset, gameType, minorVersion );
   Serialize::In( data, bufferOffset, notificationCount, minorVersion );

   return true;
}

bool  PacketNotification_UpdateNotificationCount::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketNotification::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, userId, minorVersion );
   Serialize::Out( data, bufferOffset, gameType, minorVersion );
   Serialize::Out( data, bufferOffset, notificationCount, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketNotification_RequestListOfDevices::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketNotification::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, platformId, minorVersion );

   return true;
}

bool  PacketNotification_RequestListOfDevices::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketNotification::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, platformId, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketNotification_RequestListOfDevicesResponse::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketNotification::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, devices, minorVersion );

   return true;
}

bool  PacketNotification_RequestListOfDevicesResponse::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketNotification::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, devices, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketNotification_RemoveDevice::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketNotification::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, deviceUuid, minorVersion );

   return true;
}

bool  PacketNotification_RemoveDevice::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketNotification::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, deviceUuid, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketNotification_RemoveDeviceResponse::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketNotification::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, deviceUuid, minorVersion );
   Serialize::In( data, bufferOffset, success, minorVersion );

   return true;
}

bool  PacketNotification_RemoveDeviceResponse::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketNotification::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, deviceUuid, minorVersion );
   Serialize::Out( data, bufferOffset, success, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////