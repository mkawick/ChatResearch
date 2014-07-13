// BasePacket.cpp


#include "../ServerConstants.h"
#if PLATFORM == PLATFORM_WINDOWS
#pragma warning( disable:4996 )
#endif

#include "BasePacket.h"
#include "Serialize.h"
#include "PacketFactory.h"

#include <assert.h>


const U8   GlobalNetworkProtocolVersion = 41;

#ifdef _MEMORY_TEST_
int BasePacket::m_counter = 0;
#endif

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

const char* GetPacketTypename( PacketType type )
{
   switch( type )
   {
   case PacketType_Base:
      return "Base";

   case PacketType_Login:
      return "Login";

   case PacketType_Chat:
      return "Chat";

   case PacketType_UserInfo:
      return "UserInfo";

   case PacketType_Contact:
      return "Contact";

   case PacketType_Asset:
      return "Asset";

   case PacketType_UserStateChange: // from server to client, usually
      return "User state change";

   case PacketType_DbQuery:
      return "DbQuery";

   case PacketType_Gameplay:
      return "Gameplay";

   case PacketType_GatewayWrapper:
      return "GW wrapper";

   case PacketType_ServerToServerWrapper:
      return "S2S Wrapper";

   case PacketType_ServerJobWrapper:
      return "Server job wrapper";

   case PacketType_ServerInformation:
      return "Server info";

   case PacketType_GatewayInformation: // user logged out, prepare to shutdown, etc.
      return "GW info";

   case PacketType_ErrorReport:
      return "Error";

   case PacketType_Cheat:
      return "Cheat";

   case PacketType_Purchase:
      return "Purchase";

   case PacketType_Tournament:
      return "Tournament";

   case PacketType_Analytics:
      return "Analytics";

   case PacketType_Notification:
      return "Notification";

   case PacketType_Invitation:
      return "Invitation";

   case PacketType_UserStats:
      return "UserStats";

   default:
      return "";
   }
}

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

bool  BasePacket::SerializeIn( const U8* data, int& bufferOffset )
{ 
   Serialize::In( data, bufferOffset, packetType );
   Serialize::In( data, bufferOffset, packetSubType );
   Serialize::In( data, bufferOffset, versionNumber );
   Serialize::In( data, bufferOffset, gameProductId );
   //Serialize::In( data, bufferOffset, packetSize ); 
   Serialize::In( data, bufferOffset, gameInstanceId );

   return true; 
}

bool  BasePacket::SerializeOut( U8* data, int& bufferOffset ) const 
{ 
   Serialize::Out( data, bufferOffset, packetType );
   Serialize::Out( data, bufferOffset, packetSubType );
   Serialize::Out( data, bufferOffset, versionNumber );
   Serialize::Out( data, bufferOffset, gameProductId );
   //Serialize::Out( data, bufferOffset, packetSize );
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
   Serialize::In( data, bufferOffset, text );

   return true;
}

bool  PacketErrorReport::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, errorCode );
   Serialize::Out( data, bufferOffset, statusInfo );
   Serialize::Out( data, bufferOffset, text );

   return true;
}

///////////////////////////////////////////////////////////////

bool  Packet_QOS_ReportToClient::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, errorText );
   Serialize::In( data, bufferOffset, errorState );
   Serialize::In( data, bufferOffset, param1 );
   Serialize::In( data, bufferOffset, param2 );

   return true;
}

bool  Packet_QOS_ReportToClient::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, errorText );
   Serialize::Out( data, bufferOffset, errorState );
   Serialize::Out( data, bufferOffset, param1 );
   Serialize::Out( data, bufferOffset, param2 );

   return true;
}

///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////