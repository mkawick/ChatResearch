// InvitationPacket.cpp

#include "InvitationPacket.h"

//////////////////////////////////////////////////////////////

bool     IsUserInThisInvitation( const Invitation& invite, const string& userUuid )
{
   if( invite.inviteeUuid == userUuid || invite.inviterUuid == userUuid ) 
      return true; 
   return false;
}

bool     IsInvitationGroup( const Invitation& invite, const string& groupUuid )
{
   if( invite.groupUuid == groupUuid ) 
      return true; 
   return false;
}

//////////////////////////////////////////////////////////////

bool  Invitation::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   Serialize::In( data, bufferOffset, invitationUuid, minorVersion );
   Serialize::In( data, bufferOffset, inviterName, minorVersion );
   Serialize::In( data, bufferOffset, inviterUuid, minorVersion );
   Serialize::In( data, bufferOffset, inviteeName, minorVersion );
   Serialize::In( data, bufferOffset, inviteeUuid, minorVersion );
   Serialize::In( data, bufferOffset, groupName, minorVersion );
   Serialize::In( data, bufferOffset, groupUuid, minorVersion );
   //Serialize::In( data, bufferOffset, message, minorVersion );
   Serialize::In( data, bufferOffset, date, minorVersion );
   Serialize::In( data, bufferOffset, type, minorVersion );

   return true;
}

bool  Invitation::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   Serialize::Out( data, bufferOffset, invitationUuid, minorVersion );
   Serialize::Out( data, bufferOffset, inviterName, minorVersion );
   Serialize::Out( data, bufferOffset, inviterUuid, minorVersion );
   Serialize::Out( data, bufferOffset, inviteeName, minorVersion );
   Serialize::Out( data, bufferOffset, inviteeUuid, minorVersion );
   Serialize::Out( data, bufferOffset, groupName, minorVersion );
   Serialize::Out( data, bufferOffset, groupUuid, minorVersion );
   //Serialize::Out( data, bufferOffset, message, minorVersion );
   Serialize::Out( data, bufferOffset, date, minorVersion );
   Serialize::Out( data, bufferOffset, type, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketInvitation::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, invitationType, minorVersion );

   return true;
}

bool  PacketInvitation::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, invitationType, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
/*
bool  PacketInvitation_TestNotification::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketInvitation::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, message );
   Serialize::In( data, bufferOffset, type );

   return true;
}

bool  PacketInvitation_TestNotification::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketInvitation::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, message, minorVersion );
   Serialize::Out( data, bufferOffset, type, minorVersion );

   return true;
}*/

///////////////////////////////////////////////////////////////
/*
bool  PacketInvitation_EchoToServer::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );

   return true;
}

bool  PacketInvitation_EchoToServer::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );

   return true;
}


bool  PacketInvitation_EchoToClient::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );

   return true;
}

bool  PacketInvitation_EchoToClient::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );

   return true;
}*/

///////////////////////////////////////////////////////////////

bool  PacketInvitation_InviteUser::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketInvitation::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, userUuid, minorVersion );
   Serialize::In( data, bufferOffset, inviteGroup, minorVersion );
   Serialize::In( data, bufferOffset, message, minorVersion );

   return true;
}

bool  PacketInvitation_InviteUser::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketInvitation::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, userUuid, minorVersion );
   Serialize::Out( data, bufferOffset, inviteGroup, minorVersion );
   Serialize::Out( data, bufferOffset, message, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketInvitation_InviteUserResponse::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketInvitation::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, newInvitationUuid, minorVersion );
   Serialize::In( data, bufferOffset, succeeded, minorVersion );

   return true;
}

bool  PacketInvitation_InviteUserResponse::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketInvitation::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, newInvitationUuid, minorVersion );
   Serialize::Out( data, bufferOffset, succeeded, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketInvitation_CancelInvitation::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketInvitation::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, invitationUuid, minorVersion );

   return true;
}

bool  PacketInvitation_CancelInvitation::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketInvitation::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, invitationUuid, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketInvitation_InvitationWasCancelled::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketInvitation::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, invitationUuid, minorVersion );

   return true;
}

bool  PacketInvitation_InvitationWasCancelled::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketInvitation::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, invitationUuid, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketInvitation_RejectInvitation::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketInvitation::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, invitationUuid, minorVersion );
   Serialize::In( data, bufferOffset, reason, minorVersion );

   return true;
}

bool  PacketInvitation_RejectInvitation::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketInvitation::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, invitationUuid, minorVersion );
   Serialize::Out( data, bufferOffset, reason, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketInvitation_AcceptInvitation::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketInvitation::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, invitationUuid, minorVersion );
   return true;
}

bool  PacketInvitation_AcceptInvitation::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketInvitation::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, invitationUuid, minorVersion );
   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketInvitation_GetListOfInvitations::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketInvitation::SerializeIn( data, bufferOffset, minorVersion );

   return true;
}

bool  PacketInvitation_GetListOfInvitations::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketInvitation::SerializeOut( data, bufferOffset, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketInvitation_GetListOfInvitationsResponse::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketInvitation::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, uuid, minorVersion );
   Serialize::In( data, bufferOffset, invitationList, minorVersion );

   return true;
}

bool  PacketInvitation_GetListOfInvitationsResponse::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketInvitation::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, uuid, minorVersion );
   Serialize::Out( data, bufferOffset, invitationList, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketInvitation_GetListOfInvitationsForGroup::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketInvitation::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, groupUuid, minorVersion );

   return true;
}

bool  PacketInvitation_GetListOfInvitationsForGroup::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketInvitation::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, groupUuid, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

/*
bool  PacketInvitation_GetListOfInvitationsForGroupResponse::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketInvitation::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, groupUuid );
   Serialize::In( data, bufferOffset, invitationList );

   return true;
}

bool  PacketInvitation_GetListOfInvitationsForGroupResponse::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketInvitation::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, groupUuid );
   Serialize::Out( data, bufferOffset, invitationList );

   return true;
}*/

///////////////////////////////////////////////////////////////