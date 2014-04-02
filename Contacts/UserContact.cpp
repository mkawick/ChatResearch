// UserContact.cpp

#include <string>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
using boost::format;

#include "../NetworkCommon/Utils/CommandLineParser.h"
#include "../NetworkCommon/Packets/BasePacket.h"
#include "../NetworkCommon/Packets/LoginPacket.h"
#include "UserContact.h"
#include "DiplodocusContact.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////


UserContact::UserContact( const UserInfo& info, U32 connectionId ): 
               m_connectionId( connectionId ),
               m_userInfo( info ), 
               m_requiresUpdate( false ),
               m_isLoggedOut( false ),
               m_hasBeenInitialized( false ),
               m_friendListFilled( false ),
               m_friendRequestSentListFilled( false ),
               m_friendRequestReceivedListFilled( false ),
               m_contactServer( NULL ),
               m_invitationQueryIndex( 0 )
{
   m_timeLoggedOut = 0;
}

//------------------------------------------------------------------------------------------------

UserContact::~UserContact()
{
}

//------------------------------------------------------------------------------------------------


void  UserContact::Init() // send queries
{
   assert( m_contactServer );

   //m_userInfo.uuid;
  // m_userInfo.id;

   string idString = boost::lexical_cast< string >( m_userInfo.id );

   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =           m_connectionId;
   dbQuery->meta =         "";
   dbQuery->lookup =       QueryType_UserProfile;
   dbQuery->serverLookup = m_userInfo.id;
   dbQuery->query = "SELECT * FROM user_profile WHERE user_id='";
   dbQuery->query += idString;
   dbQuery->query += "'";

   m_contactServer->AddQueryToOutput( dbQuery );

   //-------------------------------

   InitContactsAndInvitations();
}

//------------------------------------------------------------------------------------------------

void  UserContact::InitContactsAndInvitations()
{
   PrepFriendQuery();
   PrepInvitationsQueries();
}

//------------------------------------------------------------------------------------------------

void  UserContact::PrepFriendQuery()
{
   string idString = boost::lexical_cast< string >( m_userInfo.id );

   m_friendListFilled = false;

   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery = new PacketDbQuery;
   dbQuery->id =           m_connectionId;
   dbQuery->meta =         "";
   dbQuery->lookup =       QueryType_Friends;
   dbQuery->serverLookup = m_userInfo.id;
   dbQuery->query = "SELECT * FROM users INNER JOIN friends ON users.user_id=friends.userid2 WHERE friends.userId1='";
   dbQuery->query += idString;
   dbQuery->query += "'";

   m_contactServer->AddQueryToOutput( dbQuery );
}

//------------------------------------------------------------------------------------------------

void  UserContact::PrepInvitationsQueries()
{
   string idString = boost::lexical_cast< string >( m_userInfo.id );

   m_friendRequestSentListFilled = false;
   m_friendRequestReceivedListFilled = false;

   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =           m_connectionId;
   dbQuery->meta =         "";
   dbQuery->lookup =       QueryType_FriendRequestsSent;
   dbQuery->serverLookup = m_userInfo.id;
   dbQuery->query = "SELECT * FROM users INNER JOIN friend_pending ON users.user_id=friend_pending.invitee_id WHERE friend_pending.inviter_id='";
   dbQuery->query += idString;
   dbQuery->query += "'";

   m_contactServer->AddQueryToOutput( dbQuery );

   //-------------------------------

   dbQuery = new PacketDbQuery;
   dbQuery->id =           m_connectionId;
   dbQuery->meta =         "";
   dbQuery->lookup =       QueryType_FriendRequestReceived;
   dbQuery->serverLookup = m_userInfo.id;
   dbQuery->query = "SELECT * FROM users INNER JOIN friend_pending ON users.user_id=friend_pending.inviter_id WHERE friend_pending.invitee_id='";
   dbQuery->query += idString;
   dbQuery->query += "'";

   m_contactServer->AddQueryToOutput( dbQuery );
}

//------------------------------------------------------------------------------------------------

void  UserContact::Update()
{
   if( m_isLoggedOut == true )
      return;

   if( m_hasBeenInitialized == false )
   {
      Init();
      m_hasBeenInitialized = true;
      return;
   }
   if( m_requiresUpdate == false )
   {
      return;
   }
   m_requiresUpdate =  false;
}


bool  UserContact::UpdateProfile( const PacketUserUpdateProfile* profile )
{
   // update the real profile
   if( m_displayOnlineStatusToOtherUsers != profile->displayOnlineStatusToOtherUsers )
   {
      m_displayOnlineStatusToOtherUsers = profile->displayOnlineStatusToOtherUsers;
      InformFriendsOfOnlineStatus( true );
   }

   // to be done.. what do I do here?
   if( m_blockContactInvitations != profile->blockContactInvitations )
   {
      m_blockContactInvitations = profile->blockContactInvitations;
   }
   if( m_blockGroupInvitations != profile->blockGroupInvitations )
   {
      m_blockGroupInvitations = profile->blockGroupInvitations;
   }
   ;
   return true;
}

//------------------------------------------------------------------------------------------------

void  UserContact::UserLoggedOut() 
{ 
   m_isLoggedOut = true; 
   time( &m_timeLoggedOut );
   InformFriendsOfOnlineStatus( false );
}

int   UserContact::SecondsExpiredSinceLoggedOut()
{
   if( m_timeLoggedOut == 0 )
      return 0;

   time_t currentTime;
   time( &currentTime );

   return  static_cast<int> ( difftime( currentTime, m_timeLoggedOut ) );
}

//------------------------------------------------------------------------------------------------

bool  UserContact::HandleDbQueryResult( const PacketDbQueryResult* dbResult )
{
   switch( dbResult->lookup )
   {
   case QueryType_UserProfile:
      {
         UserProfileTable            enigma( dbResult->bucket );
         UserProfileTable::iterator  it = enigma.begin();
         if( it != enigma.end() )
         {
            UserProfileTable::row       row = *it++;
            m_displayOnlineStatusToOtherUsers =    boost::lexical_cast< bool >( row[ TableUserProfile::Column_display_online_status_to_other_users ] );
            m_blockContactInvitations =            boost::lexical_cast< bool >( row[ TableUserProfile::Column_block_contact_invitations ] );
            m_blockGroupInvitations =              boost::lexical_cast< bool >( row[ TableUserProfile::Column_block_group_invitations ] );
         }
      }
      return true;
   case QueryType_Friends:
      {
         m_friends.clear();
         UserTable            enigma( dbResult->bucket );
         UserTable::iterator  it = enigma.begin();
         while( it != enigma.end() )
         {
            UserTable::row row = *it++;
            UserInfo ui;

            ui.id =               boost::lexical_cast< U32 >( row[ TableUser::Column_id ] );
            ui.userName =         row[ TableUser::Column_name ];
            ui.uuid =             row[ TableUser::Column_uuid ];
            ui.email =            row[ TableUser::Column_email ];
            ui.passwordHash =     row[ TableUser::Column_password_hash ];
            ui.connectionId = 0;
            string active = row[ TableUser::Column_active];
            if( active == "NULL" )
               ui.active = false;
            else
               ui.active =           boost::lexical_cast< bool >( active );
            // note that we are using logout for our last login time.
            m_friends.push_back( ui );
         }

         m_friendListFilled = true;
         FinishLoginBySendingUserFriendsAndInvitations();
      }
      return true;
   case QueryType_FriendRequestsSent:
      {
         m_invitationsOut.clear();
         UserJoinPendingTable            enigma( dbResult->bucket );
         UserJoinPendingTable::iterator  it = enigma.begin();
         while( it != enigma.end() )
         {
            UserJoinPendingTable::row row = *it++;
            Invitation invite;
            invite.invitationNumber =  boost::lexical_cast< U32 >( row[ TableUserJoinPending::Column_pending_id ] );
            invite.inviteeId =         boost::lexical_cast< U32 >( row[ TableUserJoinPending::Column_invitee_id ] );
            invite.inviterId =         boost::lexical_cast< U32 >( row[ TableUserJoinPending::Column_inviter_id ] );

            string notified = row[ TableUserJoinPending::Column_was_notified ];
            if( notified == "NULL" )
               invite.wasNotified = false;
            else 
               invite.wasNotified =       boost::lexical_cast< bool >( notified );

            invite.userName =          row[ TableUserJoinPending::Column_name ];
            invite.userUuid =          row[ TableUserJoinPending::Column_uuid ];
            invite.message  =          row[ TableUserJoinPending::Column_message ];
            invite.uuid  =             row[ TableUserJoinPending::Column_pending_uuid ];
            invite.date  =             row[ TableUserJoinPending::Column_sent_date ];
            m_invitationsOut.push_back( invite );
         }

         m_friendRequestSentListFilled = true;
         FinishLoginBySendingUserFriendsAndInvitations();
      }
      return true;
   case QueryType_FriendRequestReceived:
      {
         m_invitationsIn.clear();
         UserJoinPendingTable            enigma( dbResult->bucket );
         UserJoinPendingTable::iterator  it = enigma.begin();
         while( it != enigma.end() )
         {
            UserJoinPendingTable::row row = *it++;
            Invitation invite;
            invite.invitationNumber =  boost::lexical_cast< U32 >( row[ TableUserJoinPending::Column_pending_id ] );
            invite.inviteeId =         boost::lexical_cast< U32 >( row[ TableUserJoinPending::Column_invitee_id ] );
            invite.inviterId =         boost::lexical_cast< U32 >( row[ TableUserJoinPending::Column_inviter_id ] );

            string notified = row[ TableUserJoinPending::Column_was_notified ];
            if( notified == "NULL" )
               invite.wasNotified = false;
            else 
               invite.wasNotified =       boost::lexical_cast< bool >( notified );
            //invite.wasNotified =       boost::lexical_cast< bool >( row[ TableUserJoinPending::Column_was_notified ] );

            invite.userName =          row[ TableUserJoinPending::Column_name ];
            invite.userUuid =          row[ TableUserJoinPending::Column_uuid ];
            invite.message  =          row[ TableUserJoinPending::Column_message ];
            invite.uuid  =             row[ TableUserJoinPending::Column_pending_uuid ];
            invite.date  =             row[ TableUserJoinPending::Column_sent_date ];
            m_invitationsIn.push_back( invite );
         }

         m_friendRequestReceivedListFilled = true;
         FinishLoginBySendingUserFriendsAndInvitations();
      }
      return true;
   case QueryType_GetInviteeDetails:
      {
         if( dbResult->successfulQuery == false || dbResult->bucket.bucket.size() == 0 )
         {
            m_contactServer->SendErrorToClient( m_connectionId, PacketErrorReport::ErrorType_Contact_Invitation_ProblemFindingUser );
            return true;
         }

         if( dbResult->bucket.bucket.size() > 1 )
         {
            assert( 0 );
         }

         int lookupIndex = dbResult->serverLookup;
         list< InvitationQueryLookup >::iterator it = m_invitationQueryLookup.begin();
         while( it != m_invitationQueryLookup.end() )
         {
            if( it->index == lookupIndex )
            {
               UserTable            enigma( dbResult->bucket );
               UserTable::row  row = *enigma.begin();
               U32 id = boost::lexical_cast< U32 >( row[ TableUser::Column_id ] );
               if( id > 0 )
               {
                  FinishInvitation( id, it->message );
               }
               m_invitationQueryLookup.erase( it );
               break;
            }
            else
            {
               it++;
            }
         }
         GetListOfInvitationsReceived();
         GetListOfInvitationsSent();
      }
      return true;
   case QueryType_AddInvitationToUser:
      {
         InitContactsAndInvitations();
      }
      return true;
   case QueryType_GetInvitationPriorToAcceptance:
      {
         if( dbResult->successfulQuery == false || dbResult->bucket.bucket.size() == 0 )
         {
            m_contactServer->SendErrorToClient( m_connectionId, PacketErrorReport::ErrorType_Contact_Invitation_BadInvitation );
            return true;
         }

         FinishAcceptingInvitation( dbResult );
      }
      return true;
   case QueryType_GetInvitationPriorToDeclination:
      {
         if( dbResult->successfulQuery == false || dbResult->bucket.bucket.size() == 0 )
         {
            m_contactServer->SendErrorToClient( m_connectionId, PacketErrorReport::ErrorType_Contact_Invitation_BadInvitation );
            return true;
         }
         FinishDecliningingInvitation( dbResult );
      }
      return true;
   case QueryType_SearchForUser:
      {
         FinishSearchResult( dbResult );
      }
      return true;
   case QueryType_InsertNewFriend:
      {
         InitContactsAndInvitations();// when we finish this, reset the user's contacts.

         UserContact* contact = m_contactServer->GetUserByUuid( dbResult->meta );
         if( contact )
         {
            contact->InitContactsAndInvitations();
         }
      }
      return true;
   }
   return false;
}

//------------------------------------------------------------------------------------------------

bool     UserContact::HandleRequestFromClient( const PacketContact* packet )
{
   switch( packet->packetSubType )
   {
   case PacketContact::ContactType_GetListOfContacts:
      return GetListOfContacts();

   case PacketContact::ContactType_GetListOfInvitations:
      return GetListOfInvitationsReceived();

   case PacketContact::ContactType_GetListOfInvitationsSent:
      return GetListOfInvitationsSent();

   case PacketContact::ContactType_InviteContact:
      return InviteUser( static_cast< const PacketContact_InviteContact* >( packet ) );
      
   case PacketContact::ContactType_AcceptInvite:
      return AcceptInvitation( static_cast< const PacketContact_AcceptInvite* >( packet ) );

   case PacketContact::ContactType_DeclineInvitation:
      return DeclineInvitation( static_cast< const PacketContact_DeclineInvitation* >( packet ) );

   case PacketContact::ContactType_RemoveInvitation:
      return RemoveSentInvitation( static_cast< const PacketContact_RemoveInvitation* >( packet ) );      

   case PacketContact::ContactType_Search:
      return PerformSearch( static_cast< const PacketContact_SearchForUser* >( packet ) );

   case PacketContact::ContactType_RemoveContact:
      return RemoveContact( static_cast< const PacketContact_ContactRemove* >( packet ) );

   case PacketContact::ContactType_EchoToServer:
      return EchoHandler();
   }

   return false;
}

//------------------------------------------------------------------------------------------------

bool     UserContact::InformFriendsOfOnlineStatus( bool isOnline )
{
   if( m_friends.size() == 0 )
      return false;

   if( m_displayOnlineStatusToOtherUsers == false )
      isOnline = false;

   vector< UserInfo >::iterator  it = m_friends.begin();
   while ( it != m_friends.end() )
   {
      const UserInfo& ui = *it++; 
      UserContact* contact = m_contactServer->GetUser( ui.id );
      if( contact )
      {
         contact->YourFriendsOnlineStatusChange( m_connectionId, m_userInfo.userName, m_userInfo.uuid, isOnline );
         YourFriendsOnlineStatusChange( ui.connectionId, ui.userName, ui.uuid, isOnline );
      }
   }


   return true;
}

//------------------------------------------------------------------------------------------------

void     UserContact::FinishLoginBySendingUserFriendsAndInvitations()
{
   if( m_friendListFilled == true && m_friendRequestSentListFilled == true && m_friendRequestReceivedListFilled == true )
   {
      InformFriendsOfOnlineStatus( true );
      //GetListOfContacts();
      //GetListOfInvitationsReceived();
      //GetListOfInvitationsSent();
   }
}

//------------------------------------------------------------------------------------------------

bool     UserContact::YourFriendsOnlineStatusChange( U32 connectionId, const string& userName, const string& UUID, bool isOnline )
{
   if( m_friends.size() == 0 )
      return false;

   PacketContact_FriendOnlineStatusChange* packet = new PacketContact_FriendOnlineStatusChange;
   packet->friendInfo.userName = userName;
   packet->uuid = UUID;
   packet->friendInfo.isOnline = isOnline;
   // PacketContact_FriendOnlineStatusChange
    m_contactServer->SendPacketToGateway( packet, m_connectionId );

    return true;
}

//------------------------------------------------------------------------------------------------

bool     UserContact::GetListOfContacts()
{
   PacketContact_GetListOfContactsResponse* packet = new PacketContact_GetListOfContactsResponse;

   vector< UserInfo >::iterator  it = m_friends.begin();
   while ( it != m_friends.end() )
   {
      const UserInfo& ui = *it++; 

      const UserContact* testUser = m_contactServer->GetUser( ui.id );
      bool isLoggedIn = testUser != NULL;
      packet->friends.insert( ui.uuid, FriendInfo( ui.userName, ui.avatarId, isLoggedIn ) );
   }
   
   m_contactServer->SendPacketToGateway( packet, m_connectionId );
   
   return true;
}

//------------------------------------------------------------------------------------------------

bool  UserContact::GetListOfInvitationsReceived()
{
   PacketContact_GetListOfInvitationsResponse* packet = new PacketContact_GetListOfInvitationsResponse;
   
   vector< Invitation >::iterator  it = m_invitationsIn.begin();
   while ( it != m_invitationsIn.end() )
   {
      const Invitation& invite = *it++; 
      InvitationInfo ii;
      ii.message = invite.message;
      ii.inviterName = invite.userName;// note who is who here
      ii.inviteeName = m_userInfo.userName;
      ii.uuid = invite.uuid;
      ii.date = invite.date;

      packet->invitations.insert( invite.uuid, ii );
   }
   
   m_contactServer->SendPacketToGateway( packet, m_connectionId );

   return true;
}

//------------------------------------------------------------------------------------------------

bool  UserContact::GetListOfInvitationsSent()
{
   PacketContact_GetListOfInvitationsSentResponse* packet = new PacketContact_GetListOfInvitationsSentResponse;
   
   vector< Invitation >::iterator  it = m_invitationsOut.begin();
   while ( it != m_invitationsOut.end() )
   {
      const Invitation& invite = *it++; 
      InvitationInfo ii;
      ii.message = invite.message;
      ii.inviterName = m_userInfo.userName;// note who is who here
      ii.inviteeName = invite.userName;
      ii.uuid = invite.uuid;
      ii.date = invite.date;

      packet->invitations.insert( invite.uuid, ii );
   }
   
   m_contactServer->SendPacketToGateway( packet, m_connectionId );

   return true;
}

//------------------------------------------------------------------------------------------------

bool     UserContact::DoesPendingInvitationExist( const string& inviteeUuid, const string& inviteeName )
{
   // pending list
   list< InvitationQueryLookup >::iterator inviteIt = m_invitationQueryLookup.begin();
   while( inviteIt != m_invitationQueryLookup.end() )
   {
      const InvitationQueryLookup& inviteTest = *inviteIt++;
      if( inviteeUuid.size() && inviteeUuid == inviteTest.inviteeUuid )
      {
         return true;
      }
      if( inviteeName.size() && inviteeName == inviteTest.inviteeName )
      {
         return true;
      }
   }

   vector< Invitation >::iterator it = m_invitationsOut.begin();
   if( inviteeUuid.size() )
   {
      // we have all of your current invitations in memory... verify that we don't already have a pending invitation
      while( it != m_invitationsOut.end() )
      {
         const Invitation& invite = *it++;
         if( invite.userUuid == inviteeUuid )
         {
            return true;
         }
      }
   }
   else
   {
      while( it != m_invitationsOut.end() )
      {
         const Invitation& invite = *it++;
         if( invite.userName == inviteeName )
         {
            return true;
         }
      }
   }

   return false;
}

//------------------------------------------------------------------------------------------------

bool  UserContact::HaveIAlreadyBeenInvited( const string& userUuid )
{
   vector< Invitation >::iterator it = m_invitationsIn.begin();
   while( it != m_invitationsIn.end() )
   {
      const Invitation& invite = *it++;
      if( invite.userUuid == userUuid )
      {
         return true;
      }
   }
   return false;
}

//------------------------------------------------------------------------------------------------

void  UserContact::RemoveInvitationReceived( const string& invitationUuid )
{
   vector< Invitation >::iterator  it = m_invitationsIn.begin();
   while ( it != m_invitationsIn.end() )
   {
      const Invitation& invite = *it; 
      if( invite.uuid == invitationUuid )
      {
         m_invitationsIn.erase( it );
         break;
      }
      it++;
   }
}

//------------------------------------------------------------------------------------------------

bool     UserContact::InviteUser( const PacketContact_InviteContact* packet )
{
   const string& inviteeUuid = packet->uuid;
   const string& inviteeName = packet->userName;
   const string& message = packet->message;
   

   // major problem here... they may already be friends
   vector< UserInfo >::const_iterator it = m_friends.begin();
   while( it != m_friends.end() )
   {
      const UserInfo& fr = *it++;
      if( fr.userName == inviteeName || fr.uuid == inviteeUuid )
      {
         m_contactServer->SendErrorToClient( m_connectionId, PacketErrorReport::ErrorType_Contact_Invitation_InviteeAlreadyFriend );
         return false;
      }
   }

   if( inviteeUuid == m_userInfo.uuid )
   {
       m_contactServer->SendErrorToClient( m_connectionId, PacketErrorReport::ErrorType_Contact_Invitation_CannotInviteSelf );
       return false;
   }

   if( inviteeUuid.size() == 0 && inviteeName.size() == 0 )
   {
       m_contactServer->SendErrorToClient( m_connectionId, PacketErrorReport::ErrorType_Contact_Invitation_InvalidUser );
       return false;
   }

   // we should validate that we do not already have a pending invitation
   
   bool alreadyExists = DoesPendingInvitationExist( inviteeUuid, inviteeName );
   if( alreadyExists )
   {
      m_contactServer->SendErrorToClient( m_connectionId, PacketErrorReport::ErrorType_Contact_Invitation_InviteeAlreadyInvited );
      return false;
   }

   bool  didHeInviteMe = HaveIAlreadyBeenInvited( inviteeUuid );
   if( didHeInviteMe == true )
   {
      m_contactServer->SendErrorToClient( m_connectionId, PacketErrorReport::ErrorType_Contact_Invitation_InviteeSentYouInvitation );
      return false;
   }

   UserContact* contact;
   if( inviteeUuid.size() )
   {
      contact = m_contactServer->GetUserByUuid( inviteeUuid );// this user needs to know that he's been invited.
   }
   else 
   {
      contact = m_contactServer->GetUserByUsername( inviteeName );
   }
   if( contact )
   {
      if( contact->IsBlockingFriendInvites() == true )
      {
         m_contactServer->SendErrorToClient( m_connectionId, PacketErrorReport::ErrorType_UserIsBlockingFriendInvites );
         return false;
      }
      else
      {
         // we still need to write this to the db
         FinishInvitation( contact->GetUserInfo().id, message, contact );
      }
   }
   else
   {
      // we need to store something by which we can find this query to pair up the user results.
      int lookup = m_invitationQueryIndex++;
      PacketDbQuery* dbQuery = new PacketDbQuery;
      dbQuery->id =           m_connectionId;
      dbQuery->meta =         message;
      dbQuery->lookup =       QueryType_GetInviteeDetails;
      dbQuery->serverLookup = lookup;

      dbQuery->query = "SELECT * FROM users WHERE ";
      if( inviteeUuid.size() )
      {
         dbQuery->query += "uuid='%s'";
         dbQuery->escapedStrings.insert( inviteeUuid );
      }
      else
      {
         dbQuery->query += "user_name='%s'";
         dbQuery->escapedStrings.insert( inviteeName );
      }
      m_contactServer->AddQueryToOutput( dbQuery );
      m_contactServer->TrackInviteSent();// stat

      //m_invitationsPendingUserLookup.push_back( *packet );

      InvitationQueryLookup queryLookup;
      queryLookup.index = lookup;
      queryLookup.inviteeUuid = inviteeUuid;
      queryLookup.inviteeName = inviteeName;
      queryLookup.message = message;
      m_invitationQueryLookup.push_back( queryLookup );
   }

   return true;
}

//------------------------------------------------------------------------------------------------

bool  UserContact::AcceptInvitation( const PacketContact_AcceptInvite* packet )// we are the invitee
{
   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =           m_connectionId;
   dbQuery->meta =         "";
   dbQuery->lookup =       QueryType_GetInvitationPriorToAcceptance;
   dbQuery->serverLookup = m_userInfo.id;

   /*string query = "SELECT * FROM friend_pending WHERE invitee_id='";
   query += boost::lexical_cast< string >( m_userInfo.id );
   query += "' AND uuid='%s'";*/

   dbQuery->query = "SELECT * FROM users INNER JOIN friend_pending ON users.user_id=friend_pending.inviter_id WHERE friend_pending.uuid='%s'";

   dbQuery->escapedStrings.insert( packet->invitationUuid );

   m_contactServer->AddQueryToOutput( dbQuery );

   return true;
}

//------------------------------------------------------------------------------------------------

bool  UserContact::DeclineInvitation( const PacketContact_DeclineInvitation* packet )
{
   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =           m_connectionId;
   dbQuery->meta =         packet->message;
   dbQuery->lookup =       QueryType_GetInvitationPriorToDeclination;
   dbQuery->serverLookup = m_userInfo.id;

   dbQuery->query = "SELECT * FROM users INNER JOIN friend_pending ON users.user_id=friend_pending.inviter_id WHERE friend_pending.uuid='%s'";

   dbQuery->escapedStrings.insert( packet->invitationUuid );

   m_contactServer->AddQueryToOutput( dbQuery );
   return true;
}

//------------------------------------------------------------------------------------------------

bool  UserContact::RemoveSentInvitation( const PacketContact_RemoveInvitation* packet )
{
   bool found = false;
   const string& invitationUuid = packet->invitationUuid;

   // verify that the sent invite is valid
   vector< Invitation >::iterator  it = m_invitationsOut.begin();
   while ( it != m_invitationsOut.end() )
   {
      const Invitation& invite = *it; 
      if( invite.uuid == invitationUuid )
      {
         found = true;
         break;
      }
      it++;
   }

   if( found == false )
   {
      m_contactServer->SendErrorToClient( m_connectionId, PacketErrorReport::ErrorType_Contact_Invitation_BadInvitation );
      return false;
   }

   // 'it' now contains the pointer to the right item
   U32 invitationNumber = it->invitationNumber;
   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =           m_connectionId;
   dbQuery->meta =         invitationUuid;
   dbQuery->lookup =       QueryType_DeleteInvitation;
   dbQuery->serverLookup = m_userInfo.id;
   dbQuery->isFireAndForget = true;

   string query = "DELETE FROM friend_pending WHERE id=";
   query += boost::lexical_cast< string >( invitationNumber );
   dbQuery->query = query;
   m_contactServer->AddQueryToOutput( dbQuery );

   // resend invites to invitee
   UserContact* invitee = m_contactServer->GetUser( it->inviteeId );
   if( invitee )
   {
      invitee->RemoveInvitationReceived( invitationUuid );
      invitee->GetListOfInvitationsReceived();
   }

   m_invitationsOut.erase( it );// remove this entry
   GetListOfInvitationsSent();
   return true;
}


//------------------------------------------------------------------------------------------------

bool  UserContact::PerformSearch( const PacketContact_SearchForUser* packet )
{
   cout << "Search packet received: " << packet->searchString << endl;
   cout << "num requested: " << packet->limit << endl;
   cout << "offset: " << packet->offset << endl;
   if( packet->searchString.size() < 2 )
   {
      // do not add any contacts
      PacketContact_SearchForUserResult* response = new PacketContact_SearchForUserResult;
      m_contactServer->SendPacketToGateway( response, m_connectionId );

      m_contactServer->SendErrorToClient( m_connectionId, PacketErrorReport::ErrorType_Contact_BadSearchString );
      return false;
   }

   if( packet->limit > 25 )
   {
      // do not add any contacts
      PacketContact_SearchForUserResult* response = new PacketContact_SearchForUserResult;
      m_contactServer->SendPacketToGateway( response, m_connectionId );

      m_contactServer->SendErrorToClient( m_connectionId, PacketErrorReport::ErrorType_Contact_SearchRequestHasTooMany );
      return false;
   }

   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =           m_connectionId;
   dbQuery->meta =         packet->searchString;
   dbQuery->lookup =       QueryType_SearchForUser;
   dbQuery->serverLookup = m_userInfo.id;

   dbQuery->query = "SELECT user_name, uuid, user_id FROM users WHERE user_name LIKE '%%s%' ORDER BY user_id LIMIT ";
   int limit = 25;// always limit the numebr of searched items
   if( packet->limit )
   {
      limit = packet->limit;
   }

   dbQuery->query += boost::lexical_cast< string >( limit );
   if( packet->offset )
   {
      dbQuery->query += " OFFSET ";
      dbQuery->query += boost::lexical_cast< string >( packet->offset );
   }

   dbQuery->escapedStrings.insert( packet->searchString );

   m_contactServer->AddQueryToOutput( dbQuery );
   m_contactServer->TrackNumSearches();

   return true;
}

//------------------------------------------------------------------------------------------------

bool  UserContact::RemoveContact( const PacketContact_ContactRemove* packet )
{
   const string& contactUuid = packet->contactUuid;
   const string& message = packet->message;

   bool found = false;
    // major problem here... they may not be friends
   vector< UserInfo >::const_iterator it = m_friends.begin();
   while( it != m_friends.end() )
   {
      const UserInfo& fr = *it;
      if( fr.uuid == contactUuid )
      {
         found = true;
         break;
      }
      it++;
   }

   if( found == false )
   {
      m_contactServer->SendErrorToClient( m_connectionId, PacketErrorReport::ErrorType_Contact_NotAUserContact );
      return false;
   }

   
   string query = "DELETE FROM friends WHERE userid1='";
   query += boost::lexical_cast< string >( it->id );
   query += "' AND userid2='";
   query += boost::lexical_cast< string >( m_userInfo.id );
   query += "'";

   PacketDbQuery* dbQuery = new PacketDbQuery;   
   dbQuery->query =        query;
   dbQuery->id =           m_connectionId;
   dbQuery->meta =         "";
   dbQuery->lookup =       QueryType_DeleteFriend;
   dbQuery->serverLookup = m_userInfo.id;
   dbQuery->isFireAndForget = true;
   m_contactServer->AddQueryToOutput( dbQuery );
     
   query = "DELETE FROM friends WHERE userid2='";
   query += boost::lexical_cast< string >( it->id );
   query += "' AND userid1='";
   query += boost::lexical_cast< string >( m_userInfo.id );
   query += "'";

   dbQuery = new PacketDbQuery; 
   dbQuery->query =        query;
   dbQuery->id =           m_connectionId;
   dbQuery->meta =         "";
   dbQuery->lookup =       QueryType_DeleteFriend;
   dbQuery->serverLookup = m_userInfo.id;
   dbQuery->isFireAndForget = true;
   m_contactServer->AddQueryToOutput( dbQuery );

   // send notification to both people if online.
   UserContact* contact = m_contactServer->GetUserByUuid( contactUuid );
   if( contact )
   {
      contact->PrepFriendQuery();
   }

   PrepFriendQuery();

   return true;
}

//------------------------------------------------------------------------------------------------

bool  UserContact::EchoHandler()
{
   cout << " Echo " << endl;
   PacketContact_EchoToClient* echo = new PacketContact_EchoToClient;
   m_contactServer->SendPacketToGateway( echo, m_connectionId );
   return true;
}

//------------------------------------------------------------------------------------------------

void  UserContact::FinishAcceptingInvitation( const PacketDbQueryResult* dbResult )
{
   U32 invitationId = 0;
   U32 inviterId = 0;
   U32 inviteeId = 0;
   string inviterUsername;
   string inviterUuid;
   UserJoinPendingTable            enigma( dbResult->bucket );
   UserJoinPendingTable::iterator  it = enigma.begin();
   if( it != enigma.end() )
   {
      UserJoinPendingTable::row row = *it;
      invitationId = boost::lexical_cast< U32 >( row[ TableUserJoinPending::Column_pending_id ] );
      inviterId = boost::lexical_cast< U32 >( row[ TableUserJoinPending::Column_inviter_id ] );
      inviteeId = boost::lexical_cast< U32 >( row[ TableUserJoinPending::Column_invitee_id ] );
      inviterUuid = row[ TableUserJoinPending::Column_uuid ];
      inviterUsername = row[ TableUserJoinPending::Column_name ];
   }

   assert( invitationId != 0 && inviterId != 0 && inviteeId != 0 );

   // delete invitation
   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =           m_connectionId;
   dbQuery->meta =         "";
   dbQuery->lookup =       QueryType_DeleteInvitation;
   dbQuery->serverLookup = m_userInfo.id;
   dbQuery->isFireAndForget = true;

   string query = "DELETE FROM friend_pending WHERE id=";
   query += boost::lexical_cast< string >( invitationId );
   dbQuery->query = query;
   m_contactServer->AddQueryToOutput( dbQuery );

   // insert friend entry
   dbQuery = new PacketDbQuery;
   dbQuery->id =           m_connectionId;
   dbQuery->meta =         "";
   dbQuery->lookup =       QueryType_InsertNewFriend;
   dbQuery->serverLookup = m_userInfo.id;
   dbQuery->isFireAndForget = true;

   // both directions at once
   query = "INSERT INTO friends (userid1,userid2) VALUES (";
   query += boost::lexical_cast< string >( inviterId );
   query += ",";
   query += boost::lexical_cast< string >( inviteeId );
   query += "); ";

   dbQuery->query = query;
   m_contactServer->AddQueryToOutput( dbQuery );

   // the following is tricky. Once this returns, I need to tell both the accepter and the inviter to reload their contacts.
   dbQuery = new PacketDbQuery;
   dbQuery->id =           m_connectionId;
   dbQuery->meta =         inviterUuid; // << store off some lookup info
   dbQuery->lookup =       QueryType_InsertNewFriend;
   dbQuery->serverLookup = m_userInfo.id;
   dbQuery->isFireAndForget = false;

   query = "INSERT INTO friends (userid1,userid2) VALUES (";
   query += boost::lexical_cast< string >( inviteeId );
   query += ",";
   query += boost::lexical_cast< string >( inviterId );
   query += ");";

   dbQuery->query = query;
   m_contactServer->AddQueryToOutput( dbQuery );

   // send notification to both people if online.
   UserContact* contact = m_contactServer->GetUserByUuid( inviterUuid );// this user needs to know that he's been invited.
   if( contact )
   {
      contact->InvitationAccepted( inviterUsername, m_userInfo.userName, "", true );// this seems backwards, but its not
   }
   InvitationAccepted( inviterUsername, m_userInfo.userName, "", true );
}

//------------------------------------------------------------------------------------------------

void  UserContact::FinishDecliningingInvitation( const PacketDbQueryResult* dbResult )
{
   U32 invitationId = 0;
   U32 inviterId = 0;
   U32 inviteeId = 0;
   string inviterUsername;
   string inviterUuid;
   UserJoinPendingTable            enigma( dbResult->bucket );
   UserJoinPendingTable::iterator  it = enigma.begin();
   if( it != enigma.end() )
   {
      UserJoinPendingTable::row row = *it;
      invitationId = boost::lexical_cast< U32 >( row[ TableUserJoinPending::Column_pending_id ] );
      inviterId = boost::lexical_cast< U32 >( row[ TableUserJoinPending::Column_inviter_id ] );
      inviteeId = boost::lexical_cast< U32 >( row[ TableUserJoinPending::Column_invitee_id ] );
      inviterUuid = row[ TableUserJoinPending::Column_uuid ];
      inviterUsername = row[ TableUserJoinPending::Column_name ];
   }

   assert( invitationId != 0 && inviterId != 0 && inviteeId != 0 );

   string userMessage = dbResult->meta;

   // delete invitation
   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =           m_connectionId;
   dbQuery->meta =         "";
   dbQuery->lookup =       QueryType_DeleteInvitation;
   dbQuery->serverLookup = m_userInfo.id;
   dbQuery->isFireAndForget = true;

   string query = "DELETE FROM friend_pending WHERE id=";
   query += boost::lexical_cast< string >( invitationId );
   dbQuery->query = query;
   m_contactServer->AddQueryToOutput( dbQuery );

   // do not notify sender for now
   InvitationAccepted( inviterUsername, m_userInfo.userName, userMessage, false );
   PrepInvitationsQueries();
}

//------------------------------------------------------------------------------------------------

void  UserContact::FinishSearchResult( const PacketDbQueryResult* dbResult )
{
   PacketContact_SearchForUserResult* response = new PacketContact_SearchForUserResult;
   if( dbResult->successfulQuery == false || dbResult->bucket.bucket.size() == 0 )
   {
      m_contactServer->SendPacketToGateway( response, m_connectionId );
      m_contactServer->SendErrorToClient( m_connectionId, PacketErrorReport::ErrorType_Contact_NoSearchResult );
      return;
   }

   SerializedKeyValueVector< FriendInfo > & foundList = response->found;
   SimpleUserTable            enigma( dbResult->bucket );
   SimpleUserTable::iterator  it = enigma.begin();
   while( it != enigma.end() )
   {
      SimpleUserTable::row row = *it++;
      const string& userName = row[ TableSimpleUser::Column_name ];
      const string& uuid = row[ TableSimpleUser::Column_uuid ];
      U32 userId = boost::lexical_cast< U32 >( row[ TableSimpleUser::Column_id ] );
      foundList.insert( uuid, FriendInfo( userName, 0, false ) );// we never inform users if someone is online during a search.. privacy and spamming reasons
   }

   m_contactServer->SendPacketToGateway( response, m_connectionId );
}

//------------------------------------------------------------------------------------------------

void  UserContact::FinishInvitation( U32 inviteeId, const string& message, UserContact* contact )
{
   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =           m_connectionId;
   dbQuery->meta =         "";
   dbQuery->lookup =       QueryType_AddInvitationToUser;
   dbQuery->serverLookup = m_userInfo.id;
   
   time_t currentTime;
   time( &currentTime );
   string currentTimeInUTC = GetDateInUTC();
   string invitationUuid = GenerateUUID( static_cast< U32 >( currentTime ) );

   string query = "INSERT INTO friend_pending (inviter_id, invitee_id, was_notified, sent_date, message, uuid) VALUES ( '";
   query += boost::lexical_cast< string >( m_userInfo.id );
   query += "', '";
   query += boost::lexical_cast< string >( inviteeId );
   query += "', 1, '";
   query += currentTimeInUTC;
   query += "', '%s', '";
   query += invitationUuid;
   query += "')";

   dbQuery->query = query;
   dbQuery->escapedStrings.insert( message );

   m_contactServer->AddQueryToOutput( dbQuery );

   if( contact )
   {
      contact->YouHaveBeenInvitedToBeAFriend( m_userInfo.userName, invitationUuid, message, currentTimeInUTC );
   }
}

//------------------------------------------------------------------------------------------------

void     UserContact::YouHaveBeenInvitedToBeAFriend( const string& userName, const string& uuid, const string& message, const string& currentTime )
{
   PacketContact_InviteSentNotification* packet = new PacketContact_InviteSentNotification;
   packet->info.date = currentTime;
   packet->info.inviterName = userName;
   packet->info.uuid = uuid;
   packet->info.message = message;

   m_contactServer->SendPacketToGateway( packet, m_connectionId );

   InitContactsAndInvitations();

}

//------------------------------------------------------------------------------------------------

void     UserContact::InvitationAccepted( const string& sentFromuserName, const string& sentToUserName, const string& message, bool accepted )
{
   PacketContact_InvitationAccepted* packet = new PacketContact_InvitationAccepted;
   packet->wasAccepted = accepted;
   packet->fromUsername = sentFromuserName;
   packet->toUsername = sentToUserName;
   packet->message = message;

   m_contactServer->SendPacketToGateway( packet, m_connectionId );

   if( accepted == true )
   {
      m_contactServer->TrackInviteAccepted();
   }
   else
   {
      m_contactServer->TrackInviteRejected();
   }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

