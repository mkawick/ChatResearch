// ContactPacket.h
#pragma once

#include "BasePacket.h"

///////////////////////////////////////////////////////////////////

struct UserInfo
{
   string   userName;
   string   uuid;
   string   apple_id;
   U32      connectionId;
   U32      avatarId;
   U8       gameProductId;
   bool     active;
   string   email;
   string   passwordHash;
   U32      id;
};

///////////////////////////////////////////////////////////////////

class FriendInfo     //string userUuid.. will be sotred by id using uuid
{
public:
   FriendInfo(): avatarId( 0 ), isOnline( false ) {}
   FriendInfo( const string& name, int avatar, bool online ): userName( name ), avatarId( avatar ), isOnline( online )  {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   userName;
   int      avatarId;
   bool     isOnline;
};

class InvitationInfo // string   uuid; .. stored in the associated container
{
public:
   InvitationInfo() {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   message;
   string   inviterName;
   string   inviteeName;
   string   uuid;
   string   date;
};

///////////////////////////////////////////////////////////////////

class PacketContact : public BasePacket 
{
public:
   enum ContactType
   {
      ContactType_Base,
      ContactType_TestNotification,

      ContactType_GetListOfContacts,
      ContactType_GetListOfContactsResponse,
      ContactType_GetListOfInvitations,  // not sure about this
      ContactType_GetListOfInvitationsResponse,

      ContactType_GetListOfInvitationsSent,
      ContactType_GetListOfInvitationsSentResponse,

      ContactType_InviteContact,
      ContactType_InviteSentNotification,
      ContactType_InviteReceived,
      ContactType_RemoveContact,
      ContactType_RemoveInvitation,
      ContactType_AcceptInvite,
      ContactType_DeclineInvitation,
      ContactType_InvitationAccepted,
      ContactType_BlockUser, // remove user from contacts or simply block an invitator. Make sure to remove invitations from that usr as well

      ContactType_Search,
      ContactType_SearchResults,

      ContactType_UserOnlineStatusChange,
      ContactType_GetProfile,
      ContactType_GetProfileResponse,
      ContactType_UpdateProfile,
      ContactType_UserProfileUpdated,
   };
public:
   PacketContact( int packet_type = PacketType_Contact, int packet_sub_type = ContactType_Base ) : BasePacket( packet_type, packet_sub_type ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;
};

///////////////////////////////////////////////////////////////

class PacketContact_TestNotification : public PacketContact
{
public:
   PacketContact_TestNotification() : PacketContact( PacketType_Contact, ContactType_TestNotification ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   message;
   string   senderName;
   string   senderUuid;
   int      type;
};


///////////////////////////////////////////////////////////////

class PacketContact_GetListOfContacts : public PacketContact
{
public:
   PacketContact_GetListOfContacts() : PacketContact( PacketType_Contact, ContactType_GetListOfContacts ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;
};

///////////////////////////////////////////////////////////////////

class PacketContact_GetListOfContactsResponse : public PacketContact
{
public:
   PacketContact_GetListOfContactsResponse() : PacketContact( PacketType_Contact, ContactType_GetListOfContactsResponse ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   SerializedKeyValueVector< FriendInfo >   friends;
};

///////////////////////////////////////////////////////////////////

class PacketContact_GetListOfInvitations : public PacketContact
{
public:
   PacketContact_GetListOfInvitations() : PacketContact( PacketType_Contact, ContactType_GetListOfInvitations ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;
};

///////////////////////////////////////////////////////////////////


class PacketContact_GetListOfInvitationsResponse : public PacketContact
{
public:
   PacketContact_GetListOfInvitationsResponse() : PacketContact( PacketType_Contact, ContactType_GetListOfInvitationsResponse ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   SerializedKeyValueVector< InvitationInfo >   invitations;
};

///////////////////////////////////////////////////////////////////

class PacketContact_GetListOfInvitationsSent : public PacketContact
{
public:
   PacketContact_GetListOfInvitationsSent() : PacketContact( PacketType_Contact, ContactType_GetListOfInvitationsSent ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;
};

///////////////////////////////////////////////////////////////////

class PacketContact_GetListOfInvitationsSentResponse : public PacketContact
{
public:
   PacketContact_GetListOfInvitationsSentResponse() : PacketContact( PacketType_Contact, ContactType_GetListOfInvitationsSentResponse ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   SerializedKeyValueVector< InvitationInfo >   invitations;
};

///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////

class PacketContact_InviteContact : public PacketContact
{
public:
   PacketContact_InviteContact() : PacketContact( PacketType_Contact, ContactType_InviteContact ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string userName;// one or the other
   string uuid;
   string message;
};

class PacketContact_InviteSentNotification : public PacketContact
{
public:
   PacketContact_InviteSentNotification() : PacketContact( PacketType_Contact, ContactType_InviteSentNotification ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   InvitationInfo info;
};

class PacketContact_InviteReceivedNotification : public PacketContact
{
public:
   PacketContact_InviteReceivedNotification() : PacketContact( PacketType_Contact, ContactType_InviteReceived ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   InvitationInfo info;
};


class PacketContact_ContactRemove : public PacketContact
{
public:
   PacketContact_ContactRemove() : PacketContact( PacketType_Contact, ContactType_RemoveContact ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string contactUuid;
   string message;
};

///////////////////////////////////////////////////////////////////

class PacketContact_RemoveInvitation : public PacketContact
{
public:
   PacketContact_RemoveInvitation() : PacketContact( PacketType_Contact, ContactType_RemoveInvitation ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   invitationUuid;
};

class PacketContact_AcceptInvite : public PacketContact
{
public:
   PacketContact_AcceptInvite() : PacketContact( PacketType_Contact, ContactType_AcceptInvite ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   invitationUuid;
};


class PacketContact_InvitationAccepted : public PacketContact
{
public:
   PacketContact_InvitationAccepted() : PacketContact( PacketType_Contact, ContactType_InvitationAccepted ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   fromUsername;
   string   toUsername;
   string   message;
   bool     wasAccepted;
};


class PacketContact_DeclineInvitation : public PacketContact
{
public:
   PacketContact_DeclineInvitation() : PacketContact( PacketType_Contact, ContactType_DeclineInvitation ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   invitationUuid;
   string   message;
};



class PacketContact_InviteBlockUser : public PacketContact
{
public:
   PacketContact_InviteBlockUser() : PacketContact( PacketType_Contact, ContactType_BlockUser ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string userName;
   string uuid;
};

///////////////////////////////////////////////////////////////////

class PacketContact_FriendOnlineStatusChange : public PacketContact
{
public:
   PacketContact_FriendOnlineStatusChange() : PacketContact( PacketType_Contact, ContactType_UserOnlineStatusChange ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string         uuid;
   FriendInfo     friendInfo;
};

///////////////////////////////////////////////////////////////////
