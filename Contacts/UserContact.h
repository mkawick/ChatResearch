#pragma once

#include "../NetworkCommon/DataTypes.h"
#include "../NetworkCommon/Packets/ContactPacket.h"
#include "../NetworkCommon/Utils/TableWrapper.h"
#include "../NetworkCommon/UserAccount/UserAccountCommon.h"

class PacketDbQueryResult;
class DiplodocusContact;
class PacketContact;
class PacketUserUpdateProfile;
class PacketContact_SetNotationOnUser;

struct P2PInvitation
{
   int   inviterId;
   int   inviteeId;
   //int   invitationNumber;
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

class UserContact : public UserLoginBase
{
public:
   enum 
   {
      QueryType_UserProfile,
      QueryType_Friends,
      QueryType_FriendRequestReceived,
      QueryType_FriendRequestsSent,
      QueryType_DeleteFriend,
      QueryType_FriendAddNotation,

      QueryType_GetInviteeDetails,
      QueryType_AddInvitationToUser,
      QueryType_GetInvitationPriorToAcceptance,
      QueryType_GetInvitationPriorToDeclination,
      QueryType_DeleteInvitation,
      QueryType_SearchForUser,
      QueryType_InsertNewFriend,
   };

public:
   //---------------------------------------

   UserContact();
   ~UserContact();

   //---------------------------------------

   void                 PostLogin();
   void                 PostLogout();

   bool                 IsLoggedOut() const { if( GetFirstConnectedId() == 0 ) return true; return false; }

   void                 SetServer( DiplodocusContact* contactServer ) { m_contactServer = contactServer; }

   void                 Init(); // send queries
   bool                 HandleDbQueryResult( const PacketDbQueryResult* result );
   bool                 HandleRequestFromClient( const PacketContact* packet, U32 connectionId );

   void                 Update();
   bool                 UpdateProfile( const PacketUserUpdateProfile* profile );

   bool                 IsBlockingFriendInvites() const { return m_blockContactInvitations; }
   void                 AfterFriendQuery_SendListToClient() { m_afterFriendQuery_SendListToClient = true; }

private:
   void                 FinishLoginBySendingUserFriendsAndInvitations( U32 connectionId );
   void                 InitContactsAndInvitations();
   void                 PrepFriendQuery();
   void                 PrepInvitationsQueries();

   bool                 GetListOfContacts( U32 connectionId );
   bool                 ListOfInvitationsReceived_SendToClient( U32 connectionId );
   bool                 ListOfInvitationsSent_SendToClient( U32 connectionId );

   bool                 InviteUser( const PacketContact_InviteContact* packet, U32 connectionId );
   bool                 AcceptInvitation( const PacketContact_AcceptInvite* packet, U32 connectionId );
   bool                 DeclineInvitation( const PacketContact_DeclineInvitation* packet, U32 connectionId );
   bool                 RemoveSentInvitation( const PacketContact_RemoveInvitation* packet, U32 connectionId );
   bool                 PerformSearch( const PacketContact_SearchForUser* packet, U32 connectionId );
   bool                 RemoveContact( const PacketContact_ContactRemove* packet, U32 connectionId );
   bool                 EchoHandler( U32 connectionId );
   bool                 AddNotationToContact( const PacketContact_SetNotationOnUser* notationPacket, U32 connectionId );

   void                 FinishAcceptingInvitation( const PacketDbQueryResult* result, U32 connectionId ); 
   void                 FinishDecliningingInvitation(  const PacketDbQueryResult* dbResult, U32 connectionId );
   void                 FinishSearchResult( const PacketDbQueryResult* dbResult, U32 connectionId );
   void                 FinishInvitation( U32 inviteeId, const string& message, const string& inviteeName, const string& inviteeUuid, U32 connectionId, UserContact* contact = NULL );
   void                 YouHaveBeenInvitedToBeAFriend( const string& userName, const string& uuid, const string& message, const string& curentTime );
   void                 InvitationAccepted( const string& sentFromuserName, const string& sentToUserName, const string& invitationUuid, const string& message, bool accepted );
   bool                 InformFriendsOfOnlineStatus( bool isOnline );

   bool                 YourFriendsOnlineStatusChange( const string& userName, const string& UUID, int avatarId, bool isOnline );

   bool                 DoesPendingInvitationExist( const string& inviteeUuid, const string& inviteeName );
   bool                 HaveIAlreadyBeenInvited( const string& userUuid );

   void                 InsertInvitationReceived( U32 inviteeId, U32 inviterId, bool wasNotified, const string& userName, const string& userUuid, const string& message, const string& invitationUuid, const string& date );
   void                 InsertInvitationSent( U32 inviteeId, U32 inviterId, bool wasNotified, const string& userName, const string& userUuid, const string& message, const string& invitationUuid, const string& date );
   void                 InsertFriend( UserInfo& ui );
   void                 RemoveInvitationReceived( const string& uuid );
   void                 RemoveInvitationSent( const string& invitationUuid );

   DiplodocusContact*   m_contactServer;

   bool                 m_requiresUpdate;
   bool                 m_hasBeenInitialized;
   bool                 m_friendListFilled;
   bool                 m_friendRequestSentListFilled;
   bool                 m_friendRequestReceivedListFilled;

   bool                 m_displayOnlineStatusToOtherUsers;
   bool                 m_blockContactInvitations;
   bool                 m_blockGroupInvitations;
   bool                 m_afterFriendQuery_SendListToClient;

   vector< UserInfo >   m_contacts;
   vector< P2PInvitation > m_invitationsOut;
   vector< P2PInvitation > m_invitationsIn;

   int                  m_invitationQueryIndex;
   list< InvitationQueryLookup >  m_invitationQueryLookup;
};
///////////////////////////////////////////////////////////////////
/*
class UserContact
{
public:
   UserContact( const UserInfo& info, U32 connectionId, U32 gatewayId );
   ~UserContact();

   const UserInfo& GetUserInfo() const { return m_userInfo;}
   void  SetGatewayId( U32 id ) { m_gatewayId = id; }
   U32   GetGatewayId() const { return m_gatewayId; }
   void  SetConnectionId( U32 newConnectionId ) { m_connectionId = newConnectionId; m_userInfo.connectionId = newConnectionId; } // be super cautios here. this is meant for users who relog in.

   void  Init(); // send queries
   bool  HandleDbQueryResult( const PacketDbQueryResult* result );
   bool  HandleRequestFromClient( const PacketContact* packet );

   void  FinishLoginBySendingUserFriendsAndInvitations();

   void  SetServer( DiplodocusContact* contactServer ) { m_contactServer = contactServer; }

   void  NeedsUpdate() { m_requiresUpdate = true; }
   void  UserLoggedOut();

   bool  IsLoggedOut() const { return m_isLoggedOut; }
   void  ClearLoggedOut() { m_isLoggedOut = false; }
   int   SecondsExpiredSinceLoggedOut();

   void  Update();
   bool  UpdateProfile( const PacketUserUpdateProfile* profile );

   bool  IsBlockingFriendInvites() const { return m_blockContactInvitations; }

   void  AfterFriendQuery_SendListToClient() { m_afterFriendQuery_SendListToClient = true; }

private:

   void  InitContactsAndInvitations();
   void  PrepFriendQuery();
   void  PrepInvitationsQueries();

   bool  GetListOfContacts();
   bool  ListOfInvitationsReceived_SendToClient();
   bool  ListOfInvitationsSent_SendToClient();

   bool  InviteUser( const PacketContact_InviteContact* packet );
   bool  AcceptInvitation( const PacketContact_AcceptInvite* packet );
   bool  DeclineInvitation( const PacketContact_DeclineInvitation* packet );
   bool  RemoveSentInvitation( const PacketContact_RemoveInvitation* packet );
   bool  PerformSearch( const PacketContact_SearchForUser* packet );
   bool  RemoveContact( const PacketContact_ContactRemove* packet );
   bool  EchoHandler();
   bool  AddNotationToContact( const PacketContact_SetNotationOnUser* notationPacket );

   void  FinishAcceptingInvitation( const PacketDbQueryResult* result ); 
   void  FinishDecliningingInvitation(  const PacketDbQueryResult* dbResult );
   void  FinishSearchResult( const PacketDbQueryResult* dbResult );
   void  FinishInvitation( U32 inviteeId, const string& message, const string& inviteeName, const string& inviteeUuid, UserContact* contact = NULL );
   void  YouHaveBeenInvitedToBeAFriend( const string& userName, const string& uuid, const string& message, const string& curentTime );
   void  InvitationAccepted( const string& sentFromuserName, const string& sentToUserName, const string& invitationUuid, const string& message, bool accepted );
   bool  InformFriendsOfOnlineStatus( bool isOnline );

   bool  YourFriendsOnlineStatusChange( U32 connectionId, const string& userName, const string& UUID, int avatarId, bool isOnline );

   bool  DoesPendingInvitationExist( const string& inviteeUuid, const string& inviteeName );
   bool  HaveIAlreadyBeenInvited( const string& userUuid );

   void  InsertInvitationReceived( U32 inviteeId, U32 inviterId, bool wasNotified, const string& userName, const string& userUuid, const string& message, const string& invitationUuid, const string& date );
   void  InsertInvitationSent( U32 inviteeId, U32 inviterId, bool wasNotified, const string& userName, const string& userUuid, const string& message, const string& invitationUuid, const string& date );
   void  InsertFriend( UserInfo& ui );
   void  RemoveInvitationReceived( const string& uuid );
   void  RemoveInvitationSent( const string& invitationUuid );

   enum 
   {
      QueryType_UserProfile,
      QueryType_Friends,
      QueryType_FriendRequestReceived,
      QueryType_FriendRequestsSent,
      QueryType_DeleteFriend,
      QueryType_FriendAddNotation,

      QueryType_GetInviteeDetails,
      QueryType_AddInvitationToUser,
      QueryType_GetInvitationPriorToAcceptance,
      QueryType_GetInvitationPriorToDeclination,
      QueryType_DeleteInvitation,
      QueryType_SearchForUser,
      QueryType_InsertNewFriend,
   };


   U32                  m_connectionId;
   U32                  m_gatewayId;
   
   UserInfo             m_userInfo;
   bool                 m_requiresUpdate;
   bool                 m_isLoggedOut;
   bool                 m_hasBeenInitialized;
   bool                 m_friendListFilled;
   bool                 m_friendRequestSentListFilled;
   bool                 m_friendRequestReceivedListFilled;

   bool                 m_displayOnlineStatusToOtherUsers;
   bool                 m_blockContactInvitations;
   bool                 m_blockGroupInvitations;
   bool                 m_afterFriendQuery_SendListToClient;

   DiplodocusContact*   m_contactServer;

   time_t               m_timeLoggedOut;

   vector< UserInfo >   m_contacts;
   vector< P2PInvitation > m_invitationsOut;
   vector< P2PInvitation > m_invitationsIn;
   //vector< PacketContact_InviteContact > m_invitationsPendingUserLookup;

   int                  m_invitationQueryIndex;
   list< InvitationQueryLookup >  m_invitationQueryLookup;
};
*/
///////////////////////////////////////////////////////////////////
