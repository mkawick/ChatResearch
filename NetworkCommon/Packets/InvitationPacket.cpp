// InvitationPacket.cpp

#include "InvitationPacket.h"


//////////////////////////////////////////////////////////////

bool  Invitation::SerializeIn( const U8* data, int& bufferOffset )
{
   Serialize::In( data, bufferOffset, invitationUuid );
   Serialize::In( data, bufferOffset, inviterName );
   Serialize::In( data, bufferOffset, inviterUuid );
   Serialize::In( data, bufferOffset, inviteeName );
   Serialize::In( data, bufferOffset, inviteeUuid );
   Serialize::In( data, bufferOffset, groupUuid );
   //Serialize::In( data, bufferOffset, message );
   Serialize::In( data, bufferOffset, date );
   Serialize::In( data, bufferOffset, type );

   return true;
}

bool  Invitation::SerializeOut( U8* data, int& bufferOffset ) const
{
   Serialize::Out( data, bufferOffset, invitationUuid );
   Serialize::Out( data, bufferOffset, inviterName );
   Serialize::Out( data, bufferOffset, inviterUuid );
   Serialize::Out( data, bufferOffset, inviteeName );
   Serialize::Out( data, bufferOffset, inviteeUuid );
   Serialize::Out( data, bufferOffset, groupUuid );
   //Serialize::Out( data, bufferOffset, message );
   Serialize::Out( data, bufferOffset, date );
   Serialize::Out( data, bufferOffset, type );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketInvitation::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );

   return true;
}

bool  PacketInvitation::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );

   return true;
}

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
/*
bool  PacketInvitation_TestNotification::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketInvitation::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, message );
   Serialize::In( data, bufferOffset, type );

   return true;
}

bool  PacketInvitation_TestNotification::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketInvitation::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, message );
   Serialize::Out( data, bufferOffset, type );

   return true;
}*/

///////////////////////////////////////////////////////////////
/*
bool  PacketInvitation_EchoToServer::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );

   return true;
}

bool  PacketInvitation_EchoToServer::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );

   return true;
}


bool  PacketInvitation_EchoToClient::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );

   return true;
}

bool  PacketInvitation_EchoToClient::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );

   return true;
}*/

///////////////////////////////////////////////////////////////

bool  PacketInvitation_InviteUser::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketInvitation::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, userUuid );
   Serialize::In( data, bufferOffset, inviteGroup );
   Serialize::In( data, bufferOffset, message );
   Serialize::In( data, bufferOffset, invitationType );

   return true;
}

bool  PacketInvitation_InviteUser::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketInvitation::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, userUuid );
   Serialize::Out( data, bufferOffset, inviteGroup );
   Serialize::Out( data, bufferOffset, message );
   Serialize::Out( data, bufferOffset, invitationType );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketInvitation_InviteUserResponse::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketInvitation::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, newInvitationUuid );
   Serialize::In( data, bufferOffset, succeeded );

   return true;
}

bool  PacketInvitation_InviteUserResponse::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketInvitation::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, newInvitationUuid );
   Serialize::Out( data, bufferOffset, succeeded );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketInvitation_CancelInvitation::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketInvitation::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, invitationUuid );

   return true;
}

bool  PacketInvitation_CancelInvitation::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketInvitation::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, invitationUuid );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketInvitation_InvitationWasCancelled::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketInvitation::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, invitationUuid );

   return true;
}

bool  PacketInvitation_InvitationWasCancelled::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketInvitation::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, invitationUuid );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketInvitation_RejectInvitation::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketInvitation::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, invitationUuid );
   Serialize::In( data, bufferOffset, reason );

   return true;
}

bool  PacketInvitation_RejectInvitation::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketInvitation::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, invitationUuid );
   Serialize::Out( data, bufferOffset, reason );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketInvitation_AcceptInvitation::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketInvitation::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, invitationUuid );
   return true;
}

bool  PacketInvitation_AcceptInvitation::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketInvitation::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, invitationUuid );
   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketInvitation_GetListOfInvitations::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketInvitation::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, userUuid );

   return true;
}

bool  PacketInvitation_GetListOfInvitations::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketInvitation::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, userUuid );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketInvitation_GetListOfInvitationsResponse::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketInvitation::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, userUuid );
   Serialize::In( data, bufferOffset, invitationList );

   return true;
}

bool  PacketInvitation_GetListOfInvitationsResponse::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketInvitation::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, userUuid );
   Serialize::Out( data, bufferOffset, invitationList );

   return true;
}

///////////////////////////////////////////////////////////////