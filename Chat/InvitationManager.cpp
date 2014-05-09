// InvitationManager.cpp

#include <assert.h>

#include <iomanip>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
using boost::format;

#include "ChatRoomManager.h"
#include "../NetworkCommon/Packets/BasePacket.h"
#include "../NetworkCommon/Packets/ChatPacket.h"
#include "../NetworkCommon/Packets/DbPacket.h"
#include "../NetworkCommon/Packets/InvitationPacket.h"

#include "DiplodocusChat.h"
#include "ChatUser.h"
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

DiplodocusChat*         InvitationManager::m_chatServer;
ChatRoomManager*        InvitationManager::m_chatRoomManager;

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////


InvitationManager::InvitationManager() : m_dbIdentifier (0), 
                                         m_isInitialized( false ),
                                         m_initialRequestForAllInvitesSent( false )
{
}

InvitationManager::~InvitationManager()
{
}

void     InvitationManager::Init()
{
   // load all invitations
   if( m_initialRequestForAllInvitesSent == false )
   {
      PacketDbQuery* dbQuery = new PacketDbQuery;
      dbQuery->id =           0;
      dbQuery->meta =         "";
      dbQuery->lookup =       QueryType_AllInvitations;
      dbQuery->serverLookup = m_dbIdentifier;
      dbQuery->query = "SELECT invitation.*, U1.user_name, U2.user_name FROM invitation "\
                        "INNER JOIN users AS U1 ON inviter_id = U1.uuid INNER JOIN users AS U2 ON invitee_id = U2.uuid";

      m_chatServer->AddQueryToOutput( dbQuery, 0 );
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
   m_chatServer->InvitationManagerNeedsUpdate();   
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
   m_chatServer->SendPacketToGateway( echo, connectionId );
   return true;
}


///////////////////////////////////////////////////////////////////

bool     InvitationManager::InviteUserToChatRoom( const PacketInvitation_InviteUser* invitationPacket, U32 connectionId )
{
   ChatUser* sender = m_chatServer->GetUserByConnectionId( connectionId );
   if( sender == NULL )
   {
      m_chatServer->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_UserUnknown );
      return false;
   }

   UsersChatRoomList userInChatRoom( invitationPacket->userUuid );
   if( m_chatRoomManager->GetUserInfo( invitationPacket->userUuid, userInChatRoom ) == false )
   {
      m_chatServer->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_UserUnknown );
      return false;
   }

   if( m_chatRoomManager->IsRoomValid( invitationPacket->inviteGroup ) == false )
   {
      m_chatServer->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_BadChatChannel );
      return false;
   }

   if( IsThereAlreadyAnInvitationToThisGroupInvolvingTheseTwoUsers( invitationPacket->userUuid, sender->GetUuid(), invitationPacket->inviteGroup ) )
   {
      PacketInvitation_InviteUserResponse* response = new PacketInvitation_InviteUserResponse;
      response->succeeded = false;
      response->newInvitationUuid = "";

      m_chatServer->SendPacketToGateway( response, connectionId );
      m_chatServer->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_Invitation_ExistingInvitationWithThatUser );
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
   invite.inviterUuid = sender->GetUuid();
   invite.message = invitationPacket->message;
   invite.type = invitationPacket->invitationType;//Invitation::InvitationType_ChatRoom;
   invite.inviteeName = userInChatRoom.userName;
   invite.inviterName = sender->GetUserName();

   InsertInvitationIntoDb( invite );
   AddInvitationToStorage( invite );

   // notify sender of the new invitation sent
   SendUserHisInvitations( sender );
   
   PacketInvitation_InviteUserResponse* response = new PacketInvitation_InviteUserResponse;
   response->succeeded = true;
   response->newInvitationUuid = invitationUuid;

   m_chatServer->SendPacketToGateway( response, connectionId );

   ChatUser* receiver = m_chatServer->GetUserByUuid( invitationPacket->userUuid ); // only used to send updated invitations
   // notify recipient of the new invitation received.
   if( receiver != NULL )
   {
      SendUserHisInvitations( receiver );
     /* PacketInvitation_InviteUserResponse* response = new PacketInvitation_InviteUserResponse;
      response->succeeded = true;
      response->newInvitationUuid = invitationUuid;
      m_chatServer->SendPacketToGateway( response, receiver->GetConnectionId() );*/
   }

   return true;
}

///////////////////////////////////////////////////////////////////

bool     InvitationManager::CancelInvitation( const PacketInvitation_CancelInvitation* invite, U32 connectionId  )
{
   m_chatServer->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_IncompleteFeature );
      return false;
/*
   stringhash lookup = GenerateUniqueHash( invitation->invitationUuid );
   InvitationMapIterator it = m_invitationMap.find( lookup );
   if( it == m_invitationMap.end() )
   {
      m_chatServer->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_Invitation_DoesNotExist );
      return false;
   }

   const Invitation& invite = it->second;

   ChatUser* sender = m_chatServer->GetUserByConnectionId( connectionId );
   if( sender == NULL )
   {
      m_chatServer->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_UserUnknown );
      return false;
   }
   if( invite.inviteeUuid != sender->GetUuid () )
   {
      m_chatServer->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_Invitation_DoesNotExist );// you are not allowed to accept this.
      return false;
   }

   // PacketInvitation_InvitationWasCancelled
   return true;*/
}

///////////////////////////////////////////////////////////////////

bool     InvitationManager::RejectInvitation( const PacketInvitation_RejectInvitation* invitation, U32 connectionId  )
{
   stringhash lookup = GenerateUniqueHash( invitation->invitationUuid );
   InvitationMapIterator it = m_invitationMap.find( lookup );
   if( it == m_invitationMap.end() )
   {
      m_chatServer->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_Invitation_DoesNotExist );
      return false;
   }

   const Invitation& invite = it->second;

   ChatUser* rejecter = m_chatServer->GetUserByConnectionId( connectionId );
   if( rejecter == NULL )
   {
      m_chatServer->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_UserUnknown );
      return false;
   }
   if( invite.inviteeUuid != rejecter->GetUuid () )
   {
      m_chatServer->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_Invitation_DoesNotExist );// you are not allowed to accept this.
      return false;
   }

   U32 invitationId = invite.invitationId;
   ChatUser* inviter = m_chatServer->GetUserByUuid( invite.inviterUuid ); // only used to send updated invitations
   m_invitationMap.erase( it );// must erase before sending the updated lists.

   PacketInvitation_RejectInvitationResponse* response = new PacketInvitation_RejectInvitationResponse;
   SendMessageToClient( response, connectionId );
   SendUserHisInvitations( rejecter );

   
   if( inviter != NULL )
   {
      SendUserHisInvitations( inviter );
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
      m_chatServer->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_Invitation_DoesNotExist );
      return false;
   }

   const Invitation& invite = it->second;

   ChatUser* sender = m_chatServer->GetUserByConnectionId( connectionId );
   if( sender == NULL )
   {
      m_chatServer->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_UserUnknown );
      return false;
   }
   if( invite.inviteeUuid != sender->GetUuid () )
   {
      m_chatServer->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_Invitation_DoesNotExist );// you are not allowed to accept this.
      return false;
   }

   if( m_chatRoomManager->UserAddsSelfToRoom( invite.groupUuid, invite.inviteeUuid ) == false )// bad chat room
   {
      m_chatServer->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_Invitation_DoesNotExist );
      return false;
   }

   U32 invitationId = invite.invitationId;
   ChatUser* inviter = m_chatServer->GetUserByUuid( invite.inviterUuid ); // only used to send updated invitations
   m_invitationMap.erase( it );// must erase before sending the updated lists.

   SendUserHisInvitations( sender );

   
   if( inviter != NULL )
   {
      SendUserHisInvitations( inviter );
   }

   DeleteInvitationFromDb( invitationId );   

   return true;
}

///////////////////////////////////////////////////////////////////

bool     InvitationManager::RequestListOfInvitations( const PacketInvitation_GetListOfInvitations* request, U32 connectionId )
{
   ChatUser* user = m_chatServer->GetUserByConnectionId( connectionId );
   if( user == NULL )
   {
      m_chatServer->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_UserUnknown );
      return false;
   }

   SendUserHisInvitations( user );

   return true;
}

///////////////////////////////////////////////////////////////////

 void    InvitationManager::SendUserHisInvitations( const ChatUser* user )
{
   PacketInvitation_GetListOfInvitationsResponse* response = new PacketInvitation_GetListOfInvitationsResponse;
   const string& userUuid = user->GetUuid();
   response->userUuid = userUuid;
   
   InvitationMapIterator it = m_invitationMap.begin();
   while( it != m_invitationMap.end() )
   {
      const Invitation& invite = it->second;
      if( invite.inviteeUuid == userUuid || invite.inviterUuid == userUuid )
      {
         response->invitationList.insert( invite.groupUuid, invite );
      }
     /* if( response->invitationList.size() >= 10 )
      {
         SendMessageToClient( response, connectionId );
         PacketInvitation_GetListOfInvitationsResponse* response = new PacketInvitation_GetListOfInvitationsResponse;
         response->userUuid = userUuid;
      }*/
      it++;
   }

   SendMessageToClient( response, user->GetConnectionId() );
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

bool           InvitationManager::IsInvitationAlreadyStored( const Invitation& invite )
{
   assert( 0 );// todo

   return false;
}

void           InvitationManager::AddInvitationToStorage( const Invitation& invite )
{
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

bool           InvitationManager::DoesUserHavePendingInvite( const string& inviterUuid, const string& inviteeUuid )
{
   /*InvitationMapIterator it = m_invitationMap.begin();
   while( it != m_invitationMap.end() )
   {
      const Invitation& invite = it->second;
      if( invite.
   }*/

   return false;
}

bool           InvitationManager::IsThereAlreadyAnInvitationToThisGroupInvolvingTheseTwoUsers( const string& user1, const string& user2, const string& groupUuid ) const 
{
   InvitationMapConstIterator it = m_invitationMap.begin();
   while( it != m_invitationMap.end() )
   {
      const Invitation& invite = it->second;
      if( invite.groupUuid == groupUuid ) // this seems like the best way to do a first pass test
      {
         if( invite.inviteeUuid == user1 || invite.inviterUuid == user1 ||
            invite.inviteeUuid == user2 || invite.inviterUuid == user2 )
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
  /* if( IsInvitationAlreadyStored( const Invitation& invite ) == true )
      return;*/

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

   m_chatServer->AddQueryToOutput( dbQuery, 0 );
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

   m_chatServer->AddQueryToOutput( dbQuery, 0 );
}

/*
void           InvitationManager::SendInvitationToSenderAndRecipient( const Invitation& invite )
{
}
*/
//---------------------------------------------------------------

bool     InvitationManager::SendMessageToClient( BasePacket* packet, U32 connectionId ) const
{
   PacketGatewayWrapper* wrapper = new PacketGatewayWrapper();
   wrapper->SetupPacket( packet, connectionId );

   m_chatServer->SendMessageToClient( wrapper, connectionId );
   return true;
}

//-

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
