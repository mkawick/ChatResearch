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



///////////////////////////////////////////////////////////////


bool  ChannelInfo::SerializeIn( const U8* data, int& bufferOffset )
{
   Serialize::In( data, bufferOffset, channelName );
   Serialize::In( data, bufferOffset, channelUuid );
   Serialize::In( data, bufferOffset, gameProduct );
   Serialize::In( data, bufferOffset, gameId );
   Serialize::In( data, bufferOffset, numNewChats );
   Serialize::In( data, bufferOffset, isActive );

   return true;
}

bool  ChannelInfo::SerializeOut( U8* data, int& bufferOffset ) const
{
   Serialize::Out( data, bufferOffset, channelName );
   Serialize::Out( data, bufferOffset, channelUuid );
   Serialize::Out( data, bufferOffset, gameProduct );
   Serialize::Out( data, bufferOffset, gameId );
   Serialize::Out( data, bufferOffset, numNewChats );
   Serialize::Out( data, bufferOffset, isActive );

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
   groupList.SerializeIn( data, bufferOffset );

   return true;
}

bool  PacketGroupsList::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   groupList.SerializeOut( data, bufferOffset );

   return true;
}

///////////////////////////////////////////////////////////////

PacketChatChannelList::~PacketChatChannelList()
{
   channelList.clear();
}

bool  PacketChatChannelList::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   channelList.SerializeIn( data, bufferOffset );

   return true;
}

bool  PacketChatChannelList::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   channelList.SerializeOut( data, bufferOffset );

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
   //packet->
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