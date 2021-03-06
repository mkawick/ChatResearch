#pragma once

#include "../Packets/BasePacket.h"
#include "../Packets/ChatPacket.h"
#include "../Packets/DbPacket.h"
#include "../Packets/InvitationPacket.h"
#include "../UserAccount/UserAccountCommon.h"
#include <deque>
using namespace std;

class PacketInvitation;
class PacketDbQueryResult;
class PacketInvitation_InviteUser;
class PacketInvitation_CancelInvitation;
class PacketInvitation_RejectInvitation;
class PacketInvitation_AcceptInvitation;
class PacketInvitation_GetListOfInvitations;
class PacketInvitation_GetListOfInvitationsForGroup;
class PacketDbQuery;

///////////////////////////////////////////////////////////////////

class PacketSendingInterface
{
public:
   virtual bool     SendMessageToClient( BasePacket* packet, U32 connectionId, U32 gatewayId ) = 0;
   virtual bool     AddQueryToOutput( PacketDbQuery* packet, U32 connectionId ) = 0;
   
   virtual bool     SendErrorToClient( U32 connectionId, U32 gatewayId, PacketErrorReport::ErrorType error ) = 0;
   virtual void     GetUserConnectionId( const string& uuid, vector< SimpleConnectionDetails >& listOfConnections ) = 0;
   virtual string   GetUserName( const string& uuid ) = 0;
   virtual string   GetUserUuidByConnectionId( U32 connectionId ) = 0;

   virtual void     InvitationManagerNeedsUpdate() = 0;
};

class GroupLookupInterface
{
public:
   virtual bool      IsGroupValid( const string& inviteGroup ) const = 0;
   virtual bool      GetGroupName( const string& groupUuid, string& name ) const = 0;
   virtual string    GetUserName( const string& uuid ) const = 0;
   virtual bool      UserAddsSelfToGroup( const string& channelUuid, const string& addedUserUuid, U32 connectionId ) = 0;
};

///////////////////////////////////////////////////////////////////

class InvitationManager
{
public:
   InvitationManager( Invitation:: InvitationType type );
   ~InvitationManager();

   void           Init();

   static void    Set( PacketSendingInterface* svr ) { m_mainServer = svr; }
   static void    Set( GroupLookupInterface* svr ) { m_groupLookup = svr; }

   void           SetDbIdentifier( U32 dbIdentifier ) { m_dbIdentifier = dbIdentifier; }
   U32            GetDbIdentifier() const { return m_dbIdentifier; }

   bool           HandleDbResult( PacketDbQueryResult* packet );

   bool           Update();

   bool           HandlePacketRequest( const BasePacket* pPacket, U32 connectionId, U32 gatewayId );

   ///-----------------------------------------------------------

   bool           RemoveAnyRelatedInvitations( const string& channelUuid );

protected:

   enum 
   {
      QueryType_AllInvitations,
      QueryType_InsertInvitation,
      QueryType_DeleteInvitation,
   };

   static PacketSendingInterface*            m_mainServer;
   static GroupLookupInterface*              m_groupLookup;
   deque< PacketDbQueryResult* > m_dbResults;

   typedef map< stringhash, Invitation >     InvitationMap;
   typedef pair< stringhash, Invitation >    InvitationPair;
   typedef InvitationMap::iterator           InvitationMapIterator;
   typedef InvitationMap::const_iterator     InvitationMapConstIterator;

protected:

   bool           EchoHandler( U32 connectionId, U32 gatewayId );
   bool           InviteUserToChatRoom( const PacketInvitation_InviteUser* invitation, U32 connectionId, U32 gatewayId );
   bool           CancelInvitation( const PacketInvitation_CancelInvitation* invitation, U32 connectionId, U32 gatewayId );
   bool           RejectInvitation( const PacketInvitation_RejectInvitation* invitation, U32 connectionId, U32 gatewayId );
   bool           AcceptInvitation( const PacketInvitation_AcceptInvitation* invitation, U32 connectionId, U32 gatewayId );
   bool           RequestListOfInvitations( const PacketInvitation_GetListOfInvitations* request, U32 connectionId, U32 gatewayId );
   bool           RequestListOfIntivationsToGroup( const PacketInvitation_GetListOfInvitationsForGroup* request, U32 connectionId, U32 gatewayId );

   void           DeleteAllInvitationsToThisGroup( const string& groupUuid, const string& inviteeUuid );
   bool           ProcessDbResult( PacketDbQueryResult* dbResult );
   bool           SendMessageToClient( BasePacket* packet, U32 connectionId, U32 gatewayId ) const;

   void           AddInvitationToStorage( Invitation& inv );
   void           InsertInvitationIntoDb( const Invitation& inv );
   void           DeleteInvitationFromDb( U32 invitationId );
   //void           SendInvitationToSenderAndRecipient( const Invitation& inv );

   InvitationMapIterator  FindInvitation( const string& inviter, const string& invitationUuid );
   InvitationMapIterator  FindInvitation( const string& inviter, const string& invitee, const string& groupUuid );
   bool           IsThereAlreadyAnInvitationToThisGroupInvolvingTheseTwoUsers( const string& user1, const string& user2, const string& groupUuid ) const;
   

   

   Invitation::InvitationType          m_type;
   InvitationMap                       m_invitationMap;

   U32                                 m_dbIdentifier;

   bool                                m_initialRequestForAllInvitesSent;
   bool                                m_isInitialized;

   time_t                              m_timeOutBeforeInitializing;


protected:
   template< typename PacketType >
   void           SendUserHisInvitations( const InvitationMap& listOfInvitations, MatchInvitationType compare, const string& userUuid, U32 connectionId, U32 gatewayId );
};

///////////////////////////////////////////////////////////////////
