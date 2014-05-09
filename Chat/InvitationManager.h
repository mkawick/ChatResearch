#pragma once


#include <deque>
using namespace std;

class PacketInvitation;
class DiplodocusChat;
class PacketDbQueryResult;
class ChatRoomManager;
class PacketInvitation_InviteUser;
class PacketInvitation_CancelInvitation;
class PacketInvitation_RejectInvitation;
class PacketInvitation_AcceptInvitation;
class PacketInvitation_GetListOfInvitations;

///////////////////////////////////////////////////////////////////

class InvitationManager
{
public:
   InvitationManager();
   ~InvitationManager();

   void           Init();

   static void    Set( DiplodocusChat* svr ) { m_chatServer = svr; }
   static void    Set( ChatRoomManager* svr ) { m_chatRoomManager = svr; }

   void           SetDbIdentifier( int dbIdentifier ) { m_dbIdentifier = dbIdentifier; }
   int            GetDbIdentifier() const { return m_dbIdentifier; }

   bool           HandleDbResult( PacketDbQueryResult* packet );

   bool           Update();

   bool           HandlePacketRequest( const BasePacket* pPacket, U32 connectionId );

   ///-----------------------------------------------------------

protected:

   bool           EchoHandler( U32 connectionId );
   bool           InviteUserToChatRoom( const PacketInvitation_InviteUser* invitation, U32 connectionId );
   bool           CancelInvitation( const PacketInvitation_CancelInvitation* invitation, U32 connectionId );
   bool           RejectInvitation( const PacketInvitation_RejectInvitation* invitation, U32 connectionId );
   bool           AcceptInvitation( const PacketInvitation_AcceptInvitation* invitation, U32 connectionId );
   bool           RequestListOfInvitations( const PacketInvitation_GetListOfInvitations* request, U32 connectionId );

   bool           ProcessDbResult( PacketDbQueryResult* dbResult );
   bool           SendMessageToClient( BasePacket* packet, U32 connectionId ) const;

   void           AddInvitationToStorage( const Invitation& inv );
   void           InsertInvitationIntoDb( const Invitation& inv );
   void           DeleteInvitationFromDb( U32 invitationId );
   //void           SendInvitationToSenderAndRecipient( const Invitation& inv );

   bool           IsThereAlreadyAnInvitationToThisGroupInvolvingTheseTwoUsers( const string& user1, const string& user2, const string& groupUuid ) const;
   bool           IsInvitationAlreadyStored( const Invitation& inv );
   bool           DoesUserHavePendingInvite( const string& inviterUuid, const string& inviteeUuid );
   void           SendUserHisInvitations( const ChatUser* user );

   enum 
   {
      QueryType_AllInvitations,
      QueryType_InsertInvitation,
      QueryType_DeleteInvitation,
   };

   static DiplodocusChat*        m_chatServer;
   static ChatRoomManager*       m_chatRoomManager;
   deque< PacketDbQueryResult* > m_dbResults;

   typedef map< stringhash, Invitation >     InvitationMap;
   typedef pair< stringhash, Invitation >    InvitationPair;
   typedef InvitationMap::iterator           InvitationMapIterator;
   typedef InvitationMap::const_iterator     InvitationMapConstIterator;
   InvitationMap                 m_invitationMap;

   int            m_dbIdentifier;

   bool           m_initialRequestForAllInvitesSent;
   bool           m_isInitialized;
};

///////////////////////////////////////////////////////////////////
