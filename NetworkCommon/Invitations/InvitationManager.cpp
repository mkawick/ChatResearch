// InvitationManager.cpp

#include <assert.h>
#include <map>
using namespace std;

#include <iomanip>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/version.hpp>
using boost::format;

#include "../Packets/PacketFactory.h"
#include "../Utils/Utils.h"
#include "../Utils/Enigmosaurus.h"
//#include "../NetworkIn/Diplodocus.h"
#include "InvitationManager.h"

/*
 `id` int(4) unsigned zerofill NOT NULL AUTO_INCREMENT,
  `inviter_id` int(11) DEFAULT NULL,
  `invitee_id` int(11) DEFAULT NULL,
  `group_uuid` varchar(16) DEFAULT NULL,
  `date` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `message` varchar(64) DEFAULT NULL,
  `uuid` varchar(16) DEFAULT NULL,
  `type` int(11) DEFAULT '0',*/

///////////////////////////////////////////////////////////////////

class TableUser_Invitation
{
public:
   enum Columns
   {
      Column_id,
      Column_inviter_id,
      Column_invitee_id,
      Column_group_uuid,
      Column_date,
      Column_message,
      Column_uuid,
      Column_type,
      Column_inviter_name,
      Column_invitee_name
   };
   static const char* const column_names[];
};

typedef Enigmosaurus <TableUser_Invitation> InvitationTable;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PacketSendingInterface* InvitationManager::m_mainServer;
GroupLookupInterface*   InvitationManager::m_groupLookup;

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////


InvitationManager::InvitationManager( Invitation:: InvitationType type ) : m_dbIdentifier (0), 
                                         m_isInitialized( false ),
                                         m_initialRequestForAllInvitesSent( false ),
                                         m_type( type )
{
   time_t currentTime;
   time( &currentTime );
   m_timeOutBeforeInitializing = currentTime;
}

InvitationManager::~InvitationManager()
{
}

void     InvitationManager::Init()
{
   time_t currentTime;
   time( &currentTime );

   if( difftime( currentTime, m_timeOutBeforeInitializing ) < 15 ) // let the other services finish because we rely on them
   {
      return;
   }
   // load all invitations
   if( m_initialRequestForAllInvitesSent == false )
   {
      PacketDbQuery* dbQuery = new PacketDbQuery;
      dbQuery->id =           0;
      dbQuery->meta =         "";
      dbQuery->lookup =       QueryType_AllInvitations;
      dbQuery->serverLookup = m_dbIdentifier;
      dbQuery->query = "SELECT invitation.*, U1.user_name, U2.user_name FROM invitation "\
                        "INNER JOIN users AS U1 ON inviter_id = U1.uuid INNER JOIN users AS U2 ON invitee_id = U2.uuid  WHERE type=";
      dbQuery->query += boost::lexical_cast< string >( m_type );

      m_mainServer->AddQueryToOutput( dbQuery, 0 );
      m_initialRequestForAllInvitesSent = true;
   }
}

//---------------------------------------------------------------

bool     InvitationManager::Update()
{
   if( m_isInitialized == false )
   {
      Init();
      return false;
   }

   PacketFactory factory;

   deque< PacketDbQueryResult* >::iterator itDb = m_dbResults.begin();
   while( itDb != m_dbResults.end() )
   {
      PacketDbQueryResult* dbResult = *itDb++;
      ProcessDbResult( dbResult );
      
      BasePacket* packet = static_cast< BasePacket* >( dbResult );
      factory.CleanupPacket( packet );
   }

   return true;
}

//---------------------------------------------------------------

bool     InvitationManager::HandleDbResult( PacketDbQueryResult* packet ) // not thread safe.. obviously
{
   m_isInitialized = true;

   m_dbResults.push_back( packet );
   m_mainServer->InvitationManagerNeedsUpdate();   
   return true;
}

//---------------------------------------------------------------

bool     InvitationManager::ProcessDbResult( PacketDbQueryResult* dbResult )
{
   if( dbResult->lookup == QueryType_AllInvitations )
   {
      InvitationTable  enigma( dbResult->bucket );
      InvitationTable::iterator    it = enigma.begin();
      while( it != enigma.end() )
      {
         InvitationTable::row    row = *it++;

         Invitation invite;         
         invite.invitationId =      boost::lexical_cast< int >( row[ TableUser_Invitation::Column_id ] );
         invite.inviterUuid =        row[ TableUser_Invitation::Column_inviter_id ];
         invite.inviteeUuid =        row[ TableUser_Invitation::Column_invitee_id ];
         invite.groupUuid =          row[ TableUser_Invitation::Column_group_uuid ];
         invite.date =         row[ TableUser_Invitation::Column_date ];
         invite.message =            row[ TableUser_Invitation::Column_message ];
         invite.invitationUuid =     row[ TableUser_Invitation::Column_uuid ];
         invite.type =              boost::lexical_cast< int >( row[ TableUser_Invitation::Column_type ] );
         invite.inviteeName =        row[ TableUser_Invitation::Column_invitee_name ];
         invite.inviterName =        row[ TableUser_Invitation::Column_inviter_name ];

         AddInvitationToStorage( invite );
      }
      m_isInitialized = true;
      return true;
   }

   return false;
}


///////////////////////////////////////////////////////////////////

bool     InvitationManager::HandlePacketRequest( const BasePacket* packet, U32 connectionId )
{
   switch( packet->packetSubType )
   {
   case PacketInvitation::InvitationType_Base:
      cout << "Invitation base arrived at server" << endl;
      assert( 0 );
      break;
   case PacketInvitation::InvitationType_EchoToServer:
      return EchoHandler( connectionId );
   case PacketInvitation::InvitationType_EchoToClient:
      cout << "Echo to client arrived at server" << endl;
      assert( 0 );
      break;

   case PacketInvitation::InvitationType_InviteUser:
      return InviteUserToChatRoom( static_cast <const PacketInvitation_InviteUser*>( packet ), connectionId );

   case PacketInvitation::InvitationType_InviteUserResponse:
      cout << "InviteUserResponse arrived at server" << endl;
      assert( 0 );
      break;

   case PacketInvitation::InvitationType_CancelInvitation:
      return CancelInvitation( static_cast< const PacketInvitation_CancelInvitation* >( packet ), connectionId );

   case PacketInvitation::InvitationType_InvitationWasCancelled:
      cout << "InvitationType_InvitationWasCancelled arrived at server" << endl;
      assert( 0 );
      break;

   case PacketInvitation::InvitationType_RejectInvitation:
      return RejectInvitation( static_cast< const PacketInvitation_RejectInvitation*> ( packet ), connectionId );

   case PacketInvitation::InvitationType_AcceptInvitation: 
      return AcceptInvitation( static_cast< const PacketInvitation_AcceptInvitation*> ( packet ), connectionId );

   case PacketInvitation::InvitationType_GetListOfInvitations:
      return RequestListOfInvitations( static_cast < const PacketInvitation_GetListOfInvitations*> ( packet ), connectionId );

   case PacketInvitation::InvitationType_GetListOfInvitationsForGroup: 
      return RequestListOfIntivationsToGroup( static_cast< const PacketInvitation_GetListOfInvitationsForGroup*> ( packet ), connectionId );

   case PacketInvitation::InvitationType_GetListOfInvitationsForGroupResponse:
      cout << "InvitationType_InvitationWasCancelled arrived at server" << endl;
      assert( 0 );
      break;
      
   default:
      break;
   }
   return false;
}


//------------------------------------------------------------------------------------------------

bool     InvitationManager::EchoHandler( U32 connectionId )
{
   cout << " Echo " << endl;
   PacketInvitation_EchoToClient* echo = new PacketInvitation_EchoToClient;
   SendMessageToClient( echo, connectionId );
   return true;
}


///////////////////////////////////////////////////////////////////

bool     InvitationManager::InviteUserToChatRoom( const PacketInvitation_InviteUser* invitationPacket, U32 connectionId )
{
   if( m_groupLookup == NULL )
   {
      m_mainServer->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_Invitation_BadServerSetup );
      return false;
   }

   const string& senderUuid = m_mainServer->GetUserUuidByConnectionId( connectionId );
   if( senderUuid.size() == 0 )
   {
      m_mainServer->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_UserUnknown );
      return false;
   }

   //UsersChatRoomList userInChatRoom( invitationPacket->userUuid );
   string inviteeName = m_groupLookup->GetUserName( invitationPacket->userUuid );
   if( inviteeName.size() == 0 )
   {
      m_mainServer->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_UserUnknown );
      return false;
   }

   if( m_groupLookup->IsGroupValid( invitationPacket->inviteGroup ) == false )
   {
      m_mainServer->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_BadChatChannel );
      return false;
   }

   if( IsThereAlreadyAnInvitationToThisGroupInvolvingTheseTwoUsers( invitationPacket->userUuid, senderUuid, invitationPacket->inviteGroup ) )
   {
      PacketInvitation_InviteUserResponse* response = new PacketInvitation_InviteUserResponse;
      response->succeeded = false;
      response->newInvitationUuid = "";

      SendMessageToClient( response, connectionId );
      m_mainServer->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_Invitation_ExistingInvitationWithThatUser );
      return false;
   }

   U32 xorValue = 0xFFFFFFFF;
   xorValue  =  GetCurrentMilliseconds();
   string      invitationUuid = GenerateUUID( xorValue );

   Invitation invite;
   invite.date = GetDateInUTC();
   invite.groupUuid = invitationPacket->inviteGroup;
   invite.invitationUuid = invitationUuid;

   invite.inviteeUuid = invitationPacket->userUuid;
   invite.inviterUuid = senderUuid;
   invite.message = invitationPacket->message;
   invite.type = invitationPacket->invitationType;//Invitation::InvitationType_ChatRoom;
   invite.inviteeName = inviteeName;
   invite.inviterName = m_mainServer->GetUserName( senderUuid );
   AddInvitationToStorage( invite );


   InsertInvitationIntoDb( invite );   

   // notify sender of the new invitation sent
   SendUserHisInvitations <PacketInvitation_GetListOfInvitationsResponse> ( m_invitationMap, IsUserInThisInvitation, senderUuid, connectionId );
   
   PacketInvitation_InviteUserResponse* response = new PacketInvitation_InviteUserResponse;
   response->succeeded = true;
   response->newInvitationUuid = invitationUuid;

   SendMessageToClient( response, connectionId );

   U32 receivedConnectionId = 0;
   m_mainServer->GetUserConnectionId( invitationPacket->userUuid, receivedConnectionId );
   // notify recipient of the new invitation received.
   if( receivedConnectionId != 0 )
   {
      SendUserHisInvitations <PacketInvitation_GetListOfInvitationsResponse> ( m_invitationMap, IsUserInThisInvitation, invitationPacket->userUuid, receivedConnectionId );
     /* PacketInvitation_InviteUserResponse* response = new PacketInvitation_InviteUserResponse;
      response->succeeded = true;
      response->newInvitationUuid = invitationUuid;
      m_mainServer->SendPacketToGateway( response, receiver->GetConnectionId() );*/
   }

   return true;
}

///////////////////////////////////////////////////////////////////

bool     InvitationManager::CancelInvitation( const PacketInvitation_CancelInvitation* invite, U32 connectionId )
{
  /* m_mainServer->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_IncompleteFeature );
      return false;*/

   if( m_groupLookup == NULL )
   {
      m_mainServer->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_Invitation_BadServerSetup );
      return false;
   }

   const string& senderUuid = m_mainServer->GetUserUuidByConnectionId( connectionId );
   if( senderUuid.size() == 0 )
   {
      m_mainServer->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_UserUnknown );
      return false;
   }


   InvitationMapConstIterator it = FindInvitation( senderUuid, invite->invitationUuid );
   if( it == m_invitationMap.end() )
   {
      m_mainServer->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_Invitation_DoesNotExist );
      return false;
   }

   U32 invitationId = it->second.invitationId;
   const string inviteeUuid = it->second.inviteeUuid;
   
   m_invitationMap.erase( it );
   DeleteInvitationFromDb( invitationId );

   SendUserHisInvitations <PacketInvitation_GetListOfInvitationsResponse> ( m_invitationMap, IsUserInThisInvitation, senderUuid, connectionId );

   U32 receivedConnectionId = 0;
   m_mainServer->GetUserConnectionId( inviteeUuid, receivedConnectionId );
   if( receivedConnectionId != 0 )
   {
      SendUserHisInvitations <PacketInvitation_GetListOfInvitationsResponse> ( m_invitationMap, IsUserInThisInvitation, inviteeUuid, receivedConnectionId );
   }

   return true;
}

///////////////////////////////////////////////////////////////////

bool     InvitationManager::RejectInvitation( const PacketInvitation_RejectInvitation* invitation, U32 connectionId  )
{
   stringhash lookup = GenerateUniqueHash( invitation->invitationUuid );
   InvitationMapIterator it = m_invitationMap.find( lookup );
   if( it == m_invitationMap.end() )
   {
      m_mainServer->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_Invitation_DoesNotExist );
      return false;
   }

   const Invitation& invite = it->second;

   string rejecterUuid = m_mainServer->GetUserUuidByConnectionId( connectionId );
   if( rejecterUuid.size() == 0 )
   {
      m_mainServer->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_UserUnknown );
      return false;
   }
   if( invite.inviteeUuid != rejecterUuid )
   {
      m_mainServer->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_Invitation_DoesNotExist );// you are not allowed to accept this.
      return false;
   }

   U32 invitationId = invite.invitationId;
   string inviterUuid = invite.inviterUuid;
   U32 inviterConnectionId = 0;
   m_mainServer->GetUserConnectionId( inviterUuid, inviterConnectionId );
   m_invitationMap.erase( it );// must erase before sending the updated lists.

   PacketInvitation_RejectInvitationResponse* response = new PacketInvitation_RejectInvitationResponse;
   SendMessageToClient( response, connectionId );
   SendUserHisInvitations <PacketInvitation_GetListOfInvitationsResponse> ( m_invitationMap, IsUserInThisInvitation, rejecterUuid, connectionId );

   
   if( inviterConnectionId != 0 )
   {
      SendUserHisInvitations <PacketInvitation_GetListOfInvitationsResponse> ( m_invitationMap, IsUserInThisInvitation, inviterUuid, inviterConnectionId );
   }

   DeleteInvitationFromDb( invitationId );

   return true;
}

///////////////////////////////////////////////////////////////////

bool     InvitationManager::AcceptInvitation( const PacketInvitation_AcceptInvitation* invitation, U32 connectionId  )
{
   stringhash lookup = GenerateUniqueHash( invitation->invitationUuid );
   InvitationMapIterator it = m_invitationMap.find( lookup );
   if( it == m_invitationMap.end() )
   {
      m_mainServer->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_Invitation_DoesNotExist );
      return false;
   }

   const Invitation& invite = it->second;

   U32 senderConnectionId = 0;
   string senderUuid = m_mainServer->GetUserUuidByConnectionId( connectionId );
   if( senderUuid.size() == 0 )
   {
      m_mainServer->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_UserUnknown );
      return false;
   }
   if( invite.inviteeUuid != senderUuid )
   {
      m_mainServer->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_Invitation_DoesNotExist );// you are not allowed to accept this.
      return false;
   }

   if( invite.groupUuid.size() > 0 ) // accounting for non-group invites
   {
      if( m_groupLookup == NULL )
      {
         m_mainServer->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_Invitation_BadServerSetup );
         return false;
      }

      if( m_groupLookup->UserAddsSelfToGroup( invite.groupUuid, invite.inviteeUuid ) == false )// bad chat room
      {
         m_mainServer->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_Invitation_DoesNotExist );
         return false;
      }
   }

   U32 invitationId = invite.invitationId;
   string inviterUuid = invite.inviterUuid;
   U32 inviterConnectionId = 0;
   m_mainServer->GetUserConnectionId( inviterUuid, inviterConnectionId );
   m_invitationMap.erase( it );// must erase before sending the updated lists.

   SendUserHisInvitations <PacketInvitation_GetListOfInvitationsResponse> ( m_invitationMap, IsUserInThisInvitation, senderUuid, connectionId );

   
   if( inviterConnectionId != 0 )
   {
      SendUserHisInvitations <PacketInvitation_GetListOfInvitationsResponse> ( m_invitationMap, IsUserInThisInvitation, inviterUuid, inviterConnectionId );
   }

   DeleteInvitationFromDb( invitationId );   

   return true;
}

///////////////////////////////////////////////////////////////////

bool     InvitationManager::RequestListOfInvitations( const PacketInvitation_GetListOfInvitations* request, U32 connectionId )
{
   string senderUuid = m_mainServer->GetUserUuidByConnectionId( connectionId );
   if( senderUuid.size() == 0 )
   {
      m_mainServer->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_UserUnknown );
      return false;
   }

   SendUserHisInvitations <PacketInvitation_GetListOfInvitationsResponse> ( m_invitationMap, IsUserInThisInvitation, senderUuid, connectionId );
   return true;
}

bool     InvitationManager::RequestListOfIntivationsToGroup( const PacketInvitation_GetListOfInvitationsForGroup* request, U32 connectionId )
{
   string senderUuid = m_mainServer->GetUserUuidByConnectionId( connectionId );
   if( senderUuid.size() == 0 )
   {
      m_mainServer->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_UserUnknown );
      return false;
   }

   // we do not want to allow a user to simply pull down all of the invitations for random groups. 
   // because we are using uuids, we do not need to filter, in theory, but definitely keep an eye on this.
   // we could go to the chat channel manager and pull a list of groups and verify that the user is
   // in that group, and if not, look at the invitations for this user and verify that he has been invited to 
   // that group ... at least

   //m_invitationMap;

   SendUserHisInvitations <PacketInvitation_GetListOfInvitationsForGroupResponse> ( m_invitationMap, IsInvitationGroup, request->groupUuid, connectionId );
   return true;
}

///////////////////////////////////////////////////////////////////

template< typename PacketType >
 void    InvitationManager::SendUserHisInvitations( const InvitationMap& listOfInvitations, MatchInvitationType compare, const string& uuidId, U32 connectionId )
{
   // todo, add looking up the group names if the name is blank

   const int maxInvitationsToSend = 15; // given the roughly 130 bytes just for the structure of the chat channel, 
   // we need to be sure that we don't overflow the buffer.
   if( listOfInvitations.size() == 0 )
   {
      PacketType* response = new PacketType;
      response->uuid = uuidId;
      response->invitationList.SetIndexParams( 0, 0 );
      SendMessageToClient( response, connectionId );
      return;
   }   

   int   currentOffset = 0;
   int   totalCount = listOfInvitations.size();
   int   countForThisUser = 0;
   InvitationMapConstIterator it = listOfInvitations.begin();
   while( it != listOfInvitations.end() )
   {
      const Invitation& invite = it->second;
      if( compare( invite, uuidId ) == true )
      {
         countForThisUser++;
      }
      it++;
   }

   //------------- here is where we loop and send out all matching invitations -----------
   PacketType* response = new PacketType;
   response->uuid = uuidId;
   response->invitationList.SetIndexParams( currentOffset, countForThisUser );
   if( countForThisUser == 0 )
   {
      SendMessageToClient( response, connectionId );
      return;
   }

   //for( int i=0; i<totalCount; i++ )// walk the entire list looking for appropriate matching invitations
   it = listOfInvitations.begin();
   while( it != listOfInvitations.end() )
   {
      const Invitation& invite = it->second;
      if( compare( invite, uuidId ) == true )
      {
         response->invitationList.insert( invite.groupUuid, invite );
         if( response->invitationList.size() >= maxInvitationsToSend )
         {
            SendMessageToClient( response, connectionId );

            currentOffset += maxInvitationsToSend; // important here

            if( currentOffset < countForThisUser ) // we may have just sent the last invitation
            {
            // restart with a new packet
               response = new PacketType;
               response->uuid = uuidId;
               response->invitationList.SetIndexParams( currentOffset, countForThisUser );
            }
            else
            {
               return;// we're done
            }
         }
      }
      it++;
   }
   if( response && response->invitationList.size() != 0 )
   {
      SendMessageToClient( response, connectionId );
   }
   else
   {
      if( response == NULL )
         cout << "response packet is invalid" << endl;
      else
         cout << "response packet has 0 inviations" << endl;
      assert( 0 );
   }
   //------------- end loop and send out all matching invitations -----------
}

/*
//void     SaveInvitation
void     InvitationManager::QueryInvitationsForUserAsRecipient()
{
}

///////////////////////////////////////////////////////////////////

void     InvitationManager:: QueryInvitationsForUserAsSender()
{
}


void     InvitationManager::SendInvitationToUser( const Invitation& invite )// recipient is in struct
{
}
*/


///////////////////////////////////////////////////////////////////

void           InvitationManager::AddInvitationToStorage( Invitation& invite )
{
   string name;
   if( m_groupLookup != NULL && 
      m_groupLookup->GetGroupName( invite.groupUuid, name ) == true )
   {
      invite.groupName = name;
   }
   stringhash lookup = GenerateUniqueHash( invite.invitationUuid );
   InvitationMapIterator it = m_invitationMap.find( lookup );
   if( it != m_invitationMap.end() )      
   {
      it->second = invite;
   }
   else
   {
      m_invitationMap.insert( InvitationPair( GenerateUniqueHash( invite.invitationUuid ), invite ) );
   }
}

///////////////////////////////////////////////////////////////////

InvitationManager::InvitationMapConstIterator  
InvitationManager::FindInvitation( const string& inviter, const string& invitationUuid ) const 
{
   InvitationMapConstIterator it = m_invitationMap.begin();
   while( it != m_invitationMap.end() )
   {
      const Invitation& invite = it->second;
      if( invite.invitationUuid == invitationUuid ) // this seems like the best way to do a first pass test
      {
         if( invite.inviterUuid == inviter )
         {
            return it;
         }
      }
      it++;
   }
   return m_invitationMap.end();
}

///////////////////////////////////////////////////////////////////

InvitationManager::InvitationMapConstIterator  
InvitationManager::FindInvitation( const string& inviter, const string& invitee, const string& groupUuid ) const 
{
   InvitationMapConstIterator it = m_invitationMap.begin();
   while( it != m_invitationMap.end() )
   {
      const Invitation& invite = it->second;
      if( invite.groupUuid == groupUuid ) // this seems like the best way to do a first pass test
      {
         if( invite.inviteeUuid == inviter && invite.inviterUuid == invitee )
         {
            return it;
         }
      }
      it++;
   }
   return m_invitationMap.end();
}

///////////////////////////////////////////////////////////////////

bool           InvitationManager::IsThereAlreadyAnInvitationToThisGroupInvolvingTheseTwoUsers( const string& user1, const string& user2, const string& groupUuid ) const 
{
   InvitationMapConstIterator it = m_invitationMap.begin();
   while( it != m_invitationMap.end() )
   {
      const Invitation& invite = it->second;
      if( invite.groupUuid == groupUuid ) // this seems like the best way to do a first pass test
      {
         if( ( invite.inviteeUuid == user1 && invite.inviterUuid == user2 ) ||
            ( invite.inviteeUuid == user2 && invite.inviterUuid == user1 ) )
         {
            return true;
         }
      }
      it++;
   }
   return false;
}

void           InvitationManager::InsertInvitationIntoDb( const Invitation& invite )
{
   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =           0;
   dbQuery->meta =         "";
   dbQuery->lookup =       QueryType_InsertInvitation;
   dbQuery->serverLookup = m_dbIdentifier;
   dbQuery->isFireAndForget = true;

   string query = "INSERT INTO invitation SET inviter_id='";
   query += invite.inviterUuid;
   query += "', invitee_id='";
   query += invite.inviteeUuid;
   query += "', group_uuid='";
   query += invite.groupUuid;
   query += "', message='";
   query += invite.message;
   query += "', uuid='";
   query += invite.invitationUuid;
   query += "', type=";
   query += boost::lexical_cast< string >( (U32) invite.type );
   dbQuery->query = query;

   m_mainServer->AddQueryToOutput( dbQuery, 0 );
}

void           InvitationManager::DeleteInvitationFromDb( U32 invitationId )
{
   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =           0;
   dbQuery->meta =         "";
   dbQuery->lookup =       QueryType_DeleteInvitation;
   dbQuery->serverLookup = m_dbIdentifier;
   dbQuery->isFireAndForget = true;

   //delete from invitation where id=326
   string query = "DELETE FROM invitation WHERE id=";
   query += boost::lexical_cast< string >( invitationId );
   dbQuery->query = query;

   m_mainServer->AddQueryToOutput( dbQuery, 0 );
}

/*
void           InvitationManager::SendInvitationToSenderAndRecipient( const Invitation& invite )
{
}
*/
//---------------------------------------------------------------

bool     InvitationManager::SendMessageToClient( BasePacket* packet, U32 connectionId ) const
{
   if( packet->packetType != PacketType_Invitation )
   {
      cout << "Invitation manager is sending the wrong type of data" << endl;
      assert( 0 );
   }
   PacketInvitation* p = static_cast<PacketInvitation*>( packet );
   p->invitationType = m_type;

   PacketGatewayWrapper* wrapper = new PacketGatewayWrapper();
   wrapper->SetupPacket( packet, connectionId );

   m_mainServer->SendMessageToClient( wrapper, connectionId );
   return true;
}

//-

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
