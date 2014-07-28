// ChatRoomManager.cpp

#include <assert.h>

#include <iomanip>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
using boost::format;

#include "../NetworkCommon/ServerConstants.h"

#if PLATFORM == PLATFORM_WINDOWS
#pragma warning( disable:4996 )
#endif

#include "ChatRoomManager.h"
#include "../NetworkCommon/Packets/BasePacket.h"
#include "../NetworkCommon/Packets/ChatPacket.h"
#include "../NetworkCommon/Packets/DbPacket.h"
#include "../NetworkCommon/Packets/PacketFactory.h"
#include "../NetworkCommon/Packets/ServerToServerPacket.h"
#include "DiplodocusChat.h"
#include "ChatUser.h"


DiplodocusChat*        ChatRoomManager::m_chatServer;
const int maxNumPlayersInChatRoom = 32;

// ChatRoomManagerNeedsUpdate

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

ChatRoomManager::ChatRoomManager(): m_dbIdTracker( 0 ),
                                          m_isInitialized( false ),
                                          m_numChannelsToLoad( 0 ),
                                          m_numUsersToLoadPerQueryForInitialLoad( 1000 ),
                                          m_offsetIndex_QueryForInitialLoad( 0 ),
                                          m_isPullingAllUsersAndChannels( false ),
                                          m_numChannelChatsSent( 0 ),
                                          m_numP2PChatsSent( 0 ),
                                          m_numChangesToChatRoom( 0 ),
                                          m_dbIdentifier( 0 )
{
   //m_inboundPackets.reserve( 120 );// absolutely arbitrary.. it seems big enough and log(n) means that this will never be reallocated much
}

//---------------------------------------------------------------

ChatRoomManager::~ChatRoomManager()
{
   m_inboundPackets.clear();
}

//---------------------------------------------------------------

void     ChatRoomManager::Init()
{
   string queryString = "SELECT * FROM chat_channel WHERE is_active=1";

   cout << "Chat channel manager initializing" << endl;
   PacketDbQuery* dbQuery = DbQueryFactory( queryString, false );
   SaveQueryDetails( dbQuery, "", "", 0, ChatRoomDbJob::JobType_LoadAllChannels );
   Send( dbQuery );

  /* queryString = "SELECT user_name, uuid, users.user_id, user_profile.block_contact_invitations, user_profile.block_group_invitations ";
   queryString += "FROM users INNER JOIN user_profile ON users.user_id=user_profile.user_id ";
   queryString += "WHERE user_email IS NOT NULL";
   PacketDbQuery* dbQuery2 = DbQueryFactory( queryString, false );
   SaveQueryDetails( dbQuery2, "", "", 0, ChatRoomDbJob::JobType_LoadAllUsers );
   Send( dbQuery2 );*/

   QueryAllChatUsers( m_offsetIndex_QueryForInitialLoad, m_numUsersToLoadPerQueryForInitialLoad );

   m_isInitialized = true;
   m_isPullingAllUsersAndChannels = true;

   time( &m_initializationTimeStamp );
}
/*
Example:
PacketDbQuery* query = DbQueryFactory( const string& queryString, bool isFandF );
SaveQueryDetails( dbQuery, const string& channelUuid, const string& authUuid, const string& chatUserLookup, ChatRoomDbJob::JobType jobType, U32 serverId = 0, const string& debugString = "debug_string" );
AddSanitizedStrings( dbQuery, list< string >* sanitizedStrings );
AddCustomData( dbQuery, void* data );
Send( dbQuery );*/

void     ChatRoomManager::ClearStats()
{
   m_numChannelChatsSent = 0;
   m_numP2PChatsSent = 0;
   m_numChangesToChatRoom = 0;
}

//---------------------------------------------------------------

bool     ChatRoomManager::Update()
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
         ChatRoomDbJob& job = *iter;
         ProcessDbResult( dbResult, job );
         m_jobsPending.erase( iter );

         BasePacket* packet = static_cast< BasePacket* >( dbResult );
         factory.CleanupPacket( packet );
      }
      else
      {
         //ssert( 0 );
         cout << "********************************" << endl;
         cout << " DATABASE FAILURE: could not find job during update" << endl;
         cout << "********************************" << endl;
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

bool     ChatRoomManager::UserHasLoggedIn( const string& userUuid )
{
   stringhash userHash = GenerateUniqueHash( userUuid );
   UserMapIterator userIter = m_userMap.find( userHash );
   if( userIter == m_userMap.end() )// new user account creation.
   {
      pair< UserMapIterator, bool > ins = m_userMap.insert( UserPair( userHash, UsersChatRoomList( userUuid ) ) );
      userIter = ins.first;
   }

   userIter->second.isOnline = true;

   return true;
}

//---------------------------------------------------------------

bool     ChatRoomManager::UserHasLoggedOut( const string& userUuid )
{
   stringhash userHash = GenerateUniqueHash( userUuid );
   UserMapIterator userIter = m_userMap.find( userHash );
   if( userIter == m_userMap.end() )
   {
      return false;
   }
   userIter->second.isOnline = false;

   return true;
}

//---------------------------------------------------------------

bool     ChatRoomManager::AddInboundPacket( BasePacket* packet ) // not thread safe.. obviously
{
   m_inboundPackets.push_back( packet );
   return true;
}

//---------------------------------------------------------------

bool     ChatRoomManager::HandleDbResult( PacketDbQueryResult* packet ) // not thread safe.. obviously
{
   m_dbResults.push_back( packet );
   return true;
}

//---------------------------------------------------------------

void     ChatRoomManager::ProcessIncomingPacket( BasePacket* packet )// should mostly be database 
{
   PacketFactory factory;
   factory.CleanupPacket( packet );
}

//---------------------------------------------------------------

bool     ChatRoomManager::ProcessDbResult( PacketDbQueryResult* dbResult, ChatRoomDbJob& job )
{
   switch( job.jobType )
   {
 /*  case ChatRoomDbJob::JobType_Create:
      {
        HandleChatChannelCreateResult( dbResult, job );
      }
      break;*/
   case ChatRoomDbJob::JobType_LoadSingleChannel:
      {
         HandleSingleRoomLoad( dbResult, job );
      }
      break;
   case ChatRoomDbJob::JobType_LoadAllChannels:
      {
         HandleLoadAllRoomsResult( dbResult, job );       
      }
      break;
   case ChatRoomDbJob::JobType_LoadAllUsers:
      {
         HandleLoadAllUsersResult( dbResult, job );
      }
      break;
   case  ChatRoomDbJob::JobType_AllUsersInChannel:
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

               //StoreUserInChannel( channelUuid, UserBasics& ub )

               usersAndIds.insert( uuid, name );
            }

            StoreAllUsersInChannel( job.lookupString, usersAndIds );
         }
         m_numChannelsToLoad--;// bad channels setup should not stop us

      }
      break;
   case  ChatRoomDbJob::JobType_AllUsersInAllChannels:
      {         
         HandleAllUsersInAllChannelsResult( dbResult, job );

      }
      break;
      
   }
   return true;
}

//---------------------------------------------------------------------

int     ChatRoomManager::AddDbJob( const string& debugString, const string& lookupString, U32 serverId, stringhash chatUserLookup, stringhash authUserLookup, ChatRoomDbJob::JobType type )
{
   ChatRoomDbJob job;
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

PacketDbQuery* ChatRoomManager::DbQueryFactory( const string& queryString, bool isFandF )
{
   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id = m_dbIdentifier;
   dbQuery->isFireAndForget = isFandF;
   dbQuery->query = queryString;
   return dbQuery;
}

//---------------------------------------------------------------------

bool  ChatRoomManager::SaveQueryDetails( PacketDbQuery* dbQuery, const string& channelUuid, const string& authUuid, stringhash chatUserLookup, ChatRoomDbJob::JobType jobType, U32 serverId, const string& debugString)
{
   assert( dbQuery != NULL );
   dbQuery->lookup = AddDbJob( debugString, channelUuid, serverId, chatUserLookup, GenerateUniqueHash( authUuid ), jobType );
   return true;
}

//---------------------------------------------------------------------

bool  ChatRoomManager::AddSanitizedStrings( PacketDbQuery* dbQuery, list< string >& sanitizedStrings )
{
   list< string >::iterator it = sanitizedStrings.begin();
   while( it != sanitizedStrings.end() )
   {
      dbQuery->escapedStrings.insert( *it++ );
   }
   return true;
}

/*
bool  ChatRoomManager::AddSanitizedStrings( PacketDbQuery* dbQuery, int numParams, ... )
{
   StringBucket::DataSet& bucket = dbQuery->escapedStrings.bucket; // optimization
   va_list vl;
   va_start( vl, numParams );

   for (int i=1;i< numParams;i++)
   {
      string& val = va_arg( vl, string );
      bucket.push_back( val );
   }
   va_end(vl);

   return true;
}*/

//---------------------------------------------------------------------

bool  ChatRoomManager::AddCustomData( PacketDbQuery* dbQuery, void* data )
{
   assert( dbQuery->customData == NULL );
   dbQuery->customData = data;
   return true;
}

//---------------------------------------------------------------------

bool  ChatRoomManager::Send( PacketDbQuery* dbQuery )
{
   dbQuery->serverLookup = m_dbIdentifier;
   if( m_chatServer->AddQueryToOutput( dbQuery, 0 ) == false )
   {
      cout << "ChatRoomManager:: Query packet rejected" << endl;
      
      delete dbQuery;
      return false;
   }
   return true;
}


//---------------------------------------------------------------------

void     ChatRoomManager::PackageAndSendToOtherServer( BasePacket* packet, U32 serverId )
{
   PacketServerJobWrapper* wrapper = new PacketServerJobWrapper;
   wrapper->pPacket = packet;
   wrapper->serverId = serverId;

   m_chatServer->AddOutputChainData( wrapper, serverId );
}


//---------------------------------------------------------------

bool     ChatRoomManager::GetGroupName( const string& groupUuid, string& name ) const
{
   name.clear();
   stringhash roomLookup = GenerateUniqueHash( groupUuid );
   ChannelMapConstIterator channelIter = m_channelMap.find( roomLookup );   
   if( channelIter != m_channelMap.end() )
   {
      name = channelIter->second.name;
      return true;
   }
   return false;
}
//---------------------------------------------------------------

bool     ChatRoomManager::GetChatRooms( const string& userUuid, ChannelFullKeyValue& availableChannels )
{
   availableChannels.clear();

   stringhash userLookup = GenerateUniqueHash( userUuid );
   UserMapIterator userIter = m_userMap.find( userLookup );
   if( userIter != m_userMap.end() )
   {
      const list< stringhash >& listOfChannels = userIter->second.channels;// cache this pointer (optimization)
      int numChannels = listOfChannels.size();
      int numToReserve = numChannels;
      if( numToReserve == 0 )
         numToReserve = 1;
      availableChannels.reserve( numToReserve );

      list< stringhash >::const_iterator it = listOfChannels.begin();
      while( it != listOfChannels.end() )
      {
         stringhash channelLookupHash = *it++;
         ChannelMapIterator channelIter = m_channelMap.find( channelLookupHash );   
         if( channelIter != m_channelMap.end() )
         {
            const ChatRoom& channel = channelIter->second;
            //channel.userBasics
            // ChannelInfo( channel.name, channel.uuid, channel.gameType, channel.gameInstanceId, 0, channel.isActive )
            ChannelInfoFullList temp( channel.name, channel.uuid, channel.gameType, channel.gameInstanceId, 0, channel.isActive );
            list< UserBasics >::const_iterator ub = channel.userBasics.begin();
            while( ub != channel.userBasics.end() )
            {
               temp.userList.insert( ub->userUuid, ub->userName );
               ub++;
            }
            availableChannels.insert( channel.uuid, temp );
         }
      }
      return true;
   }
   return false;
}

//---------------------------------------------------------------------

U32      ChatRoomManager::GetUserId( const string& userUuid ) const
{
   UsersChatRoomList user ( userUuid );
   GetUserInfo( userUuid, user );
   return user.userId;
}

//---------------------------------------------------------------------

string   ChatRoomManager::GetUserName( const string& userUuid ) const
{
   UsersChatRoomList user ( userUuid );
   GetUserInfo( userUuid, user );
   return user.userName;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------

int      ChatRoomManager::NewDbId()
{
   if( ++m_dbIdTracker < 0 ) // wrap around
      m_dbIdTracker = 1;
   return m_dbIdTracker;
}

ChatRoomManager::ChannelMapIterator   
ChatRoomManager::FindChatRoom( U32 gameInstanceId, U8 gameType )
{
   ChannelMapIterator    iter = m_channelMap.begin();
   while( iter != m_channelMap.end() )
   {
      //if( iter->second.gameType == gameType )
      {
         if( iter->second.gameInstanceId == gameInstanceId )
            return iter;
      }
      iter ++;
   }
   return iter;
}


//---------------------------------------------------------------------

bool     ChatRoomManager::FindDbJob( int jobId, list< ChatRoomDbJob >& listOfJobs, DbJobIterator& iter )
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

void     ChatRoomManager::AddChatRoom( int id, const string& channelName, const string& channelUuid, bool isActive, int maxPlayers, int gameType, U32 gameInstanceId, const string& createDate )
{
   stringhash  keyLookup = GenerateUniqueHash( channelUuid );

   ChannelMapIterator iter = m_channelMap.find( keyLookup );
   
   if( iter != m_channelMap.end() )
   {
      ChatRoom& channel = iter->second;
      channel.recordId = id;
      channel.name = channelName;
      channel.uuid = channelUuid;
      channel.maxPlayers = maxPlayers;
      channel.gameType = gameType;
      channel.gameInstanceId = gameInstanceId;
      channel.createDate = createDate;

      QueryAllUsersInChatRoom( channelUuid );
   }
   else
   {    
      // the way I am doing this is 3x faster than the alternative which is to fill out the struct first and then insert the pair. (2 copies)
      ChannelMapPair channelMapData;
      channelMapData.first = keyLookup;
      pair< ChannelMapIterator, bool> newChannelIter  =  m_channelMap.insert( channelMapData );

      ChatRoom& newChannel = newChannelIter.first->second; // this is confusing, but the first element is an iterator and its second param is our new object
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

void     ChatRoomManager::StoreUser( const string& userUuid, const string& userName )
{
   stringhash  userKeyLookup = GenerateUniqueHash( userUuid );
   UserMapIterator userIter = m_userMap.find( userKeyLookup );
   if( userIter == m_userMap.end() )
   {
      assert( 0 );// this needs to be rewritten.. depricated iow
      // we need to load this user from the db

      pair< UserMapIterator,bool > newNode = m_userMap.insert( UserPair( userKeyLookup, UsersChatRoomList( userUuid ) ) );
      userIter = newNode.first;
      UsersChatRoomList& userInstance = userIter->second;
      userInstance.userName = userName;
      userInstance.isOnline = false;
   }
}

void     ChatRoomManager::StoreUser( const string& userUuid, U32 userId, const string& userName, bool blockContactInvites, bool blockGroupInvites )
{
   stringhash  userKeyLookup = GenerateUniqueHash( userUuid );
   UserMapIterator userIter = m_userMap.find( userKeyLookup );
   if( userIter == m_userMap.end() )
   {
      pair< UserMapIterator,bool > newNode = m_userMap.insert( UserPair( userKeyLookup, UsersChatRoomList( userUuid ) ) );
      userIter = newNode.first;
      UsersChatRoomList& userInstance = userIter->second;
      userInstance.userId = userId;
      userInstance.userName = userName;
      userInstance.isOnline = false;
      userInstance.blockContactInvites = blockContactInvites;
      userInstance.blockGroupInvites = blockGroupInvites;
   }
}

// we should return a bool instead
bool        ChatRoomManager::GetUserInfo( const string& userUuid, UsersChatRoomList& chatRoom ) const
{
   stringhash  userKeyLookup = GenerateUniqueHash( userUuid );
   UserMapConstIterator userIter = m_userMap.find( userKeyLookup );
   if( userIter != m_userMap.end() )
   {
      chatRoom = userIter->second;
      return true;
   }
   //chatRoom.cl
   return false;
}

bool     ChatRoomManager::IsGroupValid( const string& channelUuid ) const
{
   stringhash  channelHash = GenerateUniqueHash( channelUuid );
   if( m_channelMap.find( channelHash ) == m_channelMap.end() )
      return false;

   return true;
}
   

void     ChatRoomManager::AddRoomToUser( const string& userUuid, stringhash channelHash )
{
   stringhash  userKeyLookup = GenerateUniqueHash( userUuid );
   UserMapIterator userIter = m_userMap.find( userKeyLookup );
   if( userIter != m_userMap.end() )
   {
      UsersChatRoomList& userInstance = userIter->second;
      list< stringhash >&   channels = userInstance.channels;
      list< stringhash >::iterator it = channels.begin();
      while( it != channels.end() )// prevent dups
      {
         if( channelHash == *it++ )
            return;
      }
      userInstance.channels.push_back( channelHash );
   }
}

void     ChatRoomManager::DeleteRoomFromUser( const string& userUuid, stringhash channelHash )
{
   stringhash  userKeyLookup = GenerateUniqueHash( userUuid );
   UserMapIterator userIter = m_userMap.find( userKeyLookup );
   if( userIter != m_userMap.end() )
   {
      UsersChatRoomList& userInstance = userIter->second;
      list< stringhash >&   channels = userInstance.channels;
      list< stringhash >::iterator it = channels.begin();
      while( it != channels.end() )// prevent dups
      {
         if( channelHash == *it )
         {
            channels.erase( it );
            return;
         }
         it++;
      }
   }
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
/*
bool    ChatRoomManager::QueryDeleteRoom( const string& channelUuid, const U32 serverId )
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
      text += " but that channel does not exist in ChatRoomManager";
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
   DbQueryAndPacket( channelUuid, channelUuid, serverId, authUuid, authHash, queryString, ChatRoomDbJob::JobType_Delete, false, &listOfStrings );

   return true;
}
 
//---------------------------------------------------------------------
 
bool     ChatRoomManager::RemoveChatChannel( const string& channelUuid )
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

bool   ChatRoomManager::CreateNewRoom( const PacketChatCreateChatChannelFromGameServer* request )
{
   time_t t = time(0);   // get time now
   struct tm * now = gmtime( & t );
   
   int year = now->tm_year - 100; // 113 => 2013.
   int month = now->tm_mon + 1; // 0-based
   int day = now->tm_mday;

   string date = GetDateInUTC();

   U32 gameInstanceId = request->gameId;
   if( gameInstanceId == 0 )
   {
      cout << ":::::::::::::::: Error ::::::::::::::::::::" << endl;
      cout << "ChatRoomManager::CreateNewRoom " << endl;
      cout << "Server id is 0" << endl;
      cout << ":::::::::::::::::::::::::::::::::::::::::::" << endl;
   }
   //request->ser
   U32 gameServerUniqueInstanceId = request->gameInstanceId;
   if( gameServerUniqueInstanceId == 0 )
   {
      cout << ":::::::::::::::: Error ::::::::::::::::::::" << endl;
      cout << "ChatRoomManager::CreateNewRoom " << endl;
      cout << "Game instance is 0" << endl;
      cout << ":::::::::::::::::::::::::::::::::::::::::::" << endl;
   }
   U32 gameProductId = request->gameProductId;
   if( gameProductId == 0 )
   {
      cout << ":::::::::::::::: Error ::::::::::::::::::::" << endl;
      cout << "ChatRoomManager::CreateNewRoom " << endl;
      cout << "Game product is 0" << endl;
      cout << ":::::::::::::::::::::::::::::::::::::::::::" << endl;
   }
   std::stringstream ss;
   const int maxNumberGameInstanceDigits = 7; // into the tens of millions of games
   ss  << std::setfill('0') << request->gameName << '_' 
            << std::setw(2) << year << ':' 
            << std::setw(2) << month << ':' 
            << std::setw(2) << day << '_' 
            << std::setw( maxNumberGameInstanceDigits ) << request->gameId;
   string channelName = ss.str();

// CreateNewRoom( const string& channelName, const string userUuid, U32 serverId, U8 gameType, U32 gameInstanceId )
   // the gameId and the GameInstandId need to be verified here.. the order matters, and I can't figure out which one is which.
   string channelUuid = CreateNewRoom( channelName, "", gameServerUniqueInstanceId, gameProductId, gameInstanceId );

   if( channelUuid.size() )
   {
      PacketChatCreateChatChannelFromGameServerResponse* response = new PacketChatCreateChatChannelFromGameServerResponse;
      response->gameId = request->gameId;
      response->channelUuid = channelUuid;

      PackageAndSendToOtherServer( response, request->gameInstanceId );

      const StringBucket::DataSet& uuidList = request->userUuidList.bucket;
      list< string >::const_iterator it = uuidList.begin();
      while( it != uuidList.end() )
      {
         const string& userUuid = *it++;
         UsersChatRoomList user( userUuid );
         if( GetUserInfo( userUuid, user ) == true )
         {
            string userName = user.userUuid;
            AddUserToRoomAndWriteToDB( channelUuid, userUuid, userName );
            NotifyUserThatHeWasAddedToChannel( userUuid, channelName, channelUuid );
         }
      }

   }
   return true;
}

//---------------------------------------------------------------------

bool ChatRoomManager::CreateNewRoom( const string& channelName, const string& userUuid )
{
   string channelUuid = CreateNewRoom( channelName, userUuid, 0, 0, 0 );
   UsersChatRoomList userInChatRoom( userUuid );
   if( GetUserInfo( userUuid, userInChatRoom ) == false )
      return false;
   string userName = userInChatRoom.userName;

   ChatUser* user = m_chatServer->GetUserByUuid( userUuid );// this is a little odd logic. However, the creator should be online 
   if( user )
   {
      if( channelUuid.size() )
      {
         AddUserToRoomAndWriteToDB( channelUuid, userUuid, userName );
         NotifyUserThatHeWasAddedToChannel( userUuid, channelName, channelUuid );
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

string  ChatRoomManager::CreateUniqueChatRoomId()
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

string   ChatRoomManager::CreateNewRoom( const string& channelName, const string userUuid, U32 serverId, U8 gameType, U32 gameInstanceId )
{
   stringhash  userKeyLookup = GenerateUniqueHash( userUuid );

   UserMapIterator iter = m_userMap.find( userKeyLookup );
   if( iter == m_userMap.end() )
   {
      userKeyLookup = 0;
   }

   string      channelUuid = CreateUniqueChatRoomId();
   bool isActive = true;
   string createDate = GetDateInUTC();
   AddChatRoom( 0, channelName, channelUuid, isActive, maxNumPlayersInChatRoom, gameType, gameInstanceId, createDate );

   string queryString = "INSERT INTO chat_channel ( name, uuid, is_active, max_num_users, game_type, game_instance_id, date_created ) VALUES( '%s','";
   queryString += channelUuid;
   queryString += "', 1, "; // active
   queryString += boost::lexical_cast< string >( maxNumPlayersInChatRoom );
   queryString += ", ";
   queryString += boost::lexical_cast< string >( (U32)( gameType ) );
   queryString += ", ";
   queryString += boost::lexical_cast< string >( gameInstanceId );
   queryString += ", '";
   queryString += createDate;
   queryString += "' )"; 

   list< string > sanitizedStrings;
   sanitizedStrings.push_back( channelName );

   PacketDbQuery* dbQuery = DbQueryFactory( queryString, true );
   AddSanitizedStrings( dbQuery, sanitizedStrings );
   Send( dbQuery );

   LoadSingleRoom( channelUuid ); // just grab the db defaults( id, timestamp, etc );
   return channelUuid;
}

//---------------------------------------------------------------------

bool     ChatRoomManager::LoadSingleRoom( const string& channelUuid )
{
   cout << "LoadSingleRoom " << channelUuid << endl;

   string queryString = "SELECT * FROM chat_channel WHERE uuid='%s'";
   U32 serverId = 0;
   
   list< string > sanitizedStrings;
   sanitizedStrings.push_back( channelUuid );

   PacketDbQuery* dbQuery = DbQueryFactory( queryString, false );
   SaveQueryDetails( dbQuery, "", "", 0, ChatRoomDbJob::JobType_LoadSingleChannel );
   AddSanitizedStrings( dbQuery, sanitizedStrings );
   Send( dbQuery );

   return true;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------

bool   ChatRoomManager::DeleteRoom( const PacketChatDeleteChatChannelFromGameServer* request )
{
   PacketChatDeleteChatChannelFromGameServerResponse* response = new PacketChatDeleteChatChannelFromGameServerResponse;
   response->gameId = request->gameId;
   response->gameProductId = request->gameProductId;
   response->gameInstanceId = request->gameInstanceId;
   
   bool success = false;
   U8 gameType = request->gameProductId;
   U32 gameInstanceId = request->gameId;// 
   ChannelMapIterator iter = FindChatRoom( gameInstanceId, gameType );
   if( iter == m_channelMap.end() )
   {
      response->numUsersRemoved = 0;
   }
   else
   {
      success = DeleteRoom( iter->second.uuid );
      response->numUsersRemoved = static_cast< int >( iter->second.userBasics.size() );
   }

   response->successfullyDeleted = success;
   PackageAndSendToOtherServer( response, response->gameInstanceId );// TODO verify that this is the correct server id

   return success;
}


//---------------------------------------------------------------

bool     ChatRoomManager::SendMessageToClient( BasePacket* packet, U32 connectionId ) const
{
   PacketGatewayWrapper* wrapper = new PacketGatewayWrapper();
   wrapper->SetupPacket( packet, connectionId );

   m_chatServer->SendMessageToClient( wrapper, connectionId );
   return true;
}

//---------------------------------------------------------------------

bool     ChatRoomManager::DeleteRoom( const string& channelUuid, const string& userUuid )
{
   ChatUser* userSender = m_chatServer->GetUserByUuid( userUuid );
   if( userSender == NULL )
   {
      string text = " User ";
      text += userUuid;
      text += " tried to send delete channel ";
      text += channelUuid;
      text += " but the user does not exist in ChatRoomManager";
      m_chatServer->Log( text, 1 );
      return false;
   }
   U32 connectionId = userSender->GetConnectionId();

   stringhash  keyLookup = GenerateUniqueHash( channelUuid );
   ChannelMapIterator channelIter = m_channelMap.find( keyLookup );
   if( channelIter == m_channelMap.end() )
   {
      string text = " User ";
      text += userUuid;
      text += " tried to send delete channel ";
      text += channelUuid;
      text += " but the channel does not exist in ChatRoomManager";
      m_chatServer->Log( text, 1 );
      m_chatServer->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_BadChatChannel );
      return false;
   }

   int numUsers = static_cast< int >( channelIter->second.userBasics.size() );

   bool success = DeleteRoom( channelUuid );   
   
   if( success )
   {
      userSender->NotifyChannelRemoved( channelUuid, numUsers );
   }
   else 
   {
      m_chatServer->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_ChatChannelCannotBeDeleted );
   }

   return true;
}

//---------------------------------------------------------------------

bool     ChatRoomManager::DeleteRoom( const string& channelUuid )
{
   stringhash  keyLookup = GenerateUniqueHash( channelUuid );
   ChannelMapIterator channelIter = m_channelMap.find( keyLookup );
   if( channelIter == m_channelMap.end() )
   {
      return false;
   }


   // do we remove everyone?
   list< UserBasics >& listOfUsers = channelIter->second.userBasics;
   if( listOfUsers.size() < 1 )
   {
      int numUsers = static_cast< int >( listOfUsers.size() );
   
      list< UserBasics >::iterator userIt = listOfUsers.begin();
      while( userIt != listOfUsers.end() )
      {
         UserBasics& ub = *userIt++;
         ChatUser* user = m_chatServer->GetUserByUuid( ub.userUuid );
         if( user )
         {
            user->NotifyChannelRemoved( channelUuid, numUsers );
         }
      }
   }

   U32 gameType = channelIter->second.gameType;
   U32 gameInstanceId = channelIter->second.gameInstanceId;

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

   m_numChangesToChatRoom ++;

   return true;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------

bool     ChatRoomManager::UserSendP2PChat( const string& senderUuid, const string& destinationUuid, const string& message )
{
   cout << "Sending p2p chat from " << senderUuid << " to " << destinationUuid << ": msg:" << message << endl;
   stringhash senderHash = GenerateUniqueHash( senderUuid );
   stringhash receiverHash = GenerateUniqueHash( destinationUuid );

   UserMapIterator senderIter = m_userMap.find( senderHash );
   if( senderIter == m_userMap.end() )
   {
      string text = " User ";
      text += senderUuid;
      text += " tried to send text to user ";
      text += destinationUuid;
      text += " but the sender does not exist in ChatRoomManager";
      m_chatServer->Log( text, 1 );
      return false;
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
      text += " but the receiver does not exist in ChatRoomManager";
      m_chatServer->Log( text, 1 );
      // this is totally fine.. you can send a message to another player who is not online
      m_chatServer->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_UserNotOnline );
   }
   else
   {
      ChatUser* userReceiver = m_chatServer->GetUserByUuid( receiverIter->second.userUuid );
      if( userReceiver )
      {
         userReceiver->ChatReceived( message, senderUuid, userSender->GetUserName(), "", GetDateInUTC(), 0 );
      }
   }

   //-----------------------------------------

   string channelUuid;
   WriteChatToDb( message, senderUuid, destinationUuid, channelUuid, 0, connectionId );

   return true;
}

//---------------------------------------------------------------------

bool     ChatRoomManager::UserSendsChatToChannel( const string& senderUuid, const string& channelUuid, const string& message, U16 gameTurn )
{
   cout << "Sending p2p chat from " << senderUuid << " to channel " << channelUuid << ": msg:" << message << endl;
   stringhash userHash = GenerateUniqueHash( senderUuid );
   stringhash channelHash = GenerateUniqueHash( channelUuid );

   UserMapIterator senderIter = m_userMap.find( userHash );
   if( senderIter == m_userMap.end() )
   {
      string text = " User ";
      text += senderUuid;
      text += " tried to send text on chat channel ";
      text += channelUuid;
      text += " but that user does not exist in ChatRoomManager";
      m_chatServer->Log( text, 1 );
      return false;
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
      return false;
   }

   U32 connectionId = userSender->GetConnectionId();

   if( channelUuid.size() == 0 )
   {
      m_chatServer->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_NoChatChannel );
      return false;
   }

   //-----------------------------------------

   ChannelMapIterator channelIter = m_channelMap.find( channelHash );
   if( channelIter == m_channelMap.end() )
   {
      string text = " User ";
      text += senderUuid;
      text += " tried to send text on chat channel ";
      text += channelUuid;
      text += " but that channel does not exist in ChatRoomManager";
      m_chatServer->Log( text, 1 );
      return false;
   }

   ChatRoom& channel = channelIter->second;
   if( channel.userBasics.size() < 1 )
   {
      m_chatServer->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_YouAreTheOnlyPersonInThatChatChannel  );
      string text = " User ";
      text += senderUuid;
      text += " tried to send text on chat channel ";
      text += channelUuid;
      text += " but there is no one in that group";
      m_chatServer->Log( text, 1 );
      return false;
   }
   
   list< UserBasics >::iterator userIt = channel.userBasics.begin();
   while( userIt != channel.userBasics.end() )
   {
      UserBasics& ub = *userIt++;
      ChatUser* user = m_chatServer->GetUserByUuid( ub.userUuid );
      if( user )
      {
         user->ChatReceived( message, senderUuid, userSender->GetUserName(), channelUuid, GetDateInUTC(), userSender->GetUserId() );
      }
   }

   string friendUuid;
   WriteChatToDb( message, senderUuid, friendUuid, channelUuid, gameTurn, connectionId );
   return true;
}

bool     ChatRoomManager::RequestChatRoomInfo( const PacketChatListAllMembersInChatChannel* packet, U32 connectionId )
{
   const string& channelUuid = packet->chatChannelUuid.c_str();
   stringhash channelHash = GenerateUniqueHash( channelUuid );

   ChannelMapIterator channelIter = m_channelMap.find( channelHash );
   if( channelIter == m_channelMap.end() )
   {
      m_chatServer->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_BadChatChannel );
      return false;
   }

   PacketChatListAllMembersInChatChannelResponse* response = new PacketChatListAllMembersInChatChannelResponse;
   SerializedKeyValueVector< string >& userList = response->userList;
   response->chatChannelUuid = packet->chatChannelUuid;

   ChatRoom& channel = channelIter->second;
   list< UserBasics >::iterator userIt = channel.userBasics.begin();
   while( userIt != channel.userBasics.end() )
   {
      UserBasics& ub = *userIt++;
      userList.insert( ub.userUuid, ub.userName );
   }

   SendMessageToClient( response, connectionId );

   return true;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------

bool     ChatRoomManager::RenameChatRoom( const string& channelUuid, const string& newName, const string& userUuid, string& oldName )
{
   // I can totally see users renaming this channel repeatedly over each other...
   string errorText = " RenameChatRoom: User ";
   errorText += userUuid;
   errorText += " renamed channel ";
   errorText += channelUuid;

   stringhash requesterHash = GenerateUniqueHash( userUuid );
   UserMapIterator requesterIter = m_userMap.find( requesterHash );
   if( requesterIter == m_userMap.end() )
   {
      errorText += " but the user does not exist in ChatRoomManager";
      m_chatServer->Log( errorText, 1 );
      return false;
   }

   ChatUser* userRequester = m_chatServer->GetUserByUuid( requesterIter->second.userUuid );
   if( userRequester == NULL )
   {
      errorText += " but the user does not exist in ChatRoomManager";
      m_chatServer->Log( errorText, 1 );
      return false;
   }

   U32 connectionId = userRequester->GetConnectionId();

   stringhash channelHash = GenerateUniqueHash( channelUuid );   
   ChannelMapIterator channelIter = m_channelMap.find( channelHash );
   if( channelIter == m_channelMap.end() )
   {
      errorText += " but that channel does not exist in ChatRoomManager";
      m_chatServer->Log( errorText, 1 );
      m_chatServer->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_NoChatChannel );
      return false;
   }

   oldName = channelIter->second.name;

   // verify that we don't have a duplicate name

   // inform all members that a rename has taken place.

   /*

   U32 connectionId = userRequester->GetConnectionId();

   stringhash addedUserHash = GenerateUniqueHash( addedUserUuid );
   UserMapIterator addedUserIter = m_userMap.find( addedUserHash );
   if( addedUserIter == m_userMap.end() )
   {
      errorText += " but the added user does not exist in ChatRoomManager";
      m_chatServer->Log( errorText, 1 );
      m_chatServer->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_UserUnknown );
      return false;
   }



   ChatRoom& channelInfo = channelIter->second;
   list< UserBasics >   ::iterator it = channelInfo.userBasics.begin();
   while( it != channelInfo.userBasics.end() )
   {
      if( it->userUuid == addedUserUuid )
      {
         errorText += " but that added user is already in that channel";
         m_chatServer->Log( errorText, 1 );
         m_chatServer->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_CannotAddUserToRoom_AlreadyExists );
         return false;
      }
      it++;
   }

   AddUserToRoomAndWriteToDB( channelUuid, addedUserUuid, addedUserIter->second.userName );

   // inform users of success
   list< UserBasics >::iterator userIt = channelInfo.userBasics.begin();
   while( userIt != channelInfo.userBasics.end() )
   {
      UserBasics& ub = *userIt++;
      ChatUser* user = m_chatServer->GetUserByUuid( ub.userUuid );
      if( user )
      {
         user->NotifyAddedToChannel( channelInfo.name, channelUuid, addedUserIter->second.userName, addedUserUuid );
      }
   }
   */
   return true;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------

bool     ChatRoomManager::SetUserPreferences( const string& userUuid, bool blockContactInvitations, bool blockGroupInvitations )
{
   stringhash userHash = GenerateUniqueHash( userUuid );
   UserMapIterator userIter = m_userMap.find( userHash );
   if( userIter != m_userMap.end() )
   {
      userIter->second.blockContactInvites = blockContactInvitations;
      userIter->second.blockGroupInvites = blockGroupInvitations;
      return true;
   }
   return false;
}

//---------------------------------------------------------------------

bool     ChatRoomManager::AddUserToRoom( const PacketChatAddUserToChatChannelGameServer* request )
{
   U32 gameInstanceId = request->gameInstanceId;
   const string addedUserUuid = request->userUuid.c_str(); // user to remove

   string errorText = " AddUserToRoom: Game server ";
   errorText += boost::lexical_cast< string >( request->gameInstanceId );
   errorText += " tried to add another user ";
   errorText += addedUserUuid;
   errorText += " to channel ";
   errorText += request->gameName;
   

   PacketChatAddUserToChatChannelGameServerResponse* response = new PacketChatAddUserToChatChannelGameServerResponse;
   response->gameId = request->gameId;
   response->userUuid = addedUserUuid;
   response->success = false;


   ChannelMapIterator  channelIter = FindChatRoom( request->gameId, 0 );
   if( channelIter == m_channelMap.end() )
   {
      errorText += " but channel does not exist ";
      m_chatServer->Log( errorText, 1 );
      PackageAndSendToOtherServer( response, request->gameInstanceId );
      return false;
   }

   ChatRoom& channelMapData = channelIter->second;
   list< UserBasics >   ::iterator userIt = channelMapData.userBasics.begin();
   while( userIt != channelMapData.userBasics.end() )
   {
      if( userIt->userUuid == addedUserUuid )
      {
         PackageAndSendToOtherServer( response, request->gameInstanceId );// failure
         return false;
      }
      userIt++;
   }

   stringhash addedUserHash = GenerateUniqueHash( addedUserUuid );
   UserMapIterator addedUserIter = m_userMap.find( addedUserHash );
   if( addedUserIter == m_userMap.end() )
   {
      errorText += " but the added user does not exist in ChatRoomManager";
      m_chatServer->Log( errorText, 1 );
      PackageAndSendToOtherServer( response, request->gameInstanceId );// failure
      return false;
   }

   const string& channelName = channelMapData.name;
   const string& channelUuid = channelMapData.uuid;
   const string& userName = addedUserIter->second.userName;
   AddUserToRoomAndWriteToDB( channelUuid, addedUserUuid, userName );

   response->success = true;
   PackageAndSendToOtherServer( response, request->gameInstanceId );

   // tell all clients that a user was added to the chat channel   
   userIt = channelMapData.userBasics.begin();
   while( userIt != channelMapData.userBasics.end() )
   {
      ChatUser* user = m_chatServer->GetUserByUuid( userIt->userUuid );
      if( user )
      {
         user->NotifyAddedToChannel( channelName, channelUuid, addedUserUuid, userName );
      }
      userIt++;
   }

   return true;
}

//---------------------------------------------------------------------

bool     ChatRoomManager::AddUserToRoom( const string& channelUuid, const string& addedUserUuid, const string& requesterUuid )
{
   string errorText = " AddUserToRoom: User ";
   errorText += requesterUuid;
   errorText += " tried to add another user ";
   errorText += addedUserUuid;
   errorText += " to channel ";
   errorText += channelUuid;

   ///-------------------------------------------------------------
   stringhash requesterHash = GenerateUniqueHash( requesterUuid );
   UserMapIterator requesterIter = m_userMap.find( requesterHash );
   if( requesterIter == m_userMap.end() )
   {
      errorText += " but the requester does not exist in ChatRoomManager";
      m_chatServer->Log( errorText, 1 );
      return false;
   }

   U32 connectionId = 0;
   
   ChatUser* userRequester = m_chatServer->GetUserByUuid( requesterIter->second.userUuid );
   if( userRequester == NULL )
   {
      errorText += " but the requester does not exist in ChatRoomManager";
      m_chatServer->Log( errorText, 1 );
   }
   else
   {
      connectionId = userRequester->GetConnectionId();
   }
   stringhash addedUserHash = GenerateUniqueHash( addedUserUuid );
   UserMapIterator addedUserIter = m_userMap.find( addedUserHash );
   if( addedUserIter == m_userMap.end() )
   {
      errorText += " but the added user does not exist in ChatRoomManager";
      m_chatServer->Log( errorText, 1 );
      m_chatServer->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_UserUnknown );
      return false;
   }

   const UsersChatRoomList& recipient = addedUserIter->second;
   // NOTE: This code is only used in P2P inites...
   if( recipient.blockGroupInvites == true )
   {
      errorText += " but the added user is blocking group invites";
      m_chatServer->Log( errorText, 1 );
      m_chatServer->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_ChatChannel_UserNotAcceptingInvites );
      return false;
   }

   stringhash channelHash = GenerateUniqueHash( channelUuid );   
   ChannelMapIterator channelIter = m_channelMap.find( channelHash );
   if( channelIter == m_channelMap.end() )
   {
      errorText += " but that channel does not exist in ChatRoomManager";
      m_chatServer->Log( errorText, 1 );
      m_chatServer->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_NoChatChannel );
      return false;
   }

   ChatRoom& channelInfo = channelIter->second;
   list< UserBasics >   ::iterator it = channelInfo.userBasics.begin();
   while( it != channelInfo.userBasics.end() )
   {
      if( it->userUuid == addedUserUuid )
      {
         errorText += " but that added user is already in that channel";
         m_chatServer->Log( errorText, 1 );
         m_chatServer->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_CannotAddUserToChannel_AlreadyExists );
         return false;
      }
      it++;
   }

   AddUserToRoomAndWriteToDB( channelUuid, addedUserUuid, addedUserIter->second.userName );

   // inform users of success
   list< UserBasics >::iterator userIt = channelInfo.userBasics.begin();
   while( userIt != channelInfo.userBasics.end() )
   {
      UserBasics& ub = *userIt++;
      ChatUser* user = m_chatServer->GetUserByUuid( ub.userUuid );
      if( user )
      {
         user->NotifyAddedToChannel( channelInfo.name, channelUuid, addedUserIter->second.userName, addedUserUuid );
      }
   }

   return true;
}

//---------------------------------------------------------------------

bool     ChatRoomManager::UserAddsSelfToGroup( const string& channelUuid, const string& addedUserUuid )
{
   string errorText = " UserAddsSelfToGroup: User ";
   errorText += addedUserUuid;
   errorText += " to channel ";
   errorText += channelUuid;

   ///-------------------------------------------------------------

   U32 connectionId = 0; 
   ChatUser* userRequester = m_chatServer->GetUserByUuid( addedUserUuid );
   if( userRequester == NULL )
   {
      return false;
   }

   connectionId = userRequester->GetConnectionId();

   stringhash addedUserHash = GenerateUniqueHash( addedUserUuid );
   UserMapIterator addedUserIter = m_userMap.find( addedUserHash );
   if( addedUserIter == m_userMap.end() )
   {
      errorText += " but the added user does not exist in ChatRoomManager";
      m_chatServer->Log( errorText, 1 );
      m_chatServer->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_UserUnknown );
      return false;
   }

   const UsersChatRoomList& recipient = addedUserIter->second;


   stringhash channelHash = GenerateUniqueHash( channelUuid );   
   ChannelMapIterator channelIter = m_channelMap.find( channelHash );
   if( channelIter == m_channelMap.end() )
   {
      errorText += " but that channel does not exist in ChatRoomManager";
      m_chatServer->Log( errorText, 1 );
      m_chatServer->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_NoChatChannel );
      return false;
   }

   ChatRoom& channelInfo = channelIter->second;
   list< UserBasics >   ::iterator it = channelInfo.userBasics.begin();
   while( it != channelInfo.userBasics.end() )
   {
      if( it->userUuid == addedUserUuid )
      {
         errorText += " but that added user is already in that channel";
         m_chatServer->Log( errorText, 1 );
         m_chatServer->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_CannotAddUserToChannel_AlreadyExists );
         return false;
      }
      it++;
   }

   AddUserToRoomAndWriteToDB( channelUuid, addedUserUuid, addedUserIter->second.userName );

   // inform users of success
   list< UserBasics >::iterator userIt = channelInfo.userBasics.begin();
   while( userIt != channelInfo.userBasics.end() )
   {
      UserBasics& ub = *userIt++;
      ChatUser* user = m_chatServer->GetUserByUuid( ub.userUuid );
      if( user )
      {
         user->NotifyAddedToChannel( channelInfo.name, channelUuid, addedUserIter->second.userName, addedUserUuid );
      }
   }

   return true;
}

//---------------------------------------------------------------------

bool     ChatRoomManager::AddUserToRoomAndWriteToDB( const string& channelUuid, const string& addedUserUuid, const string& addedUserName )
{
// insert into the list of users for that channel
   StoreUserInChannel( channelUuid, addedUserUuid, addedUserName );
   
   string createDate = GetDateInUTC();
   // add single entry to db for that chat channel
   string queryString = "INSERT INTO user_join_chat_channel ( user_uuid, channel_uuid, added_date ) VALUES ('%s','%s', '";
   queryString += createDate;
   queryString += "')";

   PacketDbQuery* dbQuery = DbQueryFactory( queryString, true );

   list< string > sanitizedStrings;
   sanitizedStrings.push_back( addedUserUuid );
   sanitizedStrings.push_back( channelUuid );
   AddSanitizedStrings( dbQuery, sanitizedStrings );

   Send( dbQuery );

   m_numChangesToChatRoom ++;

   return true;
}

//---------------------------------------------------------------------

bool     ChatRoomManager::RemoveUserFromRoomAndWriteToDB( const string& channelUuid, const string& removedUserUuid )
{
   if( DeleteUserFromChannel( channelUuid, removedUserUuid ) == false )
      return false;

   string createDate = GetDateInUTC();
   // add single entry to db for that chat channel
   string queryString = "DELETE FROM user_join_chat_channel WHERE user_uuid='%s' and channel_uuid='%s'";

   PacketDbQuery* dbQuery = DbQueryFactory( queryString, true );

   list< string > sanitizedStrings;
   sanitizedStrings.push_back( removedUserUuid );
   sanitizedStrings.push_back( channelUuid );
   AddSanitizedStrings( dbQuery, sanitizedStrings );

   Send( dbQuery );

   m_numChangesToChatRoom ++;

   return true;
}


//---------------------------------------------------------------------

bool     ChatRoomManager::RemoveRoomAndMarkRoomInDB( const string& channelUuid )
{
   stringhash channelHash = GenerateUniqueHash( channelUuid );   
   ChannelMapIterator channelIter = m_channelMap.find( channelHash );
   if( channelIter == m_channelMap.end() )
   {
      return false;
   }

   string queryString = "UPDATE chat_channel SET is_active=0 WHERE uuid='%s'";

   list< string > sanitizedStrings;
   sanitizedStrings.push_back( channelUuid );
   PacketDbQuery* dbQuery = DbQueryFactory( queryString, true );
   AddSanitizedStrings( dbQuery, sanitizedStrings );

   Send( dbQuery );

   // remove... it's likely that this list is already empty, but just to be sure.
   // we definitely want it removed from memory
   
   ChatRoom& channelMapData = channelIter->second;
   string channelName = channelIter->second.name;

   list< UserBasics >   ::iterator it = channelMapData.userBasics.begin();
   while( it != channelMapData.userBasics.end() )
   {
      list< UserBasics >   ::iterator temp = it++;
      string removedUserUuid = temp->userUuid;
      DeleteUserFromChannel( channelUuid, removedUserUuid );
   }

   m_channelMap.erase( channelIter );

   m_numChangesToChatRoom ++;

   return true;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------

bool     ChatRoomManager::RemoveUserFromRoom( const PacketChatRemoveUserFromChatChannelGameServer* request )
{
   U32 gameInstanceId = request->gameInstanceId;
   const string userUuid = request->userUuid.c_str(); // user to remove

   PacketChatRemoveUserFromChatChannelGameServerResponse* response = new PacketChatRemoveUserFromChatChannelGameServerResponse;
   response->gameId = request->gameId;
   response->userUuid = userUuid;
   response->success = false;

   ChannelMapIterator  channelIter = FindChatRoom( request->gameId, 0 );
   if( channelIter == m_channelMap.end() )
   {
      string errorText = " RemoveUserFromRoom: Game server ";
      errorText += boost::lexical_cast< string >( request->gameInstanceId );
      errorText += " tried to remove user ";
      errorText += userUuid;
      errorText += " from channel ";
      errorText += request->gameName;
      errorText += " but this does channel not exist ";
      m_chatServer->Log( errorText, 1 );
       

      PackageAndSendToOtherServer( response, request->gameInstanceId );
      return false;
   }

   ChatRoom& channelMapData = channelIter->second;
   list< UserBasics >   ::iterator userIt = channelMapData.userBasics.begin();
   while( userIt != channelMapData.userBasics.end() )
   {
      if( userIt->userUuid == userUuid )
      {
         break;// found
      }
      userIt++;
   }
   if( userIt == channelMapData.userBasics.end() )
   {
      PackageAndSendToOtherServer( response, request->gameInstanceId );
      return false;
   }

   const string& channelName = channelMapData.name;
   const string& channelUuid = channelMapData.uuid;
   const string& userName = userIt->userName;
   RemoveUserFromRoomAndWriteToDB( channelUuid, userUuid );

   response->success = false;
   PackageAndSendToOtherServer( response, request->gameInstanceId );
  
   // tell all clients that a user was removed from the chat channel   
   userIt = channelMapData.userBasics.begin();
   while( userIt != channelMapData.userBasics.end() )
   {
      ChatUser* user = m_chatServer->GetUserByUuid( userIt->userUuid );
      if( user )
      {
         user->NotifyRemovedFromChannel( channelName, channelUuid, true, userUuid );
      }
      userIt++;
   }


   return true;
}

//---------------------------------------------------------------------

bool     ChatRoomManager::RemoveUserFromRoom( const string& channelUuid, const string& removedUserUuid )
{
   stringhash  channelKeyLookup = GenerateUniqueHash( channelUuid );
   stringhash  userKeyLookup = GenerateUniqueHash( removedUserUuid );

   ChannelMapIterator channelIter = m_channelMap.find( channelKeyLookup );
   if( channelIter == m_channelMap.end() )
   {
      string text = " User ";
      text += removedUserUuid;
      text += " tried to be removed from channel ";
      text += channelUuid;
      text += " but the channel does not exist in ChatRoomManager";
      m_chatServer->Log( text, 1 );
      return false;
   }

   UserMapIterator userIter = m_userMap.find( userKeyLookup );
   if( userIter == m_userMap.end() )
   {
      string text = " User ";
      text += removedUserUuid;
      text += " tried to be removed from channel ";
      text += channelUuid;
      text += " but the user does not exist in ChatRoomManager";
      m_chatServer->Log( text, 1 );
      //pair< UserMapIterator, bool > ins = m_userMap.insert( UserPair( userHash, UsersChatRoomList( userUuid ) ) );
      //userIter = ins.first;
   }

   bool success = RemoveUserFromRoomAndWriteToDB( channelUuid, removedUserUuid );
   ChatUser* user = m_chatServer->GetUserByUuid( removedUserUuid );
   if( user )
   {
      user->NotifyRemovedFromChannel( channelIter->second.name, channelUuid, success );
      //user->SendErrorMessage( PacketErrorReport::ErrorType_ChatChannelCannotBeCreated );
   }

   if( success )
   {
      if( channelIter->second.userBasics.size() == 0 )
      {
         RemoveRoomAndMarkRoomInDB( channelUuid );
      }
      else // notify everyone remaining
      {
         ChatRoom& channelMapData = channelIter->second;
         string channelName = channelIter->second.name;

         list< UserBasics >   ::iterator it = channelMapData.userBasics.begin();
         while( it != channelMapData.userBasics.end() )
         {
            string notifyUserUuid = it->userUuid;
            ChatUser* user = m_chatServer->GetUserByUuid( notifyUserUuid );
            if( user )
            {
               user->NotifyRemovedFromChannel( channelIter->second.name, channelUuid, success, removedUserUuid );
            }
            it++;
         }
      }
   }

   return true;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------

void     ChatRoomManager::StoreUserInChannel( const string& channelUuid, const string& userUuid, const string userName )
{
   stringhash  channelKeyLookup = GenerateUniqueHash( channelUuid );
   ChannelMapIterator iter = m_channelMap.find( channelKeyLookup );
   if( iter == m_channelMap.end() )
   {
      return;
   }

   ChatRoom& channelMapData = iter->second;
   list< UserBasics >   ::iterator it = channelMapData.userBasics.begin();
   while( it != channelMapData.userBasics.end() )
   {
      if( it->userUuid == userUuid )
         return;
      it++;
   }
   
   channelMapData.userBasics.push_back( UserBasics( userName, userUuid ) );

   //---------------------------------------------
   StoreUser( userUuid, userName );
   AddRoomToUser( userUuid, channelKeyLookup );
}

//---------------------------------------------------------------------

bool     ChatRoomManager::DeleteUserFromChannel( const string& channelUuid, const string& userUuid )
{
   stringhash  channelKeyLookup = GenerateUniqueHash( channelUuid );
   ChannelMapIterator iter = m_channelMap.find( channelKeyLookup );
   if( iter == m_channelMap.end() )
   {
      return false;
   }

   bool success = false;
   ChatRoom& channelMapData = iter->second;
   list< UserBasics >   ::iterator it = channelMapData.userBasics.begin();
   while( it != channelMapData.userBasics.end() )
   {
      if( it->userUuid == userUuid )
      {
         success = true;
         break;
      }
      it++;
   }
   if( success == false )
      return false;

   channelMapData.userBasics.erase( it );

   //---------------------------------------------

   DeleteRoomFromUser( userUuid, channelKeyLookup );

   return true;
}

//---------------------------------------------------------------------

void     ChatRoomManager::QueryAllChatUsers( int startIndex, int numToFetch )
{
   assert( numToFetch > 0 );
   assert( startIndex >= 0 );

   string 
   queryString = "SELECT user_name, uuid, users.user_id, user_profile.block_contact_invitations, user_profile.block_group_invitations ";
   queryString += "FROM users INNER JOIN user_profile ON users.user_id=user_profile.user_id ";
   queryString += "WHERE user_email IS NOT NULL AND users.active=1 LIMIT ";

   queryString += boost::lexical_cast< string >( startIndex );
   queryString += ",";
   queryString += boost::lexical_cast< string >( numToFetch );

   PacketDbQuery* dbQuery = DbQueryFactory( queryString, false );
   SaveQueryDetails( dbQuery, "", "", 0, ChatRoomDbJob::JobType_LoadAllUsers );

   Send( dbQuery );
}

//---------------------------------------------------------------------

void     ChatRoomManager::StoreAllUsersInChannel( const string& channelUuid, const SerializedKeyValueVector< string >& usersAndIds, bool sendNotification )
{
   stringhash  channelKeyLookup = GenerateUniqueHash( channelUuid );
   ChannelMapIterator channelIter = m_channelMap.find( channelKeyLookup );
   if( channelIter == m_channelMap.end() )
   {
      assert( 0 );// wtf
   }

   ChatRoom& channelMapData = channelIter->second;

   SerializedKeyValueVector< string >::const_KVIterator userIt = usersAndIds.begin();
   while( userIt != usersAndIds.end() )
   {
      // first add the user to the channel info
      const string& userName = userIt->value;
      const string& userUuid = userIt->key;
      StoreUserInChannel( channelUuid, userUuid, userName );
      
      StoreUser( userUuid, userName );
      AddRoomToUser( userUuid, channelKeyLookup );

      //userInstance.userUuid = userUuid;// happens in c'tor

      if( sendNotification )
      {
         NotifyUserThatHeWasAddedToChannel( userUuid, channelMapData.name, channelUuid );
      }
      userIt ++;
   }
}

//---------------------------------------------------------------------

bool     ChatRoomManager::NotifyUserThatHeWasAddedToChannel( const string& userUuid, const string& channelName, const string& channelUuid )
{
   ChatUser* user = m_chatServer->GetUserByUuid( userUuid );
   if( user )
   {
      UsersChatRoomList userInChannel ( userUuid );
      if( GetUserInfo( userUuid, userInChannel ) == false )
         return false;

   // todo, change this UUID to the person who added these people
      return user->NotifyAddedToChannel( channelName, channelUuid, userInChannel.userName, userUuid );
   }
   return false;
}

//---------------------------------------------------------------------

//---------------------------------------------------------------------

bool     ChatRoomManager::QueryAllUsersInChatRoom( const string& channelUuid )
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
   SaveQueryDetails( dbQuery, channelUuid, "", 0, ChatRoomDbJob::JobType_AllUsersInChannel );
   AddSanitizedStrings( dbQuery, sanitizedStrings );
   return Send( dbQuery );

   //return CreateAndSendUserListJob( queryString, channelIter->second.name, channelUuid, 0, authUuid, userHashLookup, ChatRoomDbJob::JobType_AllUsersInChannel, &listOfStrings );
};

//---------------------------------------------------------------------

U32     ChatRoomManager::QueryAllUsersInAllChatRooms()
{
   string 
   queryString =  "SELECT user.user_name, user.uuid, user.user_id, joiner.channel_uuid  FROM users AS user ";
   queryString += "join user_join_chat_channel as joiner on joiner.user_uuid=user.uuid ";
   queryString += "join chat_channel as channel on joiner.channel_uuid=channel.uuid ";


   PacketDbQuery* dbQuery = DbQueryFactory( queryString, false );
   SaveQueryDetails( dbQuery, "", "", 0, ChatRoomDbJob::JobType_AllUsersInAllChannels );
   Send( dbQuery );

   return dbQuery->id;
   //return CreateAndSendUserListJob( queryString, channelIter->second.name, channelUuid, 0, authUuid, userHashLookup, ChatRoomDbJob::JobType_AllUsersInChannel, &listOfStrings );
}

//---------------------------------------------------------------------

bool     ChatRoomManager::HandleAllUsersInAllChannelsResult( PacketDbQueryResult* dbResult, ChatRoomDbJob& job )
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

         StoreUserInChannel( channelUuid, uuid, name );
      }
   }
   m_numChannelsToLoad--;// bad channels setup should not stop us
   m_isPullingAllUsersAndChannels = false;

   return true;
}

//---------------------------------------------------------------------

bool     ChatRoomManager::HandleLoadAllRoomsResult( PacketDbQueryResult* dbResult, ChatRoomDbJob& job )
{
   if( dbResult->successfulQuery == true )
   {
      ChatChannelTable              enigma( dbResult->bucket );
      ChatChannelTable::iterator    it = enigma.begin();
      m_numChannelsToLoad = static_cast< int >( enigma.m_bucket.size() );
      while( it != enigma.end() )
      {
         SaveChatRoomLoadResult( *it++ );
      }
   }

   return true;
}

//---------------------------------------------------------------------

bool     ChatRoomManager::HandleLoadAllUsersResult( PacketDbQueryResult* dbResult, ChatRoomDbJob& job )
{
   if( dbResult->successfulQuery == true )
   {
      UserWithChatPreferencesTable  enigma( dbResult->bucket );
      UserWithChatPreferencesTable::iterator    it = enigma.begin();
      if( it == enigma.end() ) // we are done querying piecemeal
      {
         QueryAllUsersInAllChatRooms();// now that we've loaded all users, load all of the chat info
         return true;
      }
      while( it != enigma.end() )
      {
         UserWithChatPreferencesTable::row row = *it++;
         string   userName =         row[ TableUserWithChatPreferences::Column_name ];
         string   userUuid =         row[ TableUserWithChatPreferences::Column_uuid ];
         string   blockContactInvitesString =   row[ TableUserWithChatPreferences::Column_block_contact_invites ];
         string   blockGroupInvitesString =     row[ TableUserWithChatPreferences::Column_block_group_invites ];

         bool blockContactInvites = false;
         if( blockContactInvitesString == "1" )
            blockContactInvites = true;
         bool blockGroupInvites = false;
         if( blockGroupInvitesString == "1" )
            blockGroupInvites = true;

         string id = row[ TableUserWithChatPreferences::Column_id ];
         U32 userId = 0;
         if( id.size() )
            userId = boost::lexical_cast< U32 >( id );

         StoreUser( userUuid, userId, userName, blockContactInvites, blockGroupInvites );
      }

      m_offsetIndex_QueryForInitialLoad += m_numUsersToLoadPerQueryForInitialLoad;
      QueryAllChatUsers( m_offsetIndex_QueryForInitialLoad, m_numUsersToLoadPerQueryForInitialLoad );
   }

   return true;
}

//---------------------------------------------------------------------

bool     ChatRoomManager::HandleSingleRoomLoad( PacketDbQueryResult* dbResult, ChatRoomDbJob& job )
{
   if( dbResult->successfulQuery == true )
   {
      ChatChannelTable              enigma( dbResult->bucket );
      ChatChannelTable::iterator    it = enigma.begin();
      m_numChannelsToLoad = static_cast< int >( enigma.m_bucket.size() );
      while( it != enigma.end() )
      {
         SaveChatRoomLoadResult( *it++ );
      }
   }

   return true;
}

//---------------------------------------------------------------------

void     ChatRoomManager::SaveChatRoomLoadResult( ChatChannelTable::row row )
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
   
   AddChatRoom( channelId, name, uuid, isActive, maxPlayers, gameType, gameId, createDate );
}

//---------------------------------------------------------------------
/*
void     ChatRoomManager::SaveUserLoadResult( SimpleUserTable::row row )
{
   //int      id =    boost::lexical_cast< int >( row[ TableSimpleUser::Column_id ] );
   string   name =         row[ TableSimpleUser::Column_name ];
   string   uuid =         row[ TableSimpleUser::Column_uuid ];
   
   StoreUser( uuid,  name );
}*/


//---------------------------------------------------------------------

void     ChatRoomManager::WriteChatToDb( const string& message, const string& senderUuid, const string& friendUuid, const string& channelUuid, U16 gameTurn, U32 connectionId )
{
   bool isGameChannel = false;
   if( channelUuid.size() )
   {
      stringhash  keyLookup = GenerateUniqueHash( channelUuid );
      ChannelMapIterator iter = m_channelMap.find( keyLookup );      
      if( iter != m_channelMap.end() )
      {
         ChatRoom& channel = iter->second;
         if( channel.gameType && channel.gameInstanceId )
            isGameChannel = true;
      }
   }
   
   string queryString = "INSERT INTO chat_message ( text, user_id_sender, user_id_recipient, chat_channel_id, timestamp, game_turn ) VALUES( _utf8'%s', '%s', ";
   if( friendUuid.size() > 0 )
   {
      queryString += "'%s'";
   }
   else
   {
      queryString += "null";
   }
   queryString += ", '%s', UTC_TIMESTAMP(), ";
   if( isGameChannel == true )
   {
      queryString += boost::lexical_cast< string >( gameTurn );
   }
   else
   {
      queryString += "null";
   }
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

   if( friendUuid.size() )
      m_numP2PChatsSent ++;
   else
      m_numChannelChatsSent ++;
}

//---------------------------------------------------------------------

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////