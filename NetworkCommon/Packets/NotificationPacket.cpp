// NotificationPacket.cpp

#include "NotificationPacket.h"


///////////////////////////////////////////////////////////////

bool  NotificationPacket::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );

   return true;
}

bool  NotificationPacket::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );

   return true;
}

///////////////////////////////////////////////////////////////

bool  NotificationPacket_TestNotification::SerializeIn( const U8* data, int& bufferOffset )
{
   NotificationPacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, message );
   Serialize::In( data, bufferOffset, type );

   return true;
}

bool  NotificationPacket_TestNotification::SerializeOut( U8* data, int& bufferOffset ) const
{
   NotificationPacket::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, message );
   Serialize::Out( data, bufferOffset, type );

   return true;
}

///////////////////////////////////////////////////////////////
/*
bool  NotificationPacket_EchoToServer::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, name );
   Serialize::In( data, bufferOffset, uuid );

   return true;
}

bool  NotificationPacket_EchoToServer::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, name );
   Serialize::Out( data, bufferOffset, uuid );

   return true;
}*/

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
/*
bool  NotificationPacket_EchoToClient::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, name );
   Serialize::In( data, bufferOffset, uuid );

   return true;
}

bool  NotificationPacket_EchoToClient::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, name );
   Serialize::Out( data, bufferOffset, uuid );

   return true;
}*/

///////////////////////////////////////////////////////////////

bool  NotificationPacket_RegisterDevice::SerializeIn( const U8* data, int& bufferOffset )
{
   NotificationPacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, userUuid );
   Serialize::In( data, bufferOffset, userId );
   Serialize::In( data, bufferOffset, deviceId );

   return true;
}

bool  NotificationPacket_RegisterDevice::SerializeOut( U8* data, int& bufferOffset ) const
{
   NotificationPacket::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, userUuid );
   Serialize::Out( data, bufferOffset, userId );
   Serialize::Out( data, bufferOffset, deviceId );

   return true;
}

///////////////////////////////////////////////////////////////

bool  NotificationPacket_SendNotification::SerializeIn( const U8* data, int& bufferOffset )
{
   NotificationPacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, userUuid );
   Serialize::In( data, bufferOffset, userId );
   Serialize::In( data, bufferOffset, notificationType );

   return true;
}

bool  NotificationPacket_SendNotification::SerializeOut( U8* data, int& bufferOffset ) const
{
   NotificationPacket::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, userUuid );
   Serialize::Out( data, bufferOffset, userId );
   Serialize::Out( data, bufferOffset, notificationType );

   return true;
}

///////////////////////////////////////////////////////////////