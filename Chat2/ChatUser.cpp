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
#include "diplodocusChat.h"
#include "../NetworkCommon/Utils/TableWrapper.h"
#include "ChatChannelManager.h"

#include <boost/lexical_cast.hpp>

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

DiplodocusChat*      ChatUser::  m_chatServer = NULL;
ChatChannelManager*  ChatUser::  m_chatChannelManager = NULL;

void ChatUser::Set( DiplodocusChat* chat )
{
   m_chatServer = chat;
}

void ChatUser::Set( ChatChannelManager* mgr )
{
   m_chatChannelManager = mgr;
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

ChatUser::ChatUser( U32 connectionId ) : m_userId ( 0 ), 
                                          m_connectionId( connectionId ), 
                                          m_pendingQueries( QueryType_All ),
                                          m_isLoggedIn( false ),
                                          m_initialRequestForInfoSent( false )
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
   m_userId =      userId;
}

//---------------------------------------------------------

void     ChatUser::LoggedOut() 
{ 
   m_isLoggedIn = false; 
   time( &m_loggedOutTime );
}

//---------------------------------------------------------

bool     ChatUser::Update()
{
   if( m_initialRequestForInfoSent == false )
   {
      m_initialRequestForInfoSent = true;
      RequestAllBasicChatInfo();
   }
   return true;
}

//---------------------------------------------------------

void     ChatUser::RequestAllBasicChatInfo()
{
   GetAllChatHistroySinceLastLogin();
   RequestChatChannels();
}

bool     ChatUser::HandleClientRequest( BasePacket* packet )
{
   switch( packet->packetType )
   {
  /* case PacketType_DbQuery:
      {
         switch( packet->packetSubType )
         {
         case BasePacketDbQuery::QueryType_Result:
            {
               HandleDbQueryResult( packet );
            }
            break;
         }
      }
      break;*/
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
      case PacketType_Chat:
      {
         switch( packet->packetSubType )
         {
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
                  QueryChatChannelHistory( request->chatChannelUuid, request->numRecords, request->startingIndex );
               }
               else
               {
                  QueryChatP2PHistory( request->userUuid, request->numRecords, request->startingIndex );
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
               m_chatChannelManager->CreateNewChannel( request->name, m_uuid );
            }
            break;
      /*   case PacketChatToServer::ChatType_DeleteChatChannel:
            {
               PacketChatDeleteChatChannel* request = static_cast< PacketChatDeleteChatChannel* > ( packet );
               m_chatChannelManager->DeleteChannel( request->uuid, m_uuid );
            }
            break;
         case PacketChatToServer::ChatType_InviteUserToChatChannel:
            {
               PacketChatInviteUserToChatChannel* request = static_cast< PacketChatInviteUserToChatChannel* > ( packet );
               m_chatChannelManager->InviteUserToChannel( request->chatChannelUuid, request->userUuid, m_uuid );
            }
            break;
         case PacketChatToServer::ChatType_AddUserToChatChannel:
            {
               PacketChatAddUserToChatChannel* request = static_cast< PacketChatAddUserToChatChannel* > ( packet );
               m_chatChannelManager->AddUserToChannel( request->chatChannelUuid, request->userUuid, m_uuid );
            }
            break;
         case PacketChatToServer::ChatType_RemoveUserFromChatChannel:
            {
               PacketChatRemoveUserFromChatChannel* request = static_cast< PacketChatRemoveUserFromChatChannel* > ( packet );
               m_chatChannelManager->RemoveUserFromChannel( request->chatChannelUuid, request->userUuid, m_uuid );
            }
            break;
         case PacketChatToServer::ChatType_RequestChatters:
            {
               PacketChatRequestChatters* request = static_cast< PacketChatRequestChatters* > ( packet );
               m_chatChannelManager->RequestChatters( request->chatChannelUuid, m_uuid );
            }
            break;
         case PacketChatToServer::ChatType_EnableDisableFiltering:
            {
               //PacketChatEnableFiltering* request = static_cast< PacketChatEnableFiltering* > ( packet );
               //m_chatChannelManager->AddUserToChannel( request->name, m_uuid );
            }
            break;
         case PacketChatToServer::ChatType_ListAllMembersInChatChannel:
            {
               PacketChatListAllMembersInChatChannel* request = static_cast< PacketChatListAllMembersInChatChannel* > ( packet );
               m_chatChannelManager->RequestAllUsersInChatChannel( request->chatChannelUuid, request->fullList, m_uuid );
            }
            break;
         case PacketChatToServer::ChatType_AdminLoadAllChannels:
            {
               //PacketChatAdminLoadAllChannels* request = static_cast< PacketChatAdminLoadAllChannels* > ( packet );
               m_chatChannelManager->LoadAllChannels( m_uuid );
            }
            break;
         case PacketChatToServer::ChatType_AdminRequestChatChannelList:
            {
               PacketChatAdminRequestChatChannelList* request = static_cast< PacketChatAdminRequestChatChannelList* > ( packet );
               m_chatChannelManager->RequestChatChannelList( m_uuid, request->isFullList );
            }
            break;
         case PacketChatToServer::ChatType_AdminRequestUsersList:
            {
               PacketChatAdminRequestUsersList* request = static_cast< PacketChatAdminRequestUsersList* > ( packet );
               m_chatChannelManager->RequestUsersList( m_uuid, request->isFullList );
            }
            break;*/
         }
      }
      break;
   }

   return false;
}


//------------------------------------------------------------------------------------------------

void     ChatUser::QueryChatChannelHistory( const string& channelUuid, int numRecords, int startingIndex  )
{
   // SELECT chat.text, user.name, chat.timestamp, chat.game_turn
   // FROM chat as chat, user as user
   // where chat_channel_id='ABCDEFGHIJKLMNOP' 
   // AND chat.user_id_sender=user.uuid

   if( channelUuid.size() == 0 )
   {
      m_chatServer->SendErrorToClient( m_connectionId, PacketErrorReport::ErrorType_BadChatChannel );
      return;
   }

   if( numRecords < 1 )
      numRecords = 1;
   if( numRecords > 100 )
      numRecords = 100;

   if( startingIndex < 0 )
      startingIndex = 0;
   if( startingIndex > 1000000 )
      startingIndex = 1000000;

   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id = m_connectionId;
   dbQuery->lookup = QueryType_ChatChannelHistory;

   string queryString = "SELECT chat.text, users.user_name, users.uuid, chat.game_turn, chat.timestamp FROM chat_message AS chat, users WHERE chat.user_id_sender=users.uuid AND chat_channel_id='%s'";
   queryString += " ORDER BY chat.timestamp DESC LIMIT ";
   queryString += boost::lexical_cast< string >( startingIndex );
   queryString += ",";
   queryString += boost::lexical_cast< string >( numRecords );

   dbQuery->query = queryString;
   dbQuery->escapedStrings.insert( channelUuid );
   dbQuery->meta = channelUuid;

   m_chatServer->AddQueryToOutput( dbQuery, m_connectionId, false );
}

//------------------------------------------------------------------------------------------------

void     ChatUser::QueryChatP2PHistory( const string& userUuid, int numRecords, int startingIndex )
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
      m_chatServer->SendErrorToClient( m_connectionId, PacketErrorReport::ErrorType_NoChatHistoryExistsForThisUser  );
      return;
   }

   if( numRecords < 1 )
      numRecords = 1;
   if( numRecords > 100 )
      numRecords = 100;

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
                  ") "\
                  "ORDER BY timestamp DESC LIMIT ";
   queryString += boost::lexical_cast< string >( startingIndex );
   queryString += ",";
   queryString += boost::lexical_cast< string >( numRecords );

   dbQuery->query = queryString;
   dbQuery->escapedStrings.insert( userUuid );
   dbQuery->escapedStrings.insert( m_uuid );
   dbQuery->escapedStrings.insert( m_uuid );
   dbQuery->escapedStrings.insert( userUuid );
   dbQuery->meta = userUuid;

   m_chatServer->AddQueryToOutput( dbQuery, m_connectionId, false );
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

   m_chatServer->AddQueryToOutput( dbQuery, m_connectionId, false );
}

//---------------------------------------------------------

bool     ChatUser::HandleDbResult( PacketDbQueryResult * dbResult )
{
   BasePacket* packet = static_cast<BasePacket*>( dbResult );
   PacketCleaner cleaner( packet );
   //const int maxNumMessagesPerPacket = 20;

   switch( dbResult->lookup )
   {
     /* case QueryType_UserFriendsList:
         {
            m_userFriendsComplete = true;

            availableFriends.clear();

            UserTable            enigma( dbResult->bucket );
            UserTable::iterator  it = enigma.begin();
            while( it != enigma.end() )
            {
               UserTable::row    row = *it++;
               string   name =   row[ TableUser::Column_name ];
               string   uuid =   row[ TableUser::Column_uuid ];
              
               availableFriends.push_back( KeyValueString( uuid, name ) );
            }

            InformUserOfSuccessfulLogin();
         }
         break;*/
    /*  case QueryType_UserChannelList:
         {
            m_userChannelsComplete = true;
            availableChannels.clear();

            //const int maxNumMessagesPerPacket = 2;
            ChatChannelTable              enigma( dbResult->bucket );
            ChatChannelTable::iterator    it = enigma.begin();
            while( it != enigma.end() )
            {
               ChatChannelTable::row      row = *it++;
               string   name =         row[ TableChatChannel::Column_name ];
               string   uuid =         row[ TableChatChannel::Column_uuid ];
               int      gameType =     boost::lexical_cast< int >( row[ TableChatChannel::Column_game_type ] );
               int      gameId =       boost::lexical_cast< int >( row[ TableChatChannel::Column_game_instance_id ] );
               //int      numNewChats =  boost::lexical_cast< int >( row[ TableChatChannel::Column_record_count ] );
               int numNewChats = 0;
               bool     isActive = boost::lexical_cast< bool >( row[ TableChatChannel::Column_is_active ] );

               availableChannels.insert( uuid, ChannelInfo( name, uuid, gameType, gameId, numNewChats, isActive ) );
            }

            InformUserOfSuccessfulLogin();
         }
         break;*/
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

void     ChatUser::ChatReceived( const string& message, const string& senderUuid, const string& senderDisplayName, string channelUuid, string timeStamp )
{
   PacketChatToClient* packet = new PacketChatToClient;
   packet->message = message;
   packet->userUuid = senderUuid;
   packet->userName = senderDisplayName;
   packet->channelUuid = channelUuid;
   packet->timeStamp = timeStamp;

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
      entry.senderUuid =      row[ TableChat::Column_user_id_sender ];
      entry.chatChannelUuid = row[ TableChat::Column_chat_channel_id ];
      if( row[ TableChat::Column_game_turn ] != "NULL" )
      {
         entry.isGamechannel =  true;
      }
      else
      {
         entry.isGamechannel = false;
      }
      entry.numMessages = 1;

      vector< MissedChatChannelEntry > ::iterator it = Find( entry, history );
      if( it == history.end() )
      {
         history.push_back( entry );
      }
      else
      {
         ++(it->numMessages);
      }
   }
   SendChatHistorySinceLastLogin( history );
}

//---------------------------------------------------------------

void     ChatUser::SendChatChannelHistoryToClient( PacketDbQueryResult * dbResult )
{
   if( dbResult->bucket.bucket.size() == 0 )
   {
      m_chatServer->SendErrorToClient( m_connectionId, PacketErrorReport::ErrorType_NoChatHistoryExistsOnSelectedChannel );
      return;
   }
   const int maxNumMessagesPerPacket = 20;

   PacketChatHistoryResult* result = new PacketChatHistoryResult;
   result->chatChannelUuid = dbResult->meta;

   SimpleChatTable              enigma( dbResult->bucket );
   SimpleChatTable::iterator    it = enigma.begin();
   while( it != enigma.end() )
   {
      ChatTable::row       row = *it++;
      ChatEntry            entry;
      entry.message =      row[ SimpleChat::Column_text ];
      entry.userName =     row[ SimpleChat::Column_user_name ];
      entry.useruuid =     row[ SimpleChat::Column_user_uuid ];
      entry.timestamp =    row[ SimpleChat::Column_timestamp ];

      if( row[ SimpleChat::Column_game_turn ] != "NULL" )
      {
         entry.gameTurn =  boost::lexical_cast< int >( row[ SimpleChat::Column_game_turn ] );
      }
      else
      {
         entry.gameTurn  = 0;
      }
     
      result->chat.push_back( entry );
      // send once we have so many items
      if( ( result->chat.size() % maxNumMessagesPerPacket) == 0 )
      {
         SendMessageToClient( result );
         if( it != enigma.end() )
         {
            result = new PacketChatHistoryResult;
            result->chatChannelUuid = dbResult->meta;
         }
         else 
         {
            result = NULL;
         }
      }
   }
   // send the 'residual'
   if( result && result->chat.size() )
   {
      SendMessageToClient( result );
   }
}

//---------------------------------------------------------------

void     ChatUser::SendChatp2pHistoryToClient( PacketDbQueryResult * dbResult )
{
   if( dbResult->bucket.bucket.size() == 0 )
   {
      m_chatServer->SendErrorToClient( m_connectionId, PacketErrorReport::ErrorType_NoChatHistoryExistsForThisUser );
      return;
   }
   const int maxNumMessagesPerPacket = 20;
   PacketChatHistoryResult* result = new PacketChatHistoryResult;
   result->userUuid = dbResult->meta;

   SimpleChatTable              enigma( dbResult->bucket );
   SimpleChatTable::iterator    it = enigma.begin();
   while( it != enigma.end() )
   {
      ChatTable::row       row = *it++;
      ChatEntry            entry;
      entry.message =      row[ SimpleChat::Column_text ];
      entry.userName =     row[ SimpleChat::Column_user_name ];
      entry.useruuid =     row[ SimpleChat::Column_user_uuid ];
      entry.timestamp =    row[ SimpleChat::Column_timestamp ];

      if( row[ SimpleChat::Column_game_turn ] != "NULL" )
      {
         entry.gameTurn =  boost::lexical_cast< int >( row[ SimpleChat::Column_game_turn ] );
      }
      else
      {
         entry.gameTurn  = 0;
      }
     
      result->chat.push_back( entry );
      // send once we have so many items
      if( ( result->chat.size() % maxNumMessagesPerPacket) == 0 )
      {
         SendMessageToClient( result );
         if( it != enigma.end() )
         {
            result = new PacketChatHistoryResult;
            result->userUuid = dbResult->meta;
         }
         else 
         {
            result = NULL;
         }
      }
   }
   // send the 'residual'
   if( result && result->chat.size() )
   {
      SendMessageToClient( result );
   }
}
//---------------------------------------------------------------

void     ChatUser::SendChatHistorySinceLastLogin( const vector< MissedChatChannelEntry >& history )
{
   const int maxNumMessagesPerPacket = 20;
   if( history.size() == 0 )
   {
      m_chatServer->SendErrorToClient( m_connectionId, PacketErrorReport::ErrorType_NoChatHistoryExistsForThisUser );
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

bool     ChatUser::NotifyAddedToChannel( const string& channelUuid, const string& userUuid, bool wasSuccessful )
{
   PacketChatAddUserToChatChannelResponse* response = new PacketChatAddUserToChatChannelResponse;
   response->chatChannelUuid = channelUuid;
   response->userUuid = userUuid;
   response->success = wasSuccessful;

   SendMessageToClient( response );

   // send notification then go to the db and pull all groups again and send to client
   if( userUuid == m_uuid )
   {
      RequestChatChannels();
   }
   
   return true;
}

//---------------------------------------------------------------

bool     ChatUser::NotifyYouWereAddedToChannel( const string& channelUuid )
{
   return true;
}

//---------------------------------------------------------------

bool     ChatUser::NotifyRemovedFromChannel( const string& channelName, const string& channelUuid, bool wasSuccessful )
{
   PacketChatRemoveUserFromChatChannelResponse* response = new PacketChatRemoveUserFromChatChannelResponse;
   response->chatChannelUuid = channelUuid;
   response->userUuid = m_uuid;
   response->success = wasSuccessful;

   SendMessageToClient( response );
   RequestChatChannels();

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
      m_chatChannelManager->UserSendP2PChat( m_uuid, sendeeUuid, message );
      return true;
   }
   else if( channelUuid.size() > 0 )
   {
      m_chatChannelManager->UserSendsChatToChannel( m_uuid, channelUuid, message, gameTurn );
      return true;
   }
   return false;
}

//---------------------------------------------------------------

bool     ChatUser::SendMessageToClient( BasePacket* packet ) const
{
   PacketGatewayWrapper* wrapper = new PacketGatewayWrapper();
   wrapper->SetupPacket( packet, m_connectionId );

   m_chatServer->SendMessageToClient( wrapper, m_connectionId );
   return true;
}

//------------------------------------------------------------------------------------------------

void     ChatUser::RequestChatChannels()
{
   ChannelKeyValue            availableChannels;
   m_chatChannelManager->GetChatChannels( m_uuid, availableChannels );

   PacketChatChannelList* packetChannels = new PacketChatChannelList;// this may need to be moved into a separate function
   packetChannels->channelList = availableChannels;

   SendMessageToClient( packetChannels );
}

//------------------------------------------------------------------------------------------------

bool     ChatUser::SendErrorMessage( PacketErrorReport::ErrorType error )
{
   return m_chatServer->SendErrorToClient( m_connectionId, error );
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////