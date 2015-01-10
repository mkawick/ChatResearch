// ChatUser.cpp

#include "../NetworkCommon/Packets/BasePacket.h"
#include "../NetworkCommon/Packets/ChatPacket.h"
#include "../NetworkCommon/Packets/DbPacket.h"
#include "../NetworkCommon/Packets/LoginPacket.h"
#include "../NetworkCommon/Utils/CommandLineParser.h"

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
using boost::format;

#include "ChatUser.h"
#include "DiplodocusChat.h"
#include "../NetworkCommon/Utils/TableWrapper.h"
#include "ChatRoomManager.h"
//#include "InvitationManager.h"

#include <boost/lexical_cast.hpp>

///////////////////////////////////////////////////////////////////

template< typename PacketType >
bool     MultiSendMessageToClient( vector< SimpleConnectionDetails >& connectionList,
                             DiplodocusChat* chatServer,
                             PacketType* packetToSend )
{
   PacketType* p = packetToSend;
   vector< SimpleConnectionDetails >::iterator it = connectionList.begin();
   while( it != connectionList.end() )
   {
      U32 connectionId = it->connectionId;
      U32 gatewayId = it->gatewayId;

      PacketGatewayWrapper* wrapper = new PacketGatewayWrapper();
      wrapper->SetupPacket( p, connectionId );
      chatServer->SendMessageToClient( wrapper, connectionId, gatewayId );
      it++;

      if( it != connectionList.end() )// here we replicate the last packet if there are more to send
      {
         PacketType* p = new PacketType;
         *p = *packetToSend;
      }
   }
   return true;
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

DiplodocusChat*      ChatUser::  m_chatServer = NULL;
ChatRoomManager*     ChatUser::  m_chatRoomManager = NULL;
//InvitationManager*   ChatUser::  m_invitationManager = NULL;

void ChatUser::Set( DiplodocusChat* chat )
{
   m_chatServer = chat;
}

void ChatUser::Set( ChatRoomManager* mgr )
{
   m_chatRoomManager = mgr;
}
/*
void ChatUser::Set( InvitationManager* mgr )
{
   m_invitationManager = mgr;
}*/

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

ChatUser::ChatUser(  ) : 
                           m_isLoggedIn( false ),
                           m_initialRequestForInfoSent( false ),
                           m_blockContactInvitations( false ),
                           m_blockGroupInvitations ( false )
{
}

//---------------------------------------------------------

ChatUser::~ChatUser()
{
}

//---------------------------------------------------------

void  ChatUser::Init( /*U32 userId, const string& name, const string& uuid, const string& lastLoginTime*/ )
{
 /*  m_userName =      name;
   m_userUuid =          uuid;
   m_lastLoginTime = lastLoginTime;

   cout << "--------------------------------------------" << endl;
   cout << "User login: " << name << endl;
   cout << "User uuid: " << uuid << endl;
   cout << "lastLoginTime = " << lastLoginTime << endl;
   cout << "--------------------------------------------" << endl;
   m_userId =      userId;*/
}

//---------------------------------------------------------
/*
void     ChatUser::LoggedIn() 
{
   m_isLoggedIn = true;
   m_loggedOutTime = 0;
   m_initialRequestForInfoSent = false;// relogging rerequests the data.
   m_chatRoomManager->UserHasLoggedIn( m_userUuid );
}

//---------------------------------------------------------

void     ChatUser::LoggedOut() 
{ 
   m_isLoggedIn = false; 
   time( &m_loggedOutTime );
   m_chatRoomManager->UserHasLoggedOut( m_userUuid );
}
*/
//---------------------------------------------------------

void     ChatUser::Update()
{
   if( m_initialRequestForInfoSent == false )
   {
      m_initialRequestForInfoSent = true;
      RequestAllBasicChatInfo();
      RequestUserProfileInfo();
   }
}

//---------------------------------------------------------

void     ChatUser::RequestAllBasicChatInfo()
{
   GetAllChatHistroySinceLastLogin();
   //RequestChatChannels();
}

bool     ChatUser::HandleClientRequest( const BasePacket* packet, U32 connectionId )
{
   switch( packet->packetType )
   {
      //case PacketType_DbQuery:// not handled here

      case PacketType_UserInfo:
      {
         switch( packet->packetSubType )
         {
         case PacketUserInfo::InfoType_ChatChannelListRequest:
            {
               RequestChatChannels( connectionId );
            }
            break;
         }
      }
      break;
    /*  case PacketType_Invitation:
      {
         m_invitationManager->HandlePacketRequest( packet, m_connectionId );
      }
      break;*/
      case PacketType_Chat:
      {
         switch( packet->packetSubType )
         {
         case PacketChatToServer::ChatType_EchoToServer:
            {
               const PacketChatToServer* chat = static_cast< const PacketChatToServer* >( packet );
               chat = chat;
               EchoHandler( connectionId );
            }
            break;
         case PacketChatToServer::ChatType_ChatToServer:
            {
               const PacketChatToServer* chat = static_cast< const PacketChatToServer* >( packet );
               SendChat( chat->message, chat->userUuid, chat->channelUuid, chat->gameTurn, connectionId );
            }
            break;
         /*case PacketChatToServer::ChatType_ChangeChatChannel:
            {
               PacketChangeChatChannel* channelChange = static_cast< PacketChangeChatChannel* >( packet );
               channelChange = channelChange;
               //if( channelChange->chatChannelUuid != m_currentChannel )// ignore changes to the same channel
               {
                  //SetChatChannel( channelChange->chatChannelUuid );
               }
            }
            break;*/
         case PacketChatToServer::ChatType_RequestHistory:
            {
               const PacketChatHistoryRequest* request = static_cast< const PacketChatHistoryRequest* > ( packet );
               if( request->chatChannelUuid.size() )
               {
                  QueryChatChannelHistory( request->chatChannelUuid, request->numRecords, request->startingIndex, request->startingTimestamp, connectionId );
               }
               else
               {
                  QueryChatP2PHistory( request->userUuid, request->numRecords, request->startingIndex, request->startingTimestamp, connectionId );
               }
            }
            break;
         case PacketChatToServer::ChatType_RequestHistorySinceLastLogin:
            {
               //PacketChatMissedHistoryRequest* request = static_cast< PacketChatMissedHistoryRequest* > ( packet );
               //GetAllChatHistroySinceLastLogin();
               //SendChatHistorySinceLastLogin();
            }
            break;
            //-----------------------------------------------
         case PacketChatToServer::ChatType_CreateChatChannel:
            {
               const PacketChatCreateChatChannel* request = static_cast< const PacketChatCreateChatChannel* > ( packet );
               m_chatRoomManager->CreateNewRoom( request->name, m_userUuid, connectionId );
            }
            break;
         case PacketChatToServer::ChatType_DeleteChatChannel:
            {
               const PacketChatDeleteChatChannel* request = static_cast< const PacketChatDeleteChatChannel* > ( packet );
               m_chatRoomManager->DeleteRoom( request->uuid, m_userUuid, connectionId );
            }
            break;
         /*case PacketChatToServer::ChatType_InviteUserToChatChannel:
            {
               PacketChatInviteUserToChatChannel* request = static_cast< PacketChatInviteUserToChatChannel* > ( packet );
               m_chatRoomManager->InviteUserToChannel( request->chatChannelUuid, request->userUuid, m_userUuid );
            }
            break;*/
         case PacketChatToServer::ChatType_AddUserToChatChannel:
            {
               const PacketChatAddUserToChatChannel* request = static_cast< const PacketChatAddUserToChatChannel* > ( packet );
               bool success = m_chatRoomManager->AddUserToRoom( request->chatChannelUuid, request->userUuid, m_userUuid, connectionId );

               /*PacketChatAddUserToChatChannelResponse* response = new PacketChatAddUserToChatChannelResponse;
               response->channelUuid = request->chatChannelUuid;
               response->userUuid = request->userUuid;
               response->success = success;
               SendMessageToClient( response );*/

               if( success == false )
               {
                  SendErrorMessage( connectionId, GetGatewayId( connectionId ), PacketErrorReport::ErrorType_CannotAddUserToChannel_AlreadyExists );
               }

            }
            break;
         case PacketChatToServer::ChatType_RemoveUserFromChatChannel:
            {
               const PacketChatRemoveUserFromChatChannel* request = static_cast< const PacketChatRemoveUserFromChatChannel* > ( packet );
               bool success = m_chatRoomManager->RemoveUserFromRoom( request->chatChannelUuid, request->userUuid );

              /* PacketChatRemoveUserFromChatChannelResponse* response = new PacketChatRemoveUserFromChatChannelResponse;
               response->chatChannelUuid = request->chatChannelUuid;
               response->userUuid = request->userUuid;
               response->success = success;
               SendMessageToClient( response );*/

               if( success == false )// usually self
               {
                  SendErrorMessage( connectionId, GetGatewayId( connectionId ), PacketErrorReport::ErrorType_BadChatChannel );
               }
            }
            break;
         case PacketChatToServer::ChatType_RenameChatChannel:
            {
               const PacketChatRenameChannel* request = static_cast< const PacketChatRenameChannel* > ( packet );
               string oldName;
               bool success = m_chatRoomManager->RenameChatRoom( request->channelUuid, request->newName, m_userUuid, oldName, connectionId );

               PacketChatRenameChannelResponse* response = new PacketChatRenameChannelResponse;
               response->success = success;
               response->channelUuid = request->channelUuid;
               if( success == true )
               {
                  response->newName = request->newName;
               }
               else
               {
                  response->newName = oldName;
                  SendErrorMessage( connectionId, GetGatewayId( connectionId ), PacketErrorReport::ErrorType_BadChatChannel );
               }
               SendMessageToClient( response, connectionId );
            }             
            break;
         case PacketChatToServer::ChatType_UpdateProfile:
            {
               const PacketChat_UserProfileChange* request = static_cast< const PacketChat_UserProfileChange* > ( packet );
               m_blockGroupInvitations = request->blockChannelInvites;
               m_chatRoomManager->SetUserPreferences( m_userUuid, m_blockContactInvitations, m_blockGroupInvitations );
            }             
            break; 
         case PacketChatToServer::ChatType_ListAllMembersInChatChannel:
            {
               const PacketChatListAllMembersInChatChannel* request = static_cast< const PacketChatListAllMembersInChatChannel* > ( packet );
               m_chatRoomManager->RequestChatRoomInfo( request, connectionId );
            }
            break;
      /*   case PacketChatToServer::ChatType_RequestChatters:
            {
               PacketChatRequestChatters* request = static_cast< PacketChatRequestChatters* > ( packet );
               m_chatRoomManager->RequestChatters( request->chatChannelUuid, m_userUuid );
            }
            break;
      /*   case PacketChatToServer::ChatType_EnableDisableFiltering:
            {
               //PacketChatEnableFiltering* request = static_cast< PacketChatEnableFiltering* > ( packet );
               //m_chatRoomManager->AddUserToChannel( request->name, m_userUuid );
            }
            break;
         
         case PacketChatToServer::ChatType_AdminLoadAllChannels:
            {
               //PacketChatAdminLoadAllChannels* request = static_cast< PacketChatAdminLoadAllChannels* > ( packet );
               m_chatRoomManager->LoadAllChannels( m_userUuid );
            }
            break;
         case PacketChatToServer::ChatType_AdminRequestChatChannelList:
            {
               PacketChatAdminRequestChatChannelList* request = static_cast< PacketChatAdminRequestChatChannelList* > ( packet );
               m_chatRoomManager->RequestChatChannelList( m_userUuid, request->isFullList );
            }
            break;
         case PacketChatToServer::ChatType_AdminRequestUsersList:
            {
               PacketChatAdminRequestUsersList* request = static_cast< PacketChatAdminRequestUsersList* > ( packet );
               m_chatRoomManager->RequestUsersList( m_userUuid, request->isFullList );
            }
            break;*/
         case PacketChatToServer::ChatType_MarkChannelHistoryAsRead:
            {
               const PacketChat_MarkChannelHistoryAsRead* request = static_cast< const PacketChat_MarkChannelHistoryAsRead* > ( packet );
               MarkChatChannelLastReadDate( request->channelUuid, connectionId );
            }
            break;
         case PacketChatToServer::ChatType_MarkP2PHistoryAsRead:
            {
               const PacketChat_MarkP2PHistoryAsRead* request = static_cast< const PacketChat_MarkP2PHistoryAsRead* > ( packet );

               MarkP2PLastReadDate( request->userUuid, connectionId );
            }
            break;
         }
      }
      break;
   }

   return false;
}


//------------------------------------------------------------------------------------------------

struct ChatHistoryLookupData 
{
   string   userUuid;
   string   channelUuid;
   string   startingTimestamp;
   int      startingIndex;
   int      numRecordsRequested;
};

void     ChatUser::QueryChatChannelHistory( const string& channelUuid, int numRecords, int startingIndex, const string& startingTimestamp, U32 connectionId )
{
   cout << "ChatUser::QueryChatChannelHistory" << endl;
   // SELECT chat.text, user.name, chat.timestamp, chat.game_turn
   // FROM chat as chat, user as user
   // where chat_channel_id='ABCDEFGHIJKLMNOP' 
   // AND chat.user_id_sender=user.uuid

   if( channelUuid.size() == 0 )
   {
      m_chatServer->SendErrorToClient( connectionId, GetGatewayId( connectionId ), PacketErrorReport::ErrorType_BadChatChannel );
      return;
   }

   if( numRecords < 1 )
      numRecords = 1;
   if( numRecords > 100 )
      numRecords = 100;
   numRecords++;// always request 1 extra to use as a sentinel

   if( startingIndex < 0 )
      startingIndex = 0;
   if( startingIndex > 1000000 )
      startingIndex = 1000000;

   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id = connectionId;
   dbQuery->lookup = QueryType_ChatChannelHistory;
   dbQuery->serverLookup = m_userId;

   string queryString = "SELECT chat.text, users.user_name, users.uuid, chat.game_turn, chat.timestamp ";
   queryString += " FROM chat_message AS chat, users WHERE chat.user_id_sender=users.uuid AND ";
   queryString += "chat_channel_id='%s'";

   if( startingTimestamp.size() != 0 )
   {
      queryString += " AND timestamp<'%s' ";
   }

   string limitString = " ORDER BY chat.timestamp DESC LIMIT ";
   if( startingIndex != 0 )
   {
      limitString += boost::lexical_cast< string >( startingIndex );
      limitString += ",";
   }
   limitString += boost::lexical_cast< string >( numRecords );

   queryString += limitString;

   dbQuery->query = queryString;
   dbQuery->escapedStrings.insert( channelUuid );
   if( startingTimestamp.size() )// this might be empty
   {
      dbQuery->escapedStrings.insert( startingTimestamp );
   }
   //dbQuery->meta = channelUuid;
   ChatHistoryLookupData* extras = new ChatHistoryLookupData; 
   extras->channelUuid = channelUuid;
   extras->startingIndex = startingIndex;
   extras->startingTimestamp = startingTimestamp;
   extras->numRecordsRequested = numRecords;
   dbQuery->customData = extras;

   m_chatServer->AddQueryToOutput( dbQuery, connectionId );
}

//------------------------------------------------------------------------------------------------

void     ChatUser::MarkChatChannelLastReadDate( const string& chatChannelUuid, U32 connectionId )
{
   if( chatChannelUuid.size() == 0 )
      return;

   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id = connectionId;
   dbQuery->lookup = QueryType_UpdateLastReadDate;
   dbQuery->serverLookup = m_userId;
   dbQuery->isFireAndForget = true;
   dbQuery->query = "UPDATE playdek.user_join_chat_channel SET date_last_viewed=NOW() WHERE user_uuid='%s' AND channel_uuid='%s';";

   dbQuery->escapedStrings.insert( m_userUuid );
   dbQuery->escapedStrings.insert( chatChannelUuid );
   m_chatServer->AddQueryToOutput( dbQuery, connectionId );
}

//------------------------------------------------------------------------------------------------

void     ChatUser::MarkP2PLastReadDate( const string& friendUuid, U32 connectionId )
{
   if( friendUuid.length() == 0 )
      return;

   // NOTE: todo, see if the chat room manager already has the list of users and that 
   // it's updated when new users are added.

   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id = connectionId;
   dbQuery->lookup = QueryType_LookupUserIdToMarkAsRead;
   dbQuery->serverLookup = m_userId;
   dbQuery->isFireAndForget = false;
   dbQuery->query = "SELECT user_id FROM playdek.users WHERE uuid='%s'";

   dbQuery->escapedStrings.insert( friendUuid.c_str() );
 
   m_chatServer->AddQueryToOutput( dbQuery, connectionId );
}

//------------------------------------------------------------------------------------------------

void     ChatUser::MarkFriendLastReadDateFinish( const PacketDbQueryResult * dbResult, U32 connectionId )
{
   if( dbResult->bucket.bucket.size() == 0 || dbResult->successfulQuery == false )
   {
      return;
   }

   U32 friendId = 0;

   SingleColumnTable            enigma( dbResult->bucket );
   SingleColumnTable::iterator  it = enigma.begin();
   if( it != enigma.end() )
   {
      SingleColumnTable::row       row = *it++;
      friendId =   boost::lexical_cast< U32 >( row[ TableSingleColumn::Column_text ] );
     
   }


   if( friendId == 0 )
      return;

   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id = connectionId;
   dbQuery->lookup = QueryType_UpdateLastReadDate;
   dbQuery->serverLookup = m_userId;
   dbQuery->isFireAndForget = true;
   dbQuery->query = "UPDATE playdek.friends SET date_chat_viewed=NOW() WHERE userid1=";
   dbQuery->query += boost::lexical_cast< string > ( friendId );
   dbQuery->query += " AND userid2=";
   dbQuery->query += boost::lexical_cast< string > ( m_userId );// I am the recipient

   m_chatServer->AddQueryToOutput( dbQuery, connectionId );
}

//------------------------------------------------------------------------------------------------

void     ChatUser::QueryChatP2PHistory( const string& userUuid, int numRecords, int startingIndex, const string& startingTimestamp, U32 connectionId )
{
   /*
   SELECT chat.text, users.user_name, users.uuid, chat.game_turn, chat.timestamp

   FROM playdek.chat_message AS chat, users WHERE 
   chat.user_id_sender=users.uuid  AND
   chat_channel_id="" AND 
   ( 
   (user_id_sender='987654321' AND user_id_recipient='9635748152' ) or
   (user_id_sender='9635748152' AND user_id_recipient='987654321' )
   )
   ORDER BY timestamp DESC LIMIT 0,20
   */

   
   if( userUuid.size() == 0 )
   {
      m_chatServer->SendErrorToClient( connectionId, GetGatewayId( connectionId ), PacketErrorReport::ErrorType_NoChatHistoryExistsForThisUser  );
      return;
   }

   if( numRecords < 1 )
      numRecords = 1;
   if( numRecords > 100 )
      numRecords = 100;
   numRecords++;// always request 1 extra to use as a sentinel

   if( startingIndex < 0 )
      startingIndex = 0;
   if( startingIndex > 1000000 )
      startingIndex = 1000000;

   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id = connectionId;
   dbQuery->lookup = QueryType_ChatP2PHistory;
   dbQuery->serverLookup = m_userId;

   string queryString = "SELECT chat.text, users.user_name, users.uuid, chat.game_turn, chat.timestamp FROM playdek.chat_message AS chat, users WHERE chat.user_id_sender=users.uuid  AND " \
                  "chat_channel_id='' AND  ( "\
                  "(user_id_sender='%s' AND user_id_recipient='%s' ) or " \
                  "(user_id_sender='%s' AND user_id_recipient='%s' ) " \
                  ") ";

   string limitString = "ORDER BY timestamp DESC LIMIT ";
   if( startingTimestamp.size() == 0 )
   {
      queryString += limitString;
      queryString += boost::lexical_cast< string >( startingIndex );
      queryString += ",";
   }
   else
   {
      queryString += "AND timestamp<'%s' ";
      queryString += limitString;
   }
   queryString += boost::lexical_cast< string >( numRecords );

   dbQuery->query = queryString;
   dbQuery->escapedStrings.insert( userUuid );
   dbQuery->escapedStrings.insert( m_userUuid );
   dbQuery->escapedStrings.insert( m_userUuid );
   dbQuery->escapedStrings.insert( userUuid );
   
   if( startingTimestamp.size() )// this might be empty
   {
      dbQuery->escapedStrings.insert( startingTimestamp );
   }
   //dbQuery->meta = userUuid;

   ChatHistoryLookupData* extras = new ChatHistoryLookupData; 
   extras->userUuid = userUuid;
   extras->startingIndex = startingIndex;
   extras->startingTimestamp = startingTimestamp;
   extras->numRecordsRequested = numRecords;

   dbQuery->customData = extras;

   m_chatServer->AddQueryToOutput( dbQuery, connectionId );
}

//------------------------------------------------------------------------------------------------

bool  ChatUser:: RequestUserProfileInfo()
{
   string idString = boost::lexical_cast< string >( m_userId );

   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =           0;
   dbQuery->meta =         "";
   dbQuery->lookup =       QueryType_UserProfile;
   dbQuery->serverLookup = m_userId;
   dbQuery->query = "SELECT * FROM user_profile WHERE user_id='";
   dbQuery->query += idString;
   dbQuery->query += "'";

   bool success = m_chatServer->AddQueryToOutput( dbQuery, 0 );
   success = success;

   return true;
}

//------------------------------------------------------------------------------------------------

void     ChatUser::GetAllChatHistroySinceLastLogin()
{
   // we need this to do a few different things.. 
   // 1) pull all of your chat channels and last_read_date (in user_join_chat_channel)
   // 2) pull all of your friends ana last_read_date (int friends, you are the recipient).
   // 3) pull all chat-messages where the last-read_date is older
   // Note: do not update any dates.

   // we'll break this into two queries. One for chat channels and 
   // one for friends.

   //m_chatRoomManager->GetUserId( entry.senderUuid );
   string friendQueryString = "SELECT uuid FROM users inner join friends on users.user_id=friends.userid1 WHERE friends.userid2=";
   friendQueryString += boost::lexical_cast< string >( m_userId );
   friendQueryString += " AND date_last_chat>date_chat_viewed";

   string channelHistoryQueryString = "SELECT channel_uuid FROM playdek.user_join_chat_channel " \
      "AS ujoin INNER JOIN chat_channel AS channel ON ujoin.channel_uuid=channel.uuid " \
      "WHERE user_uuid='%s' AND ujoin.date_last_viewed < channel.date_last_chat;";

   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id = 0;
   dbQuery->lookup = QueryType_ManageChatHistoryUsers;
   dbQuery->isFireAndForget = false;
   dbQuery->serverLookup = m_userId;
   dbQuery->query = friendQueryString;

   m_chatServer->AddQueryToOutput( dbQuery, 0 );

   dbQuery = new PacketDbQuery;
   dbQuery->id = 0;
   dbQuery->lookup = QueryType_ManageChatHistoryChannels;
   dbQuery->serverLookup = m_userId;
   dbQuery->isFireAndForget = false;
   dbQuery->query = channelHistoryQueryString;

   dbQuery->escapedStrings.insert( m_userUuid );
   m_chatServer->AddQueryToOutput( dbQuery, 0 );
}

// dates:
// each channel has a date when last posted to
// each user has a last read time on the user_join_chat_channel table
// each friend-pair stores a date when the last chat was sent ... the recipient (userid2)
//   is the owner of the date_chat_viewed field.

/*
void  AggregateAllChannelsThatHaveANewerTimestamp()
{
   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id = m_connectionId;
   dbQuery->lookup = QueryType_ChatChannel_NMissedSinceLastLogin;

   string queryString = "";
   m_lastLoginTime;
}*/

//---------------------------------------------------------

bool     ChatUser::HandleDbResult( const PacketDbQueryResult * dbResult )
{
   cout << "ChatUser::HandleDbResult" << endl;
   const BasePacket* packet = static_cast< const BasePacket*>( dbResult );
   PacketCleaner cleaner( packet );
   U32 connectionId = dbResult->id;
   //const int maxNumMessagesPerPacket = 20;

   switch( dbResult->lookup )
   {
   case QueryType_UserProfile:
      {
         LoadUserProfile( dbResult, connectionId );
      }
      break;
   case QueryType_ChatChannelHistory:
      {
         SendChatChannelHistoryToClient( dbResult, connectionId );
      }
      break;
   case QueryType_ChatP2PHistory:
      {
         SendChatp2pHistoryToClient( dbResult, connectionId );
      }
      break;
   case QueryType_ManageChatHistoryUsers:
      {
         ManageChatHistoryUsers( dbResult, connectionId );
      }
      break;
   case QueryType_ManageChatHistoryChannels:
      {
         ManageChatHistoryChannels( dbResult, connectionId );
      }
      break;
   case QueryType_StoreChatHistoryMissedSinceLastLogin:
      {
         StoreChatHistoryMissedSinceLastLogin( dbResult, connectionId );
      }
      break;
   case QueryType_LookupUserIdToMarkAsRead:
      {
         MarkFriendLastReadDateFinish( dbResult, connectionId );
      }
      break;
   }

   return true;
}

//---------------------------------------------------------------

void     ChatUser::ChatReceived( const string& message, const string& senderUuid, const string& senderDisplayName, string channelUuid, string timeStamp, U32 userId )
{
   PacketChatToClient* packet = new PacketChatToClient;
   packet->message = message;
   packet->userUuid = senderUuid;
   packet->userName = senderDisplayName;
   packet->channelUuid = channelUuid;
   packet->timeStamp = timeStamp;
   packet->userTempId = userId;

   vector< SimpleConnectionDetails > connectionList;
   AssembleAllConnections( connectionList );

   MultiSendMessageToClient< PacketChatToClient >( connectionList, m_chatServer, packet );

  /* vector< SimpleConnectionDetails >::iterator it = connectionList.begin();
   while( it != connectionList.end() )
   {
      PacketChatToClient* packet = new PacketChatToClient;
      packet->message = message;
      packet->userUuid = senderUuid;
      packet->userName = senderDisplayName;
      packet->channelUuid = channelUuid;
      packet->timeStamp = timeStamp;
      packet->userTempId = userId;// naming for obfuscation

      if( SendMessageToClient( packet, it->connectionId, it->gatewayId ) == false )
      {
         BasePacket* tempPacket = packet;
         factory.CleanupPacket( tempPacket );
      }
      it++;
   }*/
}

//---------------------------------------------------------------

vector< MissedChatChannelEntry > ::iterator 
Find( const MissedChatChannelEntry& newEntry, vector< MissedChatChannelEntry >& history )
{
   vector< MissedChatChannelEntry > ::iterator it = history.begin();
   while( it != history.end() )
   {
      MissedChatChannelEntry& test = *it;
      if( test.chatChannelUuid == newEntry.chatChannelUuid &&
         test.senderUuid == newEntry.senderUuid )
         return it;

      it++;
   }
   return history.end();
}

//---------------------------------------------------------------

void     ChatUser::ManageChatHistoryUsers( const PacketDbQueryResult * dbResult, U32 connectionId )
{
   if( dbResult->bucket.bucket.size() == 0 || dbResult->successfulQuery == false )
   {
      return;
   }

   PacketChat_UserChatHistory* history = new PacketChat_UserChatHistory;
   SingleColumnTable            enigma( dbResult->bucket );
   SingleColumnTable::iterator  it = enigma.begin();
   if( it != enigma.end() )
   {
      SingleColumnTable::row       row = *it++;
      string userUuid = row[ TableSingleColumn::Column_text ];
      history->userUuidList.push_back( userUuid );
   }
   
   SendMessageToClient( history, connectionId );
}

void     ChatUser::ManageChatHistoryChannels( const PacketDbQueryResult * dbResult, U32 connectionId )
{
   if( dbResult->bucket.bucket.size() == 0 || dbResult->successfulQuery == false )
   {
      return;
   }

   PacketChat_ChannelChatHistory* history = new PacketChat_ChannelChatHistory;
   SingleColumnTable            enigma( dbResult->bucket );
   SingleColumnTable::iterator  it = enigma.begin();
   if( it != enigma.end() )
   {
      SingleColumnTable::row       row = *it++;
      string chatChannelUuid = row[ TableSingleColumn::Column_text ];
      history->channelUuidList.push_back( chatChannelUuid );
   }
   
   SendMessageToClient( history, connectionId );
}


//---------------------------------------------------------------

void     ChatUser::StoreChatHistoryMissedSinceLastLogin( const PacketDbQueryResult * dbResult, U32 connectionId )
{
   vector< MissedChatChannelEntry > history;

   ChatTable              enigma( dbResult->bucket );
   ChatTable::iterator    it = enigma.begin();
   while( it != enigma.end() )
   {
      ChatTable::row       row = *it++;
      MissedChatChannelEntry  entry;
      entry.message =         row[ TableChat::Column_text ];
      entry.senderUuid =      row[ TableChat::Column_user_id_sender ];
      entry.chatChannelUuid = row[ TableChat::Column_chat_channel_id ];
      string gameTurn =       row[ TableChat::Column_game_turn ];
      if( entry.chatChannelUuid.size() && gameTurn.size() )
      {
         entry.isGamechannel =  true;
         entry.senderTempId = m_chatRoomManager->GetUserId( entry.senderUuid );
      }
      else
      {
         entry.isGamechannel = false;
         entry.senderTempId = 0;
      }
      //entry.numMessages = 1;

      vector< MissedChatChannelEntry > ::iterator it = Find( entry, history );
      if( it == history.end() )
      {
         history.push_back( entry );
      }
     /* else
      {
         ++(it->numMessages);
      }*/
   }
   SendChatHistorySinceLastLogin( history );
}

//---------------------------------------------------------------

void     ChatUser::LoadUserProfile( const PacketDbQueryResult * dbResult, U32 connectionId )
{
   if( dbResult->bucket.bucket.size() == 0 )
   {
      m_chatServer->SendErrorToClient( connectionId, GetGatewayId( connectionId ), PacketErrorReport::ErrorType_UserUnknown );
      return;
   }

   UserProfileTable            enigma( dbResult->bucket );
   UserProfileTable::iterator  it = enigma.begin();
   if( it != enigma.end() )
   {
      UserProfileTable::row       row = *it++;
      //m_displayOnlineStatusToOtherUsers =    boost::lexical_cast< bool >( row[ TableUserProfile::Column_display_online_status_to_other_users ] );
      m_blockContactInvitations =            boost::lexical_cast< bool >( row[ TableUserProfile::Column_block_contact_invitations ] );
      m_blockGroupInvitations =              boost::lexical_cast< bool >( row[ TableUserProfile::Column_block_group_invitations ] );

      m_chatRoomManager->SetUserPreferences( m_userUuid, m_blockContactInvitations, m_blockGroupInvitations );
   }
}

//---------------------------------------------------------------

void     ChatUser::SendChatChannelHistoryToClient( const PacketDbQueryResult * dbResult, U32 connectionId )
{
   cout << "ChatUser::SendChatChannelHistoryToClient" << endl;
   if( dbResult->bucket.bucket.size() == 0 )
   {
      m_chatServer->SendErrorToClient( connectionId, GetGatewayId( connectionId ), PacketErrorReport::ErrorType_NoChatHistoryExistsOnSelectedChannel );
      //return;
   }

   ChatHistoryLookupData* extras = reinterpret_cast<ChatHistoryLookupData*> ( dbResult->customData  );
   if( extras )
   {
      SendChatHistoryToClientCommonSetup( dbResult->bucket, extras->userUuid , extras->channelUuid, extras->startingTimestamp, extras->startingIndex, extras->numRecordsRequested );
      delete extras;
   }   
}

//---------------------------------------------------------------

void     ChatUser::SendChatp2pHistoryToClient( const PacketDbQueryResult * dbResult, U32 connectionId )
{
   if( dbResult->bucket.bucket.size() == 0 )
   {
      m_chatServer->SendErrorToClient( connectionId, GetGatewayId( connectionId ), PacketErrorReport::ErrorType_NoChatHistoryExistsForThisUser );
      //return;
   }

   ChatHistoryLookupData* extras = reinterpret_cast<ChatHistoryLookupData*> ( dbResult->customData  );
   if( extras )
   {
      SendChatHistoryToClientCommonSetup( dbResult->bucket, extras->userUuid , extras->channelUuid, extras->startingTimestamp, extras->startingIndex, extras->numRecordsRequested );
      delete extras;
   }   
}

//---------------------------------------------------------------

void     ChatUser::SendChatHistoryToClientCommonSetup( const DynamicDataBucket& bucket, const string& userUuid, const string& chatChannelUuid, const string& startingTimestamp, int startingIndex, int numExpected )
{
   PacketFactory factory;
   vector< SimpleConnectionDetails > connectionList;
   AssembleAllConnections( connectionList );
   vector< SimpleConnectionDetails >::iterator it = connectionList.begin();
   while( it != connectionList.end() )
   {
      SendChatHistoryToClientCommon( bucket, userUuid, chatChannelUuid, startingTimestamp, startingIndex, numExpected, it->connectionId, it->gatewayId );
      it++;
   }
}

void     ChatUser::SendChatHistoryToClientCommon( const DynamicDataBucket& bucket, const string& userUuid, const string& chatChannelUuid, const string& startingTimestamp, int startingIndex, int numExpected, U32 connectionId, U32 gatewayId )
{
   SimpleChatTable              enigma( bucket );
   const int maxNumMessagesPerPacket = 20;
   PacketChatHistoryResult* result = new PacketChatHistoryResult;
   result->userUuid =            userUuid;
   result->chatChannelUuid =     chatChannelUuid;
   result->startingIndex =       startingIndex;
   result->startingTimestamp =   startingTimestamp;

   int countPackaged = 0;
   SimpleChatTable::iterator    it = enigma.begin();
   bool sendSentinel = true;
   if( it != enigma.end() )
   {
      int numMessages = enigma.size();      
      if( numMessages < numExpected ) // 0
      {
         sendSentinel = true;// this means that we don't have any more records
      }
      else // ( numMessages == numExpected )
      {
         sendSentinel = false;
         numMessages --;// we don't want the last item which is used as a sentinel.
      }


      for( int i=0; i<numMessages; i++ ) //countPackaged
      {
         ChatTable::row       row = *it++;
         ChatEntry            entry;
         entry.message =      row[ SimpleChat::Column_text ];
         entry.userName =     row[ SimpleChat::Column_user_name ];
         entry.useruuid =     row[ SimpleChat::Column_user_uuid ];
         entry.timestamp =    row[ SimpleChat::Column_timestamp ];
         entry.userTempId = m_chatRoomManager->GetUserId( entry.useruuid );

         const string& gameTurn = row[ SimpleChat::Column_game_turn ];

         if( gameTurn.size() && gameTurn != "NULL" ) // can be ""
         {
            entry.gameTurn =  boost::lexical_cast< int >( gameTurn );
         }
         else
         {
            entry.gameTurn  = 0;
         }
        
         result->chat.push_back( entry );
         countPackaged ++;

         // send once we have so many items
         if( ( result->chat.size() % maxNumMessagesPerPacket) == 0 )
         {
            SendMessageToClient( result, connectionId, gatewayId );
            if( i != numMessages-1 )
            {
               result = new PacketChatHistoryResult;
               result->userUuid = userUuid;
               result->chatChannelUuid = chatChannelUuid;
            }
            else 
            {
               result = NULL;
            }
         }
      }
   }

   if( sendSentinel == true && 
      result != NULL )
   {
      ChatEntry            entry;// empty record
      result->chat.push_back( entry );
   }

   if( result )
   {
      SendMessageToClient( result, connectionId, gatewayId );
   }
}




//---------------------------------------------------------------

void     ChatUser::SendChatHistorySinceLastLogin( const vector< MissedChatChannelEntry >& history )
{
   vector< SimpleConnectionDetails > connectionList;
   AssembleAllConnections( connectionList );

   cout << "SendChatHistorySinceLastLogin for user " << m_userName << endl;
   cout << " history size = " << history.size() << endl;

   const U32 maxNumMessagesPerPacket = 20;
   if( history.size() == 0 )
   {
      //m_chatServer->SendErrorToClient( m_connectionId, PacketErrorReport::ErrorType_NoChatHistoryExistsForThisUser );
      return;
   }
   
   U32 startingIndex = 0;

   while( history.size() - startingIndex > maxNumMessagesPerPacket )
   {
      PacketChatMissedHistoryResult* result = new PacketChatMissedHistoryResult;
      for( U32 i=0; i< maxNumMessagesPerPacket; i++ )
      {
         result->history.push_back( history [ startingIndex ] );
         startingIndex++;
      }
      MultiSendMessageToClient< PacketChatMissedHistoryResult >( connectionList, m_chatServer, result );
   }

   if( history.size() )
   {
      PacketChatMissedHistoryResult* result = new PacketChatMissedHistoryResult;
      while( startingIndex < history.size() )
      {
         result->history.push_back( history [ startingIndex ] );
         startingIndex++;
      }

      MultiSendMessageToClient< PacketChatMissedHistoryResult >( connectionList, m_chatServer, result );
   }
}

//---------------------------------------------------------------
//---------------------------------------------------------------

bool     ChatUser::NotifyChannelAdded( const string& channelName, const string& channelUuid, bool success )
{
   PacketChatCreateChatChannelResponse* response = new PacketChatCreateChatChannelResponse;
   response->name = channelName;
   response->uuid = channelUuid;
   response->successfullyCreated = success;

   vector< SimpleConnectionDetails > connectionList;
   AssembleAllConnections( connectionList );
   //SendMessageToClient( response, connectionId );
   MultiSendMessageToClient< PacketChatCreateChatChannelResponse >( connectionList, m_chatServer, response );

   return true;
}

//---------------------------------------------------------------

bool     ChatUser::NotifyChannelMovedToInactive( const string& channelUuid, int numremoved, bool success )
{
   PacketChatDeleteChatChannelResponse* response = new PacketChatDeleteChatChannelResponse;
   response->uuid = channelUuid;
   response->successfullyDeleted = success;
   response->numUsersRemoved = 0;

   vector< SimpleConnectionDetails > connectionList;
   AssembleAllConnections( connectionList );
   //SendMessageToClient( response, connectionId );
   MultiSendMessageToClient< PacketChatDeleteChatChannelResponse >( connectionList, m_chatServer, response );

   return true;
}

//---------------------------------------------------------------

bool     ChatUser::NotifyChannelRemoved( const string& channelUuid, int numremoved )
{
   PacketChatDeleteChatChannelResponse* response = new PacketChatDeleteChatChannelResponse;
   response->uuid = channelUuid;
   response->successfullyDeleted = true;
   response->numUsersRemoved = numremoved;

   vector< SimpleConnectionDetails > connectionList;
   AssembleAllConnections( connectionList );
   MultiSendMessageToClient< PacketChatDeleteChatChannelResponse >( connectionList, m_chatServer, response );

   //SendMessageToClient( response, connectionId );

   return true;
}

/*
//---------------------------------------------------------------

bool     ChatUser::NotifyAllChannelsLoaded( bool loaded )
{
   PacketChatAdminLoadAllChannelsResponse* response = new PacketChatAdminLoadAllChannelsResponse;
   response->success = loaded;

   SendMessageToClient( response );

   return true;
}
*/
//---------------------------------------------------------------

bool     ChatUser::NotifyUserStatusHasChanged( const string& userName, const string& userUuid, int statusChange )
{
   PacketChatUserStatusChangeBase* notification = new PacketChatUserStatusChangeBase;
   notification->userName = userName;
   notification->uuid = userUuid;
   notification->statusChange = statusChange;

   vector< SimpleConnectionDetails > connectionList;
   AssembleAllConnections( connectionList );
   MultiSendMessageToClient< PacketChatUserStatusChangeBase >( connectionList, m_chatServer, notification );

   //SendMessageToClient( notification, connectionId );

   return true;
}

//---------------------------------------------------------------
//---------------------------------------------------------------

bool     ChatUser::NotifyAddedToChannel( const string& channelName, const string& channelUuid, const string userName, const string userUuid )
{
   // send notification, then go to the db and pull all groups again and send to client before sending the new notification
   if( userUuid.size() == 0 || userUuid == m_userUuid )
   {
      RequestChatChannels();
   }

   // now send the notification
   PacketChatAddUserToChatChannelResponse* packet = new PacketChatAddUserToChatChannelResponse;
   packet->channelName = channelName;
   packet->channelUuid = channelUuid;
   packet->userUuid = userUuid;
   if( userUuid.size() == 0 )
      packet->userUuid = m_userUuid;

   packet->userName = userName;
   if( userName.size() == 0 )
      packet->userName = m_userName;

   packet->success = true;

   vector< SimpleConnectionDetails > connectionList;
   AssembleAllConnections( connectionList );
   MultiSendMessageToClient< PacketChatAddUserToChatChannelResponse >( connectionList, m_chatServer, packet );

   //SendMessageToClient( packet, connectionId );

   return true;
}

//---------------------------------------------------------------
/*
bool     ChatUser::NotifyYouWereAddedToChannel( const string& channelUuid )
{
   return true;
}
*/
//---------------------------------------------------------------

bool     ChatUser::NotifyRemovedFromChannel( const string& channelName, const string& channelUuid, bool wasSuccessful, string userUuid )
{
   PacketChatRemoveUserFromChatChannelResponse* response = new PacketChatRemoveUserFromChatChannelResponse;
   response->chatChannelUuid = channelUuid;
   response->userUuid = m_userUuid;
   if( userUuid.size() )
   {
      response->userUuid = userUuid;
   }
   response->success = wasSuccessful;

   vector< SimpleConnectionDetails > connectionList;
   AssembleAllConnections( connectionList );

   //SendMessageToClient( response, connectionId );
   MultiSendMessageToClient< PacketChatRemoveUserFromChatChannelResponse >( connectionList, m_chatServer, response );
   RequestChatChannels();

   return true;
}

//---------------------------------------------------------------

bool     ChatUser::EchoHandler( U32 connectionId )
{
   cout << " Echo " << endl;
   PacketChat_EchoToClient* echo = new PacketChat_EchoToClient;
   SendMessageToClient( echo, connectionId );
   return true;
}

//---------------------------------------------------------------

bool     ChatUser::SendChat( const string& message, const string& sendeeUuid, const string& channelUuid, U32 gameTurn, U32 connectionId )
{
   // the only thing that we do here is let the chat channel manager know what to do.
   // if the userUuid is valid, we send p2p
   // if the channel uuid is valid, we send to the channel.

   if( sendeeUuid.size() > 0 )
   {
      m_chatRoomManager->UserSendP2PChat( m_userUuid, sendeeUuid, message, connectionId );
      return true;
   }
   else if( channelUuid.size() > 0 )
   {
      m_chatRoomManager->UserSendsChatToChannel( m_userUuid, channelUuid, message, gameTurn, connectionId );
      return true;
   }
   return false;
}

//---------------------------------------------------------------

bool     ChatUser::SendMessageToClient( BasePacket* packetconnectionId, U32 connectionId, U32 gatewayId ) const
{
   if( gatewayId == 0 )
      gatewayId = GetGatewayId( connectionId );
   if( connectionId ==0 || gatewayId == 0 )
      return false;

   PacketGatewayWrapper* wrapper = new PacketGatewayWrapper();
   wrapper->SetupPacket( packetconnectionId, connectionId );

   //cout << "ChatUser::SendMessageToClient <<<" << endl;
   m_chatServer->SendMessageToClient( wrapper, connectionId, gatewayId );
   //cout << "ChatUser::SendMessageToClient >>>" << endl;
   return true;
}

//------------------------------------------------------------------------------------------------

void     ChatUser::RequestChatChannels( U32 connectionId )
{
   vector< SimpleConnectionDetails > connectionList;
   if( connectionId == 0 )
   {
      AssembleAllConnections( connectionList );
   }
   const int maxChannelsToSend = 15; // given the roughly 130 bytes just for the structure of the chat channel, 
   // we need to be sure that we don't overflow the buffer.
   SerializedKeyValueVector< ChannelInfoFullList >  channelList;
   m_chatRoomManager->GetChatRooms( m_userUuid, channelList );

   if( channelList.size() == 0 )
      return;

   int currentOffset = 0;
   int totalCount = channelList.size();

   while( currentOffset + maxChannelsToSend < totalCount )
   {
      PacketChatChannelList* packetChannels = new PacketChatChannelList;
      packetChannels->channelList.SetIndexParams( currentOffset, totalCount );

      int destinationOffset = currentOffset + maxChannelsToSend;
      for( int i=currentOffset; i<destinationOffset; i++ )
      {
         packetChannels->channelList.insert( channelList[i].key, channelList[i].value );
      }
      if( connectionId == 0 )
      {
         MultiSendMessageToClient< PacketChatChannelList >( connectionList, m_chatServer, packetChannels );
      }
      else
      {
         SendMessageToClient( packetChannels, connectionId );
      }
      currentOffset += maxChannelsToSend;
   }

   if( currentOffset < totalCount )
   {
      PacketChatChannelList* packetChannels = new PacketChatChannelList;
      packetChannels->channelList.SetIndexParams( currentOffset, totalCount );

      for( int i=currentOffset; i< totalCount; i++ )
      {
         packetChannels->channelList.insert( channelList[i].key, channelList[i].value );
      }
      if( connectionId == 0 )
      {
         MultiSendMessageToClient< PacketChatChannelList >( connectionList, m_chatServer, packetChannels );
      }
      else
      {
         SendMessageToClient( packetChannels, connectionId );
      }
   }

  /* PacketChatChannelList* packetChannels = new PacketChatChannelList;// this may need to be moved into a separate function

   m_chatRoomManager->GetChatChannels( m_userUuid, packetChannels->channelList );

   SendMessageToClient( packetChannels );*/
}

//------------------------------------------------------------------------------------------------

bool     ChatUser::SendErrorMessage( U32 connectionId, U32 gatewayId, PacketErrorReport::ErrorType error )
{
   return m_chatServer->SendErrorToClient( connectionId, GetGatewayId( connectionId ), error );
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////