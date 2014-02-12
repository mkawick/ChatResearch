// ChatChannelManager.cpp

#include <assert.h>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
using boost::format;

#include "ChatChannelManager.h"
#include "../NetworkCommon/Packets/BasePacket.h"
#include "../NetworkCommon/Packets/ChatPacket.h"
#include "../NetworkCommon/Packets/DbPacket.h"
#include "../NetworkCommon/Packets/PacketFactory.h"
#include "DiplodocusChat.h"

#include "../NetworkCommon/Utils/TableWrapper.h"

DiplodocusChat*        ChatChannelManager::m_chatServer;

// ChatChannelManagerNeedsUpdate

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

ChatChannelManager::ChatChannelManager(): m_dbIdTracker( 0 ),
                                          m_isInitialized( false )
{
   //m_inboundPackets.reserve( 120 );// absolutely arbitrary.. it seems big enough and log(n) means that this will never be reallocated much
}

//---------------------------------------------------------------

ChatChannelManager::~ChatChannelManager()
{
   m_inboundPackets.clear();
}

//---------------------------------------------------------------

void     ChatChannelManager::Init()
{
   string queryString = "SELECT * FROM chat_channel";
   U32 serverId = 0;

   PacketDbQuery* dbQuery = DbQueryFactory( queryString, false );
   SaveQueryDetails( dbQuery, "", "", 0, ChatChannelDbJob::JobType_LoadAllChannels );
   Send( dbQuery );

   m_isInitialized = true;

   //DbQueryAndPacket( "", "", serverId, "", 0, queryString, ChatChannelDbJob::JobType_LoadAllChannels, false );
}
/*
Example:
PacketDbQuery* query = DbQueryFactory( const string& queryString, bool isFandF );
SaveQueryDetails( dbQuery, const string& channelUuid, const string& authUuid, const string& chatUserLookup, ChatChannelDbJob::JobType jobType, U32 serverId = 0, const string& debugString = "debug_string" );
AddSanitizedStrings( dbQuery, list< string >* sanitizedStrings );
AddCustomData( dbQuery, void* data );
Send( dbQuery );*/

//---------------------------------------------------------------

bool     ChatChannelManager::Update()
{
   if( m_isInitialized == false )
   {
      Init();
      return false;
   }

   PacketFactory factory;

   deque< BasePacket* >::iterator it = m_inboundPackets.begin();
   while( it != m_inboundPackets.end() )
   {
      BasePacket* packet = *it++;
      ProcessIncomingPacket( packet );
      //factory.
   }

   m_inboundPackets.clear();

   //----------------------------

   deque< PacketDbQueryResult* >::iterator itDb = m_dbResults.begin();
   while( itDb != m_dbResults.end() )
   {
      PacketDbQueryResult* dbResult = *itDb++;
      int jobId = dbResult->lookup;

      DbJobIterator iter;
      bool success = FindDbJob( jobId, m_jobsPending, iter );
      if( success )
      {
         ChatChannelDbJob& job = *iter;
         ProcessDbResult( dbResult, job );
         m_jobsPending.erase( iter );

         BasePacket* packet = static_cast< BasePacket* >( dbResult );
         factory.CleanupPacket( packet );
      }
   }

   m_dbResults.clear();

   return true;
}

//---------------------------------------------------------------

bool     ChatChannelManager::AddInboundPacket( BasePacket* packet ) // not thread safe.. obviously
{
   m_inboundPackets.push_back( packet );
   return true;
}

//---------------------------------------------------------------

bool     ChatChannelManager::HandleDbResult( PacketDbQueryResult* packet ) // not thread safe.. obviously
{
   m_dbResults.push_back( packet );
   return true;
}

//---------------------------------------------------------------

void     ChatChannelManager::ProcessIncomingPacket( BasePacket* packet )// should mostly be database 
{
   PacketFactory factory;
   factory.CleanupPacket( packet );
}

//---------------------------------------------------------------

bool     ChatChannelManager::ProcessDbResult( PacketDbQueryResult* dbResult, ChatChannelDbJob& job )
{
   switch( job.jobType )
   {
   case ChatChannelDbJob::JobType_LoadAllChannels:
      {
         /*if( job.chatUserLookup != 0 )// when users request this which is the majority of cases
         {
            UserUuidMapIterator creatorIter = m_userUuidMap.find( job.chatUserLookup );
            if( creatorIter == m_userUuidMap.end() )
            {
               return false;
            }
            UserConnection* connection = creatorIter->second.connection;
            connection->NotifyAllChannelsLoaded( dbResult->successfulQuery );
         }*/

         if( dbResult->successfulQuery == true )
         {
            //bool  testOnlyLoadOne = false;
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
               
               //if( testOnlyLoadOne == false )
                  AddChatchannel( channelId, name, uuid, isActive, maxPlayers, gameType, gameId, createDate );
               
               //testOnlyLoadOne = true;
            }
         };
      }
      break;
   case  ChatChannelDbJob::JobType_AllUsersInChannel:
      {
         /*UserConnection* connection = NULL;
         UserUuidMapIterator creatorIter = m_userUuidMap.find( job.chatUserLookup );
         if( creatorIter != m_userUuidMap.end() )
         {
            connection = creatorIter->second.connection;
         }*/
         
         
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
          /*  if( connection )
            {
               connection->SendListOfAllUsersInChatChannel( job.uuid, usersAndIds );
            }
            else
            {*/
               AddAllUsersToChannel( job.lookupString, usersAndIds );
           // }
         }

      }
      break;
   }
   return true;
}

//---------------------------------------------------------------------

int     ChatChannelManager::AddDbJob( const string& debugString, const string& lookupString, U32 serverId, stringhash chatUserLookup, stringhash authUserLookup, ChatChannelDbJob::JobType type )
{
   ChatChannelDbJob job;
   job.debugString = debugString;
   job.lookupString = lookupString;
   job.chatUserLookup = chatUserLookup;
   job.authUserLookup = authUserLookup;
   job.jobType = type;
   job.jobId = NewDbId();
   job.serverId = serverId;

   m_jobsPending.push_back( job );

   return job.jobId;
}

//---------------------------------------------------------------------

PacketDbQuery* ChatChannelManager::DbQueryFactory( const string& queryString, bool isFandF )
{
   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id = ChatChannelManagerUniqueId;
   dbQuery->isFireAndForget = isFandF;
   dbQuery->query = queryString;
   return dbQuery;
}

//---------------------------------------------------------------------

bool  ChatChannelManager::SaveQueryDetails( PacketDbQuery* dbQuery, const string& channelUuid, const string& authUuid, stringhash chatUserLookup, ChatChannelDbJob::JobType jobType, U32 serverId, const string& debugString)
{
   assert( dbQuery != NULL );
   dbQuery->lookup = AddDbJob( debugString, channelUuid, serverId, chatUserLookup, GenerateUniqueHash( authUuid ), jobType );
   return true;
}

//---------------------------------------------------------------------

bool  ChatChannelManager::AddSanitizedStrings( PacketDbQuery* dbQuery, list< string >& sanitizedStrings )
{
   list< string >::iterator it = sanitizedStrings.begin();
   while( it != sanitizedStrings.end() )
   {
      dbQuery->escapedStrings.insert( *it++ );
   }
   return true;
}

//---------------------------------------------------------------------

bool  ChatChannelManager::AddCustomData( PacketDbQuery* dbQuery, void* data )
{
   assert( dbQuery->customData == NULL );
   dbQuery->customData = data;
   return true;
}

//---------------------------------------------------------------------

bool  ChatChannelManager::Send( PacketDbQuery* dbQuery )
{
   if( m_chatServer->AddQueryToOutput( dbQuery, ChatChannelManagerUniqueId, true ) == false )
   {
      cout << "ChatChannelManager:: Query packet rejected" << endl;
      
      delete dbQuery;
      return false;
   }
   return true;
}

//---------------------------------------------------------------
/*
int      ChatChannelManager::DbQueryAndPacket( U32 serverId, 
                                              const string& authUuid, 
                                              stringhash chatUserLookup, 
                                              const string& queryString, 
                                              ChatChannelDbJob::JobType jobType, 
                                              bool isFandF, 
                                              list< string >* sanitizedStrings,
                                              void* metaData )
{
   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id = ChatChannelManagerUniqueId;
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
   int queryId = dbQuery->lookup;

   if( m_chatServer->AddQueryToOutput( dbQuery, ChatChannelManagerUniqueId, true ) == false )
   {
      cout << "ChatChannelManager:: Query packet rejected" << endl;
      
      delete dbQuery;
   }

   return queryId;
}

//---------------------------------------------------------------

void     ChatChannelManager::AddMetaData( PacketDbQueryResult* dbQuery, void* myData )
{
   assert( dbQuery->customData == NULL );
   dbQuery->customData = myData;
}*/

//---------------------------------------------------------------------
//---------------------------------------------------------------

int      ChatChannelManager::NewDbId()
{
   if( ++m_dbIdTracker < 0 ) // wrap around
      m_dbIdTracker = 1;
   return m_dbIdTracker;
}


//---------------------------------------------------------------------

bool     ChatChannelManager::FindDbJob( int jobId, list< ChatChannelDbJob >& listOfJobs, DbJobIterator& iter )
{
   //Threading::MutexLock locker ( m_jobMutex );
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

      QueryAllUsersInChatChannel( channelUuid );
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


      QueryAllUsersInChatChannel( channelUuid );
   }
}

//---------------------------------------------------------------------

void     ChatChannelManager::AddUserToChannel( const string& channelUuid, const UserBasics& ub )
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

//---------------------------------------------------------------------

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

//---------------------------------------------------------------------

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

bool     ChatChannelManager::QueryAllUsersInChatChannel( const string& channelUuid )
{
   assert( channelUuid.size() != 0 );
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

   list< string > sanitizedStrings;
   sanitizedStrings.push_back( channelUuid );

   PacketDbQuery* dbQuery = DbQueryFactory( queryString, false );
   SaveQueryDetails( dbQuery, channelUuid, "", 0, ChatChannelDbJob::JobType_AllUsersInChannel );
   AddSanitizedStrings( dbQuery, sanitizedStrings );
    return Send( dbQuery );

   //return CreateAndSendUserListJob( queryString, channelIter->second.name, channelUuid, 0, authUuid, userHashLookup, ChatChannelDbJob::JobType_AllUsersInChannel, &listOfStrings );
};

//---------------------------------------------------------------------
/*
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
};*/

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////