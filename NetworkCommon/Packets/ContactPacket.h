// ContactPacket.h
#pragma once

#include "BasePacket.h"

///////////////////////////////////////////////////////////////////

struct UserInfo
{
   BoundedString80   userName;
   UuidString        uuid;
   BoundedString80   apple_id;
   U32               connectionId;
   U32               avatarId;
   U8                gameProductId;
   bool              active;
   BoundedString80   email;
   BoundedString32   passwordHash;
   U32               id;
   bool              favorite;
   BoundedString140  note;
   BoundedString140  motto;
};

///////////////////////////////////////////////////////////////////

struct FriendInfo     //string userUuid.. will be stored by id using uuid
{
public:
   FriendInfo(): avatarId( 0 ), isOnline( false ) {}
   FriendInfo( const string& name, int avatar, bool online ): userName( name ), avatarId( avatar ), isOnline( online )  {}
   FriendInfo( const string& name, const string& _motto, int avatar, bool online, bool favorite, const string& notes ): 
            userName( name ), 
            motto( _motto ),
            avatarId( avatar ), 
            isOnline( online ),
            markedAsFavorite( favorite ),
            notesAboutThisUser( notes ){}

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   BoundedString80   userName;
   BoundedString128  notesAboutThisUser;
   BoundedString128  motto;
   int               avatarId;
   bool              isOnline;
   bool              markedAsFavorite;
};

struct InvitationInfo // string   uuid; .. stored in the associated container
{
public:
   InvitationInfo() {}

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   BoundedString140  message;
   BoundedString80   inviterName;
   BoundedString80   inviteeName;
   UuidString        uuid;
   TimeString        date;
   UuidString        userUuid; // the non-current user... could be invitee or inviter... just not you

   void     Print( int tab = 0 );
};

///////////////////////////////////////////////////////////////////

class PacketContact : public BasePacket 
{
public:
   enum ContactType
   {
      ContactType_Base,
      ContactType_TestNotification,
      ContactType_EchoToServer,
      ContactType_EchoToClient,

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

      ContactType_SetNotation,
   };
public:
   PacketContact( int packet_type = PacketType_Contact, int packet_sub_type = ContactType_Base ) : BasePacket( packet_type, packet_sub_type ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;
};

///////////////////////////////////////////////////////////////

class PacketContact_TestNotification : public PacketContact
{
public:
   PacketContact_TestNotification() : PacketContact( PacketType_Contact, ContactType_TestNotification ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   BoundedString140  message;
   BoundedString80   senderName;
   UuidString        senderUuid;
   int               type;
};

///////////////////////////////////////////////////////////////

class PacketContact_EchoToServer : public BasePacket
{
public:
   PacketContact_EchoToServer(): BasePacket( PacketType_Contact, PacketContact::ContactType_EchoToServer ) {}
};

///////////////////////////////////////////////////////////////

class PacketContact_EchoToClient : public BasePacket
{
public:
   PacketContact_EchoToClient(): BasePacket( PacketType_Contact, PacketContact::ContactType_EchoToClient ) {}
};

///////////////////////////////////////////////////////////////

class PacketContact_GetListOfContacts : public PacketContact
{
public:
   PacketContact_GetListOfContacts() : PacketContact( PacketType_Contact, ContactType_GetListOfContacts ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;
};

///////////////////////////////////////////////////////////////////

class PacketContact_GetListOfContactsResponse : public PacketContact
{
public:
   PacketContact_GetListOfContactsResponse() : PacketContact( PacketType_Contact, ContactType_GetListOfContactsResponse ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   SerializedKeyValueVector< FriendInfo >   friends;
};

///////////////////////////////////////////////////////////////////

class PacketContact_GetListOfInvitations : public PacketContact
{
public:
   PacketContact_GetListOfInvitations() : PacketContact( PacketType_Contact, ContactType_GetListOfInvitations ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;
};

///////////////////////////////////////////////////////////////////


class PacketContact_GetListOfInvitationsResponse : public PacketContact
{
public:
   PacketContact_GetListOfInvitationsResponse() : PacketContact( PacketType_Contact, ContactType_GetListOfInvitationsResponse ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   SerializedKeyValueVector< InvitationInfo >   invitations;
};

///////////////////////////////////////////////////////////////////

class PacketContact_GetListOfInvitationsSent : public PacketContact
{
public:
   PacketContact_GetListOfInvitationsSent() : PacketContact( PacketType_Contact, ContactType_GetListOfInvitationsSent ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;
};

///////////////////////////////////////////////////////////////////

class PacketContact_GetListOfInvitationsSentResponse : public PacketContact
{
public:
   PacketContact_GetListOfInvitationsSentResponse() : PacketContact( PacketType_Contact, ContactType_GetListOfInvitationsSentResponse ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   SerializedKeyValueVector< InvitationInfo >   invitations;
};

///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////

class PacketContact_InviteContact : public PacketContact
{
public:
   PacketContact_InviteContact() : PacketContact( PacketType_Contact, ContactType_InviteContact ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   BoundedString80   userName;// one or the other
   UuidString        uuid;
   BoundedString140  message;
};

class PacketContact_InviteSentNotification : public PacketContact
{
public:
   PacketContact_InviteSentNotification() : PacketContact( PacketType_Contact, ContactType_InviteSentNotification ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   InvitationInfo info;
};

class PacketContact_InviteReceivedNotification : public PacketContact
{
public:
   PacketContact_InviteReceivedNotification() : PacketContact( PacketType_Contact, ContactType_InviteReceived ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   InvitationInfo info;
};


class PacketContact_ContactRemove : public PacketContact
{
public:
   PacketContact_ContactRemove() : PacketContact( PacketType_Contact, ContactType_RemoveContact ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   UuidString        contactUuid;
   BoundedString140  message;
};

///////////////////////////////////////////////////////////////////

class PacketContact_RemoveInvitation : public PacketContact
{
public:
   PacketContact_RemoveInvitation() : PacketContact( PacketType_Contact, ContactType_RemoveInvitation ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   UuidString  invitationUuid;
};

class PacketContact_AcceptInvite : public PacketContact
{
public:
   PacketContact_AcceptInvite() : PacketContact( PacketType_Contact, ContactType_AcceptInvite ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   UuidString  invitationUuid;
};


class PacketContact_InvitationAccepted : public PacketContact
{
public:
   PacketContact_InvitationAccepted() : PacketContact( PacketType_Contact, ContactType_InvitationAccepted ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   BoundedString80   fromUsername;
   BoundedString80   toUsername;
   UuidString        invitationUuid;
   BoundedString140  message;
   bool              wasAccepted;
};


class PacketContact_DeclineInvitation : public PacketContact
{
public:
   PacketContact_DeclineInvitation() : PacketContact( PacketType_Contact, ContactType_DeclineInvitation ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   UuidString        invitationUuid;
   BoundedString140  message;
};



class PacketContact_InviteBlockUser : public PacketContact
{
public:
   PacketContact_InviteBlockUser() : PacketContact( PacketType_Contact, ContactType_BlockUser ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   BoundedString80   userName;
   UuidString        uuid;
};

///////////////////////////////////////////////////////////////////

class PacketContact_SearchForUser : public PacketContact
{
public:
   PacketContact_SearchForUser() : PacketContact( PacketType_Contact, ContactType_Search ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   BoundedString32   searchString;
   int               limit;
   int               offset;
};

class PacketContact_SearchForUserResult : public PacketContact
{
public:
   PacketContact_SearchForUserResult() : PacketContact( PacketType_Contact, ContactType_SearchResults ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   SerializedKeyValueVector< FriendInfo >   found;
};

///////////////////////////////////////////////////////////////////

class PacketContact_FriendOnlineStatusChange : public PacketContact
{
public:
   PacketContact_FriendOnlineStatusChange() : PacketContact( PacketType_Contact, ContactType_UserOnlineStatusChange ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

  UuidString      uuid;
   FriendInfo     friendInfo;
};

///////////////////////////////////////////////////////////////////

class PacketContact_SetNotationOnUser : public PacketContact
{
public:
   PacketContact_SetNotationOnUser() : PacketContact( PacketType_Contact, ContactType_SetNotation ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   UuidString     uuid;
   FriendInfo     friendInfo;
};

///////////////////////////////////////////////////////////////
/*
namespace Serialize
{
   template<>
   inline void In( const U8* source, int& offset, FriendInfo& value, int minorVersion )
   {
      value.SerializeIn( source, offset, minorVersion );
   }
   template<>
   inline void Out( U8* dest, int& offset, const FriendInfo& value, int minorVersion )
   {
      value.SerializeOut( dest, offset, minorVersion );
   }
   template<>
   inline void In( const U8* source, int& offset, InvitationInfo& value, int minorVersion )
   {
      value.SerializeIn( source, offset, minorVersion );
   }
   template<>
   inline void Out( U8* dest, int& offset, const InvitationInfo& value, int minorVersion )
   {
      value.SerializeOut( dest, offset, minorVersion );
   }
}*/

///////////////////////////////////////////////////////////////////

