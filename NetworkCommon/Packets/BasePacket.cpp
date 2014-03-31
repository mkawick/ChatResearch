// BasePacket.cpp

#include "../ServerConstants.h"
#include "BasePacket.h"
#include "../Serialize.h"
#include "PacketFactory.h"

#include <assert.h>



#ifdef _MEMORY_TEST_
int BasePacket::m_counter = 0;
#endif

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

bool  BasePacket::SerializeIn( const U8* data, int& bufferOffset )
{ 
   Serialize::In( data, bufferOffset, packetType );
   Serialize::In( data, bufferOffset, packetSubType );
   Serialize::In( data, bufferOffset, versionNumber );
   Serialize::In( data, bufferOffset, gameProductId );   
   Serialize::In( data, bufferOffset, gameInstanceId );

   return true; 
}

bool  BasePacket::SerializeOut( U8* data, int& bufferOffset ) const 
{ 
   Serialize::Out( data, bufferOffset, packetType );
   Serialize::Out( data, bufferOffset, packetSubType );
   Serialize::Out( data, bufferOffset, versionNumber );
   Serialize::Out( data, bufferOffset, gameProductId );
   Serialize::Out( data, bufferOffset, gameInstanceId );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketCommsHandshake::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, serverHashedKey );

   return true;
}

bool  PacketCommsHandshake::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, serverHashedKey );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketRerouteRequest::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );

   return true;
}

bool  PacketRerouteRequest::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketRerouteRequestResponse::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, locations );

   return true;
}

bool  PacketRerouteRequestResponse::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, locations );

   return true;
}

///////////////////////////////////////////////////////////////

bool PacketRerouteRequestResponse::Address::SerializeIn( const U8* data, int& bufferOffset )
{
   Serialize::In( data, bufferOffset, address );
   Serialize::In( data, bufferOffset, name );
   Serialize::In( data, bufferOffset, port );
   Serialize::In( data, bufferOffset, whichLocationId );

   return true;
}

bool  PacketRerouteRequestResponse::Address::SerializeOut( U8* data, int& bufferOffset ) const
{
   Serialize::Out( data, bufferOffset, address );
   Serialize::Out( data, bufferOffset, name );
   Serialize::Out( data, bufferOffset, port );
   Serialize::Out( data, bufferOffset, whichLocationId );

   return true;
}


///////////////////////////////////////////////////////////////


bool  PacketFriendsList::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   friendList.SerializeIn( data, bufferOffset );

   return true;
}

bool  PacketFriendsList::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   friendList.SerializeOut( data, bufferOffset );

   return true;
}

///////////////////////////////////////////////////////////////


bool  PacketGroupsList::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   //groupList.SerializeIn( data, bufferOffset );

   return true;
}

bool  PacketGroupsList::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   //groupList.SerializeOut( data, bufferOffset );

   return true;
}

///////////////////////////////////////////////////////////////


bool  PacketUserStateChange::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, uuid );
   Serialize::In( data, bufferOffset, username );

   return true;
}

bool  PacketUserStateChange::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, uuid );
   Serialize::Out( data, bufferOffset, username );

   return true;
}

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

U8 PacketGatewayWrapper::SerializeBuffer[ PacketGatewayWrapper::BufferSize ];

void  PacketGatewayWrapper::SetupPacket( BasePacket* packet, U32 connId )
{
   size = 0;
   int tempSize = 0;
   packet->SerializeOut( SerializeBuffer, tempSize ); // get the size info
   size = tempSize;
   pPacket = packet;
   connectionId = connId;

   gameInstanceId = packet->gameInstanceId;
   gameProductId = packet->gameProductId;
}

///////////////////////////////////////////////////////////////

bool  PacketGatewayWrapper::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, connectionId );
   Serialize::In( data, bufferOffset, size );

   delete pPacket; pPacket = NULL;
   PacketFactory packetFactory;

   if( packetFactory.Parse( data, bufferOffset, &pPacket ) == false )
   {
      return false;
   }

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketGatewayWrapper::HeaderSerializeIn( const U8* data, int bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, connectionId );
   Serialize::In( data, bufferOffset, size );

   delete pPacket; pPacket = NULL;
   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketGatewayWrapper::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );   
   Serialize::Out( data, bufferOffset, connectionId );
   Serialize::Out( data, bufferOffset, size );

   if( pPacket == NULL )
   {
      assert( 0 );
   }
   pPacket->SerializeOut( data, bufferOffset );

   return true;
}

///////////////////////////////////////////////////////////////


bool  PacketErrorReport::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, errorCode );
   Serialize::In( data, bufferOffset, statusInfo );

   return true;
}

bool  PacketErrorReport::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, errorCode );
   Serialize::Out( data, bufferOffset, statusInfo );

   return true;
}

///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////