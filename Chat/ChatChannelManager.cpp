// ChatChannelManager.cpp

#include <map>
#include <string>
#include <list>
#include <vector>
#include <deque>
#include <iomanip>

#include "ChatChannelManager.h"
#include "DiplodocusChat.h"
#include "UserConnection.h"
#include <boost/lexical_cast.hpp>
#include <mysql.h>

#include "../NetworkCommon/Utils/TableWrapper.h"

#include "../NetworkCommon/Packets/DbPacket.h"
#include "../NetworkCommon/Packets/GamePacket.h"
#include "../NetworkCommon/Packets/ChatPacket.h"
#include "../NetworkCommon/Packets/ServerToServerPacket.h"
#include "../NetworkCommon/Packets/PacketFactory.h"


using namespace std;

DiplodocusChat*     ChatChannelManager::m_chatServer = NULL;
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

ChatChannelManager::ChatChannelManager() : m_dbJobLookupId( 1 )// better than 0 
{
}

ChatChannelManager::~ChatChannelManager()
{
}

void  ChatChannelManager::Init()
{
   string queryString = "SELECT * FROM chat_channel";
   U32 serverId = 0;
   DbQueryAndPacket( "", "", serverId, "", 0, queryString, ChatChannelDbJob::JobType_LoadAllChannels, false );
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
      m_mutex.lock();
      m_packetsIn.push_back( wrapper->pPacket );
      m_mutex.unlock();

      PacketFactory factory;
      factory.CleanupPacket( packet );
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
      m_mutex.lock();

      BasePacket* packet = m_packetsIn.front();
      m_packetsIn.pop_front();
      m_mutex.unlock();

      ProcessPacket( packet );      
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
               PacketDbQueryResult* dbResult = static_cast< PacketDbQueryResult* >( packet );
               int jobId = dbResult->lookup;

               DbJobIterator iter;
               bool success = FindDbJob( jobId, m_pendingDbJobs, iter );
               if( success )
               {
                  ChatChannelDbJob& job = *iter;
                  FinishJob( dbResult, job );
                  Threading::MutexLock locker ( m_jobMutex );
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
   string userName = changingUser.userName;
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
   Threading::MutexLock locker ( m_jobMutex );
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
            AddChatChannelToStorage( job.name, job.uuid );
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
         UserUuidMapIterator addedIter = m_userUuidMap.find( GenerateUniqueHash( job.uuid ) );
         if( addedIter != m_userUuidMap.end() )
         {
            AddUserToChannel( job.name, UserBasics( addedIter->second.userName, addedIter->second.userUuid ) );
         }

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
               //connection->SetChatChannel( job.uuid );
            }
            
            PacketAddUserToGameResponse* packet = new PacketAddUserToGameResponse;
            packet->gameUuid = job.name;
            packet->userUuid = job.uuid;
            packet->wasSuccessful = dbResult->successfulQuery;
            PackageAndSendToDiplodocusChat( packet, job.serverId );
         }
         
         // notify the person who requested the job be done of the success
         addedIter = m_userUuidMap.find( job.authUserLookup );// the user who was added
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
   case ChatChannelDbJob::JobType_LoadSingleChannel:
      {
         if( dbResult->successfulQuery == true )
         {
            ChatChannelTable              enigma( dbResult->bucket );
            ChatChannelTable::iterator    it = enigma.begin();
            while( it != enigma.end() )
            {
               ChatChannelTable::row      row = *it++;

               int      channelId =    boost::lexical_cast< int >( row[ TableChatChannel::Column_id ] );
               string   name =         row[ TableChatChannel::Column_name ];
               string   uuid =         row[ TableChatChannel::Column_uuid ];
               bool     isActive =     boost::lexical_cast< bool >( row[ TableChatChannel::Column_is_active ] );
               int      maxPlayers =   boost::lexical_cast< int >( row[ TableChatChannel::Column_max_num_members ] );
               int      gameType =     boost::lexical_cast< int >( row[ TableChatChannel::Column_game_type ] );
               int      gameId =       boost::lexical_cast< int >( row[ TableChatChannel::Column_game_instance_id ] );
               string   createDate =   row[ TableChatChannel::Column_date_created ];

               AddChatchannel( channelId, name, uuid, isActive, maxPlayers, gameType, gameId, createDate );
            }
         };
      }
      break;
   case ChatChannelDbJob::JobType_LoadAllChannels:
      {
         if( job.chatUserLookup != 0 )// when users request this which is the majority of cases
         {
            UserUuidMapIterator creatorIter = m_userUuidMap.find( job.chatUserLookup );
            if( creatorIter == m_userUuidMap.end() )
            {
               return false;
            }
            UserConnection* connection = creatorIter->second.connection;
            connection->NotifyAllChannelsLoaded( dbResult->successfulQuery );
         }

         if( dbResult->successfulQuery == true )
         {
            ChatChannelTable              enigma( dbResult->bucket );
            ChatChannelTable::iterator    it = enigma.begin();
            while( it != enigma.end() )
            {
               ChatChannelTable::row      row = *it++;

               int      channelId =    boost::lexical_cast< int >( row[ TableChatChannel::Column_id ] );
               string   name =         row[ TableChatChannel::Column_name ];
               string   uuid =         row[ TableChatChannel::Column_uuid ];
               bool     isActive =     boost::lexical_cast< bool >( row[ TableChatChannel::Column_is_active ] );
               int      maxPlayers =   boost::lexical_cast< int >( row[ TableChatChannel::Column_max_num_members ] );
               int      gameType =     boost::lexical_cast< int >( row[ TableChatChannel::Column_game_type ] );
               int      gameId =       boost::lexical_cast< int >( row[ TableChatChannel::Column_game_instance_id ] );
               string   createDate =   row[ TableChatChannel::Column_date_created ];
               if( createDate == "NULL" )
                  createDate = "";
               
               AddChatchannel( channelId, name, uuid, isActive, maxPlayers, gameType, gameId, createDate );
            }
         };
      }
      break;
 /*  case JobType_LoadAllUsersInChannel:
      {
         job.uuid
      }
      break;*/
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

           /* ChatChannelTable              enigma( dbResult->bucket );
            ChatChannelTable::iterator    it = enigma.begin();
            while( it != enigma.end() )
            {
               ChatChannelTable::row      row = *it++;
               string   name = row[ TableChatChannel::Column_name ];
               string   uuid = row[ TableChatChannel::Column_uuid ];
               bool     isActive = boost::lexical_cast< bool >( row[ TableChatChannel::Column_is_active ] );

               chatChannelsAndIds.insert( uuid, ChannelInfo( name, isActive ) );
            }*/
            ChatChannelTable              enigma( dbResult->bucket );
            ChatChannelTable::iterator    it = enigma.begin();
            while( it != enigma.end() )
            {
               ChatChannelTable::row      row = *it++;
               string   name =         row[ TableChatChannel::Column_name ];
               string   uuid =         row[ TableChatChannel::Column_uuid ];
               int      gameType =     boost::lexical_cast< int >( row[ TableChatChannel::Column_game_type ] );
               int      gameId =       boost::lexical_cast< int >( row[ TableChatChannel::Column_game_instance_id ] );
               //int      numNewChats =  boost::lexical_cast< int >( row[ TableChatChannelWithNumChats::Column_record_count ] );
               int numNewChats = 0;
               bool     isActive = boost::lexical_cast< bool >( row[ TableChatChannel::Column_is_active ] );

               chatChannelsAndIds.insert( uuid, ChannelInfo( name, uuid, gameType, gameId, numNewChats, isActive ) );
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
         UserConnection* connection = NULL;
         UserUuidMapIterator creatorIter = m_userUuidMap.find( job.chatUserLookup );
         if( creatorIter != m_userUuidMap.end() )
         {
            connection = creatorIter->second.connection;
         }
         
         
         if( dbResult->successfulQuery == true && dbResult->bucket.bucket.size() )
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
            if( connection )
            {
               connection->SendListOfAllUsersInChatChannel( job.uuid, usersAndIds );
            }
            else
            {
               AddAllUsersToChannel( job.uuid, usersAndIds );
            }
         }
       /*  else
         {
            assert( 0 );// error to user;
         }*/
      }
      break;
   case ChatChannelDbJob::JobType_CreateFromGameServer:
      {
         FinishAddingChatChannelFromGameServer( dbResult, job );
      }
      break;
   case ChatChannelDbJob::JobType_AddUserFromGameServer:
      {
         FinishAddingAddingUserToChatchannelFromGameServer( dbResult, job );
      }
      break;
   case ChatChannelDbJob::JobType_RemoveUserFromGameServer:
      {
         FinishAddingRemovingUserFromChatchannelFromGameServer( dbResult, job );
      }
      break;
   }
   
   return true;
}

//---------------------------------------------------------------------

bool     ChatChannelManager::FinishAddingChatChannelFromGameServer( PacketDbQueryResult* dbResult, ChatChannelDbJob& job )
{
   string channelUuid = job.uuid;
   string channelName = job.name;
   U32 gameServerInstanceId = boost::lexical_cast<U32>( job.chatUserLookup );

   if( dbResult->successfulQuery == true )
   {
      AddChatChannelToStorage( channelName, channelUuid );// now load up all of the channel info
      LoadSingleChannel( channelUuid, 0, 0 );// load the db entry

      list< string >& uuids = job.stringBucket.bucket;
      if( uuids.size() != 0 )
      {
         list< string >::iterator it = uuids.begin();
         while( it != uuids.end() )
         {
            string& userUuid = *it++;
            AddUserToChannel( channelUuid, userUuid, "" ); 
         }
      }
   }

   //AddUserToChannel( const string& channelUuid, const string& userUuid, "" );
   int serverId = job.serverId;
   PacketChatCreateChatChannelFromGameServerResponse* packet = new PacketChatCreateChatChannelFromGameServerResponse;
   packet->gameId = gameServerInstanceId;
   packet->channelUuid = channelUuid;

   PackageAndSendToDiplodocusChat( packet, serverId );

   return true;
}

//---------------------------------------------------------------------

bool     ChatChannelManager::FinishAddingAddingUserToChatchannelFromGameServer( PacketDbQueryResult* dbResult, ChatChannelDbJob& job )
{
   U32 gameServerInstanceId = boost::lexical_cast<U32>( job.chatUserLookup );
   int serverId = job.serverId;
   bool success = false;
   string   channelUuid;
   string   userUuid = dbResult->meta;

   if( dbResult->successfulQuery == true )
   {
      ChatChannelTable            enigma( dbResult->bucket );
      ChatChannelTable::iterator  it = enigma.begin();
      if( it != enigma.end() )
      {
         ChatChannelTable::row    row = *it++;
         string name =  row[ TableChatChannel::Column_name ];
         channelUuid =  row[ TableChatChannel::Column_uuid ];
         string id   =  row[ TableChatChannel::Column_id ];

         //AddChatChannelToStorage( name, channelUuid );// unnecessary because all channels should be loaded.
         LoadSingleChannel( channelUuid, 0, 0 );// load the db entry
         AddUserToChannel( channelUuid, userUuid, "" ); 
         success = true;
      }
   }
   PacketChatAddUserToChatChannelGameServerResponse* packet = new PacketChatAddUserToChatChannelGameServerResponse;
   packet->gameId = gameServerInstanceId;
   packet->chatChannelUuid = channelUuid;
   packet->userUuid = userUuid;
   packet->success = success;

   PackageAndSendToDiplodocusChat( packet, serverId );
   return true;;
}

//---------------------------------------------------------------------

bool     ChatChannelManager::FinishAddingRemovingUserFromChatchannelFromGameServer( PacketDbQueryResult* dbResult, ChatChannelDbJob& job )
{
   U32 gameServerInstanceId = boost::lexical_cast<U32>( job.chatUserLookup );
   int serverId = job.serverId;
   bool success = false;
   string   channelUuid;
   string   userUuid = dbResult->meta;

   if( dbResult->successfulQuery == true )
   {
      ChatChannelTable            enigma( dbResult->bucket );
      ChatChannelTable::iterator  it = enigma.begin();
      if( it != enigma.end() )
      {
         ChatChannelTable::row    row = *it++;
         string name =  row[ TableChatChannel::Column_name ];
         channelUuid =  row[ TableChatChannel::Column_uuid ];
         string id   =  row[ TableChatChannel::Column_id ];

         AddChatChannelToStorage( name, channelUuid );
         RemoveUserFromChannel( channelUuid, userUuid, "" );
         success = true;
      }
   }
   PacketChatRemoveUserFromChatChannelGameServerResponse* packet = new PacketChatRemoveUserFromChatChannelGameServerResponse;
   packet->gameId = gameServerInstanceId;
   packet->chatChannelUuid = channelUuid;
   packet->userUuid = userUuid;
   packet->success = success;

   PackageAndSendToDiplodocusChat( packet, serverId );
   return true;
}

//---------------------------------------------------------------------

void     ChatChannelManager::UserLoggedIn( const string& userName, const string& userUuid, UserConnection* newConnection )
{
   stringhash  userHashLookup = GenerateUniqueHash( userUuid );

   UserUuidMapIterator itUserUuid = m_userUuidMap.find( userHashLookup );
   if( itUserUuid != m_userUuidMap.end() )
   {
      string text = " User ";
      text += CreatePrintablePair( userUuid, userName );
      text += " tried to login twice to ChatChannelManager ";
      m_chatServer->Log( text, 1 );
      return;
   }

   ChatUser userInfo;
   userInfo.userName = userName;
   userInfo.userUuid = userUuid;
   userInfo.isOnline = true;
   userInfo.connection = newConnection;

   UserUuidSet otherUsersToNotify;

   if( newConnection->GetChatChannels().size() == 0 )
   {
      string text = " User ";
      text += CreatePrintablePair( userUuid, userName );
      text += " is not part of any chat channel ";
      m_chatServer->Log( text, 1 );
   }
   else
   {
      ChannelKeyValue::const_KVIterator kvIter = newConnection->GetChatChannels().begin();
      while( kvIter != newConnection->GetChatChannels().end() )
      {
         const string& channelUuid = kvIter->key;
         const string& channelName = kvIter->value.channelName;
         userInfo.channels.push_back( GenerateUniqueHash( channelUuid ) );
         AddUserToChannelInternal( userHashLookup, channelUuid, channelName, otherUsersToNotify );
         kvIter++;
      }

    /*  KeyValueConstIterator kvIter2 = newConnection->GetFriends().begin();
      while( kvIter2 != newConnection->GetFriends().end() )
      {
         const string& friendUuid = kvIter2->key;
         otherUsersToNotify.insert( GenerateUniqueHash( friendUuid ) );
         kvIter2++;
      }*/
   }
   
   //**************************************************************
   // todo, we now need to step through friends and tell each friend that this user has logged in
   //**************************************************************

   m_userUuidMap.insert( UserUuidPair( userHashLookup, userInfo ) );

   InformUsersAboutUserStatusChange( userHashLookup, otherUsersToNotify, UserStatusChange_Login );

   // reciprical notify
   UserUuidSet::const_iterator othersIter = otherUsersToNotify.begin();
   while( othersIter != otherUsersToNotify.end() )
   {
      stringhash lookup = *othersIter++;
      UserUuidMapIterator creatorIter = m_userUuidMap.find( lookup );
      if( creatorIter != m_userUuidMap.end() )
      {
         UserConnection* uc = creatorIter->second.connection;
         if( uc->GetUuid() != userUuid )
         {
            newConnection->NotifyUserStatusHasChanged( uc->GetName(), uc->GetUuid(), UserStatusChange_Login );
         }
      }
      
   }
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
      AssembleListOfUsersToNotifyForAllChannelsForUser( userHashLookup, groupUuidHash, userUuid, otherUsersToNotify );
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
      AddChatChannelToStorage( channelName, channelUuid );

      iter = m_channelMap.find( keyLookup );// used below
   }

   // verify that user is not in list before adding him/her.
   ChatChannel& channelInfo = iter->second;
   bool found = false;
   list< stringhash >::iterator verifyNotInListIter = channelInfo.userUuidList.begin();
   while( verifyNotInListIter != channelInfo.userUuidList.end() )
   {
      if( *verifyNotInListIter++ == hashedUserUuid )
      {
         found = true;
         break;
      }  
   }
   if( found == false )
   {
      channelInfo.userUuidList.push_back( hashedUserUuid );
   }

   list< stringhash >::iterator usersToInform = channelInfo.userUuidList.begin();
   while( usersToInform != channelInfo.userUuidList.end() )
   {
      if( *usersToInform != hashedUserUuid )
      {
         otherUsersToNotify.insert( *usersToInform );
      }
      usersToInform ++;
   }
}

//---------------------------------------------------------------------

void     ChatChannelManager::AddChatchannel( int id, const string& channelName, const string& channelUuid, bool isActive, int maxPlayers, int gameType, int gameId, const string& createDate )
{
   stringhash  keyLookup = GenerateUniqueHash( channelUuid );

   ChannelMapIterator iter = m_channelMap.find( keyLookup );
   
   if( iter != m_channelMap.end() )
   {
      ChatChannel& channel = iter->second;
      channel.recordId = id;
      channel.name = channelName;
      channel.uuid = channelUuid;
      channel.maxPlayers = maxPlayers;
      channel.gameType = gameType;
      channel.gameId = gameId;
      channel.createDate = createDate;

      RequestAllUsersInChatChannel( channelUuid, true, "" );
   }
   else
   {    
      ChatChannel newChannel;
      newChannel.recordId = id;
      newChannel.name = channelName;
      newChannel.uuid = channelUuid;
      newChannel.maxPlayers = maxPlayers;
      newChannel.gameType = gameType;
      newChannel.gameId = gameId;
      newChannel.createDate = createDate;

      ChannelMapPair channelMapData;
      channelMapData.first = keyLookup;
      channelMapData.second = newChannel;

      m_channelMap.insert( channelMapData );


      RequestAllUsersInChatChannel( channelUuid, true, "" );
   }
}

void     ChatChannelManager::AddUserToChannel( const string& channelUuid, UserBasics& ub )
{
   stringhash  keyLookup = GenerateUniqueHash( channelUuid );

   ChannelMapIterator iter = m_channelMap.find( keyLookup );
   if( iter == m_channelMap.end() )
   {
      return;
   }

   ChatChannel& channelMapData = iter->second;
   list< UserBasics >   ::iterator it = channelMapData.userBasics.begin();
   while( it != channelMapData.userBasics.end() )
   {
      if( it->userUuid == ub.userUuid )
         return;
      it++;
   }
   channelMapData.userBasics.push_back( ub );
}

void     ChatChannelManager::AddAllUsersToChannel( const string& channelUuid, const SerializedKeyValueVector< string >& usersAndIds )
{
   stringhash  keyLookup = GenerateUniqueHash( channelUuid );

   ChannelMapIterator iter = m_channelMap.find( keyLookup );
   if( iter == m_channelMap.end() )
   {
      assert( 0 );// wtf
   }

   ChatChannel& channelMapData = iter->second;
   SerializedKeyValueVector< string >::const_KVIterator userIt = usersAndIds.begin();
   while( userIt != usersAndIds.end() )
   {
      channelMapData.userBasics.push_back( UserBasics( userIt->value, userIt->key ) );
      userIt ++;
      //userBasics ub;
      /*
      UserUuidMapIterator receiverIter = m_userUuidMap.find( receiverLookup );
      if( receiverIter != m_userUuidMap.end() )
      {
         UserConnection* connection = receiverIter->second.connection;
         */
   }

   //NotifyAddedToChannel( channelUuid, const string& userUuid, true );

}

bool     ChatChannelManager::GetChatChannels( const string& uuid, ChannelKeyValue& availableChannels )
{
   availableChannels.clear();

   ChannelMapIterator iter = m_channelMap.begin();
   while( iter != m_channelMap.end() )
   {
      ChatChannel& channelMapData = iter->second;
      iter++;
      list< UserBasics >::iterator it = channelMapData.userBasics.begin();
      while( it != channelMapData.userBasics.end() )
      {
         UserBasics& ub = *it++;
         if( ub.userUuid == uuid )
         {
            availableChannels.insert( channelMapData.uuid, ChannelInfo( channelMapData.name, channelMapData.uuid, channelMapData.gameType, channelMapData.gameId, 0, channelMapData.isActive ) );
         }
      }

   }
   if( availableChannels.size() > 0 )
      return true;

   return false;
}

//---------------------------------------------------------------------

bool     ChatChannelManager::AddChatChannelToStorage( const string& channelName, const string& channelUuid )
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
      //m_channelsNeedingQueryInfo.push_back( keyLookup );// we need to go to the db to pull all of the info for this channel



      m_channelMap.insert( channelMapData);
      m_chatServer->ChatChannelManagerNeedsUpdate();

      //RequestAllUsersInChatChannel( const string& channelUuid, bool fullList, const string& authUuid )

      return true;
   }

   return false;
}

//---------------------------------------------------------------------
void     ChatChannelManager::AssembleListOfUsersToNotifyForAllChannelsForUser( stringhash userUuidHash, stringhash channelUuid, const string& userUuid, UserUuidSet& otherUsersToNotify )
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
      stringhash test = *userIter++;
      if( test == userUuidHash )
      {
         ChatChannel& channelInfo = channelIter->second;

         list< stringhash >::iterator usersToInform = channelInfo.userUuidList.begin();
         while( usersToInform != channelInfo.userUuidList.end() )
         {
            otherUsersToNotify.insert( *usersToInform++ );
         }
      }
   }
}

//---------------------------------------------------------------------

void     ChatChannelManager::RemoveUserFromChannelInternal( stringhash userUuidHash, stringhash channelHashUuid, const string& userUuid, UserUuidSet& otherUsersToNotify )
{
   ChannelMapIterator channelIter = m_channelMap.find( channelHashUuid );
   if( channelIter == m_channelMap.end() )
   {
      string text = " Cleaning up channel ";
      text += userUuid;
      text += " tried to leave non-existent channel (hashed) ";
      //text += channelUuid;
      m_chatServer->Log( text, 1 );
      return;
   }
   string channelUuid = channelIter->second.uuid;

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

      if( otherUsersToNotify.size() == 0 )/// we've removed all users from this chat channel.
      {
         DeleteChannel( channelUuid, 0 );
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

void     ChatChannelManager::UserSendsChatToChannel( const string& senderUuid, const string& channelUuid, const string& message, U16 gameTurn )
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
      m_chatServer->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_NoChatChannel );
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
   if( channel.userUuidList.size() < 1 )
   {
      m_chatServer->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_YouAreTheOnlyPersonInThatChatChannel  );
      string text = " User ";
      text += senderUuid;
      text += " tried to send text on chat channel ";
      text += channelUuid;
      text += " but there is no one in that group";
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
         connection->SendChat( message, senderUuid, userSender.userName, channelUuid, GetDateInUTC() );
      }
   }

   string friendUuid;
   WriteChatToDb( message, senderUuid, friendUuid, channelUuid, gameTurn, connectionId );
}

//---------------------------------------------------------------------

void     ChatChannelManager::UserSendsChatToUser( const string& senderUuid, const string& destinationUuid, const string& message )
{
   stringhash senderHash = GenerateUniqueHash( senderUuid );
   stringhash receiverHash = GenerateUniqueHash( destinationUuid );

   UserUuidMapIterator senderIter = m_userUuidMap.find( senderHash );
   if( senderIter == m_userUuidMap.end() )
   {
      string text = " User ";
      text += senderUuid;
      text += " tried to send text to user ";
      text += destinationUuid;
      text += " but the sender does not exist in ChatChannelManager";
      m_chatServer->Log( text, 1 );
      return;
   }

   ChatUser& userSender = senderIter->second;
   U32 connectionId = userSender.connection->GetConnectionId();

   UserUuidMapIterator receiverIter = m_userUuidMap.find( receiverHash );
   if( receiverIter == m_userUuidMap.end() )
   {
      string text = " User ";
      text += senderUuid;
      text += " tried to send text on user ";
      text += destinationUuid;
      text += " but the receiver does not exist in ChatChannelManager";
      m_chatServer->Log( text, 1 );
      //return;
      // this is totally fine.. you can send a message to another player who is not online
      m_chatServer->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_UserNotOnline );
    }

   else //( receiverIter
   {
      UserConnection* connection = receiverIter->second.connection;
      connection->SendChat( message, senderUuid, senderIter->second.userName, "", GetDateInUTC() );
   }

   //-----------------------------------------

   string channelUuid;
   WriteChatToDb( message, senderUuid, destinationUuid, channelUuid, 0, connectionId );
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

   string queryString = "INSERT INTO chat_channel VALUES( null, '%s','";
   queryString += newId;
   queryString += "', 1, 32, 0, 0)";// note the 1 here used to indicate that this is a server created channel

   list< string > listOfStrings;
   listOfStrings.push_back( channelName );
   U32 serverId = 0;
   int jobId = DbQueryAndPacket( channelName, newId, serverId, authUuid, authHash, queryString, ChatChannelDbJob::JobType_Create, false, &listOfStrings );

   return true;
}

//---------------------------------------------------------------------

bool   ChatChannelManager::CreateNewChannel( const PacketChatCreateChatChannelFromGameServer* pPacket )
{
   //***********************************************
   // todo, add lookup to prevent duplicate channels
   //***********************************************

   time_t t = time(0);   // get time now
   struct tm * now = localtime( & t );
   
   int year = now->tm_year - 100; // 113 => 2013.
   int month = now->tm_mon;
   int day = now->tm_mday;

   string date = GetDateInUTC();


   std::stringstream ss;
   const int maxNumberGameInstanceDigits = 7; // into the tens of millions of games
   ss  << std::setfill('0') << pPacket->gameName << '_' << std::setw(2) << year << ':' << std::setw(2) << month << ':' << std::setw(2) << day << '_' << std::setw( maxNumberGameInstanceDigits ) << pPacket->gameId;
   string channelName = ss.str();

   U32 xorValue = 0xFFFFFFFF;
   xorValue  = GetCurrentMilliseconds();

   string newUuid = GenerateUUID( xorValue );

   string queryString = "INSERT INTO chat_channel VALUES( null, '";
   queryString += channelName;
   queryString += "', '";
   queryString += newUuid;
   queryString += "', 1, 32, '";
   queryString += boost::lexical_cast< string >( (int)( pPacket->gameProductId ) );
   queryString += "', '";
   queryString += boost::lexical_cast< string >( (int)( pPacket->gameId ) );
   queryString += "', '";
   queryString += date;
   queryString += "')";// note the 1 here used to indicate that this is a server created channel

   // no need to escape these values
   string authUuid;
   stringhash authHash = 0;
   U32 serverId = pPacket->gameInstanceId;
   int jobId = DbQueryAndPacket( channelName, newUuid, serverId, authUuid, pPacket->gameId, queryString, ChatChannelDbJob::JobType_CreateFromGameServer, false );

   DbJobIterator iter;
   bool success = FindDbJob( jobId, m_pendingDbJobs, iter );
   if( success )
   {
      iter->stringBucket = pPacket->userUuidList;
   }

   return true;
}

//---------------------------------------------------------------------

bool   ChatChannelManager::DeleteChannel( const PacketChatDeleteChatChannelFromGameServer* pPacket )
{
   //***********************************************
   // UPDATE chat_channel SET is_active=0 WHERE game_type=1 AND game_instance_id=13245
   string queryString = "UPDATE chat_channel SET is_active=0 WHERE game_type='";
   queryString += boost::lexical_cast< string >( (int)( pPacket->gameProductId ) );
   queryString += "' AND game_instance_id='";
   queryString += boost::lexical_cast< string >( (int)( pPacket->gameId ) );
   queryString += "'";
   U32 serverId = 0;

   int jobId = DbQueryAndPacket( "", "", pPacket->gameInstanceId, "", pPacket->gameId, queryString, ChatChannelDbJob::JobType_MakeInactiveFromGameServer, false );

 /*  DbJobIterator iter;
   bool success = FindDbJob( jobId, m_pendingDbJobs, iter );*/
   return true;
}

//---------------------------------------------------------------------

int  ChatChannelManager::DbQueryAndPacket( const string& channelName, const string& channelUuid, U32 serverId, const string& authUuid, stringhash chatUserLookup, const string& queryString, ChatChannelDbJob::JobType jobType, bool isFandF, list< string >* sanitizedStrings )
{
   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id = GetConnectionId();
   dbQuery->lookup = AddDbJob( channelName, channelUuid, serverId, chatUserLookup, GenerateUniqueHash( authUuid ), jobType );
   dbQuery->meta = authUuid;
   dbQuery->isFireAndForget = isFandF;
   dbQuery->query = queryString;

   if( sanitizedStrings )
   {
      list< string >::iterator it = sanitizedStrings->begin();
      while( it != sanitizedStrings->end() )
      {
         dbQuery->escapedStrings.insert( *it++ );
      }
   }
   int id = dbQuery->lookup;

   if( m_chatServer->AddPacketFromUserConnection( dbQuery, GetConnectionId() ) == false )
   {
      cout << "ChatChannelManager:: Query packet rejected" << endl;
      
      delete dbQuery;
   }

   return id;
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

   return DeleteChannel( channelUuid, 0 );
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

   string queryString = "UPDATE chat_channel SET chat_channel.is_active=0 WHERE chat_channel.uuid='%s'";

   list< string > listOfStrings;
   listOfStrings.push_back( channelUuid );

   string authUuid;
   stringhash authHash = 0;
   DbQueryAndPacket( channelUuid, channelUuid, serverId, authUuid, authHash, queryString, ChatChannelDbJob::JobType_Delete, false, &listOfStrings );

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
   stringhash channelHash = GenerateUniqueHash( channelUuid );
   UserUuidMapIterator authIter = m_userUuidMap.end();

   if( authUuid.size() )
   {
      stringhash authUserHashLookup = GenerateUniqueHash( authUuid );
      authIter = m_userUuidMap.find( authUserHashLookup );
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
   }

   if( userUuid.size() == 0 )
   {
      string text = " User ";
      text += authUuid;
      text += " tried to add a user with no UUID to a chat channel ";
      text += channelUuid;
      text += " but that user does not exist in ChatChannelManager";
      m_chatServer->Log( text, 1 );
      return false;
   }
  /* UserUuidMapIterator userIter = m_userUuidMap.find( userHashLookup );
   if( userIter == m_userUuidMap.end() )
   {
      if( authIter != m_userUuidMap.end() )
      {
         m_chatServer->SendErrorToClient( PacketErrorReport::ErrorType_UserDoesNotExist, authIter->second.connection->GetConnectionId()  );
      }

      string text = " User ";
      text += authUuid;
      text += " tried to add a user ";
      text += userUuid;
      text += " to a chat channel ";
      text += channelUuid;
      text += " but that user does not exist in ChatChannelManager";
      m_chatServer->Log( text, 1 );
      return false;
   }*/
   //-----------------------------------------

   ChannelMapIterator channelIter = m_channelMap.find( channelHash );
   if( channelIter == m_channelMap.end() )
   {
      //m_chatServer->SendErrorToClient( PacketErrorReport::ErrorType_BadChatChannel, userIter->second.connection->GetConnectionId()  );

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
  /* while( userIt != channel.userUuidList.end() )
   {
      if( userHashLookup == *userIt++ )
      {
         //m_chatServer->SendErrorToClient( PacketErrorReport::ErrorType_CannotAddUserToChannel_AlreadyExists, userIter->second.connection->GetConnectionId()  );
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
   }*/

   //***********************************************
   // todo, don't change inactive groups to inactive
   // check for user permissions
   //***********************************************

   string queryString = "INSERT INTO user_join_chat_channel VALUES ('%s','%s', null, null, null )";

   list< string > listOfStrings;
   listOfStrings.push_back( userUuid );
   listOfStrings.push_back( channelUuid );

   U32 serverId = 0;
   DbQueryAndPacket( channelUuid, userUuid, serverId, authUuid, userHashLookup, queryString, ChatChannelDbJob::JobType_AddUser, false, &listOfStrings );

   UserUuidSet otherUsersToNotify;
   AddUserToChannelInternal( userHashLookup, channelUuid, channel.name, otherUsersToNotify );
   InformUsersAboutUserStatusChange( userHashLookup, otherUsersToNotify, UserStatusChange_UserAddedToChannel );

   return false;
}

//---------------------------------------------------------------------

bool     ChatChannelManager::AddUserToChannel( const PacketChatAddUserToChatChannelGameServer* pPacket )
{
   //***********************************************
   // SELECT * FROM chat_channel
   string queryString = "SELECT * FROM chat_channel WHERE game_type='";
   queryString += boost::lexical_cast< string >( (int)( pPacket->gameProductId ) );
   queryString += "' AND game_instance_id='";
   queryString += boost::lexical_cast< string >( (int)( pPacket->gameId ) );
   queryString += "'";
   U32 serverId = 0;

   int jobId = DbQueryAndPacket( "", "", pPacket->gameInstanceId, pPacket->userUuid, pPacket->gameId, queryString, ChatChannelDbJob::JobType_AddUserFromGameServer, false );

 /*  DbJobIterator iter;
   bool success = FindDbJob( jobId, m_pendingDbJobs, iter );*/

   return true;
}

//---------------------------------------------------------------------

bool     ChatChannelManager::RemoveUserFromChannel( const string& channelUuid, const string& userUuid, const string& authUuid )
{
   stringhash userHashLookup = GenerateUniqueHash( userUuid );
   stringhash channelHash = GenerateUniqueHash( channelUuid );
   UserUuidMapIterator authIter = m_userUuidMap.end();

   if( authUuid.size() )
   {
      stringhash authUserHashLookup = GenerateUniqueHash( authUuid );
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
   }

   UserUuidMapIterator userIter = m_userUuidMap.find( userHashLookup );
   if( userIter == m_userUuidMap.end() )
   {
      if( authIter != m_userUuidMap.end() )
      {
         m_chatServer->SendErrorToClient( authIter->second.connection->GetConnectionId(), PacketErrorReport::ErrorType_UserDoesNotExist  );
      }
      string text = " User id=";
      text += authUuid;
      text += " tried to remove user id=";
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
      m_chatServer->SendErrorToClient( userIter->second.connection->GetConnectionId(), PacketErrorReport::ErrorType_BadChatChannel  );

      string text = " User id=";
      text += authUuid;
      text += " tried to remove a user id=";
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

   string queryString = "DELETE FROM user_join_chat_channel WHERE user_join_chat_channel.user_uuid ='%s' AND  user_join_chat_channel.channel_uuid='%s'";
   
   list< string > listOfStrings;
   listOfStrings.push_back( userUuid );
   listOfStrings.push_back( channelUuid );

   U32 serverId = 0;
   DbQueryAndPacket( channelUuid, userUuid, serverId, authUuid, userHashLookup, queryString, ChatChannelDbJob::JobType_RemoveUser, false, &listOfStrings );

   UserUuidSet otherUsersToNotify;
   ChatChannel& channel = channelIter->second;
   RemoveUserFromChannelInternal( userHashLookup, channelHash, userUuid, otherUsersToNotify );
   InformUsersAboutUserStatusChange( userHashLookup, otherUsersToNotify, UserStatusChange_UserRemovedFromChannel );

   return true;
}
//---------------------------------------------------------------------

bool     ChatChannelManager::RemoveUserFromChannel( const PacketChatRemoveUserFromChatChannelGameServer* pPacket )
{
   //***********************************************
   // SELECT * FROM chat_channel
   string queryString = "SELECT * FROM chat_channel WHERE game_type='";
   queryString += boost::lexical_cast< string >( (int)( pPacket->gameProductId ) );
   queryString += "' AND game_instance_id='";
   queryString += boost::lexical_cast< string >( (int)( pPacket->gameId ) );
   queryString += "'";
   U32 serverId = 0;

   int jobId = DbQueryAndPacket( "", "", pPacket->gameInstanceId, pPacket->userUuid, pPacket->gameId, queryString, ChatChannelDbJob::JobType_RemoveUserFromGameServer, false );

 /*  DbJobIterator iter;
   bool success = FindDbJob( jobId, m_pendingDbJobs, iter );*/
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
      m_chatServer->SendErrorToClient( userIter->second.connection->GetConnectionId(), PacketErrorReport::ErrorType_BadChatChannel  );

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
   queryString =  "SELECT DISTINCT user.user_name, user.uuid, user.user_id FROM users as user ";
   queryString += "join user_join_chat_channel as joiner on joiner.user_uuid=user.uuid ";
   queryString += "join chat_channel as channel on joiner.channel_uuid=channel.uuid ";
   queryString += "join chat as chat_log on channel.uuid=chat_log.chat_channel_id ";
   queryString += "where channel.uuid='%s'";

   list< string > listOfStrings;
   listOfStrings.push_back( channelUuid );

   return CreateAndSendUserListJob( queryString, channelIter->second.name, channelUuid, 0, authUuid, userHashLookup, ChatChannelDbJob::JobType_FindChatters, &listOfStrings );
}
//---------------------------------------------------------------------

bool     ChatChannelManager::RequestAllUsersInChatChannel( const string& channelUuid, bool fullList, const string& authUuid )
{
   UserUuidMapIterator userIter = m_userUuidMap.end();
   stringhash userHashLookup =  0;
   if( authUuid.size() )
   {
      userHashLookup = GenerateUniqueHash( authUuid );
      userIter = m_userUuidMap.find( userHashLookup );
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
   }

   //-----------------------------------------

   stringhash channelHash = GenerateUniqueHash( channelUuid );
   ChannelMapIterator channelIter = m_channelMap.find( channelHash );
   if( channelIter == m_channelMap.end() )
   {
      if( userIter != m_userUuidMap.end() )
      {
         m_chatServer->SendErrorToClient( userIter->second.connection->GetConnectionId(), PacketErrorReport::ErrorType_BadChatChannel  );
      }

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
   queryString =  "SELECT user.user_name, user.uuid, user.user_id FROM users AS user ";
   queryString += "join user_join_chat_channel as joiner on joiner.user_uuid=user.uuid ";
   queryString += "join chat_channel as channel on joiner.channel_uuid=channel.uuid ";
   queryString += "where channel.uuid='%s'";

   list< string > listOfStrings;
   listOfStrings.push_back( channelUuid );

   return CreateAndSendUserListJob( queryString, channelIter->second.name, channelUuid, 0, authUuid, userHashLookup, ChatChannelDbJob::JobType_AllUsersInChannel, &listOfStrings );
};

//---------------------------------------------------------------------

bool     ChatChannelManager::CreateAndSendUserListJob( const string& queryString, const string& channelName, const string& channelUuid, U32 serverId, const string& requestorUuid, stringhash chatUserLookup, ChatChannelDbJob::JobType type, list< string >* sanitizedStrings )
{
   DbQueryAndPacket( channelName, channelUuid, serverId, requestorUuid, chatUserLookup, queryString, type, false, sanitizedStrings );

   return true;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------

void     ChatChannelManager::LoadSingleChannel( string uuid, int gameId, int chatChannelId )
{
   if( uuid.size() == 0 && gameId == 0 && chatChannelId == 0 )
   {
      string text = " Bad params tp ChatChannelManager::LoadSingleChannel";
      m_chatServer->Log( text, 1 );
      return;
   }
   string query = "SELECT * FROM chat_channel WHERE ";
   if( chatChannelId != 0 )
   {
      query += "id='";
      query += boost::lexical_cast< string >( chatChannelId );
      query += "'";
   }
   else if ( uuid.size() > 0 )
   {
      query += "uuid='";
      query += uuid;
      query += "'";
   }
   else
      {
      query += "game_type='";
      query += boost::lexical_cast< string >( gameId );
      query += "'";
   }

   U32 serverId = 0;
   DbQueryAndPacket( "", "", serverId, "", 0, query, ChatChannelDbJob::JobType_LoadSingleChannel, false );
}

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
      string queryString = "SELECT * FROM chat_channel"; //"WHERE is_active=1";
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
   // SELECT * FROM chat_channel where game_instance_id=1
   string queryString = "SELECT * FROM chat_channel WHERE game_instance_id<>0";
   string authUuid;
   stringhash authHash = 0;
   DbQueryAndPacket( authUuid, authUuid, serverId, authUuid, authHash, queryString, ChatChannelDbJob::JobType_SelectAllChannelsToSendToAuth, false );
}

//---------------------------------------------------------------------
/*
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
}*/

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

   m_jobMutex.lock();
   m_pendingDbJobs.push_back( job );
   m_jobMutex.unlock();

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
   string queryString = "INSERT INTO chat_message VALUES( null, _utf8'%s', '%s', ";
   if( friendUuid.size() > 0 )
   {
      queryString += "'%s'";
   }
   else
   {
      queryString += "null";
   }
   queryString += ", '%s', CURRENT_TIMESTAMP, ";
   queryString += boost::lexical_cast< string >( gameTurn );
   queryString += ")";
   dbQuery->escapedStrings.insert( message );
   dbQuery->escapedStrings.insert( senderUuid );
   if( friendUuid.size() > 0 )
   {
      dbQuery->escapedStrings.insert( friendUuid );
   }
   dbQuery->escapedStrings.insert( channelUuid );

   dbQuery->query = queryString;
   if( m_chatServer->AddPacketFromUserConnection( dbQuery, connectionId ) == false )
   {
      cout << "ChatChannelManager:: Query packet rejected" << endl;
      delete dbQuery;
   }
}

//---------------------------------------------------------------------


//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////