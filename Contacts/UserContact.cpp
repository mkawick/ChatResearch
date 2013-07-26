// UserContact.cpp

#include <string>
using namespace std;

#include "../NetworkCommon/Packets/BasePacket.h"
#include "UserContact.h"
#include "DiplodocusContact.h"
#include <boost/lexical_cast.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////


UserContact::UserContact( const UserInfo& info, U32 connectionId ): 
               m_userInfo( info ), 
               m_connectionId( connectionId ), 
               m_requiresUpdate( false ),
               m_isLoggedOut( false ),
               m_hasBeenInitialized( false ),
               m_infoServer( NULL )
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
   assert( m_infoServer );

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

   m_infoServer->AddQueryToOutput( dbQuery );

   //-------------------------------

   dbQuery = new PacketDbQuery;
   dbQuery->id =           m_connectionId;
   dbQuery->meta =         "";
   dbQuery->lookup =       QueryType_Friends;
   dbQuery->serverLookup = m_userInfo.id;
   dbQuery->query = "SELECT * FROM users INNER JOIN friends ON users.user_id=friends.userid2 WHERE friends.userId1='";
   dbQuery->query += idString;
   dbQuery->query += "'";

   m_infoServer->AddQueryToOutput( dbQuery );

   //-------------------------------

   dbQuery = new PacketDbQuery;
   dbQuery->id =           m_connectionId;
   dbQuery->meta =         "";
   dbQuery->lookup =       QueryType_FriendRequestsSent;
   dbQuery->serverLookup = m_userInfo.id;
   dbQuery->query = "SELECT * FROM users INNER JOIN friend_pending ON users.user_id=friend_pending.invitee_id WHERE friend_pending.inviter_id='";
   dbQuery->query += idString;
   dbQuery->query += "'";

   m_infoServer->AddQueryToOutput( dbQuery );

   //-------------------------------

   dbQuery = new PacketDbQuery;
   dbQuery->id =           m_connectionId;
   dbQuery->meta =         "";
   dbQuery->lookup =       QueryType_FriendRequestReceived;
   dbQuery->serverLookup = m_userInfo.id;
   dbQuery->query = "SELECT * FROM users INNER JOIN friend_pending ON users.user_id=friend_pending.inviter_id WHERE friend_pending.invitee_id='";
   dbQuery->query += idString;
   dbQuery->query += "'";

   m_infoServer->AddQueryToOutput( dbQuery );

   //-------------------------------
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

//------------------------------------------------------------------------------------------------

void  UserContact::UserLoggedOut() 
{ 
   m_isLoggedOut = true; 
   time( &m_timeLoggedOut );
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
         }
      }
      break;
   case QueryType_Friends:
      {
         UserTable            enigma( dbResult->bucket );
         UserTable::iterator  it = enigma.begin();
         while( it != enigma.end() )
         {
            UserTable::row row = *it++;
            UserInfo ui;

            ui.id =               boost::lexical_cast< U32 >( row[ TableUser::Column_id ] );
            ui.username =         row[ TableUser::Column_name ];
            ui.uuid =             row[ TableUser::Column_uuid ];
            ui.email =            row[ TableUser::Column_email ];
            ui.passwordHash =     row[ TableUser::Column_password_hash ];
            ui.connectionId = 0;
            ui.active =           boost::lexical_cast< bool >( row[ TableUser::Column_active] );
            // note that we are using logout for our last login time.
            m_friends.push_back( ui );
         }
      }
      break;
   case QueryType_FriendRequestsSent:
      {
         UserJoinPendingTable            enigma( dbResult->bucket );
         UserJoinPendingTable::iterator  it = enigma.begin();
         while( it != enigma.end() )
         {
            UserJoinPendingTable::row row = *it++;
            Invitation invite;
            invite.invitationNumber =  boost::lexical_cast< U32 >( row[ TableUserJoinPending::Column_pending_id ] );
            invite.inviteeId =         boost::lexical_cast< U32 >( row[ TableUserJoinPending::Column_invitee_id ] );
            invite.inviterId =         boost::lexical_cast< U32 >( row[ TableUserJoinPending::Column_inviter_id ] );
            invite.wasNotified =       boost::lexical_cast< bool >( row[ TableUserJoinPending::Column_was_notified ] );

            invite.userName =          row[ TableUserJoinPending::Column_name ];
            invite.userUuid =          row[ TableUserJoinPending::Column_uuid ];
            invite.message  =          row[ TableUserJoinPending::Column_message ];
            invite.uuid  =             row[ TableUserJoinPending::Column_pending_uuid ];
            m_invitationsOut.push_back( invite );
         }
      }
      break;
   case QueryType_FriendRequestReceived:
      {
         UserJoinPendingTable            enigma( dbResult->bucket );
         UserJoinPendingTable::iterator  it = enigma.begin();
         while( it != enigma.end() )
         {
            UserJoinPendingTable::row row = *it++;
            Invitation invite;
            invite.invitationNumber =  boost::lexical_cast< U32 >( row[ TableUserJoinPending::Column_pending_id ] );
            invite.inviteeId =         boost::lexical_cast< U32 >( row[ TableUserJoinPending::Column_invitee_id ] );
            invite.inviterId =         boost::lexical_cast< U32 >( row[ TableUserJoinPending::Column_inviter_id ] );
            invite.wasNotified =       boost::lexical_cast< bool >( row[ TableUserJoinPending::Column_was_notified ] );

            invite.userName =          row[ TableUserJoinPending::Column_name ];
            invite.userUuid =          row[ TableUserJoinPending::Column_uuid ];
            invite.message  =          row[ TableUserJoinPending::Column_message ];
            invite.uuid  =             row[ TableUserJoinPending::Column_pending_uuid ];
            m_invitationsIn.push_back( invite );
         }
      }
      break;
   case QueryType_GetInviteeDetails:
      {
         if( dbResult->successfulQuery == false || dbResult->bucket.bucket.size() == 0 )
         {
            m_infoServer->SendErrorToClient( m_connectionId, PacketErrorReport::ErrorType_Contact_Invitation_ProblemFindingUser );
            return true;
         }

         UserTable            enigma( dbResult->bucket );
         UserTable::row  row = *enigma.begin();
         U32 id = boost::lexical_cast< U32 >( row[ TableUser::Column_id ] );
         if( id > 0 )
         {
            FinishInvitation( id, dbResult->meta );
         }
      }
      break;
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
      
   }

   return false;
}


//------------------------------------------------------------------------------------------------

bool     UserContact::GetListOfContacts()
{
   PacketContact_GetListOfContactsResponse* packet = new PacketContact_GetListOfContactsResponse;

   vector< UserInfo >::iterator  it = m_friends.begin();
   while ( it != m_friends.end() )
   {
      const UserInfo& ui = *it++; 

      const UserContact* testUser = m_infoServer->GetUser( ui.id );
      packet->friends.insert( ui.uuid, FriendInfo( ui.username, ui.avatarId, testUser != 0 ) );
   }
   
   m_infoServer->AddOutputChainData( packet, m_connectionId );
   
   return true;
}

//------------------------------------------------------------------------------------------------

bool     UserContact::InviteUser( const string& inviteeUuid, const string& message )
{
   //const string& inviterUuid, 
   // we should validate that we do not already have a pending invitation
   

   // we have all of your current invitations in memory... verify that we don't already have a pending invitation
   vector< Invitation >::iterator it = m_invitationsOut.begin();
   while( it != m_invitationsOut.end() )
   {
      const Invitation& invite = *it++;
      if( invite.uuid == inviteeUuid )
      {
         m_infoServer->SendErrorToClient( m_connectionId, PacketErrorReport::ErrorType_Contact_Invitation_InviteeAlreadyInvited );
         return false;
      }
   }

   UserContact* contact = m_infoServer->GetUserByUuid( inviteeUuid );// this user needs to know that he's been invited.
   if( contact )
   {
      // we still need to write this to the db
      FinishInvitation( contact->GetUserInfo().id, message );

      contact->YouHaveBeenInvitedToBeAFriend( m_userInfo.username, m_userInfo.uuid );
      //notify user
      assert( 0 );
   }
   else
   {
      PacketDbQuery* dbQuery = new PacketDbQuery;
      dbQuery->id =           m_connectionId;
      dbQuery->meta =         message;
      dbQuery->lookup =       QueryType_GetInviteeDetails;
      dbQuery->serverLookup = m_userInfo.id;

      string query = "SELECT * FROM users WHERE uuid='";
      query += inviteeUuid;
      query += "'";
      dbQuery->query = query;

      m_infoServer->AddQueryToOutput( dbQuery );
   }

   return true;
}

//------------------------------------------------------------------------------------------------

void  UserContact::FinishInvitation( U32 inviteeId, const string& message )
{
   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =           m_connectionId;
   dbQuery->meta =         "";
   dbQuery->lookup =       QueryType_AddInvitationToUser;
   dbQuery->serverLookup = m_userInfo.id;
   dbQuery->isFireAndForget = true;

   
   time_t currentTime;
   time( &currentTime );
   string currentTimeInUTC = GetDateInUTC();
   string invitationUuid = GenerateUUID( static_cast< U32 >( currentTime ) );

   string query = "INSERT INTO friend_pending (inviter_id, invitee_id, was_notified, sent, message, uuid) VALUES ( ";
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

   m_infoServer->AddQueryToOutput( dbQuery );
}

//------------------------------------------------------------------------------------------------

void     UserContact::YouHaveBeenInvitedToBeAFriend( const string& userName, const string& uuid )
{
   assert( 0 );// undone
   //m_infoServer->SendPacketToGateway( dbQuery );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

