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

   bool  GetListOfContacts();

   void  SetServer( DiplodocusContact* infoServer ) { m_infoServer = infoServer; }

   void  NeedsUpdate() { m_requiresUpdate = true; }
   void  UserLoggedOut();

   bool  IsLoggedOut() const { return m_isLoggedOut; }
   int   SecondsExpiredSinceLoggedOut();

   void  Update();

private:

   bool  InviteUser( const string& inviteeUuid, const string& message );
   void  FinishInvitation( U32 inviteeId, const string& message );
   void  YouHaveBeenInvitedToBeAFriend( const string& userName, const string& uuid );

   enum 
   {
      QueryType_UserProfile,
      QueryType_Friends,
      QueryType_FriendRequestReceived,
      QueryType_FriendRequestsSent,

      QueryType_GetInviteeDetails,
      QueryType_AddInvitationToUser,
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
};
///////////////////////////////////////////////////////////////////
