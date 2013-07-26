// ContactPacket.cpp

#include <string>
using namespace std;
#include "ContactPacket.h"

///////////////////////////////////////////////////////////////

bool  PacketContact::SerializeIn( const U8* data, int& bufferOffset )
{ 
   BasePacket::SerializeIn( data, bufferOffset );

   return true; 
}

bool  PacketContact::SerializeOut( U8* data, int& bufferOffset ) const 
{ 
   BasePacket::SerializeOut( data, bufferOffset );

   return true; 
}



bool  FriendInfo::SerializeIn( const U8* data, int& bufferOffset )
{
   Serialize::In( data, bufferOffset, userName );
   Serialize::In( data, bufferOffset, avatarId );
   Serialize::In( data, bufferOffset, isOnline );

   return true;
}

bool  FriendInfo::SerializeOut( U8* data, int& bufferOffset ) const
{
   Serialize::Out( data, bufferOffset, userName );
   Serialize::Out( data, bufferOffset, avatarId );
   Serialize::Out( data, bufferOffset, isOnline );

   return true;
}


bool  InvitationInfo::SerializeIn( const U8* data, int& bufferOffset )
{
   Serialize::In( data, bufferOffset, message );
   Serialize::In( data, bufferOffset, inviterName );
   Serialize::In( data, bufferOffset, inviteeName );

   return true;
}

bool  InvitationInfo::SerializeOut( U8* data, int& bufferOffset ) const
{
   Serialize::Out( data, bufferOffset, message );
   Serialize::Out( data, bufferOffset, inviterName );
   Serialize::Out( data, bufferOffset, inviteeName );

   return true;
}


///////////////////////////////////////////////////////////////

bool  PacketContact_TestNotification::SerializeIn( const U8* data, int& bufferOffset )
{ 
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, message );
   Serialize::In( data, bufferOffset, senderName );
   Serialize::In( data, bufferOffset, senderUuid );
   Serialize::In( data, bufferOffset, type );

   return true; 
}

bool  PacketContact_TestNotification::SerializeOut( U8* data, int& bufferOffset ) const 
{ 
   BasePacket::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, message );
   Serialize::Out( data, bufferOffset, senderName );
   Serialize::Out( data, bufferOffset, senderUuid );
   Serialize::Out( data, bufferOffset, type );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketContact_GetListOfContacts::SerializeIn( const U8* data, int& bufferOffset )
{ 
   BasePacket::SerializeIn( data, bufferOffset );

   return true; 
}

bool  PacketContact_GetListOfContacts::SerializeOut( U8* data, int& bufferOffset ) const 
{ 
   BasePacket::SerializeOut( data, bufferOffset );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketContact_GetListOfContactsResponse::SerializeIn( const U8* data, int& bufferOffset )
{ 
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, friends );

   return true; 
}

bool  PacketContact_GetListOfContactsResponse::SerializeOut( U8* data, int& bufferOffset ) const 
{ 
   BasePacket::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, friends );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketContact_GetListOfInvitations::SerializeIn( const U8* data, int& bufferOffset )
{ 
   BasePacket::SerializeIn( data, bufferOffset );

   return true; 
}

bool  PacketContact_GetListOfInvitations::SerializeOut( U8* data, int& bufferOffset ) const 
{ 
   BasePacket::SerializeOut( data, bufferOffset );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketContact_GetListOfInvitationsResponse::SerializeIn( const U8* data, int& bufferOffset )
{ 
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, invitations );

   return true; 
}

bool  PacketContact_GetListOfInvitationsResponse::SerializeOut( U8* data, int& bufferOffset ) const 
{ 
   BasePacket::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, invitations );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketContact_GetListOfInvitationsSent::SerializeIn( const U8* data, int& bufferOffset )
{ 
   BasePacket::SerializeIn( data, bufferOffset );

   return true; 
}

bool  PacketContact_GetListOfInvitationsSent::SerializeOut( U8* data, int& bufferOffset ) const 
{ 
   BasePacket::SerializeOut( data, bufferOffset );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketContact_GetListOfInvitationsSentResponse::SerializeIn( const U8* data, int& bufferOffset )
{ 
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, invitations );

   return true; 
}

bool  PacketContact_GetListOfInvitationsSentResponse::SerializeOut( U8* data, int& bufferOffset ) const 
{ 
   BasePacket::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, invitations );

   return true; 
}

///////////////////////////////////////////////////////////////