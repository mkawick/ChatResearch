// ContactPacket.h
#pragma once

#include "BasePacket.h"
#include "../DataTypes.h"

///////////////////////////////////////////////////////////////////

struct UserInfo
{
   string   username;
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
   FriendInfo() {}
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
      ContactType_RemoveInivtation,
      ContactType_AcceptInvite,

      ContactType_BlockUser, // remove user from contacts or simply block an invitator. Make sure to remove invitations from that usr as well
      ContactType_Search,
      ContactType_SearchResults,

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
