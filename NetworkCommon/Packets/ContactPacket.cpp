// ContactPacket.cpp

#include <string>

#include "ContactPacket.h"
using namespace std;

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
   Serialize::In( data, bufferOffset, uuid );
   Serialize::In( data, bufferOffset, date );
   Serialize::In( data, bufferOffset, userUuid );

   return true;
}

bool  InvitationInfo::SerializeOut( U8* data, int& bufferOffset ) const
{
   Serialize::Out( data, bufferOffset, message );
   Serialize::Out( data, bufferOffset, inviterName );
   Serialize::Out( data, bufferOffset, inviteeName );
   Serialize::Out( data, bufferOffset, uuid );
   Serialize::Out( data, bufferOffset, date );
   Serialize::Out( data, bufferOffset, userUuid );

   return true;
}


///////////////////////////////////////////////////////////////

bool  PacketContact_TestNotification::SerializeIn( const U8* data, int& bufferOffset )
{ 
   PacketContact::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, message );
   Serialize::In( data, bufferOffset, senderName );
   Serialize::In( data, bufferOffset, senderUuid );
   Serialize::In( data, bufferOffset, type );

   return true; 
}

bool  PacketContact_TestNotification::SerializeOut( U8* data, int& bufferOffset ) const 
{ 
   PacketContact::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, message );
   Serialize::Out( data, bufferOffset, senderName );
   Serialize::Out( data, bufferOffset, senderUuid );
   Serialize::Out( data, bufferOffset, type );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketContact_GetListOfContacts::SerializeIn( const U8* data, int& bufferOffset )
{ 
   PacketContact::SerializeIn( data, bufferOffset );

   return true; 
}

bool  PacketContact_GetListOfContacts::SerializeOut( U8* data, int& bufferOffset ) const 
{ 
   PacketContact::SerializeOut( data, bufferOffset );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketContact_GetListOfContactsResponse::SerializeIn( const U8* data, int& bufferOffset )
{ 
   PacketContact::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, friends );

   return true; 
}

bool  PacketContact_GetListOfContactsResponse::SerializeOut( U8* data, int& bufferOffset ) const 
{ 
   PacketContact::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, friends );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketContact_GetListOfInvitations::SerializeIn( const U8* data, int& bufferOffset )
{ 
   PacketContact::SerializeIn( data, bufferOffset );

   return true; 
}

bool  PacketContact_GetListOfInvitations::SerializeOut( U8* data, int& bufferOffset ) const 
{ 
   PacketContact::SerializeOut( data, bufferOffset );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketContact_GetListOfInvitationsResponse::SerializeIn( const U8* data, int& bufferOffset )
{ 
   PacketContact::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, invitations );

   return true; 
}

bool  PacketContact_GetListOfInvitationsResponse::SerializeOut( U8* data, int& bufferOffset ) const 
{ 
   PacketContact::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, invitations );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketContact_GetListOfInvitationsSent::SerializeIn( const U8* data, int& bufferOffset )
{ 
   PacketContact::SerializeIn( data, bufferOffset );

   return true; 
}

bool  PacketContact_GetListOfInvitationsSent::SerializeOut( U8* data, int& bufferOffset ) const 
{ 
   PacketContact::SerializeOut( data, bufferOffset );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketContact_GetListOfInvitationsSentResponse::SerializeIn( const U8* data, int& bufferOffset )
{ 
   PacketContact::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, invitations );

   return true; 
}
bool  PacketContact_GetListOfInvitationsSentResponse::SerializeOut( U8* data, int& bufferOffset ) const 
{ 
   PacketContact::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, invitations );

   return true; 
}

///////////////////////////////////////////////////////////////


bool  PacketContact_InviteContact::SerializeIn( const U8* data, int& bufferOffset )
{ 
   PacketContact::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, userName );
   Serialize::In( data, bufferOffset, uuid );
   Serialize::In( data, bufferOffset, message );

   return true; 
}

bool  PacketContact_InviteContact::SerializeOut( U8* data, int& bufferOffset ) const 
{ 
   PacketContact::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, userName );
   Serialize::Out( data, bufferOffset, uuid );
   Serialize::Out( data, bufferOffset, message );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketContact_InviteSentNotification::SerializeIn( const U8* data, int& bufferOffset )
{ 
   PacketContact::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, info );

   return true; 
}

bool  PacketContact_InviteSentNotification::SerializeOut( U8* data, int& bufferOffset ) const 
{ 
   PacketContact::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, info );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketContact_InviteReceivedNotification::SerializeIn( const U8* data, int& bufferOffset )
{ 
   PacketContact::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, info );

   return true; 
}

bool  PacketContact_InviteReceivedNotification::SerializeOut( U8* data, int& bufferOffset ) const 
{ 
   PacketContact::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, info );

   return true; 
}


///////////////////////////////////////////////////////////////

bool  PacketContact_ContactRemove::SerializeIn( const U8* data, int& bufferOffset )
{ 
   PacketContact::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, contactUuid );
   Serialize::In( data, bufferOffset, message );

   return true; 
}

bool  PacketContact_ContactRemove::SerializeOut( U8* data, int& bufferOffset ) const 
{ 
   PacketContact::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, contactUuid );
   Serialize::Out( data, bufferOffset, message );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketContact_RemoveInvitation::SerializeIn( const U8* data, int& bufferOffset )
{ 
   PacketContact::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, invitationUuid );

   return true; 
}

bool  PacketContact_RemoveInvitation::SerializeOut( U8* data, int& bufferOffset ) const 
{ 
   PacketContact::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, invitationUuid );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketContact_AcceptInvite::SerializeIn( const U8* data, int& bufferOffset )
{ 
   PacketContact::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, invitationUuid );

   return true; 
}

bool  PacketContact_AcceptInvite::SerializeOut( U8* data, int& bufferOffset ) const 
{ 
   PacketContact::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, invitationUuid );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketContact_InvitationAccepted::SerializeIn( const U8* data, int& bufferOffset )
{ 
   PacketContact::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, fromUsername );
   Serialize::In( data, bufferOffset, toUsername );
   Serialize::In( data, bufferOffset, invitationUuid );
   Serialize::In( data, bufferOffset, message );
   Serialize::In( data, bufferOffset, wasAccepted );

   return true; 
}

bool  PacketContact_InvitationAccepted::SerializeOut( U8* data, int& bufferOffset ) const 
{ 
   PacketContact::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, fromUsername );
   Serialize::Out( data, bufferOffset, toUsername );
   Serialize::Out( data, bufferOffset, invitationUuid );
   Serialize::Out( data, bufferOffset, message );
   Serialize::Out( data, bufferOffset, wasAccepted );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketContact_DeclineInvitation::SerializeIn( const U8* data, int& bufferOffset )
{ 
   PacketContact::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, invitationUuid );
   Serialize::In( data, bufferOffset, message );

   return true; 
}

bool  PacketContact_DeclineInvitation::SerializeOut( U8* data, int& bufferOffset ) const 
{ 
   PacketContact::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, invitationUuid );
   Serialize::Out( data, bufferOffset, message );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketContact_SearchForUser::SerializeIn( const U8* data, int& bufferOffset )
{ 
   PacketContact::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, searchString );
   Serialize::In( data, bufferOffset, limit );
   Serialize::In( data, bufferOffset, offset );

   return true; 
}

bool  PacketContact_SearchForUser::SerializeOut( U8* data, int& bufferOffset ) const 
{ 
   PacketContact::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, searchString );
   Serialize::Out( data, bufferOffset, limit );
   Serialize::Out( data, bufferOffset, offset );

   return true; 
}
///////////////////////////////////////////////////////////////

bool  PacketContact_SearchForUserResult::SerializeIn( const U8* data, int& bufferOffset )
{ 
   PacketContact::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, found );

   return true; 
}

bool  PacketContact_SearchForUserResult::SerializeOut( U8* data, int& bufferOffset ) const 
{ 
   PacketContact::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, found );

   return true; 
}
///////////////////////////////////////////////////////////////

bool  PacketContact_InviteBlockUser::SerializeIn( const U8* data, int& bufferOffset )
{ 
   PacketContact::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, userName );
   Serialize::In( data, bufferOffset, uuid );

   return true; 
}

bool  PacketContact_InviteBlockUser::SerializeOut( U8* data, int& bufferOffset ) const 
{ 
   PacketContact::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, userName );
   Serialize::Out( data, bufferOffset, uuid );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketContact_FriendOnlineStatusChange::SerializeIn( const U8* data, int& bufferOffset )
{ 
   PacketContact::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, uuid );
   Serialize::In( data, bufferOffset, friendInfo );

   return true; 
}

bool  PacketContact_FriendOnlineStatusChange::SerializeOut( U8* data, int& bufferOffset ) const 
{ 
   PacketContact::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, uuid );
   Serialize::Out( data, bufferOffset, friendInfo );

   return true; 
}

///////////////////////////////////////////////////////////////