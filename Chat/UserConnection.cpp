// UserConnection.cpp

#include "../NetworkCommon/Packets/BasePacket.h"
#include "../NetworkCommon/Packets/ChatPacket.h"
#include "../NetworkCommon/Packets/DbPacket.h"
#include "../NetworkCommon/Packets/LoginPacket.h"

#include "DiplodocusChat.h"
#include "chatChannelManager.h"
#include "../NetworkCommon/Utils/TableWrapper.h"

#include <boost/lexical_cast.hpp>

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

DiplodocusChat*      UserConnection::  m_chatServer = NULL;
ChatChannelManager*  UserConnection::  m_chatChannelManager = NULL;

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

UserConnection::UserConnection( U32 connectionId ) : 
                  m_userDbId( 0 ),
                  m_connectionId( connectionId ),
                  m_isUserDbLookupPending( false ),
                  m_badConnection( false ),
                  m_userFriendsComplete( false ),
                  m_userChannelsComplete( false )
{
}

//---------------------------------------------------------------

bool     UserConnection::SendChat( const string& message, const string& senderUuid, const string& senderDisplayName, string channelUuid, string timeStamp )
{
   PacketChatToClient* packet = new PacketChatToClient;
   packet->message = message;
   packet->userUuid = senderUuid;
   packet->userName = senderDisplayName;
   packet->channelUuid = channelUuid;
   packet->timeStamp = timeStamp;

   PacketGatewayWrapper* wrapper = new PacketGatewayWrapper();
   wrapper->SetupPacket( packet, m_connectionId );
  /* wrapper->connectionId = m_connectionId;
   wrapper->pPacket = packet;*/

   m_chatServer->AddPacketFromUserConnection( wrapper, m_connectionId );

   return true;
}

//---------------------------------------------------------------

bool     UserConnection::AddInputChainData( BasePacket* packet, U32 connectionId ) 
{
   if( m_connectionId == 0 )// just fix ourselves up to help with packaging later.
   {
      m_connectionId = connectionId;
   }

   // packets are likely to be either packets from the gateway or packets from the DB. Eventually, game packets will make their way here too.
   if( packet->packetType == PacketType_GatewayWrapper )
   {
      PacketGatewayWrapper* wrapper = static_cast< PacketGatewayWrapper* >( packet );
      m_packetsIn.push_back( wrapper->pPacket );
      delete packet;
   }
   else
   {
      m_packetsIn.push_back( packet );
   }

   return true;
}

//---------------------------------------------------------------

bool     UserConnection::HandleInvalidSetup()
{
   while( m_packetsIn.size() > 0 )
   {
     BasePacket* packet = m_packetsIn.front();
     delete packet;
     m_packetsIn.pop_front();
   }

   PacketGatewayWrapper* wrapper = new PacketGatewayWrapper;
   wrapper->SetupPacket( new PacketLogout(), m_connectionId );
   /*gatewayPacket->connectionId = m_connectionId;
   gatewayPacket->pPacket = new PacketLogout();*/

   m_chatServer->AddPacketFromUserConnection( wrapper, m_connectionId );// the gateway will tell us when the user has been disconnected.

   return true;
}

//---------------------------------------------------------------

void  UserConnection::CheckForProperChatServerPointer()
{
   if( m_chatServer == NULL )
   {
      assert( 0 );
   }
}

//---------------------------------------------------------------

void     UserConnection::UpdateInwardPacketList()
{
   if( m_packetsIn.size() == 0 || m_badConnection == true )
   {
      return;
   }

   // todo, we need to fix this behavior. This is flawed. Packets can come in immediately after login, potentially.
   if( m_isUserDbLookupPending == true && 
      m_packetsIn.front()->packetType != PacketType_DbQuery )
   {
      m_chatServer->Log("We are waiting for login to finish with a db query and we are receiving other packets", 2 );
      return;
   }

   CheckForProperChatServerPointer();

   // you must be logged in to accept any packet other than a login packet.
   if( IsLoggedIn() == false && 
      m_packetsIn.front()->packetType != PacketType_Login )
   {
      HandleInvalidSetup();
   }

   if( m_chatServer )
   {
      while( m_packetsIn.size() > 0 )
      {
         BasePacket* packet = m_packetsIn.front();
         ProcessPacket( packet );
         m_packetsIn.pop_front();
         delete packet;
      }
   }
}

//------------------------------------------------------------------------------------------------

bool     UserConnection::PrepInitialLogin()
{
   // step 1, request the user info.
   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id = m_connectionId;
   dbQuery->lookup = QueryType_UserLoginInfo;

   string queryString = "SELECT * FROM users WHERE user_name='%s'" ;
   dbQuery->query = queryString;
   dbQuery->escapedStrings.insert( m_userName );
   
   m_isUserDbLookupPending = true;
   m_chatServer->AddPacketFromUserConnection( dbQuery, m_connectionId );

   return true;
}

//------------------------------------------------------------------------------------------------

bool     UserConnection::RequestExtraUserInfo()
{
   //RequestFriends();
   RequestChatChannels();

   return true;
}

//------------------------------------------------------------------------------------------------
/*
bool  UserConnection::RequestFriends()
{
   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id = m_connectionId;
   dbQuery->lookup = QueryType_UserFriendsList;

  // string queryString = "SELECT * FROM users WHERE users.uuid IN (SELECT friends.userid2 as uuid FROM friends WHERE friends.userid1 = '" ;
  // queryString += m_userDbId;
  // queryString += "' union SELECT friends.userid1 as uuid FROM friends WHERE friends.userid2 = '";

   string queryString = "SELECT * FROM users INNER JOIN friends ON users.user_id=friends.userid2 WHERE friends.userid1=";// get my list of friends
   queryString += boost::lexical_cast< string >( m_userDbId );
   // WHERE FROM 
   dbQuery->query = queryString;

   m_chatServer->AddPacketFromUserConnection( dbQuery, m_connectionId );

   return true;
}*/

//------------------------------------------------------------------------------------------------

bool  UserConnection::RequestChatChannels()
{
 /*  PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id = m_connectionId ;
   dbQuery->lookup = QueryType_UserChannelList;

   string queryString = "SELECT * FROM chat_channel WHERE uuid IN ";
   queryString += "(SELECT user_join_chat_channel.channel_uuid FROM user_join_chat_channel WHERE user_join_chat_channel.user_uuid='";
   queryString += m_uuid;
   queryString += "')";
   dbQuery->query = queryString;*/

  
  /* string queryString = "SELECT channel.id,channel.name,channel.uuid,channel.is_active,channel.max_num_users,channel.game_type,channel.game_instance_id,channel.date_created, COUNT(distinct msg.id) AS record_count " \
                        " FROM playdek.chat_channel AS channel INNER JOIN playdek.chat_message AS msg " \
                        " ON channel.uuid=msg.chat_channel_id  WHERE channel.uuid IN " \
                        "(SELECT user_join_chat_channel.channel_uuid FROM user_join_chat_channel " \
                        "WHERE user_join_chat_channel.user_uuid='%s') AND msg.timestamp>='%s'";*/

   /*dbQuery->escapedStrings.insert( m_uuid );
   dbQuery->escapedStrings.insert( m_lastLoginTime );

   dbQuery->query = queryString;

   m_chatServer->AddPacketFromUserConnection( dbQuery, m_connectionId );*/
   m_chatChannelManager->GetChatChannels( m_uuid, availableChannels );

   m_userChannelsComplete = true;

   //availableChannels.insert( uuid, ChannelInfo( name, uuid, gameType, gameId, numNewChats, isActive ) );

   InformUserOfSuccessfulLogin();

   return true;
}

//------------------------------------------------------------------------------------------------

void     UserConnection::GetChatChannelDigest( const string& channelUuid, int numRecords, int startingIndex  )
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

   m_chatServer->AddPacketFromUserConnection( dbQuery, m_connectionId );
}

//------------------------------------------------------------------------------------------------

void     UserConnection::GetChatP2PDigest( const string& userUuid, int numRecords, int startingIndex )
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

   m_chatServer->AddPacketFromUserConnection( dbQuery, m_connectionId );
}

//------------------------------------------------------------------------------------------------

void     UserConnection::GetAllChatHistroySinceLastLogin()
{
 /*  SELECT * FROM chat where chat_channel_id in 

      (SELECT chat_channel.uuid from chat_channel
      join user_join_chat_channel
      on user_join_chat_channel.channel_uuid = chat_channel.uuid 
      where user_join_chat_channel.user_uuid='ABCDEFGHIJKLMNOP' )

      and timestamp>='2013-04-23 14:50:38';*/

   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id = m_connectionId;
   dbQuery->lookup = QueryType_ChatHistoryMissedSinceLastLogin;

   string queryString = "SELECT * FROM chat_message AS chat WHERE chat_channel_id IN " \
      " (SELECT chat_channel.uuid from chat_channel"\
      " JOIN user_join_chat_channel"\
      " ON user_join_chat_channel.channel_uuid = chat_channel.uuid "\
      " WHERE user_join_chat_channel.user_uuid='";
   queryString += m_uuid;
   queryString += "' ) AND timestamp>='";
   queryString += m_lastLoginTime;//"2013-04-23 14:50:38";
   queryString += "'";
   dbQuery->query = queryString;

   m_chatServer->AddPacketFromUserConnection( dbQuery, m_connectionId );
}

//------------------------------------------------------------------------------------------------

bool     UserConnection::SendLoginStatus( bool wasSuccessful )
{
   PacketLoginToGateway* loginStatus = new PacketLoginToGateway();
   loginStatus->userName = m_userName;
   loginStatus->uuid = m_uuid;
   loginStatus->wasLoginSuccessful = wasSuccessful;

   PacketGatewayWrapper* wrapper = new PacketGatewayWrapper();
   wrapper->SetupPacket( loginStatus, m_connectionId );
   /*wrapper->connectionId = m_connectionId;
   wrapper->pPacket = loginStatus;*/

   m_chatServer->AddPacketFromUserConnection( wrapper, m_connectionId );

   if( wasSuccessful == false )
   {
      m_chatServer->SendErrorToClient( m_connectionId, PacketErrorReport::ErrorType_UserBadLogin );
   }
   return true;
}

//------------------------------------------------------------------------------------------------

bool     UserConnection::SendChatOut( const string& message, const string& userUuid, const string& channelUuid, U16 gameTurn )
{
 /*  if( m_currentChannel.size() == 0 && m_currentFriendUuid.size() == 0 )
   {
      m_chatServer->SendErrorToClient( PacketErrorReport::ErrorType_NoChatChannel, m_connectionId  );
      return false;
   }*/

   if( channelUuid.size() )
   {
      m_chatChannelManager->UserSendsChatToChannel( m_uuid, channelUuid, message, gameTurn );
   }
   else
   {
      m_chatChannelManager->UserSendsChatToUser( m_uuid, userUuid, message );
   }
   return true;
}

//------------------------------------------------------------------------------------------------
/*
bool     UserConnection::SetChatChannel( const string& channelUuid )
{
   bool found = false;
   ChannelKeyValue::const_KVIterator it = availableChannels.begin();
   while( it != availableChannels.end() )
   {
      if( it->key == channelUuid )
      {
         m_currentChannel = it->key;

         string text = " User ";
         text += CreatePrintablePair( m_uuid, m_userName );
         text += " changed chat channel to ";
         string name = it->value.channelName;
         text += CreatePrintablePair( it->key, name );
         m_chatServer->Log( text, 1 );

         found = true;
         break;
      }
      it ++;
   }

   if( found == true )
   {
      PacketChangeChatChannelToClient* packet = new PacketChangeChatChannelToClient();
      packet->userName = m_userName;
      packet->chatChannel = channelUuid;
      SendPacketToGateway( packet );
   }
   else
   {
      // we could go to the db and verify that the channel does exist?
      // ErrorType_NotAMemberOfThatChatChannel 
      m_chatServer->SendErrorToClient( PacketErrorReport::ErrorType_BadChatChannel, m_connectionId  );
   }

   return false;
}*/

//------------------------------------------------------------------------------------------------

void     UserConnection::SetupFromLogin( U32 userId, const string& name, const string& uuid, const string& lastLoginTime )
{
   m_userName = name;
   m_uuid = uuid;
   //m_loginKey = loginKey;
   m_lastLoginTime = lastLoginTime;
   m_userDbId = userId;

   RequestExtraUserInfo();
}

//------------------------------------------------------------------------------------------------

bool     UserConnection::SendPacketToGateway( BasePacket* packet ) const
{
   PacketGatewayWrapper* wrapper = new PacketGatewayWrapper();
   wrapper->SetupPacket( packet, m_connectionId );
  /* wrapper->connectionId = m_connectionId;
   wrapper->pPacket = packet;*/

   m_chatServer->AddPacketFromUserConnection( wrapper, m_connectionId );
   return true;
}

//------------------------------------------------------------------------------------------------

bool     UserConnection::InformUserOfSuccessfulLogin()
{
   if( //m_userFriendsComplete == true && 
      m_userChannelsComplete == true )
   {
      m_chatServer->FinishedLogin( m_connectionId, m_uuid );

      SendListOfChannelsToGateway();
      //SendListOfFriendsToGateway();
      GetAllChatHistroySinceLastLogin();

   }
   return true;
}

//------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------
/*
bool  UserConnection::SendListOfFriendsToGateway()
{
   const int maxUsersSentAtOnce = 50;
   PacketFriendsList* packetFriends = NULL;
   KeyValueVector::iterator it = availableFriends.begin();
   while( it != availableFriends.end() )
   {
      if( packetFriends == NULL )
      {
         packetFriends = new PacketFriendsList;
      }
      KeyValueString& kv = *it++;
      packetFriends->friendList.insert( kv.key, kv.value );  // slow?
      if( packetFriends->friendList.size() >= maxUsersSentAtOnce )
      {
         SendPacketToGateway( packetFriends );
         packetFriends = NULL;
      }
   }
   if( packetFriends )
   {
      SendPacketToGateway( packetFriends );
   }

   return true;
}*/

//------------------------------------------------------------------------------------------------

bool  UserConnection::SendListOfChannelsToGateway()
{
   PacketChatChannelList* packetGroups = new PacketChatChannelList;
   packetGroups->channelList = availableChannels;
   SendPacketToGateway( packetGroups );  // slow?

   return true;
}

//------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------

bool     UserConnection::ProcessPacket( BasePacket* packet )
{
   switch( packet->packetType )
   {
   case PacketType_DbQuery:
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
      break;
   case PacketType_UserInfo:
      {
         switch( packet->packetSubType )
         {
         case PacketUserInfo::InfoType_FriendsListRequest:
            {
               //SendListOfFriendsToGateway();
            }
            break;
         case PacketUserInfo::InfoType_ChatChannelListRequest:
            {
               SendListOfChannelsToGateway();
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
               SendChatOut( chat->message, chat->userUuid, chat->channelUuid, chat->gameTurn );
            }
            break;
         case PacketChatToServer::ChatType_ChangeChatChannel:
            {
               PacketChangeChatChannel* channelChange = static_cast< PacketChangeChatChannel* >( packet );
               //if( channelChange->chatChannelUuid != m_currentChannel )// ignore changes to the same channel
               {
                  //SetChatChannel( channelChange->chatChannelUuid );
               }
            }
            break;
         case PacketChatToServer::ChatType_RequestHistory:
            {
               PacketChatHistoryRequest* request = static_cast< PacketChatHistoryRequest* > ( packet );
               if( request->chatChannelUuid.size() )
               {
                  GetChatChannelDigest( request->chatChannelUuid, request->numRecords, request->startingIndex );
               }
               else
               {
                  GetChatP2PDigest( request->userUuid, request->numRecords, request->startingIndex );
               }
            }
            break;
         case PacketChatToServer::ChatType_RequestHistorySinceLastLogin:
            {
               PacketChatMissedHistoryRequest* request = static_cast< PacketChatMissedHistoryRequest* > ( packet );
               GetAllChatHistroySinceLastLogin();
            }
            break;
            //-----------------------------------------------
         case PacketChatToServer::ChatType_CreateChatChannel:
            {
               PacketChatCreateChatChannel* request = static_cast< PacketChatCreateChatChannel* > ( packet );
               m_chatChannelManager->CreateNewChannel( request->name, m_uuid );
            }
            break;
         case PacketChatToServer::ChatType_DeleteChatChannel:
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
               PacketChatEnableFiltering* request = static_cast< PacketChatEnableFiltering* > ( packet );
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
               PacketChatAdminLoadAllChannels* request = static_cast< PacketChatAdminLoadAllChannels* > ( packet );
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
            break;
         }
      }
      break;
   }

   return false;
}

//------------------------------------------------------------------------------------------------

bool     UserConnection::HandleDbQueryResult( BasePacket* packet )
{
   const int maxNumMessagesPerPacket = 20;

   PacketDbQueryResult* dbResult = static_cast< PacketDbQueryResult* >( packet );

   switch( dbResult->lookup )
   {
      case QueryType_UserLoginInfo:
         {
            if( dbResult->successfulQuery == false || dbResult->bucket.bucket.size() == 0 )// no records found
            {
               //assert( 0 );// begin teardown. Inform gateway that user is not available. Gateway will teardown the connection
               // and send a reply to this game instance.
               string str = "User not valid and db query failed, userName: ";
               str += m_userName;
               str += ", uuid: ";
               str += m_uuid;
               m_chatServer->Log( str, 4 );
               m_badConnection = true;
               return false;
            }
            m_isUserDbLookupPending = false;
            
            UserTable            enigma( dbResult->bucket );
            UserTable::row       row = *enigma.begin();
            m_uuid =             row[ TableUser::Column_uuid ];
            // note that we are using logout for our last login time.
            m_lastLoginTime =    row[ TableUser::Column_last_logout_time ];
            

            RequestExtraUserInfo();
         }
         break;
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
            if( dbResult->bucket.bucket.size() == 0 )
            {
               m_chatServer->SendErrorToClient( m_connectionId, PacketErrorReport::ErrorType_NoChatHistoryExistsOnSelectedChannel );
               break;
            }

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
                  SendPacketToGateway( result );
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
               SendPacketToGateway( result );
            }

         }
         break;
      case QueryType_ChatP2PHistory:
         {
            if( dbResult->bucket.bucket.size() == 0 )
            {
               m_chatServer->SendErrorToClient( m_connectionId, PacketErrorReport::ErrorType_NoChatHistoryExistsForThisUser );
               break;
            }
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
                  SendPacketToGateway( result );
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
               SendPacketToGateway( result );
            }
         }
         break;
      case QueryType_ChatHistoryMissedSinceLastLogin:
         {
            if( dbResult->bucket.bucket.size() == 0 )
            {
               m_chatServer->SendErrorToClient( m_connectionId, PacketErrorReport::ErrorType_NoChatHistoryExistsForThisUser );
               break;
            }

            const int maxNumMessagesPerPacket = 20;
            PacketChatMissedHistoryResult* result = new PacketChatMissedHistoryResult;

            ChatTable              enigma( dbResult->bucket );
            ChatTable::iterator    it = enigma.begin();
            while( it != enigma.end() )
            {
               ChatTable::row       row = *it++;
               FullChatChannelEntry    entry;
               entry.message =         row[ TableChat::Column_text ];
               entry.senderUuid =      row[ TableChat::Column_user_id_sender ];
               entry.chatChannelUuid = row[ TableChat::Column_chat_channel_id ];
               entry.timeStamp =       row[ TableChat::Column_timestamp ];
               if( row[ TableChat::Column_game_turn ] != "NULL" )
               {
                  entry.gameTurn =  boost::lexical_cast< int >( row[ TableChat::Column_game_turn ] );
               }
               else
               {
                  entry.gameTurn  = 0;
               }
              /* if( row[ TableChat::Column_game_instance_id ] != "NULL" )
               {
                  entry.gameTurn =  boost::lexical_cast< int >( row[ TableChat::Column_game_instance_id ] );
               }
               else
               {
                  entry.gameTurn  = 0;
               }*/
              
               result->history.push_back( entry );
               // send once we have so many items
               if( ( result->history.size() % maxNumMessagesPerPacket) == 0 )
               {
                  SendPacketToGateway( result );
                  if( it != enigma.end() )
                  {
                     result = new PacketChatMissedHistoryResult;
                  }
                  else 
                  {
                     result = NULL;
                  }
               }
            }
            // send the 'residual'
            if( result && result->history.size() )
            {
               SendPacketToGateway( result );
            }
         }
         break;
   }

   return true;
}

//---------------------------------------------------------------

void     UserConnection::UpdateOutwardPacketList()
{
}

//---------------------------------------------------------------

void     UserConnection::Update()
{
   UpdateInwardPacketList();
   UpdateOutwardPacketList();
}

//---------------------------------------------------------------

bool     UserConnection::IsLoggedIn() const
{
   if( m_uuid.size() == 0 || m_userName.size() == 0 || m_connectionId == 0 )
   {
      return false;
   }
   return true;
}

//---------------------------------------------------------------

bool     UserConnection::NotifyChannelAdded( const string& channelName, const string& channelUuid, bool success )
{
   PacketChatCreateChatChannelResponse* response = new PacketChatCreateChatChannelResponse;
   response->name = channelName;
   response->uuid = channelUuid;
   response->successfullyCreated = success;

   SendPacketToGateway( response );

   return true;
}

//---------------------------------------------------------------

bool     UserConnection::NotifyChannelMovedToInactive( const string& channelUuid, int numremoved, bool success )
{
   PacketChatDeleteChatChannelResponse* response = new PacketChatDeleteChatChannelResponse;
   response->uuid = channelUuid;
   response->successfullyDeleted = success;
   response->numUsersRemoved = 0;

   SendPacketToGateway( response );

   return true;
}

//---------------------------------------------------------------

bool  UserConnection::NotifyChannelRemoved( const string& channelUuid, int numremoved )
{
   PacketChatDeleteChatChannelResponse* response = new PacketChatDeleteChatChannelResponse;
   response->uuid = channelUuid;
   response->successfullyDeleted = true;
   response->numUsersRemoved = numremoved;

   SendPacketToGateway( response );

   return true;
}

//---------------------------------------------------------------

bool  UserConnection::NotifyAllChannelsLoaded( bool loaded )
{
   PacketChatAdminLoadAllChannelsResponse* response = new PacketChatAdminLoadAllChannelsResponse;
   response->success = loaded;

   SendPacketToGateway( response );

   return true;
}

//---------------------------------------------------------------

bool     UserConnection::NotifyUserStatusHasChanged( const string& userName, const string& userUuid, int statusChange )
{
   PacketChatUserStatusChangeBase* notification = new PacketChatUserStatusChangeBase;
   notification->userName = userName;
   notification->uuid = userUuid;
   notification->statusChange = statusChange;

   SendPacketToGateway( notification );

   return true;
}

//---------------------------------------------------------------

bool     UserConnection::NotifyAddedToChannel( const string& channelUuid, const string& userUuid, bool wasSuccessful )
{
   PacketChatAddUserToChatChannelResponse* response = new PacketChatAddUserToChatChannelResponse;
   response->chatChannelUuid = channelUuid;
   response->userUuid = userUuid;
   response->success = wasSuccessful;

   SendPacketToGateway( response );

   // send notification then go to the db and pull all groups again and send to client
   if( userUuid == m_uuid )
   {
      RequestChatChannels();
   }
   
   return true;
}

//---------------------------------------------------------------

bool     UserConnection::NotifyRemovedFromChannel( const string& channelName, const string& channelUuid, bool wasSuccessful )
{
   PacketChatRemoveUserFromChatChannelResponse* response = new PacketChatRemoveUserFromChatChannelResponse;
   response->chatChannelUuid = channelUuid;
   response->userUuid = m_uuid;
   response->success = wasSuccessful;

   SendPacketToGateway( response );
   RequestChatChannels();

   return true;
}

//---------------------------------------------------------------

bool     UserConnection::SendListOfAllChatChannels( SerializedKeyValueVector< ChannelInfo >& chatChannelsAndIds )
{
   PacketChatAdminRequestChatChannelListResponse* response = new PacketChatAdminRequestChatChannelListResponse;
   response->chatChannels = chatChannelsAndIds;

   SendPacketToGateway( response );

   return true;
}

//---------------------------------------------------------------

bool     UserConnection::SendListOfAllUsers( SerializedKeyValueVector< string >& usersAndIds )
{
   PacketChatAdminRequestUsersListResponse* response = new PacketChatAdminRequestUsersListResponse;
   response->users = usersAndIds;

   SendPacketToGateway( response );

   return true;
}

//---------------------------------------------------------------

bool     UserConnection::SendListOfChatters( const string& channelUuid, SerializedKeyValueVector< string >& usersAndIds )
{
   PacketChatRequestChattersResponse* response = new PacketChatRequestChattersResponse;
   response->chatChannelUuid = channelUuid;
   response->userList = usersAndIds;  // slow?
   SendPacketToGateway( response );

   return true;
}

//---------------------------------------------------------------

bool     UserConnection::SendListOfAllUsersInChatChannel( const string& channelUuid, SerializedKeyValueVector< string >& usersAndIds )
{
   PacketChatListAllMembersInChatChannelResponse* response = new PacketChatListAllMembersInChatChannelResponse;
   response->chatChannelUuid = channelUuid;
   response->userList = usersAndIds;  // slow?
   SendPacketToGateway( response );

   return true;
}


//---------------------------------------------------------------
//---------------------------------------------------------------
