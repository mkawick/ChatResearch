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
#include "../NetworkCommon/Packets/ServerToServerPacket.h"
#include "DiplodocusChat.h"
#include "ChatUser.h"


DiplodocusChat*        ChatChannelManager::m_chatServer;

// ChatChannelManagerNeedsUpdate

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

ChatChannelManager::ChatChannelManager(): m_dbIdTracker( 0 ),
                                          m_isInitialized( false ),
                                          m_numChannelsToLoad( 0 ),
                                          m_isPullingAllUsersAndChannels( false )
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
   string queryString = "SELECT * FROM chat_channel WHERE is_active=1";
   U32 serverId = 0;

   cout << "Chat channel manager initializing" << endl;
   PacketDbQuery* dbQuery = DbQueryFactory( queryString, false );
   SaveQueryDetails( dbQuery, "", "", 0, ChatChannelDbJob::JobType_LoadAllChannels );
   Send( dbQuery );

   m_isInitialized = true;
   m_isPullingAllUsersAndChannels = true;

   time( &m_initializationTimeStamp );

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
      else
      {
         assert( 0 );
      }
   }

   m_dbResults.clear();

   if( m_isPullingAllUsersAndChannels == false && 
      m_initializationTimeStamp != 0 &&
      m_jobsPending.size() == 0 )
   {
      time_t currentTime;
      time( &currentTime );
      double difference = difftime( currentTime, m_initializationTimeStamp );
      m_initializationTimeStamp = 0;

      cout << "Chat channel manager finished initialization after " << difference << " seconds " << endl;
   }

   return m_isPullingAllUsersAndChannels;
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
 /*  case ChatChannelDbJob::JobType_Create:
      {
        HandleChatChannelCreateResult( dbResult, job );
      }
      break;*/
   case ChatChannelDbJob::JobType_LoadAllChannels:
      {
         /*if( job.chatUserLookup != 0 )// when users request this which is the majority of cases
         {
            UserMapIterator creatorIter = m_userMap.find( job.chatUserLookup );
            if( creatorIter == m_userMap.end() )
            {
               return false;
            }
            UserConnection* connection = creatorIter->second.connection;
            connection->NotifyAllChannelsLoaded( dbResult->successfulQuery );
         }*/
         HandleLoadAllChannelsResult( dbResult, job ); 
         
      }
      break;
   case  ChatChannelDbJob::JobType_AllUsersInChannel:
      {
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

               //AddUserToChannel( channelUuid, UserBasics& ub )

               usersAndIds.insert( uuid, name );
            }

            AddAllUsersToChannel( job.lookupString, usersAndIds );
         }
         m_numChannelsToLoad--;// bad channels setup should not stop us

      }
      break;
   case  ChatChannelDbJob::JobType_AllUsersInAllChannels:
      {         
         HandleAllUsersInAllChannelsResult( dbResult, job );

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


//---------------------------------------------------------------------

void     ChatChannelManager::PackageAndSendToOtherServer( BasePacket* packet, U32 serverId )
{
   PacketServerJobWrapper* wrapper = new PacketServerJobWrapper;
   wrapper->pPacket = packet;
   wrapper->serverId = serverId;

   m_chatServer->AddOutputChainData( wrapper, serverId );
}


//---------------------------------------------------------------

bool     ChatChannelManager::GetChatChannels( const string& userUuid, ChannelKeyValue& availableChannels )
{
   availableChannels.clear();

   stringhash userLookup = GenerateUniqueHash( userUuid );
   UserMapIterator userIter = m_userMap.find( userLookup );
   if( userIter != m_userMap.end() )
   {
      const list< stringhash >& listOfChannels = userIter->second.channels;// cache this pointer (optimization)
      list< stringhash >::const_iterator it = listOfChannels.begin();
      while( it != listOfChannels.end() )
      {
         stringhash channelLookupHash = *it++;
         m_channelMap.find( channelLookupHash );
         ChannelMapIterator channelIter = m_channelMap.begin();
         if( channelIter != m_channelMap.end() )
         {
            const ChatChannel& channel = channelIter->second;
            availableChannels.insert( channel.uuid, ChannelInfo( channel.name, channel.uuid, channel.gameType, channel.gameInstanceId, 0, channel.isActive ) );
         }
      }
      return true;
   }
   return false;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------

int      ChatChannelManager::NewDbId()
{
   if( ++m_dbIdTracker < 0 ) // wrap around
      m_dbIdTracker = 1;
   return m_dbIdTracker;
}

ChatChannelManager::ChannelMapIterator   
ChatChannelManager::FindChatChannel( U32 gameInstanceId, U8 gameType )
{
   ChannelMapIterator    iter = m_channelMap.begin();
   while( iter != m_channelMap.end() )
   {
      if( iter->second.gameType == gameType )
      {
         if( iter->second.gameInstanceId == gameInstanceId )
            return iter;
      }
      iter ++;
   }
   return iter;
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

void     ChatChannelManager::AddChatchannel( int id, const string& channelName, const string& channelUuid, bool isActive, int maxPlayers, int gameType, U32 gameInstanceId, const string& createDate )
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
      channel.gameInstanceId = gameInstanceId;
      channel.createDate = createDate;

      QueryAllUsersInChatChannel( channelUuid );
   }
   else
   {    
      // the way I am doing this is 3x faster than the alternative which is to fill out the struct first and then insert the pair. (2 copies)
      ChannelMapPair channelMapData;
      channelMapData.first = keyLookup;
      pair< ChannelMapIterator, bool> newChannelIter  =  m_channelMap.insert( channelMapData );

      ChatChannel& newChannel = newChannelIter.first->second; // this is confusing, but the first element is an iterator and its second param is our new object
      newChannel.recordId = id;
      newChannel.name = channelName;
      newChannel.uuid = channelUuid;
      newChannel.maxPlayers = maxPlayers;
      newChannel.gameType = gameType;
      newChannel.gameInstanceId = gameInstanceId;
      newChannel.createDate = createDate;
   }
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
/*
bool    ChatChannelManager::QueryDeleteChannel( const string& channelUuid, const U32 serverId )
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
      UserMapIterator userInfo = m_userMap.find( test );
      if( userInfo != m_userMap.end() )
      {
         userInfo->second.connection->NotifyChannelRemoved( channelUuid, num );
      }
      userIter ++;
   }

   m_channelMap.erase( channelIter );
   return true;
}*/


//---------------------------------------------------------------------
//---------------------------------------------------------------------

bool   ChatChannelManager::CreateNewChannel( const PacketChatCreateChatChannelFromGameServer* request )
{
   // the gameId and the GameInstandId need to be verified here.. the order matters, and I can't figure out which one is which.
   string channelUuid = CreateNewChannel( request->gameName, "", request->gameInstanceId, request->gameProductId, request->gameId );

   if( channelUuid.size() )
   {
      PacketChatCreateChatChannelFromGameServerResponse* response = new PacketChatCreateChatChannelFromGameServerResponse;
      response->gameId = request->gameInstanceId;
      response->channelUuid = channelUuid;

      PackageAndSendToOtherServer( response, request->gameInstanceId );

      const StringBucket::DataSet& uuidList = request->userUuidList.bucket;
      list< string >::const_iterator it = uuidList.begin();
      while( it != uuidList.end() )
      {
         const string& userUuid = *it++;
         AddUserToChannel( userUuid, channelUuid, "", true );
         NotifyUserThatHeWasAddedToChannel( userUuid, channelUuid );
      }

   }
   return true;
}

//---------------------------------------------------------------------

bool ChatChannelManager::CreateNewChannel( const string& channelName, const string& userUuid )
{
   string channelUuid = CreateNewChannel( channelName, userUuid, 0, 0, 0 );
   ChatUser* user = m_chatServer->GetUserByUuid( userUuid );
   if( user )
   {
      if( channelUuid.size() )
      {
         // m_chatServer->SendErrorToClient( PacketErrorReport::ErrorType_UserDoesNotExist, authIter->second.connection->GetConnectionId()  );
         user->NotifyChannelAdded( channelName, channelUuid, true );
         return true;
      }
      else
      {
         user->SendErrorMessage( PacketErrorReport::ErrorType_ChatChannelCannotBeCreated );
         return false;
      }
      
   }
   return true;
}

//---------------------------------------------------------------------

string  ChatChannelManager::CreateUniqueChatChannelId()
{
   U32 xorValue = 0xFFFFFFFF;
   xorValue  =  GetCurrentMilliseconds();

   string      channelUuid;
   stringhash  channelHashLookup;

   // this should never even loop, but if it does, it should only loop once or twice. The random number generator is mersene.
   do{
      channelUuid = GenerateUUID( xorValue );
      channelHashLookup = GenerateUniqueHash( channelUuid );
      ChannelMapIterator iter = m_channelMap.find( channelHashLookup );
      if( iter == m_channelMap.end() )
         return channelUuid;

   } while( 1 );

   return string();// compiler error only
}

//---------------------------------------------------------------------

string   ChatChannelManager::CreateNewChannel( const string& channelName, const string userUuid, U32 serverId, U8 gameType, U32 gameInstanceId )
{
   stringhash  userKeyLookup = GenerateUniqueHash( userUuid );

   UserMapIterator iter = m_userMap.find( userKeyLookup );
   if( iter == m_userMap.end() )
   {
      userKeyLookup = 0;
   }

   string      channelUuid = CreateUniqueChatChannelId();
   

   string queryString = "INSERT INTO chat_channel VALUES( null, '%s','";
   queryString += channelUuid;
   queryString += "', 1, 32, ";
   queryString += boost::lexical_cast< string >( (U32)( gameType ) );
   queryString += ", ";
   queryString += boost::lexical_cast< string >( gameInstanceId ); // almost always 0
   queryString += " )";// note the 1 here used to indicate that this is a server created channel

   list< string > sanitizedStrings;
   sanitizedStrings.push_back( channelName );

   PacketDbQuery* dbQuery = DbQueryFactory( queryString, true );
   //SaveQueryDetails( dbQuery, channelUuid, channelName, userKeyLookup, ChatChannelDbJob::JobType_Create, serverId );
   AddSanitizedStrings( dbQuery, sanitizedStrings );
   Send( dbQuery );

   return channelUuid;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------

bool   ChatChannelManager::DeleteChannel( const PacketChatDeleteChatChannelFromGameServer* request )
{
   PacketChatDeleteChatChannelFromGameServerResponse* packet = new PacketChatDeleteChatChannelFromGameServerResponse;
   packet->gameId = request->gameId;
   packet->gameProductId = request->gameProductId;
   packet->gameInstanceId = request->gameInstanceId;

   //packet->pPacket = request->pPacket;

   bool success = false;
   U8 gameType = request->gameProductId;
   U32 gameInstanceId = request->gameInstanceId;
   ChannelMapIterator iter = FindChatChannel( gameInstanceId, gameType );
   if( iter == m_channelMap.end() )
   {
      packet->numUsersRemoved = 0;
   }

   else
   {
      success = DeleteChannel( iter->second.uuid );
      packet->numUsersRemoved = iter->second.userBasics.size();
   }

   packet->successfullyDeleted = success;
   PackageAndSendToOtherServer( packet, packet->gameInstanceId );// TODO verify that this is the correct server id

   return success;
}
/*
bool   ChatChannelManager::DeleteChannel( const PacketChatDeleteChatChannelFromGameServer* pPacket )
{
}*/

bool   ChatChannelManager::DeleteChannel( const string& channelUuid )
{
   stringhash  keyLookup = GenerateUniqueHash( channelUuid );
   ChannelMapIterator iter = m_channelMap.find( keyLookup );
   if( iter == m_channelMap.end() )
   {
      return false;
   }

   U32 gameType = iter->second.gameType;
   U32 gameInstanceId = iter->second.gameInstanceId;


   string date = GetDateInUTC();

   //***********************************************
   // UPDATE chat_channel SET is_active=0 WHERE game_type=1 AND game_instance_id=13245
   string queryString = "UPDATE chat_channel SET is_active=0,date_expired='";
   queryString += date;
   queryString += "' WHERE game_type='";
   queryString += boost::lexical_cast< string >( (int)( gameType ) );
   queryString += "' AND game_instance_id='";
   queryString += boost::lexical_cast< string >( (int)( gameInstanceId ) );
   queryString += "'";
   //U32 serverId = 0;

   //int jobId = DbQueryAndPacket( "", "", pPacket->gameInstanceId, "", pPacket->gameId, queryString, ChatChannelDbJob::JobType_MakeInactiveFromGameServer, false );
   //jobId = jobId;

 /*  DbJobIterator iter;
   bool success = FindDbJob( jobId, m_pendingDbJobs, iter );*/
   return true;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------

void     ChatChannelManager::UserSendP2PChat( const string& senderUuid, const string& destinationUuid, const string& message )
{
   stringhash senderHash = GenerateUniqueHash( senderUuid );
   stringhash receiverHash = GenerateUniqueHash( destinationUuid );

   UserMapIterator senderIter = m_userMap.find( senderHash );
   if( senderIter == m_userMap.end() )
   {
      string text = " User ";
      text += senderUuid;
      text += " tried to send text to user ";
      text += destinationUuid;
      text += " but the sender does not exist in ChatChannelManager";
      m_chatServer->Log( text, 1 );
      return;
   }

   ChatUser* userSender = m_chatServer->GetUserByUuid( senderIter->second.userUuid );
   U32 connectionId = userSender->GetConnectionId();

   UserMapIterator receiverIter = m_userMap.find( receiverHash );
   if( receiverIter == m_userMap.end() )
   {
      string text = " User ";
      text += senderUuid;
      text += " tried to send text on user ";
      text += destinationUuid;
      text += " but the receiver does not exist in ChatChannelManager";
      m_chatServer->Log( text, 1 );
      // this is totally fine.. you can send a message to another player who is not online
      m_chatServer->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_UserNotOnline );
   }
   else
   {
      ChatUser* userReceiver = m_chatServer->GetUserByUuid( receiverIter->second.userUuid );
      if( userReceiver )
      {
         userReceiver->ChatReceived( message, senderUuid, userSender->GetUserName(), "", GetDateInUTC() );
      }
   }

   //-----------------------------------------

   string channelUuid;
   WriteChatToDb( message, senderUuid, destinationUuid, channelUuid, 0, connectionId );
}

//---------------------------------------------------------------------

void     ChatChannelManager::UserSendsChatToChannel( const string& senderUuid, const string& channelUuid, const string& message, U16 gameTurn )
{
   stringhash userHash = GenerateUniqueHash( senderUuid );
   stringhash channelHash = GenerateUniqueHash( channelUuid );

   UserMapIterator senderIter = m_userMap.find( userHash );
   if( senderIter == m_userMap.end() )
   {
      string text = " User ";
      text += senderUuid;
      text += " tried to send text on chat channel ";
      text += channelUuid;
      text += " but that user does not exist in ChatChannelManager";
      m_chatServer->Log( text, 1 );
      return;
   }
   ChatUser* userSender = m_chatServer->GetUserByUuid( senderIter->second.userUuid );
   if( userSender == NULL )
   {
      string text = " User ";
      text += senderUuid;
      text += " tried to send text on chat channel ";
      text += channelUuid;
      text += " but that user does not exist in DiplodocusChat";
      m_chatServer->Log( text, 1 );
      return;
   }

   U32 connectionId = userSender->GetConnectionId();

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
   if( channel.userBasics.size() < 1 )
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
   
   list< UserBasics >::iterator userIt = channel.userBasics.begin();
   while( userIt != channel.userBasics.end() )
   {
      UserBasics& ub = *userIt++;
      ChatUser* user = m_chatServer->GetUserByUuid( ub.userUuid );
      if( user )
      {
         user->ChatReceived( message, senderUuid, userSender->GetUserName(), channelUuid, GetDateInUTC() );
      }
   }

   string friendUuid;
   WriteChatToDb( message, senderUuid, friendUuid, channelUuid, gameTurn, connectionId );
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------

void     ChatChannelManager::AddUserToChannel( const string& channelUuid, const string& userUuid, const string username, bool sendNotification )
{
   stringhash  channelKeyLookup = GenerateUniqueHash( channelUuid );
   ChannelMapIterator iter = m_channelMap.find( channelKeyLookup );
   if( iter == m_channelMap.end() )
   {
      return;
   }

   ChatChannel& channelMapData = iter->second;
   list< UserBasics >   ::iterator it = channelMapData.userBasics.begin();
   while( it != channelMapData.userBasics.end() )
   {
      if( it->userUuid == userUuid )
         return;
      it++;
   }
   channelMapData.userBasics.push_back( UserBasics( username, userUuid ) );

   //---------------------------------------------
   stringhash  userLookup = GenerateUniqueHash( userUuid );

   UserMapIterator userIter = m_userMap.find( userLookup );
   if( userIter == m_userMap.end() )
   {
      pair< UserMapIterator,bool > newNode = m_userMap.insert( UserPair( userLookup, UsersChatChannelList( userUuid ) ) );
      userIter = newNode.first;
   }

   UsersChatChannelList& userInstance = userIter->second;
   userInstance.channels.push_back( channelKeyLookup );
   userInstance.isOnline = false;

   /*if( sendNotification )
   {
      ChatUser* user = m_chatServer->GetUserByUuid( userUuid );
      if( user )
      {
         // todo, change this UUID to the person who added these people
         user->NotifyAddedToChannel( channelUuid, userUuid, wasSuccessful );
      }
   }*/
}

//---------------------------------------------------------------------

void     ChatChannelManager::AddAllUsersToChannel( const string& channelUuid, const SerializedKeyValueVector< string >& usersAndIds, bool sendNotification )
{
   stringhash  channelKeyLookup = GenerateUniqueHash( channelUuid );
   ChannelMapIterator iter = m_channelMap.find( channelKeyLookup );
   if( iter == m_channelMap.end() )
   {
      assert( 0 );// wtf
   }

   ChatChannel& channelMapData = iter->second;

   SerializedKeyValueVector< string >::const_KVIterator userIt = usersAndIds.begin();
   while( userIt != usersAndIds.end() )
   {
      // first add the user to the channel info
      const string& userUuid = userIt->key;
      stringhash userLookup = GenerateUniqueHash( userUuid );
      channelMapData.userBasics.push_back( UserBasics( userIt->value, userUuid ) );
      userIt ++;
      
      // Now add the channel lookup to the user
      UserMapIterator userIter = m_userMap.find( userLookup );
      if( userIter == m_userMap.end() )
      {
         pair< UserMapIterator,bool > newNode = m_userMap.insert( UserPair( userLookup, UsersChatChannelList( userUuid ) ) );
         userIter = newNode.first;
      }

      UsersChatChannelList& userInstance = userIter->second;
      userInstance.channels.push_back( channelKeyLookup );
      userInstance.isOnline = false;
      //userInstance.userUuid = userUuid;// happens in c'tor

      if( sendNotification )
      {
         NotifyUserThatHeWasAddedToChannel( userUuid, channelUuid );
      }
      
   }
}

//---------------------------------------------------------------------

bool     ChatChannelManager::NotifyUserThatHeWasAddedToChannel( const string& userUuid, const string& channelUuid )
{
   ChatUser* user = m_chatServer->GetUserByUuid( userUuid );
   if( user )
   {
   // todo, change this UUID to the person who added these people
      return user->NotifyYouWereAddedToChannel( channelUuid );
   }
   return false;
}

//---------------------------------------------------------------------

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

U32     ChatChannelManager::QueryAllUsersInAllChatChannels()
{
   string 
   queryString =  "SELECT user.user_name, user.uuid, user.user_id, joiner.channel_uuid  FROM users AS user ";
   queryString += "join user_join_chat_channel as joiner on joiner.user_uuid=user.uuid ";
   queryString += "join chat_channel as channel on joiner.channel_uuid=channel.uuid ";


   PacketDbQuery* dbQuery = DbQueryFactory( queryString, false );
   SaveQueryDetails( dbQuery, "", "", 0, ChatChannelDbJob::JobType_AllUsersInAllChannels );
   Send( dbQuery );

   return dbQuery->id;
   //return CreateAndSendUserListJob( queryString, channelIter->second.name, channelUuid, 0, authUuid, userHashLookup, ChatChannelDbJob::JobType_AllUsersInChannel, &listOfStrings );
}

//---------------------------------------------------------------------

bool     ChatChannelManager::HandleAllUsersInAllChannelsResult( PacketDbQueryResult* dbResult, ChatChannelDbJob& job )
{
   if( dbResult->successfulQuery == true && dbResult->bucket.bucket.size() )
   {
      SerializedKeyValueVector< string > usersAndIds;

      UserJoinChatChannelTable            enigma( dbResult->bucket );
      UserJoinChatChannelTable::iterator  it = enigma.begin();
      while( it != enigma.end() )
      {
         UserTable::row    row = *it++;
         string   name = row[ TableUserJoinChatChannel::Column_name ];
         string   uuid = row[ TableUserJoinChatChannel::Column_uuid ];
         string   id =  row[ TableUserJoinChatChannel::Column_id ];
         string   channelUuid =  row[ TableUserJoinChatChannel::Column_channel_uuid ];

         AddUserToChannel( channelUuid, name, uuid, false );
      }
   }
   m_numChannelsToLoad--;// bad channels setup should not stop us
   m_isPullingAllUsersAndChannels = false;

   return true;
}

//---------------------------------------------------------------------

bool     ChatChannelManager::HandleLoadAllChannelsResult( PacketDbQueryResult* dbResult, ChatChannelDbJob& job )
{
   if( dbResult->successfulQuery == true )
   {
      ChatChannelTable              enigma( dbResult->bucket );
      ChatChannelTable::iterator    it = enigma.begin();
      m_numChannelsToLoad = enigma.m_bucket.size();
      while( it != enigma.end() )
      {
         SaveChatChannelLoadResult( *it++ );
      }
      QueryAllUsersInAllChatChannels();
   }

   return true;
}

//---------------------------------------------------------------------

void     ChatChannelManager::SaveChatChannelLoadResult( ChatChannelTable::row row )
{
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

//---------------------------------------------------------------------

void     ChatChannelManager::WriteChatToDb( const string& message, const string& senderUuid, const string& friendUuid, const string& channelUuid, U16 gameTurn, U32 connectionId )
{
   /*PacketDbQuery* dbQuery = new PacketDbQuery;
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
   }*/

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

   PacketDbQuery* dbQuery = DbQueryFactory( queryString, true );

   list< string > sanitizedStrings;
   sanitizedStrings.push_back( message );
   sanitizedStrings.push_back( senderUuid );
   if( friendUuid.size() > 0 )
   {
      sanitizedStrings.push_back( friendUuid );
   }
   sanitizedStrings.push_back( channelUuid );
   AddSanitizedStrings( dbQuery, sanitizedStrings );

   Send( dbQuery );
}

//---------------------------------------------------------------------

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////