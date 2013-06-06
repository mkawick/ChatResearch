// ChatChannelManager.cpp

#include <map>
#include <string>
#include <list>
#include <vector>
#include <deque>
using namespace std;

#include "ChatChannelManager.h"
#include "DiplodocusChat.h"
#include "UserConnection.h"
#include <boost/lexical_cast.hpp>
#include <mysql.h>

#include "../NetworkCommon/Utils/TableWrapper.h"
#include "../NetworkCommon/Packets/GamePacket.h"
#include "../NetworkCommon/Packets/ServerToServerPacket.h"

DiplodocusChat*     ChatChannelManager::m_chatServer = NULL;
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

ChatChannelManager::ChatChannelManager()
{
}

ChatChannelManager::~ChatChannelManager()
{
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------

bool     ChatChannelManager::AddInputChainData( BasePacket* packet ) // usually a query
{
   // packets are likely to be either packets from the gateway or packets from the DB. Eventually, game packets will make their way here too.
   if( packet->packetType == PacketType_GatewayWrapper )
   {
      assert( 0 );// should never happen
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

//---------------------------------------------------------------------

void     ChatChannelManager::Update()
{
   while( m_packetsIn.size() > 0 )
   {
      BasePacket* packet = m_packetsIn.front();
      ProcessPacket( packet );
      m_packetsIn.pop_front();
      delete packet;
   }
}

//---------------------------------------------------------------------

void     ChatChannelManager::PackageAndSendToDiplodocusChat( BasePacket* packet, U32 serverId )
{
   PacketServerJobWrapper* wrapper = new PacketServerJobWrapper;
   wrapper->pPacket = packet;
   wrapper->serverId = serverId;

   m_chatServer->AddOutputChainData( wrapper, serverId );
}

//---------------------------------------------------------------------

bool     ChatChannelManager::ProcessPacket( BasePacket* packet )
{
   switch( packet->packetType )
   {
   case PacketType_DbQuery:
      {
         switch( packet->packetSubType )
         {
         case BasePacketDbQuery::QueryType_Result:
            {
               PacketDbQueryResult* dbResult = reinterpret_cast< PacketDbQueryResult* >( packet );
               int jobId = dbResult->lookup;
               string lookupUuid = dbResult->meta;
               DbJobIterator iter;
               bool success = FindDbJob( jobId, m_pendingDbJobs, iter );
               if( success )
               {
                  FinishJob( dbResult, *iter );
                  m_pendingDbJobs.erase( iter );
               }
            }
            break;
         }
      }
      break;
   default:
      assert( 0 );// should not happen, we will begin to receive packets from other servers at some point.
   }

   return true;
}

//---------------------------------------------------------------------

void     ChatChannelManager::InformUsersAboutUserStatusChange( stringhash userHashLookup, const UserUuidSet& otherUsersToNotify, UserStatusChange status )
{
   UserUuidMapIterator userWhoseStatusChangedIter = m_userUuidMap.find( userHashLookup );
   if( userWhoseStatusChangedIter == m_userUuidMap.end() )
      return;

   ChatUser& changingUser = userWhoseStatusChangedIter->second;
   string userName = changingUser.username;
   string userUuid = changingUser.userUuid;  

   UserUuidSet::const_iterator othersIter = otherUsersToNotify.begin();
   while( othersIter != otherUsersToNotify.end() )
   {
      stringhash lookup = *othersIter++;
      UserUuidMapIterator creatorIter = m_userUuidMap.find( lookup );
      if( creatorIter != m_userUuidMap.end() )
      {
         UserConnection* connection = creatorIter->second.connection;
         connection->NotifyUserStatusHasChanged( userName, userUuid, status );
      }
      
   }
}

//---------------------------------------------------------------------

bool     ChatChannelManager::FindDbJob( int jobId, list< ChatChannelDbJob >& listOfJobs, DbJobIterator& iter )
{
   iter = listOfJobs.begin();
   while( iter != listOfJobs.end() )
   {
      if( iter->jobId == jobId )
      {
         return true;
      }
      iter++;
   }
   return false;
}

//---------------------------------------------------------------------

bool     ChatChannelManager::FinishJob( PacketDbQueryResult* dbResult, ChatChannelDbJob& job )
{
   //assert( dbResult->successfulQuery == true );

   switch( job.jobType )
   {
   case ChatChannelDbJob::JobType_Create:
      {
         UserUuidMapIterator creatorIter = m_userUuidMap.find( job.chatUserLookup );
         if( creatorIter != m_userUuidMap.end() )
         {
            UserConnection* connection = creatorIter->second.connection;
            connection->NotifyChannelAdded( job.name, job.uuid, dbResult->successfulQuery );
         }
         else if( job.serverId != 0 )
         {
            PacketCreateGameResponse* packet = new PacketCreateGameResponse;
            packet->name = job.name;
            packet->uuid = job.uuid;
            PackageAndSendToDiplodocusChat( packet, job.serverId );
         }
         
         if( dbResult->successfulQuery == true )
         {
            AddChatChannel( job.name, job.uuid );
         }
      }
      break;
   case ChatChannelDbJob::JobType_Delete:
      {
         UserUuidMapIterator creatorIter = m_userUuidMap.find( job.chatUserLookup );
         if( creatorIter != m_userUuidMap.end() )
         {
            UserConnection* connection = creatorIter->second.connection;
            connection->NotifyChannelMovedToInactive( job.uuid, 0, dbResult->successfulQuery );
         }
         else if( job.serverId != 0 )
         {
            PacketDeleteGameResponse* packet = new PacketDeleteGameResponse;

            stringhash channelHash = GenerateUniqueHash( job.uuid );
            ChannelMapIterator channelIter = m_channelMap.find( channelHash );
            if( channelIter != m_channelMap.end() )
            {
               packet->name = channelIter->second.name;
            }
            
            packet->uuid = job.uuid;
            PackageAndSendToDiplodocusChat( packet, job.serverId );
         }

         if( dbResult->successfulQuery == true )
         {
            RemoveChatChannel( job.uuid );
         }
      }
      break;
   case ChatChannelDbJob::JobType_Exists:
      {
         //job.
         if( dbResult->bucket.bucket.size() )
         {
            return false;
         }
      }
      break;
   case ChatChannelDbJob::JobType_AddUser:
      {
         // we will notify both the requestor and the target
         UserUuidMapIterator creatorIter = m_userUuidMap.find( job.chatUserLookup );// the person who requested the add
         UserConnection* connection = NULL;
         if( creatorIter != m_userUuidMap.end() )
         {
            connection = creatorIter->second.connection;
            connection->NotifyAddedToChannel( job.name, job.uuid, dbResult->successfulQuery );
         }
         
         if( job.serverId != 0 )// this is a server request
         {
            if( connection )
            {
               connection->SetChatChannel( job.uuid );
            }
            
            PacketAddUserToGameResponse* packet = new PacketAddUserToGameResponse;
            packet->gameUuid = job.name;
            packet->userUuid = job.uuid;
            packet->wasSuccessful = dbResult->successfulQuery;
            PackageAndSendToDiplodocusChat( packet, job.serverId );
         }
         
         // notify the person who requested the job be done of the success
         UserUuidMapIterator addedIter = m_userUuidMap.find( job.authUserLookup );// the user who was added
         if( addedIter != creatorIter && addedIter != m_userUuidMap.end() )
         {
            connection = addedIter->second.connection;
            connection->NotifyAddedToChannel( job.name, job.uuid, dbResult->successfulQuery );
         }

      }
      break;
   case ChatChannelDbJob::JobType_RemoveUser:
      {
         UserUuidMapIterator creatorIter = m_userUuidMap.find( job.chatUserLookup );
         UserConnection* connection = NULL;
         if( creatorIter != m_userUuidMap.end() )
         {
            connection = creatorIter->second.connection;
            connection->NotifyRemovedFromChannel( job.name, job.name, dbResult->successfulQuery );
         }
         
         // I don't know what to do with a user if the game server closes a game.
         if( job.serverId != 0 )// prep notification
         {
            PacketRemoveUserFromGameResponse* packet = new PacketRemoveUserFromGameResponse;
            packet->gameUuid = job.name;
            packet->userUuid = job.uuid;
            packet->wasSuccessful = dbResult->successfulQuery;
            PackageAndSendToDiplodocusChat( packet, job.serverId );
         }

         // notify the person who requested the job be done of the success
         UserUuidMapIterator removedIter = m_userUuidMap.find( job.authUserLookup );// the user who was removed
         if( removedIter != creatorIter && removedIter != m_userUuidMap.end() )
         {
            //m_userUuidMap.erase( removedIter );
            connection = removedIter->second.connection;
            connection->NotifyRemovedFromChannel( job.name, job.uuid, dbResult->successfulQuery );
         }
      }
      break;
   case ChatChannelDbJob::JobType_LoadAllChannels:
      {
         UserUuidMapIterator creatorIter = m_userUuidMap.find( job.chatUserLookup );
         if( creatorIter == m_userUuidMap.end() )
         {
            return false;
         }
         UserConnection* connection = creatorIter->second.connection;
         connection->NotifyAllChannelsLoaded( dbResult->successfulQuery );

         if( dbResult->successfulQuery == true )
         {
            ChatChannelTable              enigma( dbResult->bucket );
            ChatChannelTable::iterator    it = enigma.begin();
            while( it != enigma.end() )
            {
               ChatChannelTable::row      row = *it++;
               string   name = row[ TableChatChannel::Column_name ];
               string   uuid = row[ TableChatChannel::Column_uuid ];
              
               AddChatChannel( name, uuid );
            }
         };
      }
      break;
   case ChatChannelDbJob::JobType_SelectAllChannelsToSendToAuth:
      {
         UserUuidMapIterator creatorIter = m_userUuidMap.find( job.chatUserLookup );
         if( creatorIter == m_userUuidMap.end() && job.serverId == 0 )
         {
            return false;
         }
         UserConnection* connection = NULL;
         if( creatorIter != m_userUuidMap.end() )
         {
            connection = creatorIter->second.connection;
         }

         if( dbResult->successfulQuery == true )
         {
            SerializedKeyValueVector< ChannelInfo > chatChannelsAndIds;

            ChatChannelTable              enigma( dbResult->bucket );
            ChatChannelTable::iterator    it = enigma.begin();
            while( it != enigma.end() )
            {
               ChatChannelTable::row      row = *it++;
               string   name = row[ TableChatChannel::Column_name ];
               string   uuid = row[ TableChatChannel::Column_uuid ];
               bool     isActive = boost::lexical_cast< bool >( row[ TableChatChannel::Column_is_active ] );

               chatChannelsAndIds.insert( uuid, ChannelInfo( name, isActive ) );
            }
            if( connection )
               connection->SendListOfAllChatChannels( chatChannelsAndIds );
            else
            {
               PacketRequestListOfGamesResponse* packet = new PacketRequestListOfGamesResponse;
               packet->games = chatChannelsAndIds;
               PackageAndSendToDiplodocusChat( packet, job.serverId );
            }
         }
      }
      break;
   case ChatChannelDbJob::JobType_SelectAllUsersToSendToAuth:
      {
         UserUuidMapIterator creatorIter = m_userUuidMap.find( job.chatUserLookup );
         if( creatorIter == m_userUuidMap.end() )
         {
            return false;
         }
         UserConnection* connection = creatorIter->second.connection;
         
         if( dbResult->successfulQuery == true )
         {
            SerializedKeyValueVector< string > usersAndIds;

            UserTable            enigma( dbResult->bucket );
            UserTable::iterator  it = enigma.begin();
            while( it != enigma.end() )
            {
               UserTable::row    row = *it++;
               string   name = row[ TableUser::Column_name ];
               string   uuid = row[ TableUser::Column_uuid ];
               string   id = row[ TableUser::Column_id ];

               usersAndIds.insert( uuid, name );
            }
            connection->SendListOfAllUsers( usersAndIds );
         }
      }
      break;
   case  ChatChannelDbJob::JobType_FindChatters:
      {
         UserUuidMapIterator creatorIter = m_userUuidMap.find( job.chatUserLookup );
         if( creatorIter == m_userUuidMap.end() )
         {
            return false;
         }
         UserConnection* connection = creatorIter->second.connection;
         
         if( dbResult->successfulQuery == true )
         {
            SerializedKeyValueVector< string > usersAndIds;

            SimpleUserTable            enigma( dbResult->bucket );
            SimpleUserTable::iterator  it = enigma.begin();
            while( it != enigma.end() )
            {
               UserTable::row    row = *it++;
               string   name = row[ TableSimpleUser::Column_name ];
               string   uuid = row[ TableSimpleUser::Column_uuid ];
               string   id =  row[ TableSimpleUser::Column_id ];

               usersAndIds.insert( uuid, name );
            }
            connection->SendListOfChatters( job.uuid, usersAndIds );
         }
      }
      break;
   case  ChatChannelDbJob::JobType_AllUsersInChannel:
      {
         UserUuidMapIterator creatorIter = m_userUuidMap.find( job.chatUserLookup );
         if( creatorIter == m_userUuidMap.end() )
         {
            return false;
         }
         UserConnection* connection = creatorIter->second.connection;
         
         if( dbResult->successfulQuery == true )
         {
            SerializedKeyValueVector< string > usersAndIds;

            SimpleUserTable            enigma( dbResult->bucket );
            SimpleUserTable::iterator  it = enigma.begin();
            while( it != enigma.end() )
            {
               UserTable::row    row = *it++;
               string   name = row[ TableSimpleUser::Column_name ];
               string   uuid = row[ TableSimpleUser::Column_uuid ];
               string   id =  row[ TableSimpleUser::Column_id ];

               usersAndIds.insert( uuid, name );
            }
            connection->SendListOfAllUsersInChatChannel( job.uuid, usersAndIds );
         }
      }
      break;
   }
   
   return true;
}

//---------------------------------------------------------------------

void     ChatChannelManager::UserLoggedIn( const string& username, const string& userUuid, UserConnection* connection )
{
   stringhash  userHashLookup = GenerateUniqueHash( userUuid );

   UserUuidMapIterator itUserUuid = m_userUuidMap.find( userHashLookup );
   if( itUserUuid != m_userUuidMap.end() )
   {
      string text = " User ";
      text += CreatePrintablePair( userUuid, username );
      text += " tried to login twice to ChatChannelManager ";
      m_chatServer->Log( text, 1 );
      return;
   }

   ChatUser userInfo;
   userInfo.username = username;
   userInfo.userUuid = userUuid;
   userInfo.isOnline = true;
   userInfo.connection = connection;

   UserUuidSet otherUsersToNotify;

   if( connection->GetGroups().size() == 0 )
   {
      string text = " User ";
      text += CreatePrintablePair( userUuid, username );
      text += " is not part of any chat channel ";
      m_chatServer->Log( text, 1 );
   }
   else
   {
      UserConnection::ChannelKeyValue::const_KVIterator kvIter = connection->GetGroups().begin();
      while( kvIter != connection->GetGroups().end() )
      {
         const string& channelUuid = kvIter->key;
         const string& channelName = kvIter->value.channelName;
         userInfo.channels.push_back( GenerateUniqueHash( channelUuid ) );
         AddUserToChannelInternal( userHashLookup, channelUuid, channelName, otherUsersToNotify );
         kvIter++;
      }
   }
   
   //**************************************************************
   // todo, we now need to step through friends and tell each friend that this user has logged in
   //**************************************************************

   m_userUuidMap.insert( UserUuidPair( userHashLookup, userInfo ) );

   InformUsersAboutUserStatusChange( userHashLookup, otherUsersToNotify, UserStatusChange_Login );
}

//---------------------------------------------------------------------

void     ChatChannelManager::UserLoggedOut( const string& username, const string& userUuid, UserConnection* connection )
{
   stringhash  userHashLookup = GenerateUniqueHash( userUuid );

   UserUuidMapIterator itUserUuid = m_userUuidMap.find( userHashLookup );
   if( itUserUuid == m_userUuidMap.end() )
   {
      string text = " User ";
      text += CreatePrintablePair( userUuid, username );
      text += " tried to logout ChatChannelManager but was never logged in";
      m_chatServer->Log( text, 1 );
      return;
   }

   UserUuidSet otherUsersToNotify;
   ChatUser& userInfo = itUserUuid->second;
   list< stringhash >::iterator channelIt = userInfo.channels.begin();
   while( channelIt != userInfo.channels.end() )
   {
      stringhash groupUuidHash = *channelIt;
      RemoveUserFromChannelInternal( userHashLookup, groupUuidHash, userUuid, otherUsersToNotify );
      channelIt++;
   }

   InformUsersAboutUserStatusChange( userHashLookup, otherUsersToNotify, UserStatusChange_Logout );

   m_userUuidMap.erase( itUserUuid );// must remove last
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------

void     ChatChannelManager::AddUserToChannelInternal( stringhash hashedUserUuid, const string& channelUuid, const string& channelName, UserUuidSet& otherUsersToNotify )
{
   stringhash  keyLookup = GenerateUniqueHash( channelUuid );

   ChannelMapIterator iter = m_channelMap.find( keyLookup );
   if( iter == m_channelMap.end() )
   {
      AddChatChannel( channelName, channelUuid );

      iter = m_channelMap.find( keyLookup );// used below
   }

   ChatChannel& channelInfo = iter->second;
   channelInfo.userUuidList.push_back( hashedUserUuid );

   list< stringhash >::iterator usersToInform = channelInfo.userUuidList.begin();
   while( usersToInform != channelInfo.userUuidList.end() )
   {
      otherUsersToNotify.insert( *usersToInform ++ );
   }
}

//---------------------------------------------------------------------

bool     ChatChannelManager::AddChatChannel( const string& channelName, const string& channelUuid )
{
   stringhash  keyLookup = GenerateUniqueHash( channelUuid );

   ChannelMapIterator iter = m_channelMap.find( keyLookup );
   if( iter == m_channelMap.end() )
   {
      ChannelMapPair channelMapData;
      ChatChannel channelInfo;
      channelInfo.uuid = channelUuid;
      channelInfo.name = channelName;

      channelMapData.first = keyLookup;
      channelMapData.second = channelInfo;
      m_channelsNeedingQueryInfo.push_back( keyLookup );// we need to go to the db to pull all of the info for this channel

      m_channelMap.insert( channelMapData);
      m_chatServer->ChatChannelManagerNeedsUpdate();

      return true;
   }

   return false;
}

//---------------------------------------------------------------------

void     ChatChannelManager::RemoveUserFromChannelInternal( stringhash userUuidHash, stringhash channelUuid, const string& userUuid, UserUuidSet& otherUsersToNotify )
{
   ChannelMapIterator channelIter = m_channelMap.find( channelUuid );
   if( channelIter == m_channelMap.end() )
   {
      string text = " Cleaning up channel ";
      text += userUuid;
      text += " tried to leave non-existent channel (hashed) ";
      //text += channelUuid;
      m_chatServer->Log( text, 1 );
      return;
   }

   bool erasedFlag = false;
   list< stringhash >& listReference = channelIter->second.userUuidList;
   list< stringhash >::iterator userIter = listReference.begin();
   while( userIter != listReference.end() )
   {
      stringhash test = *userIter;
      if( test == userUuidHash )
      {
         listReference.erase( userIter );
         if( listReference.size() == 0 )
         {
            // log and remove the group
            string text = " User ";
            text += userUuid;
            text += " was the last user on channel ";
            text += boost::lexical_cast< string >( channelUuid );
            m_chatServer->Log( text, 1 );
            m_channelMap.erase( channelIter );
            //m_chatServer->ChatChannelManagerNeedsUpdate();// possibly

            return;// we were the last one... no one to notify.
         }
         erasedFlag = true;
         break;
      }
      userIter ++;
   }

   if( erasedFlag == true )
   {
      // as part of logging out, we may have deleted the list of channels...
      //ChannelMapIterator channelIter = m_channelMap.find( channelUuid );
      //if( channelIter == m_channelMap.end() )
      {
         ChatChannel& channelInfo = channelIter->second;

         list< stringhash >::iterator usersToInform = channelInfo.userUuidList.begin();
         while( usersToInform != channelInfo.userUuidList.end() )
         {
            otherUsersToNotify.insert( *usersToInform ++ );
         }
      }
   }
   //m_chatServer->ChatChannelManagerNeedsUpdate();
}

//---------------------------------------------------------------------

bool     ChatChannelManager::RemoveChatChannel( const string& channelUuid )
{
   stringhash channelHash = GenerateUniqueHash( channelUuid );
   ChannelMapIterator channelIter = m_channelMap.find( channelHash );
   if( channelIter == m_channelMap.end() )
   {
      return false;
   }

   list< stringhash >& listReference = channelIter->second.userUuidList;
   list< stringhash >::iterator userIter = listReference.begin();
   int num = listReference.size();
   while( userIter != listReference.end() )
   {
      stringhash test = *userIter;
      UserUuidMapIterator userInfo = m_userUuidMap.find( test );
      if( userInfo != m_userUuidMap.end() )
      {
         userInfo->second.connection->NotifyChannelRemoved( channelUuid, num );
      }
      userIter ++;
   }

   m_channelMap.erase( channelIter );
   return true;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------

void     ChatChannelManager::UserSendsChatToChannel( const string& senderUuid, const string& channelUuid, const string& message )
{
   stringhash userHash = GenerateUniqueHash( senderUuid );
   stringhash channelHash = GenerateUniqueHash( channelUuid );

   UserUuidMapIterator senderIter = m_userUuidMap.find( userHash );
   if( senderIter == m_userUuidMap.end() )
   {
      string text = " User ";
      text += senderUuid;
      text += " tried to send text on chat channel ";
      text += channelUuid;
      text += " but that user does not exist in ChatChannelManager";
      m_chatServer->Log( text, 1 );
      return;
   }
   ChatUser& userSender = senderIter->second;
   U32 connectionId = userSender.connection->GetConnectionId();
   if( channelUuid.size() == 0 )
   {
      m_chatServer->SendErrorReportToClient( PacketErrorReport::ErrorType_NoChatChannel, connectionId  );
      return;
   }

   //-----------------------------------------

   ChannelMapIterator channelIter = m_channelMap.find( channelHash );
   if( channelIter == m_channelMap.end() )
   {
      string text = " User ";
      text += senderUuid;
      text += " tried to send text on chat channel ";
      text += channelUuid;
      text += " but that channel does not exist in ChatChannelManager";
      m_chatServer->Log( text, 1 );
      return;
   }

   ChatChannel& channel = channelIter->second;
   if( channel.userUuidList.size() == 1 )
   {
      m_chatServer->SendErrorReportToClient( PacketErrorReport::ErrorType_YouAreTheOnlyPersonInThatChatChannel, connectionId  );
      string text = " User ";
      text += senderUuid;
      text += " tried to send text on chat channel ";
      text += channelUuid;
      text += " but s/he is the only person in that group";
      m_chatServer->Log( text, 1 );
      return;
   }
   
   list< stringhash >::iterator userIt = channel.userUuidList.begin();
   while( userIt != channel.userUuidList.end() )
   {
      stringhash receiverLookup = *userIt++;
      UserUuidMapIterator receiverIter = m_userUuidMap.find( receiverLookup );
      if( receiverIter != m_userUuidMap.end() )
      {
         UserConnection* connection = receiverIter->second.connection;
         connection->SendChat( message, senderUuid, userSender.username, channelUuid );
      }
   }

   string friendUuid;
   WriteChatToDb( message, senderUuid, friendUuid, channelUuid, channel.gameTurn, connectionId );
}

//---------------------------------------------------------------------

void     ChatChannelManager::UserSendsChatToUser( const string& userUuid, const string& destinationUuid, const string& message )
{
   assert( 0 );// not finished
}

//---------------------------------------------------------------------

bool   ChatChannelManager::CreateNewChannel( const string& channelName, const string& authUuid )
{
   stringhash authHash = GenerateUniqueHash( authUuid );

   UserUuidMapIterator creatorIter = m_userUuidMap.find( authHash );
   if( creatorIter == m_userUuidMap.end() )
   {
      string text = " User ";
      text += authUuid;
      text += " tried to create a new chat channel ";
      text += channelName;
      text += " but that user does not exist in ChatChannelManager";
      m_chatServer->Log( text, 1 );
      return false;
   }
   //***********************************************
   // todo, add lookup to prevent duplicate channels
   // check for user permissions
   //***********************************************

   U32 xorValue = 0xFFFFFFFF;
   xorValue  =  GetCurrentMilliseconds();

   string newId = GenerateUUID( xorValue );

   // insert into chat_channel values (null, 'whosit', 'defghijklmnopabc', 1);
   string queryString = "INSERT INTO chat_channel VALUES( null, '";
   queryString += channelName;
   queryString += "', '";
   queryString += newId;
   queryString += "', 1, 32, 0 )";

   U32 serverId = 0;
   DbQueryAndPacket( channelName, newId, serverId, authUuid, authHash, queryString, ChatChannelDbJob::JobType_Create, false );

   return true;
}

//---------------------------------------------------------------------

bool   ChatChannelManager::CreateNewChannel( const string& channelName, const U32 serverId )
{
   //***********************************************
   // todo, add lookup to prevent duplicate channels
   // check for user permissions
   //***********************************************

   U32 xorValue = 0xFFFFFFFF;
   xorValue  = GetCurrentMilliseconds();

   string newId = GenerateUUID( xorValue );

   // insert into chat_channel values (null, 'whosit', 'defghijklmnopabc', 1);
   string queryString = "INSERT INTO chat_channel VALUES( null, '";
   queryString += channelName;
   queryString += "', '";
   queryString += newId;
   queryString += "', 1, 32, 1 )";// note the 1 here used to indicate that this is a server created channel

   string authUuid;
   stringhash authHash = 0;
   DbQueryAndPacket( channelName, newId, serverId, authUuid, authHash, queryString, ChatChannelDbJob::JobType_Create, false );

   return true;
}

//---------------------------------------------------------------------

void ChatChannelManager::DbQueryAndPacket( const string& channelName, const string& channelUuid, U32 serverId, const string& authUuid, stringhash chatUserLookup, const string& queryString, ChatChannelDbJob::JobType jobType, bool isFandF )
{
   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id = GetConnectionId();
   dbQuery->lookup = AddDbJob( channelName, channelUuid, serverId, chatUserLookup, GenerateUniqueHash( authUuid ), jobType );
   dbQuery->meta = authUuid;
   dbQuery->isFireAndForget = isFandF;
   dbQuery->query = queryString;

   m_chatServer->AddPacketFromUserConnection( dbQuery, GetConnectionId() );
}

//---------------------------------------------------------------------

bool     ChatChannelManager::DeleteChannel( const string& channelUuid, const string& authUuid )
{
   // we'll simply move the channel to inactive

   stringhash authHash = GenerateUniqueHash( authUuid );
   stringhash channelHash = GenerateUniqueHash( channelUuid );

   UserUuidMapIterator creatorIter = m_userUuidMap.find( authHash );
   if( creatorIter == m_userUuidMap.end() )
   {
      string text = " User ";
      text += authUuid;
      text += " tried to delete a chat channel ";
      text += channelUuid;
      text += " but that user does not exist in ChatChannelManager";
      m_chatServer->Log( text, 1 );
      return false;
   }

   //-----------------------------------------

   ChannelMapIterator channelIter = m_channelMap.find( channelHash );
   if( channelIter == m_channelMap.end() )
   {
      string text = " User ";
      text += authUuid;
      text += " tried to delete a chat channel on chat channel ";
      text += channelUuid;
      text += " but that channel does not exist in ChatChannelManager";
      m_chatServer->Log( text, 1 );
      return false;
   }

   //***********************************************
   // todo, don't change inactive groups to inactive
   // check for user permissions
   //***********************************************

   // UPDATE chat_channel 
   // SET chat_channel.is_active=0 
   // WHERE chat_channel.uuid='abcdefghijklmnop'

   string queryString = "UPDATE chat_channel SET chat_channel.is_active=0 WHERE chat_channel.uuid='";
   queryString += channelUuid;
   queryString += "'";

   U32 serverId = 0;
   DbQueryAndPacket( channelUuid, channelUuid, serverId, authUuid, authHash, queryString, ChatChannelDbJob::JobType_Delete, false );

   /*
   select count(*) from user_join_chat_channel 
   where user_groupid='bcdefghijklmnopa'
   */

   return true;
}

//---------------------------------------------------------------------
 
bool    ChatChannelManager::DeleteChannel( const string& channelUuid, const U32 serverId )
{
   // we'll simply move the channel to inactive
   stringhash channelHash = GenerateUniqueHash( channelUuid );

   //-----------------------------------------

   ChannelMapIterator channelIter = m_channelMap.find( channelHash );
   if( channelIter == m_channelMap.end() )
   {
      string text = " Server ";
      text += serverId;
      text += " tried to delete a chat channel on chat channel ";
      text += channelUuid;
      text += " but that channel does not exist in ChatChannelManager";
      m_chatServer->Log( text, 1 );
      return false;
   }

   // UPDATE chat_channel 
   // SET chat_channel.is_active=0 
   // WHERE chat_channel.uuid='abcdefghijklmnop'

   string queryString = "UPDATE chat_channel SET chat_channel.is_active=0 WHERE chat_channel.uuid='";
   queryString += channelUuid;
   queryString += "'";

   string authUuid;
   stringhash authHash = 0;
   DbQueryAndPacket( channelUuid, channelUuid, serverId, authUuid, authHash, queryString, ChatChannelDbJob::JobType_Delete, false );

   return true;
}
 
//---------------------------------------------------------------------
  
bool     ChatChannelManager::InviteUserToChannel( const string& channelUuid, const string& userUuid, const string& authUuid )
{
   return false;
}

//---------------------------------------------------------------------

bool     ChatChannelManager::AddUserToChannel( const string& channelUuid, const string& userUuid, const string& authUuid )
{
   stringhash userHashLookup = GenerateUniqueHash( userUuid );
   stringhash authUserHashLookup = GenerateUniqueHash( authUuid );
   stringhash channelHash = GenerateUniqueHash( channelUuid );

   UserUuidMapIterator authIter = m_userUuidMap.find( authUserHashLookup );
   if( authIter == m_userUuidMap.end() )
   {
      string text = " User ";
      text += authUuid;
      text += " tried to add a user ";
      text += userUuid;
      text += " to a chat channel ";
      text += channelUuid;
      text += " but that authUuid does not exist in ChatChannelManager";
      m_chatServer->Log( text, 1 );
      return false;
   }

   UserUuidMapIterator userIter = m_userUuidMap.find( userHashLookup );
   if( userIter == m_userUuidMap.end() )
   {
      m_chatServer->SendErrorReportToClient( PacketErrorReport::ErrorType_UserDoesNotExist, authIter->second.connection->GetConnectionId()  );

      string text = " User ";
      text += authUuid;
      text += " tried to add a user ";
      text += userUuid;
      text += " to a chat channel ";
      text += channelUuid;
      text += " but that user does not exist in ChatChannelManager";
      m_chatServer->Log( text, 1 );
      return false;
   }
   //-----------------------------------------

   ChannelMapIterator channelIter = m_channelMap.find( channelHash );
   if( channelIter == m_channelMap.end() )
   {
      m_chatServer->SendErrorReportToClient( PacketErrorReport::ErrorType_BadChatChannel, userIter->second.connection->GetConnectionId()  );

      string text = " User ";
      text += authUuid;
      text += " tried to add a user ";
      text += userUuid;
      text += " to a chat channel ";
      text += channelUuid;
      text += " but that channel does not exist in ChatChannelManager";
      m_chatServer->Log( text, 1 );
      return false;
   }
   
   //-----------------------------------------

   ChatChannel& channel = channelIter->second;
   list< stringhash >::iterator userIt = channel.userUuidList.begin();
   while( userIt != channel.userUuidList.end() )
   {
      if( userHashLookup == *userIt++ )
      {
         m_chatServer->SendErrorReportToClient( PacketErrorReport::ErrorType_CannotAddUserToChannel_AlreadyExists, userIter->second.connection->GetConnectionId()  );
         string text = " User ";
         text += authUuid;
         text += " tried to add a user ";
         text += userUuid;
         text += " to a chat channel ";
         text += channelUuid;
         text += " but that user already exists in ChatChannelManager";
         m_chatServer->Log( text, 1 );
         return false;
      }
   }

   //***********************************************
   // todo, don't change inactive groups to inactive
   // check for user permissions
   //***********************************************

   string queryString = "INSERT INTO user_join_chat_channel VALUES ( '";
   queryString += userUuid;
   queryString += "', '";
   queryString += channelUuid;
   queryString += "', null )";

   U32 serverId = 0;
   DbQueryAndPacket( channelUuid, userUuid, serverId, authUuid, userHashLookup, queryString, ChatChannelDbJob::JobType_AddUser, false );

   UserUuidSet otherUsersToNotify;
   AddUserToChannelInternal( userHashLookup, channelUuid, channel.name, otherUsersToNotify );
   InformUsersAboutUserStatusChange( userHashLookup, otherUsersToNotify, UserStatusChange_UserAddedToChannel );

   return false;
}

//---------------------------------------------------------------------

bool     ChatChannelManager::AddUserToChannel( const string& channelUuid, const string& userUuid, U32 serverId )
{
   stringhash userHashLookup = GenerateUniqueHash( userUuid );
   stringhash channelHash = GenerateUniqueHash( channelUuid );

   /* // todo, send errors back
      PacketAddUserToGameResponse* packet = new PacketAddUserToGameResponse;
      packet->gameUuid = channelUuid;
      packet->userUuid = userUuid;
      packet->wasSuccessful = false;
      PackageAndSendToDiplodocusChat( packet, serverId );
   */

   UserUuidMapIterator userIter = m_userUuidMap.find( userHashLookup );
   if( userIter == m_userUuidMap.end() )
   {
      string text = " Server ";
      text += serverId;
      text += " tried to add a user ";
      text += userUuid;
      text += " to a chat channel ";
      text += channelUuid;
      text += " but that user does not exist in ChatChannelManager";
      m_chatServer->Log( text, 1 );
      return false;
   }
   //-----------------------------------------

   ChannelMapIterator channelIter = m_channelMap.find( channelHash );
   if( channelIter == m_channelMap.end() )
   {
      string text = " Server ";
      text += serverId;
      text += " tried to add a user ";
      text += userUuid;
      text += " to a chat channel ";
      text += channelUuid;
      text += " but that channel does not exist in ChatChannelManager";
      m_chatServer->Log( text, 1 );
      return false;
   }
   
   //-----------------------------------------

   ChatChannel& channel = channelIter->second;
   list< stringhash >::iterator userIt = channel.userUuidList.begin();
   while( userIt != channel.userUuidList.end() )
   {
      if( userHashLookup == *userIt++ )
      {
         string text = " Server ";
         text += serverId;
         text += " tried to add a user ";
         text += userUuid;
         text += " to a chat channel ";
         text += channelUuid;
         text += " but that user already exists in ChatChannelManager";
         m_chatServer->Log( text, 1 );
         return false;
      }
   }

   //***********************************************
   // todo, don't change inactive groups to active
   //***********************************************

   string queryString = "INSERT INTO user_join_chat_channel VALUES ( '";
   queryString += userUuid;
   queryString += "', '";
   queryString += channelUuid;
   queryString += "', null )";

   string authUuid;
   DbQueryAndPacket( channelUuid, userUuid, serverId, authUuid, userHashLookup, queryString, ChatChannelDbJob::JobType_AddUser, false );

   UserUuidSet otherUsersToNotify;
   AddUserToChannelInternal( userHashLookup, channelUuid, channel.name, otherUsersToNotify );
   InformUsersAboutUserStatusChange( userHashLookup, otherUsersToNotify, UserStatusChange_UserAddedToChannel );

   return false;
}

//---------------------------------------------------------------------

bool     ChatChannelManager::RemoveUserFromChannel( const string& channelUuid, const string& userUuid, const string& authUuid )
{
   stringhash userHashLookup = GenerateUniqueHash( userUuid );
   stringhash authUserHashLookup = GenerateUniqueHash( authUuid );
   stringhash channelHash = GenerateUniqueHash( channelUuid );

   UserUuidMapIterator authIter = m_userUuidMap.find( authUserHashLookup );
   if( authIter == m_userUuidMap.end() )
   {
      string text = " User ";
      text += authUuid;
      text += " tried to remove a user ";
      text += userUuid;
      text += " to a chat channel ";
      text += channelUuid;
      text += " but that authUuid does not exist in ChatChannelManager";
      m_chatServer->Log( text, 1 );
      return false;
   }

   UserUuidMapIterator userIter = m_userUuidMap.find( userHashLookup );
   if( userIter == m_userUuidMap.end() )
   {
      m_chatServer->SendErrorReportToClient( PacketErrorReport::ErrorType_UserDoesNotExist, authIter->second.connection->GetConnectionId()  );
      string text = " User ";
      text += authUuid;
      text += " tried to remove user ";
      text += userUuid;
      text += " from chat channel ";
      text += channelUuid;
      text += " but that user does not exist in ChatChannelManager";
      m_chatServer->Log( text, 1 );
      return false;
   }

   //-----------------------------------------

   ChannelMapIterator channelIter = m_channelMap.find( channelHash );
   if( channelIter == m_channelMap.end() )
   {
      m_chatServer->SendErrorReportToClient( PacketErrorReport::ErrorType_BadChatChannel, userIter->second.connection->GetConnectionId()  );

      string text = " User ";
      text += authUuid;
      text += " tried to remove a user ";
      text += userUuid;
      text += " from chat channel ";
      text += channelUuid;
      text += " but that channel does not exist in ChatChannelManager";
      m_chatServer->Log( text, 1 );
      return false;
   }

   //-----------------------------------------
   
    //***********************************************
   // todo, don't change inactive groups to inactive
   // check for user permissions
   //***********************************************

   // DELETE FROM user_join_chat_channel 
   // WHERE user_join_chat_channel.userid ='FGHIJKLMNOPABCDE' 
   // AND user_join_chat_channel.user_groupid ='bcdefghijklmnopa';

   string queryString = "DELETE FROM user_join_chat_channel WHERE user_join_chat_channel.user_uuid ='";
   queryString += userUuid;
   queryString += "' AND  user_join_chat_channel.channel_uuid='";
   queryString += channelUuid;
   queryString += "'";

   U32 serverId = 0;
   DbQueryAndPacket( channelUuid, userUuid, serverId, authUuid, userHashLookup, queryString, ChatChannelDbJob::JobType_RemoveUser, false );

   UserUuidSet otherUsersToNotify;
   ChatChannel& channel = channelIter->second;
   RemoveUserFromChannelInternal( userHashLookup, channelHash, userUuid, otherUsersToNotify );
   InformUsersAboutUserStatusChange( userHashLookup, otherUsersToNotify, UserStatusChange_UserRemovedFromChannel );

   return true;
}
//---------------------------------------------------------------------

bool     ChatChannelManager::RemoveUserFromChannel( const string& channelUuid, const string& userUuid, U32 serverId )
{
   stringhash userHashLookup = GenerateUniqueHash( userUuid );
   stringhash channelHash = GenerateUniqueHash( channelUuid );

   UserUuidMapIterator userIter = m_userUuidMap.find( userHashLookup );
   if( userIter == m_userUuidMap.end() )
   {
      string text = " Server ";
      text += serverId;
      text += " tried to remove user ";
      text += userUuid;
      text += " from chat channel ";
      text += channelUuid;
      text += " but that user does not exist in ChatChannelManager";
      m_chatServer->Log( text, 1 );
      return false;
   }

   //-----------------------------------------

   ChannelMapIterator channelIter = m_channelMap.find( channelHash );
   if( channelIter == m_channelMap.end() )
   {
      string text = " Server ";
      text += serverId;
      text += " tried to remove a user ";
      text += userUuid;
      text += " from chat channel ";
      text += channelUuid;
      text += " but that channel does not exist in ChatChannelManager";
      m_chatServer->Log( text, 1 );
      return false;
   }

   //-----------------------------------------
   
    //***********************************************
   // todo, don't change inactive groups to inactive
   //***********************************************

   // DELETE FROM user_join_chat_channel 
   // WHERE user_join_chat_channel.userid ='FGHIJKLMNOPABCDE' 
   // AND user_join_chat_channel.user_groupid ='bcdefghijklmnopa';

   string queryString = "DELETE FROM user_join_chat_channel WHERE user_join_chat_channel.user_uuid ='";
   queryString += userUuid;
   queryString += "' AND  user_join_chat_channel.channel_uuid='";
   queryString += channelUuid;
   queryString += "'";

   string authUuid;
   DbQueryAndPacket( channelUuid, userUuid, serverId, authUuid, userHashLookup, queryString, ChatChannelDbJob::JobType_RemoveUser, false );

   UserUuidSet otherUsersToNotify;
   ChatChannel& channel = channelIter->second;
   RemoveUserFromChannelInternal( userHashLookup, channelHash, userUuid, otherUsersToNotify );
   InformUsersAboutUserStatusChange( userHashLookup, otherUsersToNotify, UserStatusChange_UserRemovedFromChannel );

   return true;
}

//---------------------------------------------------------------------

bool     ChatChannelManager::RequestChatters( const string& channelUuid, const string& authUuid )
{
   stringhash userHashLookup = GenerateUniqueHash( authUuid );
   stringhash channelHash = GenerateUniqueHash( channelUuid );

   UserUuidMapIterator userIter = m_userUuidMap.find( userHashLookup );
   if( userIter == m_userUuidMap.end() )
   {
      string text = " User ";
      text += authUuid;
      text += " request chatters on channel ";
      text += channelUuid;
      text += " but that user does not exist in ChatChannelManager";
      m_chatServer->Log( text, 1 );
      return false;
   }

   //-----------------------------------------

   ChannelMapIterator channelIter = m_channelMap.find( channelHash );
   if( channelIter == m_channelMap.end() )
   {
      m_chatServer->SendErrorReportToClient( PacketErrorReport::ErrorType_BadChatChannel, userIter->second.connection->GetConnectionId()  );

      string text = " User ";
      text += authUuid;
      text += " request chatters on channel ";
      text += channelUuid;
      text += " but that channel does not exist in ChatChannelManager";
      m_chatServer->Log( text, 1 );
      return false;
   }

   //-----------------------------------------

   // SELECT DISTINCT user.uuid FROM user as user 
   // join user_join_chat_channel as joiner on joiner.user_uuid=user.uuid 
   // join chat_channel as channel on joiner.channel_uuid=channel.uuid 
   // join chat as chat_log on channel.uuid=chat_log.chat_channel_id 
   // where channel.uuid='abcdefghijklmnop'

   string 
   queryString =  "SELECT DISTINCT user.name, user.uuid, user.id FROM users as user ";
   queryString += "join user_join_chat_channel as joiner on joiner.user_uuid=user.uuid ";
   queryString += "join chat_channel as channel on joiner.channel_uuid=channel.uuid ";
   queryString += "join chat as chat_log on channel.uuid=chat_log.chat_channel_id ";
   queryString += "where channel.uuid='";
   queryString += channelUuid;
   queryString += "'";

   return CreateAndSendUserListJob( queryString, channelIter->second.name, channelUuid, 0, authUuid, userHashLookup, ChatChannelDbJob::JobType_FindChatters );
}
//---------------------------------------------------------------------

bool     ChatChannelManager::RequestAllUsersInChatChannel( const string& channelUuid, bool fullList, const string& authUuid )
{
   stringhash userHashLookup = GenerateUniqueHash( authUuid );
   stringhash channelHash = GenerateUniqueHash( channelUuid );

   UserUuidMapIterator userIter = m_userUuidMap.find( userHashLookup );
   if( userIter == m_userUuidMap.end() )
   {
      string text = " User ";
      text += authUuid;
      text += " request chatters on channel ";
      text += channelUuid;
      text += " but that user does not exist in ChatChannelManager";
      m_chatServer->Log( text, 1 );
      return false;
   }

   //-----------------------------------------

   ChannelMapIterator channelIter = m_channelMap.find( channelHash );
   if( channelIter == m_channelMap.end() )
   {
      m_chatServer->SendErrorReportToClient( PacketErrorReport::ErrorType_BadChatChannel, userIter->second.connection->GetConnectionId()  );

      string text = " User ";
      text += authUuid;
      text += " request chatters on channel ";
      text += channelUuid;
      text += " but that channel does not exist in ChatChannelManager";
      m_chatServer->Log( text, 1 );
      return false;
   }

   //-----------------------------------------

   // SELECT DISTINCT user.name, user.uuid FROM user
   // join user_join_chat_channel as joiner on joiner.user_uuid=user.uuid 
   // join chat_channel as channel on joiner.channel_uuid=channel.uuid 
   // where channel.uuid='abcdefghijklmnop'

   string 
   queryString =  "SELECT user.name, user.uuid FROM users ";
   queryString += "join user_join_chat_channel as joiner on joiner.user_uuid=user.uuid ";
   queryString += "join chat_channel as channel on joiner.channel_uuid=channel.uuid ";
   queryString += "where channel.uuid='";
   queryString += channelUuid;
   queryString += "'";

   return CreateAndSendUserListJob( queryString, channelIter->second.name, channelUuid, 0, authUuid, userHashLookup, ChatChannelDbJob::JobType_AllUsersInChannel );
};

//---------------------------------------------------------------------

bool     ChatChannelManager::CreateAndSendUserListJob( const string& queryString, const string& channelName, const string& channelUuid, U32 serverId, const string& requestorUuid, stringhash chatUserLookup, ChatChannelDbJob::JobType type )
{
   DbQueryAndPacket( channelName, channelUuid, serverId, requestorUuid, chatUserLookup, queryString, type, false );

   return true;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------

void     ChatChannelManager::LoadAllChannels( const string& authUuid )
{
   stringhash authHash = GenerateUniqueHash( authUuid );

   UserUuidMapIterator userIter = m_userUuidMap.find( authHash );
   if( userIter == m_userUuidMap.end() )
   {
      string text = " User ";
      text += authUuid;
      text += " tried to LoadAllChannels but that user does not exist in ChatChannelManager";
      m_chatServer->Log( text, 1 );
      return;
   }

   // SELECT * FROM chat_channel
   string queryString = "SELECT * FROM chat_channel";
   U32 serverId = 0;
   DbQueryAndPacket( authUuid, authUuid, serverId, authUuid, authHash, queryString, ChatChannelDbJob::JobType_LoadAllChannels, false );
}

//---------------------------------------------------------------------

void     ChatChannelManager::RequestChatChannelList( const string& authUuid, bool isFullList )
{
   if( isFullList == true ) // go to the db
   {
      stringhash authHash = GenerateUniqueHash( authUuid );
      UserUuidMapIterator userIter = m_userUuidMap.find( authHash );
      if( userIter == m_userUuidMap.end() )
      {
         string text = " User ";
         text += authUuid;
         text += " tried to RequestChatChannelList but that user does not exist in ChatChannelManager";
         m_chatServer->Log( text, 1 );
         return;
      }

      // SELECT * FROM chat_channel
      string queryString = "SELECT * FROM chat_channel";
      U32 serverId = 0;
      DbQueryAndPacket( authUuid, authUuid, serverId, authUuid, authHash, queryString, ChatChannelDbJob::JobType_SelectAllChannelsToSendToAuth, false );
   }
   else
   {
      assert( 0 );// undone
   }
}

//---------------------------------------------------------------------

void     ChatChannelManager::RequestChatChannelList( U32 serverId )
{
   // SELECT * FROM chat_channel where is_server_created=1
   string queryString = "SELECT * FROM chat_channel WHERE is_server_created=1";
   string authUuid;
   stringhash authHash = 0;
   DbQueryAndPacket( authUuid, authUuid, serverId, authUuid, authHash, queryString, ChatChannelDbJob::JobType_SelectAllChannelsToSendToAuth, false );
}

//---------------------------------------------------------------------

bool     ChatChannelManager::AdvanceGameTurn( const string& channelUuid, U32 serverId )
{
   stringhash channelHash = GenerateUniqueHash( channelUuid );
   ChannelMapIterator channelIter = m_channelMap.find( channelHash );
   if( channelIter == m_channelMap.end() )
   {
      string text = " Server ";
      text += serverId;
      text += " requests turn advancement on ";
      text += channelUuid;
      text += " but that channel does not exist in ChatChannelManager";
      m_chatServer->Log( text, 1 );
      return false;
   }

   channelIter->second.gameTurn ++;
   // possible threading issue here
   PacketGameAdvanceTurnResponse* packet = new PacketGameAdvanceTurnResponse;
   packet->gameUuid = channelUuid;
   packet->currentTurn = channelIter->second.gameTurn;

   PackageAndSendToDiplodocusChat( packet, serverId );

   return true;
}

//---------------------------------------------------------------------

void     ChatChannelManager::RequestUsersList( const string& authUuid, bool isFullList )
{
   if( isFullList == true )
   {
      stringhash authHash = GenerateUniqueHash( authUuid );
      UserUuidMapIterator userIter = m_userUuidMap.find( authHash );
      if( userIter == m_userUuidMap.end() )
      {
         string text = " User ";
         text += authUuid;
         text += " tried to RequestUsersList but that user does not exist in ChatChannelManager";
         m_chatServer->Log( text, 1 );
         return ;
      }

      string queryString = "SELECT * FROM users";
      U32 serverId = 0;
      DbQueryAndPacket( authUuid, authUuid, serverId, authUuid, authHash, queryString, ChatChannelDbJob::JobType_SelectAllUsersToSendToAuth, false );
   }
   else
   {
      assert( 0 );
   }
}

//---------------------------------------------------------------------

int     ChatChannelManager::AddDbJob( const string& channelName, const string& channelUuid, U32 serverId, stringhash chatUserLookup, stringhash authUserLookup, ChatChannelDbJob::JobType type )
{
   ChatChannelDbJob job;
   job.name = channelName;
   job.uuid = channelUuid;
   job.chatUserLookup = chatUserLookup;
   job.authUserLookup = authUserLookup;
   job.jobType = type;
   job.jobId = m_dbJobLookupId++;
   job.serverId = serverId;

   m_pendingDbJobs.push_back( job );

   return job.jobId;
}

//---------------------------------------------------------------------

void     ChatChannelManager::WriteChatToDb( const string& message, const string& senderUuid, const string& friendUuid, const string& channelUuid, U16 gameTurn, U32 connectionId )
{
   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id = GetConnectionId();
   dbQuery->lookup = 0;
   dbQuery->meta = connectionId;
   dbQuery->isFireAndForget = true;// no result is needed
   //insert into chat values( null, 'Damn sexy', 'ABCDEFGHIJKLMNOP', null, 'ABCDEFGHIJKLMNOP', null, 1, 1345);
   string queryString = "INSERT INTO chat_message VALUES( null, '";
   queryString += "%s";
   queryString += "', '";
   queryString += senderUuid;
   queryString += "', ";//;
   if( friendUuid.size() > 0 )
   {
      queryString += "'";
      queryString += friendUuid;
      queryString += "'";
   }
   else
   {
      queryString += "null";
   }
   queryString += ", '";
   queryString += channelUuid;
   queryString += "', CURRENT_TIMESTAMP, ";
   queryString += boost::lexical_cast< string >( gameTurn );
   queryString += ", ";
   queryString += "0 )"; // game instance id
   dbQuery->escapedStrings.insert( message );

   dbQuery->query = queryString;
   m_chatServer->AddPacketFromUserConnection( dbQuery, connectionId );
}

//---------------------------------------------------------------------


//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////