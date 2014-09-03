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

ChatUser::ChatUser( U32 connectionId, U32 gatewayId ) : m_userId ( 0 ), 
                                          m_connectionId( connectionId ),
                                          m_gatewayId( gatewayId ),
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

void  ChatUser::Init( U32 userId, const string& name, const string& uuid, const string& lastLoginTime )
{
   m_userName =      name;
   m_uuid =          uuid;
   m_lastLoginTime = lastLoginTime;

   cout << "--------------------------------------------" << endl;
   cout << "User login: " << name << endl;
   cout << "User uuid: " << uuid << endl;
   cout << "lastLoginTime = " << lastLoginTime << endl;
   cout << "--------------------------------------------" << endl;
   m_userId =      userId;
}

//---------------------------------------------------------

void     ChatUser::LoggedIn() 
{
   m_isLoggedIn = true;
   m_loggedOutTime = 0;
   m_initialRequestForInfoSent = false;// relogging rerequests the data.
   m_chatRoomManager->UserHasLoggedIn( m_uuid );
}

//---------------------------------------------------------

void     ChatUser::LoggedOut() 
{ 
   m_isLoggedIn = false; 
   time( &m_loggedOutTime );
   m_chatRoomManager->UserHasLoggedOut( m_uuid );
}

//---------------------------------------------------------

bool     ChatUser::Update()
{
   if( m_initialRequestForInfoSent == false )
   {
      m_initialRequestForInfoSent = true;
      RequestAllBasicChatInfo();
      RequestUserProfileInfo();
   }
   return true;
}

//---------------------------------------------------------

void     ChatUser::RequestAllBasicChatInfo()
{
   GetAllChatHistroySinceLastLogin();
   //RequestChatChannels();
}

bool     ChatUser::HandleClientRequest( BasePacket* packet )
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
               RequestChatChannels();
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
               PacketChatToServer* chat = static_cast< PacketChatToServer* >( packet );
               EchoHandler();
            }
            break;
         case PacketChatToServer::ChatType_ChatToServer:
            {
               PacketChatToServer* chat = static_cast< PacketChatToServer* >( packet );
               SendChat( chat->message, chat->userUuid, chat->channelUuid, chat->gameTurn );
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
               PacketChatHistoryRequest* request = static_cast< PacketChatHistoryRequest* > ( packet );
               if( request->chatChannelUuid.size() )
               {
                  QueryChatChannelHistory( request->chatChannelUuid, request->numRecords, request->startingIndex, request->startingTimestamp );
               }
               else
               {
                  QueryChatP2PHistory( request->userUuid, request->numRecords, request->startingIndex, request->startingTimestamp );
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
               PacketChatCreateChatChannel* request = static_cast< PacketChatCreateChatChannel* > ( packet );
               m_chatRoomManager->CreateNewRoom( request->name, m_uuid );
            }
            break;
         case PacketChatToServer::ChatType_DeleteChatChannel:
            {
               PacketChatDeleteChatChannel* request = static_cast< PacketChatDeleteChatChannel* > ( packet );
               m_chatRoomManager->DeleteRoom( request->uuid, m_uuid );
            }
            break;
         /*case PacketChatToServer::ChatType_InviteUserToChatChannel:
            {
               PacketChatInviteUserToChatChannel* request = static_cast< PacketChatInviteUserToChatChannel* > ( packet );
               m_chatRoomManager->InviteUserToChannel( request->chatChannelUuid, request->userUuid, m_uuid );
            }
            break;*/
         case PacketChatToServer::ChatType_AddUserToChatChannel:
            {
               PacketChatAddUserToChatChannel* request = static_cast< PacketChatAddUserToChatChannel* > ( packet );
               bool success = m_chatRoomManager->AddUserToRoom( request->chatChannelUuid, request->userUuid, m_uuid );

               /*PacketChatAddUserToChatChannelResponse* response = new PacketChatAddUserToChatChannelResponse;
               response->channelUuid = request->chatChannelUuid;
               response->userUuid = request->userUuid;
               response->success = success;
               SendMessageToClient( response );*/

               if( success == false )
               {
                  SendErrorMessage( PacketErrorReport::ErrorType_CannotAddUserToChannel_AlreadyExists );
               }

            }
            break;
         case PacketChatToServer::ChatType_RemoveUserFromChatChannel:
            {
               PacketChatRemoveUserFromChatChannel* request = static_cast< PacketChatRemoveUserFromChatChannel* > ( packet );
               bool success = m_chatRoomManager->RemoveUserFromRoom( request->chatChannelUuid, request->userUuid );

              /* PacketChatRemoveUserFromChatChannelResponse* response = new PacketChatRemoveUserFromChatChannelResponse;
               response->chatChannelUuid = request->chatChannelUuid;
               response->userUuid = request->userUuid;
               response->success = success;
               SendMessageToClient( response );*/

               if( success == false )// usually self
               {
                  SendErrorMessage( PacketErrorReport::ErrorType_BadChatChannel );
               }
            }
            break;
         case PacketChatToServer::ChatType_RenameChatChannel:
            {
               PacketChatRenameChannel* request = static_cast< PacketChatRenameChannel* > ( packet );
               string oldName;
               bool success = m_chatRoomManager->RenameChatRoom( request->channelUuid, request->newName, m_uuid, oldName );

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
                  SendErrorMessage( PacketErrorReport::ErrorType_BadChatChannel );
               }
               SendMessageToClient( response );
            }             
            break;
         case PacketChatToServer::ChatType_UpdateProfile:
            {
               PacketChat_UserProfileChange* request = static_cast< PacketChat_UserProfileChange* > ( packet );
               m_blockGroupInvitations = request->blockChannelInvites;
               m_chatRoomManager->SetUserPreferences( m_uuid, m_blockContactInvitations, m_blockGroupInvitations );
            }             
            break; 
         case PacketChatToServer::ChatType_ListAllMembersInChatChannel:
            {
               PacketChatListAllMembersInChatChannel* request = static_cast< PacketChatListAllMembersInChatChannel* > ( packet );
               m_chatRoomManager->RequestChatRoomInfo( request, m_connectionId );
            }
            break;
      /*   case PacketChatToServer::ChatType_RequestChatters:
            {
               PacketChatRequestChatters* request = static_cast< PacketChatRequestChatters* > ( packet );
               m_chatRoomManager->RequestChatters( request->chatChannelUuid, m_uuid );
            }
            break;
      /*   case PacketChatToServer::ChatType_EnableDisableFiltering:
            {
               //PacketChatEnableFiltering* request = static_cast< PacketChatEnableFiltering* > ( packet );
               //m_chatRoomManager->AddUserToChannel( request->name, m_uuid );
            }
            break;
         
         case PacketChatToServer::ChatType_AdminLoadAllChannels:
            {
               //PacketChatAdminLoadAllChannels* request = static_cast< PacketChatAdminLoadAllChannels* > ( packet );
               m_chatRoomManager->LoadAllChannels( m_uuid );
            }
            break;
         case PacketChatToServer::ChatType_AdminRequestChatChannelList:
            {
               PacketChatAdminRequestChatChannelList* request = static_cast< PacketChatAdminRequestChatChannelList* > ( packet );
               m_chatRoomManager->RequestChatChannelList( m_uuid, request->isFullList );
            }
            break;
         case PacketChatToServer::ChatType_AdminRequestUsersList:
            {
               PacketChatAdminRequestUsersList* request = static_cast< PacketChatAdminRequestUsersList* > ( packet );
               m_chatRoomManager->RequestUsersList( m_uuid, request->isFullList );
            }
            break;*/
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

void     ChatUser::QueryChatChannelHistory( const string& channelUuid, int numRecords, int startingIndex, const string& startingTimestamp  )
{
   // SELECT chat.text, user.name, chat.timestamp, chat.game_turn
   // FROM chat as chat, user as user
   // where chat_channel_id='ABCDEFGHIJKLMNOP' 
   // AND chat.user_id_sender=user.uuid

   if( channelUuid.size() == 0 )
   {
      m_chatServer->SendErrorToClient( m_connectionId, m_gatewayId, PacketErrorReport::ErrorType_BadChatChannel );
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
   dbQuery->id = m_connectionId;
   dbQuery->lookup = QueryType_ChatChannelHistory;

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

   m_chatServer->AddQueryToOutput( dbQuery, m_connectionId );
}

//------------------------------------------------------------------------------------------------

void     ChatUser::QueryChatP2PHistory( const string& userUuid, int numRecords, int startingIndex, const string& startingTimestamp )
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
      m_chatServer->SendErrorToClient( m_connectionId, m_gatewayId, PacketErrorReport::ErrorType_NoChatHistoryExistsForThisUser  );
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
   dbQuery->id = m_connectionId;
   dbQuery->lookup = QueryType_ChatP2PHistory;

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
   dbQuery->escapedStrings.insert( m_uuid );
   dbQuery->escapedStrings.insert( m_uuid );
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

   m_chatServer->AddQueryToOutput( dbQuery, m_connectionId );
}

//------------------------------------------------------------------------------------------------

bool  ChatUser:: RequestUserProfileInfo()
{
   string idString = boost::lexical_cast< string >( m_userId );

   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =           m_connectionId;
   dbQuery->meta =         "";
   dbQuery->lookup =       QueryType_UserProfile;
   dbQuery->serverLookup = 0;//m_userId;
   dbQuery->query = "SELECT * FROM user_profile WHERE user_id='";
   dbQuery->query += idString;
   dbQuery->query += "'";

   bool success = m_chatServer->AddQueryToOutput( dbQuery, m_connectionId );

   return true;
}

//------------------------------------------------------------------------------------------------

void     ChatUser::GetAllChatHistroySinceLastLogin()
{
 /*  SELECT * FROM chat_message AS chat WHERE 
    ( chat_channel_id IN  (SELECT chat_channel.uuid from chat_channel
    JOIN user_join_chat_channel ON 
    user_join_chat_channel.channel_uuid = chat_channel.uuid  
    WHERE user_join_chat_channel.user_uuid='user3' ) 
   OR chat.user_id_recipient='user3' ) 
   AND timestamp>='2014-02-15 10:15:36'*/

   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id = m_connectionId;
   dbQuery->lookup = QueryType_ChatHistoryMissedSinceLastLogin;

   string queryString = "SELECT * FROM chat_message AS chat WHERE ( chat_channel_id IN " \
      " (SELECT chat_channel.uuid from chat_channel"\
      " JOIN user_join_chat_channel"\
      " ON user_join_chat_channel.channel_uuid = chat_channel.uuid "\
      " WHERE user_join_chat_channel.user_uuid='";
   queryString += m_uuid;
   queryString += "' ) OR chat.user_id_recipient='";
   queryString += m_uuid;
   queryString += "') AND timestamp>='";
   queryString += m_lastLoginTime;//"2013-04-23 14:50:38";
   queryString += "'";
   dbQuery->query = queryString;

   m_chatServer->AddQueryToOutput( dbQuery, m_connectionId );
}

//---------------------------------------------------------

bool     ChatUser::HandleDbResult( PacketDbQueryResult * dbResult )
{
   BasePacket* packet = static_cast<BasePacket*>( dbResult );
   PacketCleaner cleaner( packet );
   //const int maxNumMessagesPerPacket = 20;

   switch( dbResult->lookup )
   {
   case QueryType_UserProfile:
      {
         LoadUserProfile( dbResult );
      }
      break;
   case QueryType_ChatChannelHistory:
      {
         SendChatChannelHistoryToClient( dbResult );
      }
      break;
   case QueryType_ChatP2PHistory:
      {
         SendChatp2pHistoryToClient( dbResult );
      }
      break;
   case QueryType_ChatHistoryMissedSinceLastLogin:
      {
         StoreChatHistoryMissedSinceLastLogin( dbResult );

         //SendChatHistorySinceLastLogin();
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
   packet->userTempId = userId;// naming for obfuscation

   SendMessageToClient( packet );
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

void     ChatUser::StoreChatHistoryMissedSinceLastLogin( PacketDbQueryResult * dbResult )
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

void     ChatUser::LoadUserProfile( const PacketDbQueryResult * dbResult )
{
   if( dbResult->bucket.bucket.size() == 0 )
   {
      m_chatServer->SendErrorToClient( m_connectionId, m_gatewayId, PacketErrorReport::ErrorType_UserUnknown );
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

      m_chatRoomManager->SetUserPreferences( m_uuid, m_blockContactInvitations, m_blockGroupInvitations );
   }
}

//---------------------------------------------------------------

void     ChatUser::SendChatChannelHistoryToClient( const PacketDbQueryResult * dbResult )
{
   if( dbResult->bucket.bucket.size() == 0 )
   {
      m_chatServer->SendErrorToClient( m_connectionId, m_gatewayId, PacketErrorReport::ErrorType_NoChatHistoryExistsOnSelectedChannel );
      //return;
   }

   ChatHistoryLookupData* extras = reinterpret_cast<ChatHistoryLookupData*> ( dbResult->customData  );
   if( extras )
   {
      SendChatHistoryToClientCommon( dbResult->bucket, extras->userUuid , extras->channelUuid, extras->startingTimestamp, extras->startingIndex, extras->numRecordsRequested );
      delete extras;
   }   
}

//---------------------------------------------------------------

void     ChatUser::SendChatp2pHistoryToClient( PacketDbQueryResult * dbResult )
{
   if( dbResult->bucket.bucket.size() == 0 )
   {
      m_chatServer->SendErrorToClient( m_connectionId, m_gatewayId, PacketErrorReport::ErrorType_NoChatHistoryExistsForThisUser );
      //return;
   }

   ChatHistoryLookupData* extras = reinterpret_cast<ChatHistoryLookupData*> ( dbResult->customData  );
   if( extras )
   {
      SendChatHistoryToClientCommon( dbResult->bucket, extras->userUuid , extras->channelUuid, extras->startingTimestamp, extras->startingIndex, extras->numRecordsRequested );
      delete extras;
   }   
}

//---------------------------------------------------------------

void     ChatUser::SendChatHistoryToClientCommon( const DynamicDataBucket& bucket, const string& userUuid, const string& chatChannelUuid, const string& startingTimestamp, int startingIndex, int numExpected )
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
            SendMessageToClient( result );
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
      SendMessageToClient( result );
   }
}

//---------------------------------------------------------------

void     ChatUser::SendChatHistorySinceLastLogin( const vector< MissedChatChannelEntry >& history )
{
   cout << "SendChatHistorySinceLastLogin for user " << m_userName << endl;
   cout << " history size = " << history.size() << endl;

   const int maxNumMessagesPerPacket = 20;
   if( history.size() == 0 )
   {
      //m_chatServer->SendErrorToClient( m_connectionId, PacketErrorReport::ErrorType_NoChatHistoryExistsForThisUser );
      return;
   }
   
   U32 startingIndex = 0;

   while( history.size() - startingIndex > maxNumMessagesPerPacket )
   {
      PacketChatMissedHistoryResult* result = new PacketChatMissedHistoryResult;
      for( int i=0; i< maxNumMessagesPerPacket; i++ )
      {
         result->history.push_back( history [ startingIndex ] );
         startingIndex++;
      }
      SendMessageToClient( result );
   }

   if( history.size() )
   {
      PacketChatMissedHistoryResult* result = new PacketChatMissedHistoryResult;
      while( startingIndex < history.size() )
      {
         result->history.push_back( history [ startingIndex ] );
         startingIndex++;
      }

      SendMessageToClient( result );
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

   SendMessageToClient( response );

   return true;
}

//---------------------------------------------------------------

bool     ChatUser::NotifyChannelMovedToInactive( const string& channelUuid, int numremoved, bool success )
{
   PacketChatDeleteChatChannelResponse* response = new PacketChatDeleteChatChannelResponse;
   response->uuid = channelUuid;
   response->successfullyDeleted = success;
   response->numUsersRemoved = 0;

   SendMessageToClient( response );

   return true;
}

//---------------------------------------------------------------

bool     ChatUser::NotifyChannelRemoved( const string& channelUuid, int numremoved )
{
   PacketChatDeleteChatChannelResponse* response = new PacketChatDeleteChatChannelResponse;
   response->uuid = channelUuid;
   response->successfullyDeleted = true;
   response->numUsersRemoved = numremoved;

   SendMessageToClient( response );

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

   SendMessageToClient( notification );

   return true;
}

//---------------------------------------------------------------
//---------------------------------------------------------------

bool     ChatUser::NotifyAddedToChannel( const string& channelName, const string& channelUuid, const string userName, const string userUuid )
{
   // send notification, then go to the db and pull all groups again and send to client before sending the new notification
   if( userUuid.size() == 0 || userUuid == m_uuid )
   {
      RequestChatChannels();
   }

   // now send the notification
   PacketChatAddUserToChatChannelResponse* packet = new PacketChatAddUserToChatChannelResponse;
   packet->channelName = channelName;
   packet->channelUuid = channelUuid;
   packet->userUuid = userUuid;
   if( userUuid.size() == 0 )
      packet->userUuid = m_uuid;

   packet->userName = userName;
   if( userName.size() == 0 )
      packet->userName = m_userName;

   packet->success = true;

   SendMessageToClient( packet );

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
   response->userUuid = m_uuid;
   if( userUuid.size() )
   {
      response->userUuid = userUuid;
   }
   response->success = wasSuccessful;

   SendMessageToClient( response );
   RequestChatChannels();

   return true;
}

//---------------------------------------------------------------

bool     ChatUser::EchoHandler()
{
   cout << " Echo " << endl;
   PacketChat_EchoToClient* echo = new PacketChat_EchoToClient;
   SendMessageToClient( echo );
   return true;
}

//---------------------------------------------------------------

bool     ChatUser::SendChat( const string& message, const string& sendeeUuid, const string& channelUuid, U32 gameTurn )
{
   // the only thing that we do here is let the chat channel manager know what to do.
   // if the userUuid is valid, we send p2p
   // if the channel uuid is valid, we send to the channel.

   if( sendeeUuid.size() > 0 )
   {
      m_chatRoomManager->UserSendP2PChat( m_uuid, sendeeUuid, message );
      return true;
   }
   else if( channelUuid.size() > 0 )
   {
      m_chatRoomManager->UserSendsChatToChannel( m_uuid, channelUuid, message, gameTurn );
      return true;
   }
   return false;
}

//---------------------------------------------------------------

bool     ChatUser::SendMessageToClient( BasePacket* packet ) const
{
   PacketGatewayWrapper* wrapper = new PacketGatewayWrapper();
   wrapper->SetupPacket( packet, m_connectionId );

   m_chatServer->SendMessageToClient( wrapper, m_connectionId, m_gatewayId );
   return true;
}

//------------------------------------------------------------------------------------------------

void     ChatUser::RequestChatChannels()
{
   const int maxChannelsToSend = 15; // given the roughly 130 bytes just for the structure of the chat channel, 
   // we need to be sure that we don't overflow the buffer.
   SerializedKeyValueVector< ChannelInfoFullList >  channelList;
   m_chatRoomManager->GetChatRooms( m_uuid, channelList );

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
      SendMessageToClient( packetChannels );
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
      SendMessageToClient( packetChannels );
   }

  /* PacketChatChannelList* packetChannels = new PacketChatChannelList;// this may need to be moved into a separate function

   m_chatRoomManager->GetChatChannels( m_uuid, packetChannels->channelList );

   SendMessageToClient( packetChannels );*/
}

//------------------------------------------------------------------------------------------------

bool     ChatUser::SendErrorMessage( PacketErrorReport::ErrorType error )
{
   return m_chatServer->SendErrorToClient( m_connectionId, m_gatewayId, error );
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////