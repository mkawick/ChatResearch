// BasePacket.cpp


#include "../ServerConstants.h"
#if PLATFORM == PLATFORM_WINDOWS
#pragma warning( disable:4996 )
#endif

#include "BasePacket.h"
#include "Serialize.h"
#include "PacketFactory.h"
#include <assert.h>

const U8   NetworkVersionMajor = 45;
const U8   NetworkVersionMinor = 0;

//#include <boost/static_assert.hpp>
//BOOST_STATIC_ASSERT( NetworkVersionMajor < (1<<5) );// 5 bits for major
//BOOST_STATIC_ASSERT( NetworkVersionMinor < (1<<3) );

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

int   BasePacket::GetSize() 
{ 
   return sizeof( BasePacket ) - 
      3 - //sizeof( padding )
      sizeof( long*); // this accounts for the virtual pointer.
}

bool  BasePacket::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{ 
   Serialize::In( data, bufferOffset, packetType, minorVersion );
   Serialize::In( data, bufferOffset, packetSubType, minorVersion );
   Serialize::In( data, bufferOffset, versionNumberMajor, minorVersion );
   Serialize::In( data, bufferOffset, versionNumberMinor, minorVersion );
   Serialize::In( data, bufferOffset, gameProductId, minorVersion );
   //Serialize::In( data, bufferOffset, packetSize ); 
   Serialize::In( data, bufferOffset, gameInstanceId, minorVersion );

   return true; 
}

bool  BasePacket::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{ 
   Serialize::Out( data, bufferOffset, packetType, minorVersion );
   Serialize::Out( data, bufferOffset, packetSubType, minorVersion );
   Serialize::Out( data, bufferOffset, versionNumberMajor, minorVersion );
   Serialize::Out( data, bufferOffset, versionNumberMinor, minorVersion );
   Serialize::Out( data, bufferOffset, gameProductId, minorVersion );
   //Serialize::Out( data, bufferOffset, packetSize );
   Serialize::Out( data, bufferOffset, gameInstanceId, minorVersion );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketHello::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   return true;
}

bool  PacketHello::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketCommsHandshake::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, serverHashedKey, minorVersion );

   return true;
}

bool  PacketCommsHandshake::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, serverHashedKey, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketRerouteRequest::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );

   return true;
}

bool  PacketRerouteRequest::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketRerouteRequestResponse::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, locations, minorVersion );
   //locations.SerializeIn( data, bufferOffset, minorVersion );

   return true;
}

bool  PacketRerouteRequestResponse::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, locations, minorVersion );
   //locations.SerializeOut( data, bufferOffset, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool PacketRerouteRequestResponse::Address::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   Serialize::In( data, bufferOffset, address, minorVersion );
   Serialize::In( data, bufferOffset, name, minorVersion );
   Serialize::In( data, bufferOffset, port, minorVersion );
   Serialize::In( data, bufferOffset, whichLocationId, minorVersion );

   return true;
}

bool  PacketRerouteRequestResponse::Address::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{
   Serialize::Out( data, bufferOffset, address, minorVersion );
   Serialize::Out( data, bufferOffset, name, minorVersion );
   Serialize::Out( data, bufferOffset, port, minorVersion );
   Serialize::Out( data, bufferOffset, whichLocationId, minorVersion );

   return true;
}


///////////////////////////////////////////////////////////////


bool  PacketFriendsList::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   friendList.SerializeIn( data, bufferOffset, minorVersion );

   return true;
}

bool  PacketFriendsList::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   friendList.SerializeOut( data, bufferOffset, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////


bool  PacketGroupsList::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   //groupList.SerializeIn( data, bufferOffset, minorVersion );

   return true;
}

bool  PacketGroupsList::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   //groupList.SerializeOut( data, bufferOffset, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////


bool  PacketUserStateChange::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, uuid, minorVersion );
   Serialize::In( data, bufferOffset, username, minorVersion );

   return true;
}

bool  PacketUserStateChange::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, uuid, minorVersion );
   Serialize::Out( data, bufferOffset, username, minorVersion );

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

bool  PacketGatewayWrapper::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, connectionId, minorVersion );
   Serialize::In( data, bufferOffset, size, minorVersion );

   delete pPacket; pPacket = NULL;
   PacketFactory packetFactory;

   if( packetFactory.Parse( data, bufferOffset, &pPacket, minorVersion ) == false )
   {
      return false;
   }

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketGatewayWrapper::HeaderSerializeIn( const U8* data, int bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, connectionId, minorVersion );
   Serialize::In( data, bufferOffset, size, minorVersion );

   delete pPacket; pPacket = NULL;
   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketGatewayWrapper::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );   
   Serialize::Out( data, bufferOffset, connectionId, minorVersion );
   Serialize::Out( data, bufferOffset, size, minorVersion );

   if( pPacket == NULL )
   {
      assert( 0 );
   }
   pPacket->SerializeOut( data, bufferOffset, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////


bool  PacketErrorReport::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, errorCode, minorVersion );
   Serialize::In( data, bufferOffset, statusInfo, minorVersion );
   Serialize::In( data, bufferOffset, text, minorVersion );

   return true;
}

bool  PacketErrorReport::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, errorCode, minorVersion );
   Serialize::Out( data, bufferOffset, statusInfo, minorVersion );
   Serialize::Out( data, bufferOffset, text, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  Packet_QOS_ReportToClient::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, errorText, minorVersion );
   Serialize::In( data, bufferOffset, errorState, minorVersion );
   Serialize::In( data, bufferOffset, param1, minorVersion );
   Serialize::In( data, bufferOffset, param2, minorVersion );

   return true;
}

bool  Packet_QOS_ReportToClient::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, errorText, minorVersion );
   Serialize::Out( data, bufferOffset, errorState, minorVersion );
   Serialize::Out( data, bufferOffset, param1, minorVersion );
   Serialize::Out( data, bufferOffset, param2, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketBase_TestOnly::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, testNo, minorVersion );
   Serialize::In( data, bufferOffset, testString, minorVersion );

   return true;
}

bool  PacketBase_TestOnly::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, testNo, minorVersion );
   Serialize::Out( data, bufferOffset, testString, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////