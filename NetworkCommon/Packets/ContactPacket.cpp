// ContactPacket.cpp

#include <string>
#include <iostream>
#include <iomanip>

#include "../ServerConstants.h"

#if PLATFORM == PLATFORM_WINDOWS
#pragma warning( disable:4996 )
#endif

#include "ContactPacket.h"
using namespace std;

///////////////////////////////////////////////////////////////

bool  PacketContact::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{ 
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );

   return true; 
}

bool  PacketContact::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{ 
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );

   return true; 
}



bool  FriendInfo::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   Serialize::In( data, bufferOffset, userName, minorVersion );
   Serialize::In( data, bufferOffset, notesAboutThisUser, minorVersion );
   Serialize::In( data, bufferOffset, motto, minorVersion );
   Serialize::In( data, bufferOffset, avatarId, minorVersion );
   Serialize::In( data, bufferOffset, isOnline, minorVersion );
   Serialize::In( data, bufferOffset, markedAsFavorite, minorVersion );

   return true;
}

bool  FriendInfo::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   Serialize::Out( data, bufferOffset, userName, minorVersion );
   Serialize::Out( data, bufferOffset, notesAboutThisUser, minorVersion );
   Serialize::Out( data, bufferOffset, motto, minorVersion );
   Serialize::Out( data, bufferOffset, avatarId, minorVersion );
   Serialize::Out( data, bufferOffset, isOnline, minorVersion );
   Serialize::Out( data, bufferOffset, markedAsFavorite, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  InvitationInfo::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   Serialize::In( data, bufferOffset, message, minorVersion );
   Serialize::In( data, bufferOffset, inviterName, minorVersion );
   Serialize::In( data, bufferOffset, inviteeName, minorVersion );
   Serialize::In( data, bufferOffset, uuid, minorVersion );
   Serialize::In( data, bufferOffset, date, minorVersion );
   Serialize::In( data, bufferOffset, userUuid, minorVersion );

   return true;
}

bool  InvitationInfo::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   Serialize::Out( data, bufferOffset, message, minorVersion );
   Serialize::Out( data, bufferOffset, inviterName, minorVersion );
   Serialize::Out( data, bufferOffset, inviteeName, minorVersion );
   Serialize::Out( data, bufferOffset, uuid, minorVersion );
   Serialize::Out( data, bufferOffset, date, minorVersion );
   Serialize::Out( data, bufferOffset, userUuid, minorVersion );

   return true;
}

void  InvitationInfo::Print( int tab )
{
   cout << setw( tab ) << "from:        " << setw(tab) << inviterName << endl;
   cout << setw( tab ) << "to:          " << setw(tab) << inviteeName << endl;
   cout << setw( tab ) << "date:        " << setw(tab) << date << endl;
   cout << setw( tab ) << "user-uuid:   " << setw(tab) << userUuid << endl;
   cout << setw( tab ) << "invite-uuid: " << setw(tab) << uuid << endl;
   cout << setw( tab ) << "message:     " << setw(tab) << message << endl;
}


///////////////////////////////////////////////////////////////

bool  PacketContact_TestNotification::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{ 
   PacketContact::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, message, minorVersion );
   Serialize::In( data, bufferOffset, senderName, minorVersion );
   Serialize::In( data, bufferOffset, senderUuid, minorVersion );
   Serialize::In( data, bufferOffset, type, minorVersion );

   return true; 
}

bool  PacketContact_TestNotification::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{ 
   PacketContact::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, message, minorVersion );
   Serialize::Out( data, bufferOffset, senderName, minorVersion );
   Serialize::Out( data, bufferOffset, senderUuid, minorVersion );
   Serialize::Out( data, bufferOffset, type, minorVersion );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketContact_GetListOfContacts::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{ 
   PacketContact::SerializeIn( data, bufferOffset, minorVersion );

   return true; 
}

bool  PacketContact_GetListOfContacts::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{ 
   PacketContact::SerializeOut( data, bufferOffset, minorVersion );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketContact_GetListOfContactsResponse::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{ 
   PacketContact::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, friends, minorVersion );
   //friends.SerializeIn( data, bufferOffset, minorVersion );

   return true; 
}

bool  PacketContact_GetListOfContactsResponse::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{ 
   PacketContact::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, friends, minorVersion );
   
   //friends.SerializeOut( data, bufferOffset, minorVersion );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketContact_GetListOfInvitations::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{ 
   PacketContact::SerializeIn( data, bufferOffset, minorVersion );

   return true; 
}

bool  PacketContact_GetListOfInvitations::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{ 
   PacketContact::SerializeOut( data, bufferOffset, minorVersion );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketContact_GetListOfInvitationsResponse::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{ 
   PacketContact::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, invitations, minorVersion );
   //invitations.SerializeIn( data, bufferOffset, minorVersion );

   return true; 
}

bool  PacketContact_GetListOfInvitationsResponse::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{ 
   PacketContact::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, invitations, minorVersion );
   //invitations.SerializeOut( data, bufferOffset, minorVersion );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketContact_GetListOfInvitationsSent::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{ 
   PacketContact::SerializeIn( data, bufferOffset, minorVersion );

   return true; 
}

bool  PacketContact_GetListOfInvitationsSent::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{ 
   PacketContact::SerializeOut( data, bufferOffset, minorVersion );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketContact_GetListOfInvitationsSentResponse::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{ 
   PacketContact::SerializeIn( data, bufferOffset, minorVersion );
   //Serialize::In( data, bufferOffset, invitations, minorVersion );
   invitations.SerializeIn( data, bufferOffset, minorVersion );

   return true; 
}
bool  PacketContact_GetListOfInvitationsSentResponse::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{ 
   PacketContact::SerializeOut( data, bufferOffset, minorVersion );
   //Serialize::Out( data, bufferOffset, invitations, minorVersion );
   invitations.SerializeOut( data, bufferOffset, minorVersion );

   return true; 
}

///////////////////////////////////////////////////////////////


bool  PacketContact_InviteContact::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{ 
   PacketContact::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, userName, minorVersion );
   Serialize::In( data, bufferOffset, uuid, minorVersion );
   Serialize::In( data, bufferOffset, message, minorVersion );

   return true; 
}

bool  PacketContact_InviteContact::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{ 
   PacketContact::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, userName, minorVersion );
   Serialize::Out( data, bufferOffset, uuid, minorVersion );
   Serialize::Out( data, bufferOffset, message, minorVersion );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketContact_InviteSentNotification::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{ 
   PacketContact::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, info, minorVersion );

   return true; 
}

bool  PacketContact_InviteSentNotification::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{ 
   PacketContact::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, info, minorVersion );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketContact_InviteReceivedNotification::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{ 
   PacketContact::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, info, minorVersion );

   return true; 
}

bool  PacketContact_InviteReceivedNotification::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{ 
   PacketContact::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, info, minorVersion );

   return true; 
}


///////////////////////////////////////////////////////////////

bool  PacketContact_ContactRemove::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{ 
   PacketContact::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, contactUuid, minorVersion );
   Serialize::In( data, bufferOffset, message, minorVersion );

   return true; 
}

bool  PacketContact_ContactRemove::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{ 
   PacketContact::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, contactUuid, minorVersion );
   Serialize::Out( data, bufferOffset, message, minorVersion );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketContact_RemoveInvitation::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{ 
   PacketContact::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, invitationUuid, minorVersion );

   return true; 
}

bool  PacketContact_RemoveInvitation::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{ 
   PacketContact::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, invitationUuid, minorVersion );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketContact_AcceptInvite::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{ 
   PacketContact::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, invitationUuid, minorVersion );

   return true; 
}

bool  PacketContact_AcceptInvite::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{ 
   PacketContact::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, invitationUuid, minorVersion );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketContact_InvitationAccepted::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{ 
   PacketContact::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, fromUsername, minorVersion );
   Serialize::In( data, bufferOffset, toUsername, minorVersion );
   Serialize::In( data, bufferOffset, invitationUuid, minorVersion );
   Serialize::In( data, bufferOffset, message, minorVersion );
   Serialize::In( data, bufferOffset, wasAccepted, minorVersion );

   return true; 
}

bool  PacketContact_InvitationAccepted::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{ 
   PacketContact::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, fromUsername, minorVersion );
   Serialize::Out( data, bufferOffset, toUsername, minorVersion );
   Serialize::Out( data, bufferOffset, invitationUuid, minorVersion );
   Serialize::Out( data, bufferOffset, message, minorVersion );
   Serialize::Out( data, bufferOffset, wasAccepted, minorVersion );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketContact_DeclineInvitation::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{ 
   PacketContact::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, invitationUuid, minorVersion );
   Serialize::In( data, bufferOffset, message, minorVersion );

   return true; 
}

bool  PacketContact_DeclineInvitation::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{ 
   PacketContact::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, invitationUuid, minorVersion );
   Serialize::Out( data, bufferOffset, message, minorVersion );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketContact_SearchForUser::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{ 
   PacketContact::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, searchString, minorVersion );
   Serialize::In( data, bufferOffset, limit, minorVersion );
   Serialize::In( data, bufferOffset, offset, minorVersion );

   return true; 
}

bool  PacketContact_SearchForUser::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{ 
   PacketContact::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, searchString, minorVersion );
   Serialize::Out( data, bufferOffset, limit, minorVersion );
   Serialize::Out( data, bufferOffset, offset, minorVersion );

   return true; 
}
///////////////////////////////////////////////////////////////

bool  PacketContact_SearchForUserResult::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{ 
   PacketContact::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, found, minorVersion );
   //found.SerializeIn( data, bufferOffset, minorVersion );

   return true; 
}

bool  PacketContact_SearchForUserResult::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{ 
   PacketContact::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, found, minorVersion );
   //found.SerializeOut( data, bufferOffset, minorVersion );

   return true; 
}
///////////////////////////////////////////////////////////////

bool  PacketContact_InviteBlockUser::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{ 
   PacketContact::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, userName, minorVersion );
   Serialize::In( data, bufferOffset, uuid, minorVersion );

   return true; 
}

bool  PacketContact_InviteBlockUser::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{ 
   PacketContact::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, userName, minorVersion );
   Serialize::Out( data, bufferOffset, uuid, minorVersion );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketContact_FriendOnlineStatusChange::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{ 
   PacketContact::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, uuid, minorVersion );
   Serialize::In( data, bufferOffset, friendInfo, minorVersion );

   return true; 
}

bool  PacketContact_FriendOnlineStatusChange::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{ 
   PacketContact::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, uuid, minorVersion );
   Serialize::Out( data, bufferOffset, friendInfo, minorVersion );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketContact_SetNotationOnUser::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{ 
   PacketContact::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, uuid, minorVersion );
   Serialize::In( data, bufferOffset, friendInfo, minorVersion );

   return true; 
}

bool  PacketContact_SetNotationOnUser::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{ 
   PacketContact::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, uuid, minorVersion );
   Serialize::Out( data, bufferOffset, friendInfo, minorVersion );

   return true; 
}

///////////////////////////////////////////////////////////////
