// InvitationPacket.h
#pragma once

#include "BasePacket.h"

///////////////////////////////////////////////////////////////////

struct Invitation
{
   enum  InvitationType
   {
      InvitationType_None,
      InvitationType_ChatRoom,
      InvitationType_Group,
      InvitationType_Alliance,
      InvitationType_Friend,
      InvitationType_Event,
      InvitationType_Num
   };

   string   invitationUuid;
   string   inviterName;
   string   inviterUuid;
   string   inviteeName; 
   string   inviteeUuid; 
   string   groupName; // if the group has a name
   string   groupUuid; // applies to event UUIDs, tournament UUIDs, etc.
   string   message;
   string   date;
   U8       type;
   int      invitationId; // does not transport to client
   

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   bool  IsUserInThisInvitation( const string& uuid ) const { if( inviteeUuid == uuid || inviterUuid == uuid ) return true; return false; }
};

//////////////////////////////////////////////////////////////////////////////

typedef bool (* MatchInvitationType)( const Invitation& invite, const string& userUuid ) ;

bool     IsUserInThisInvitation( const Invitation& invite, const string& userUuid );
bool     IsInvitationGroup( const Invitation& invite, const string& groupUuid );

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

class PacketInvitation : public BasePacket 
{
public:
   enum InvitationType
   {
      InvitationType_Base,
     // InvitationType_TestNotification,
      InvitationType_EchoToServer,
      InvitationType_EchoToClient,

      InvitationType_InviteUser,
      InvitationType_InviteUserResponse,
      InvitationType_CancelInvitation,
      InvitationType_InvitationWasCancelled,
      //InvitationType_CancelInvitationResponse,

      InvitationType_RejectInvitation,
      InvitationType_RejectInvitationResponse,
      InvitationType_AcceptInvitation,      

      InvitationType_GetListOfInvitations,// none=all, or specify type
      InvitationType_GetListOfInvitationsResponse,

      InvitationType_GetListOfInvitationsForGroup,
      InvitationType_GetListOfInvitationsForGroupResponse
   };

public:
   PacketInvitation( int packet_type = PacketType_Invitation, int packet_sub_type = InvitationType_Base ) : BasePacket( packet_type, packet_sub_type ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   U8       invitationType;  // Invitation:: InvitationType
};

///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////
/*
class PacketInvitation_TestNotification : public PacketInvitation
{
public:
   PacketInvitation_TestNotification() : PacketInvitation( PacketType_Invitation, InvitationType_TestNotification ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   message;
   int      type;
};*/

///////////////////////////////////////////////////////////////

class PacketInvitation_EchoToServer : public PacketInvitation
{
public:
   PacketInvitation_EchoToServer(): PacketInvitation( PacketType_Invitation, PacketInvitation::InvitationType_EchoToServer ) {}
};

///////////////////////////////////////////////////////////////

class PacketInvitation_EchoToClient : public PacketInvitation
{
public:
   PacketInvitation_EchoToClient(): PacketInvitation( PacketType_Invitation, PacketInvitation::InvitationType_EchoToClient ) {}
};

///////////////////////////////////////////////////////////////

class PacketInvitation_InviteUser : public PacketInvitation
{
public:
   PacketInvitation_InviteUser() : PacketInvitation( PacketType_Invitation, InvitationType_InviteUser ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   UuidString  userUuid; 
   string      inviteGroup;
   string      message;
};

///////////////////////////////////////////////////////////////

class PacketInvitation_InviteUserResponse : public PacketInvitation
{
public:
   PacketInvitation_InviteUserResponse() : PacketInvitation( PacketType_Invitation, InvitationType_InviteUserResponse ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   newInvitationUuid;

   bool     succeeded;
};

///////////////////////////////////////////////////////////////

class PacketInvitation_CancelInvitation : public PacketInvitation
{
public:
   PacketInvitation_CancelInvitation() : PacketInvitation( PacketType_Invitation, InvitationType_CancelInvitation ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   invitationUuid;
};

///////////////////////////////////////////////////////////////

class PacketInvitation_InvitationWasCancelled : public PacketInvitation
{
public:
   PacketInvitation_InvitationWasCancelled() : PacketInvitation( PacketType_Invitation, InvitationType_InvitationWasCancelled ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   invitationUuid; 
};

///////////////////////////////////////////////////////////////

class PacketInvitation_RejectInvitation : public PacketInvitation
{
public:
   PacketInvitation_RejectInvitation() : PacketInvitation( PacketType_Invitation, InvitationType_RejectInvitation ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   invitationUuid;
   string   reason;
};

///////////////////////////////////////////////////////////////

class PacketInvitation_RejectInvitationResponse : public PacketInvitation
{
public:
   PacketInvitation_RejectInvitationResponse() : PacketInvitation( PacketType_Invitation, InvitationType_RejectInvitationResponse ){  }
};

///////////////////////////////////////////////////////////////

class PacketInvitation_AcceptInvitation  : public PacketInvitation
{
public:
   PacketInvitation_AcceptInvitation () : PacketInvitation( PacketType_Invitation, InvitationType_AcceptInvitation ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   invitationUuid;
};


///////////////////////////////////////////////////////////////

class PacketInvitation_GetListOfInvitations : public PacketInvitation
{
public:
   PacketInvitation_GetListOfInvitations() : PacketInvitation( PacketType_Invitation, InvitationType_GetListOfInvitations ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

};

///////////////////////////////////////////////////////////////

class PacketInvitation_GetListOfInvitationsResponse : public PacketInvitation
{
public:
   PacketInvitation_GetListOfInvitationsResponse() : PacketInvitation( PacketType_Invitation, InvitationType_GetListOfInvitationsResponse ){  }
   PacketInvitation_GetListOfInvitationsResponse( U8 type, U8 subType ) : PacketInvitation( type, subType ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   uuid;
   SerializedKeyValueVector < Invitation >  invitationList;
};

///////////////////////////////////////////////////////////////

class PacketInvitation_GetListOfInvitationsForGroup : public PacketInvitation
{
public:
   PacketInvitation_GetListOfInvitationsForGroup() : PacketInvitation( PacketType_Invitation, InvitationType_GetListOfInvitationsForGroup ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   groupUuid;
};

///////////////////////////////////////////////////////////////

class PacketInvitation_GetListOfInvitationsForGroupResponse : public PacketInvitation_GetListOfInvitationsResponse
{
public:
   PacketInvitation_GetListOfInvitationsForGroupResponse() : PacketInvitation_GetListOfInvitationsResponse( PacketType_Invitation, InvitationType_GetListOfInvitationsForGroupResponse ){  }
}; 

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////