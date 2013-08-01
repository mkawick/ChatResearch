#pragma once
#include "../NetworkCommon/DataTypes.h"
#include "../NetworkCommon/Packets/ContactPacket.h"
#include "../NetworkCommon/Utils/TableWrapper.h"

class PacketDbQueryResult;
class DiplodocusContact;
class PacketContact;

struct Invitation
{
   int   inviterId;
   int   inviteeId;
   int   invitationNumber;
   bool  wasNotified;

   string userName;
   string userUuid;
   string message;
   string uuid;
   string date;
};

struct InvitationQueryLookup
{
   int      index;
   string   inviteeUuid;
   string   inviteeName;
   string   message;
};

///////////////////////////////////////////////////////////////////

class UserContact
{
public:
   UserContact( const UserInfo& info, U32 connectionId );
   ~UserContact();

   const UserInfo& GetUserInfo() const { return m_userInfo;}

   void  Init(); // send queries
   bool  HandleDbQueryResult( const PacketDbQueryResult* result );
   bool  HandleRequestFromClient( const PacketContact* packet );

   void  SetServer( DiplodocusContact* infoServer ) { m_infoServer = infoServer; }

   void  NeedsUpdate() { m_requiresUpdate = true; }
   void  UserLoggedOut();

   bool  IsLoggedOut() const { return m_isLoggedOut; }
   int   SecondsExpiredSinceLoggedOut();

   void  Update();

private:

   void  InitContactsAndInvitations();
   bool  GetListOfContacts();
   bool  GetListOfInvitations();
   bool  GetListOfInvitationsSent();

   bool  InviteUser( const PacketContact_InviteContact* packet );
   bool  AcceptInvitation( const PacketContact_AcceptInvite* packet );
   void  FinishAcceptingInvitation( const PacketDbQueryResult* result ); 
   void  FinishInvitation( U32 inviteeId, const string& message, UserContact* contact = NULL );
   void  YouHaveBeenInvitedToBeAFriend( const string& userName, const string& uuid, const string& message, const string& curentTime );
   void  InvitationAccepted( const string& sentFromuserName, const string& sentToUserName, bool accepted );
   bool  InformFriendsOfOnlineStatus( bool isOnline );

   bool  YourFriendsOnlineStatusChange( U32 connectionId, const string& userName, const string& UUID, bool isOnline );

   bool  DoesPendingInvitationExist( const string& inviteeUuid, const string& inviteeName );

   enum 
   {
      QueryType_UserProfile,
      QueryType_Friends,
      QueryType_FriendRequestReceived,
      QueryType_FriendRequestsSent,

      QueryType_GetInviteeDetails,
      QueryType_AddInvitationToUser,
      QueryType_GetInvitationPriorToAcceptance,
      QueryType_DeleteInvitation,
      QueryType_InsertNewFriend,
   };


   U32                  m_connectionId;
   
   UserInfo             m_userInfo;
   bool                 m_requiresUpdate;
   bool                 m_isLoggedOut;
   bool                 m_hasBeenInitialized;

   DiplodocusContact*   m_infoServer;

   time_t               m_timeLoggedOut;

   vector< UserInfo >   m_friends;
   vector< Invitation > m_invitationsOut;
   vector< Invitation > m_invitationsIn;
   //vector< PacketContact_InviteContact > m_invitationsPendingUserLookup;

   int                  m_invitationQueryIndex;
   list< InvitationQueryLookup >  m_invitationQueryLookup;
};
///////////////////////////////////////////////////////////////////
