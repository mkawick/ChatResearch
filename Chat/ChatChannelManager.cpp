// ChatChannelManager.cpp

#include <assert.h>

#include <iomanip>
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
const int maxNumPlayersInChatChannel = 32;

// ChatChannelManagerNeedsUpdate

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

ChatChannelManager::ChatChannelManager(): m_dbIdTracker( 0 ),
                                          m_isInitialized( false ),
                                          m_numChannelsToLoad( 0 ),
                                          m_isPullingAllUsersAndChannels( false ),
                                          m_numChannelChatsSent( 0 ),
                                          m_numP2PChatsSent( 0 ),
                                          m_numChangesToChatChannel( 0 )
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

   queryString = "SELECT user_name, uuid, user_id FROM users WHERE user_email IS NOT NULL";
   PacketDbQuery* dbQuery2 = DbQueryFactory( queryString, false );
   SaveQueryDetails( dbQuery2, "", "", 0, ChatChannelDbJob::JobType_LoadAllUsers );
   Send( dbQuery2 );

   m_isInitialized = true;
   m_isPullingAllUsersAndChannels = true;

   time( &m_initializationTimeStamp );
}
/*
Example:
PacketDbQuery* query = DbQueryFactory( const string& queryString, bool isFandF );
SaveQueryDetails( dbQuery, const string& channelUuid, const string& authUuid, const string& chatUserLookup, ChatChannelDbJob::JobType jobType, U32 serverId = 0, const string& debugString = "debug_string" );
AddSanitizedStrings( dbQuery, list< string >* sanitizedStrings );
AddCustomData( dbQuery, void* data );
Send( dbQuery );*/

void     ChatChannelManager::ClearStats()
{
   m_numChannelChatsSent = 0;
   m_numP2PChatsSent = 0;
   m_numChangesToChatChannel = 0;
}

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

bool     ChatChannelManager::UserHasLoggedIn( const string& userUuid )
{
   stringhash userHash = GenerateUniqueHash( userUuid );
   UserMapIterator userIter = m_userMap.find( userHash );
   if( userIter == m_userMap.end() )// new user account creation.
   {
      pair< UserMapIterator, bool > ins = m_userMap.insert( UserPair( userHash, UsersChatChannelList( userUuid ) ) );
      userIter = ins.first;
   }

   userIter->second.isOnline = true;

   return true;
}

//---------------------------------------------------------------

bool     ChatChannelManager::UserHasLoggedOut( const string& userUuid )
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
   case ChatChannelDbJob::JobType_LoadSingleChannel:
      {
         HandleSingleChannelLoad( dbResult, job );
      }
      break;
   case ChatChannelDbJob::JobType_LoadAllChannels:
      {
         HandleLoadAllChannelsResult( dbResult, job );       
      }
      break;
   case ChatChannelDbJob::JobType_LoadAllUsers:
      {
         HandleLoadAllUsersResult( dbResult, job );
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

               //StoreUserInChannel( channelUuid, UserBasics& ub )

               usersAndIds.insert( uuid, name );
            }

            StoreAllUsersInChannel( job.lookupString, usersAndIds );
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

/*
bool  ChatChannelManager::AddSanitizedStrings( PacketDbQuery* dbQuery, int numParams, ... )
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
         ChannelMapIterator channelIter = m_channelMap.find( channelLookupHash );   
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

void     ChatChannelManager::StoreUser( const string& userUuid, const string& userName )
{
   stringhash  userKeyLookup = GenerateUniqueHash( userUuid );
   UserMapIterator userIter = m_userMap.find( userKeyLookup );
   if( userIter == m_userMap.end() )
   {
      pair< UserMapIterator,bool > newNode = m_userMap.insert( UserPair( userKeyLookup, UsersChatChannelList( userUuid ) ) );
      userIter = newNode.first;
      UsersChatChannelList& userInstance = userIter->second;
      userInstance.userName = userName;
      userInstance.isOnline = false;
   }
}

UsersChatChannelList&        ChatChannelManager::GetUserInfo( const string& userUuid )
{
   stringhash  userKeyLookup = GenerateUniqueHash( userUuid );
   UserMapIterator userIter = m_userMap.find( userKeyLookup );
   if( userIter != m_userMap.end() )
   {
      return userIter->second;
   }
   assert( 0 );
   return userIter->second;
}


void     ChatChannelManager::AddChannelToUser( const string& userUuid, stringhash channelHash )
{
   stringhash  userKeyLookup = GenerateUniqueHash( userUuid );
   UserMapIterator userIter = m_userMap.find( userKeyLookup );
   if( userIter != m_userMap.end() )
   {
      UsersChatChannelList& userInstance = userIter->second;
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

void     ChatChannelManager::DeleteChannelFromUser( const string& userUuid, stringhash channelHash )
{
   stringhash  userKeyLookup = GenerateUniqueHash( userUuid );
   UserMapIterator userIter = m_userMap.find( userKeyLookup );
   if( userIter != m_userMap.end() )
   {
      UsersChatChannelList& userInstance = userIter->second;
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
   time_t t = time(0);   // get time now
   struct tm * now = localtime( & t );
   
   int year = now->tm_year - 100; // 113 => 2013.
   int month = now->tm_mon;
   int day = now->tm_mday;

   string date = GetDateInUTC();


   std::stringstream ss;
   const int maxNumberGameInstanceDigits = 7; // into the tens of millions of games
   ss  << std::setfill('0') << request->gameName << '_' 
            << std::setw(2) << year << ':' 
            << std::setw(2) << month << ':' 
            << std::setw(2) << day << '_' 
            << std::setw( maxNumberGameInstanceDigits ) << request->gameId;
   string channelName = ss.str();

   // the gameId and the GameInstandId need to be verified here.. the order matters, and I can't figure out which one is which.
   string channelUuid = CreateNewChannel( channelName, "", request->gameInstanceId, request->gameProductId, request->gameId );

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
         string userName = GetUserInfo( userUuid ).userName;

         AddUserToChannelAndWriteToDB( channelUuid, userUuid, userName );
         NotifyUserThatHeWasAddedToChannel( userUuid, channelName, channelUuid );
      }

   }
   return true;
}

//---------------------------------------------------------------------

bool ChatChannelManager::CreateNewChannel( const string& channelName, const string& userUuid )
{
   string channelUuid = CreateNewChannel( channelName, userUuid, 0, 0, 0 );
   string userName = GetUserInfo( userUuid ).userName;

   ChatUser* user = m_chatServer->GetUserByUuid( userUuid );// this is a little odd logic. However, the creator should be online 
   if( user )
   {
      if( channelUuid.size() )
      {
         AddUserToChannelAndWriteToDB( channelUuid, userUuid, userName );
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
   bool isActive = true;
   string createDate = GetDateInUTC();
   AddChatchannel( 0, channelName, channelUuid, isActive, maxNumPlayersInChatChannel, gameType, gameInstanceId, createDate );

   string queryString = "INSERT INTO chat_channel VALUES( null, '%s','";
   queryString += channelUuid;
   queryString += "', 1, "; // active
   queryString += boost::lexical_cast< string >( maxNumPlayersInChatChannel );
   queryString += ", ";
   queryString += boost::lexical_cast< string >( (U32)( gameType ) );
   queryString += ", ";
   queryString += boost::lexical_cast< string >( gameInstanceId ); // almost always 0
   queryString += ", '";
   queryString += createDate;
   queryString += "', null )"; // default expry date.

   list< string > sanitizedStrings;
   sanitizedStrings.push_back( channelName );

   PacketDbQuery* dbQuery = DbQueryFactory( queryString, true );
   AddSanitizedStrings( dbQuery, sanitizedStrings );
   Send( dbQuery );

   LoadSingleChannel( channelUuid ); // just grab the db defaults( id, timestamp, etc );
   return channelUuid;
}

//---------------------------------------------------------------------

bool     ChatChannelManager::LoadSingleChannel( const string& channelUuid )
{
   cout << "LoadSingleChannel " << channelUuid << endl;

   string queryString = "SELECT * FROM chat_channel WHERE uuid='%s'";
   U32 serverId = 0;
   
   list< string > sanitizedStrings;
   sanitizedStrings.push_back( channelUuid );

   PacketDbQuery* dbQuery = DbQueryFactory( queryString, false );
   SaveQueryDetails( dbQuery, "", "", 0, ChatChannelDbJob::JobType_LoadSingleChannel );
   AddSanitizedStrings( dbQuery, sanitizedStrings );
   Send( dbQuery );

   return true;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------

bool   ChatChannelManager::DeleteChannel( const PacketChatDeleteChatChannelFromGameServer* request )
{
   PacketChatDeleteChatChannelFromGameServerResponse* response = new PacketChatDeleteChatChannelFromGameServerResponse;
   response->gameId = request->gameId;
   response->gameProductId = request->gameProductId;
   response->gameInstanceId = request->gameInstanceId;
   
   bool success = false;
   U8 gameType = request->gameProductId;
   U32 gameInstanceId = request->gameId;// 
   ChannelMapIterator iter = FindChatChannel( gameInstanceId, gameType );
   if( iter == m_channelMap.end() )
   {
      response->numUsersRemoved = 0;
   }
   else
   {
      success = DeleteChannel( iter->second.uuid );
      response->numUsersRemoved = iter->second.userBasics.size();
   }

   response->successfullyDeleted = success;
   PackageAndSendToOtherServer( response, response->gameInstanceId );// TODO verify that this is the correct server id

   return success;
}

//---------------------------------------------------------------------

bool     ChatChannelManager::DeleteChannel( const string& channelUuid, const string& userUuid )
{
   ChatUser* userSender = m_chatServer->GetUserByUuid( userUuid );
   if( userSender == NULL )
   {
      string text = " User ";
      text += userUuid;
      text += " tried to send delete channel ";
      text += channelUuid;
      text += " but the user does not exist in ChatChannelManager";
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
      text += " but the channel does not exist in ChatChannelManager";
      m_chatServer->Log( text, 1 );
      m_chatServer->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_BadChatChannel );
      return false;
   }

   int numUsers = channelIter->second.userBasics.size();

   bool success = DeleteChannel( channelUuid );   
   
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

bool     ChatChannelManager::DeleteChannel( const string& channelUuid )
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
      int numUsers = listOfUsers.size();
   
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

   m_numChangesToChatChannel ++;

   return true;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------

bool     ChatChannelManager::UserSendP2PChat( const string& senderUuid, const string& destinationUuid, const string& message )
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
      text += " but the sender does not exist in ChatChannelManager";
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

   return true;
}

//---------------------------------------------------------------------

bool     ChatChannelManager::UserSendsChatToChannel( const string& senderUuid, const string& channelUuid, const string& message, U16 gameTurn )
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
      text += " but that user does not exist in ChatChannelManager";
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
      text += " but that channel does not exist in ChatChannelManager";
      m_chatServer->Log( text, 1 );
      return false;
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
      return false;
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
   return true;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------

bool     ChatChannelManager::RenameChatChannel( const string& chanelUuid, const string& newName, const string& userUuid )
{
   // TODO
   return true;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------

bool     ChatChannelManager::AddUserToChannel( const PacketChatAddUserToChatChannelGameServer* request )
{
   U32 gameInstanceId = request->gameInstanceId;
   const string& addedUserUuid = request->userUuid; // user to remove

   string errorText = " AddUserToChannel: Game server ";
   errorText += boost::lexical_cast< string >( request->gameInstanceId );
   errorText += " tried to add another user ";
   errorText += addedUserUuid;
   errorText += " to channel ";
   errorText += request->gameName;
   

   PacketChatAddUserToChatChannelGameServerResponse* response = new PacketChatAddUserToChatChannelGameServerResponse;
   response->gameId = request->gameId;
   response->userUuid = addedUserUuid;
   response->success = false;


   ChannelMapIterator  channelIter = FindChatChannel( request->gameId, 0 );
   if( channelIter == m_channelMap.end() )
   {
      errorText += " but channel does not exist ";
      m_chatServer->Log( errorText, 1 );
      PackageAndSendToOtherServer( response, request->gameInstanceId );
      return false;
   }

   ChatChannel& channelMapData = channelIter->second;
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
      errorText += " but the added user does not exist in ChatChannelManager";
      m_chatServer->Log( errorText, 1 );
      PackageAndSendToOtherServer( response, request->gameInstanceId );// failure
      return false;
   }

   const string& channelName = channelMapData.name;
   const string& channelUuid = channelMapData.uuid;
   const string& userName = addedUserIter->second.userName;
   AddUserToChannelAndWriteToDB( channelUuid, addedUserUuid, userName );

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

bool     ChatChannelManager::AddUserToChannel( const string& channelUuid, const string& addedUserUuid, const string& requesterUuid )
{
   string errorText = " AddUserToChannel: User ";
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
      errorText += " but the requester does not exist in ChatChannelManager";
      m_chatServer->Log( errorText, 1 );
      return false;
   }

   ChatUser* userRequester = m_chatServer->GetUserByUuid( requesterIter->second.userUuid );
   U32 connectionId = userRequester->GetConnectionId();

   stringhash addedUserHash = GenerateUniqueHash( addedUserUuid );
   UserMapIterator addedUserIter = m_userMap.find( addedUserHash );
   if( addedUserIter == m_userMap.end() )
   {
      errorText += " but the added user does not exist in ChatChannelManager";
      m_chatServer->Log( errorText, 1 );
      m_chatServer->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_UserUnknown );
      return false;
   }

   stringhash channelHash = GenerateUniqueHash( channelUuid );   
   ChannelMapIterator channelIter = m_channelMap.find( channelHash );
   if( channelIter == m_channelMap.end() )
   {
      errorText += " but that channel does not exist in ChatChannelManager";
      m_chatServer->Log( errorText, 1 );
      m_chatServer->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_NoChatChannel );
      return false;
   }

   ChatChannel& channelInfo = channelIter->second;
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

   AddUserToChannelAndWriteToDB( channelUuid, addedUserUuid, addedUserIter->second.userName );

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

bool     ChatChannelManager::AddUserToChannelAndWriteToDB( const string& channelUuid, const string& addedUserUuid, const string& addedUserName )
{
// insert into the list of users for that channel
   StoreUserInChannel( channelUuid, addedUserUuid, addedUserName );

   string createDate = GetDateInUTC();
   // add single entry to db for that chat channel
   string queryString = "INSERT INTO user_join_chat_channel VALUES ('%s','%s', null, '";
   queryString += createDate;
   queryString += "', null )";

   PacketDbQuery* dbQuery = DbQueryFactory( queryString, true );

   list< string > sanitizedStrings;
   sanitizedStrings.push_back( addedUserUuid );
   sanitizedStrings.push_back( channelUuid );
   AddSanitizedStrings( dbQuery, sanitizedStrings );

   Send( dbQuery );

   m_numChangesToChatChannel ++;

   return true;
}

//---------------------------------------------------------------------

bool     ChatChannelManager::RemoveUserFromChannelAndWriteToDB( const string& channelUuid, const string& removedUserUuid )
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

   m_numChangesToChatChannel ++;

   return true;
}


//---------------------------------------------------------------------

bool     ChatChannelManager::RemoveChannelAndMarkChannelInDB( const string& channelUuid )
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
   
   ChatChannel& channelMapData = channelIter->second;
   string channelName = channelIter->second.name;

   list< UserBasics >   ::iterator it = channelMapData.userBasics.begin();
   while( it != channelMapData.userBasics.end() )
   {
      list< UserBasics >   ::iterator temp = it++;
      string removedUserUuid = temp->userUuid;
      DeleteUserFromChannel( channelUuid, removedUserUuid );
   }

   m_channelMap.erase( channelIter );

   m_numChangesToChatChannel ++;

   return true;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------

bool     ChatChannelManager::RemoveUserFromChannel( const PacketChatRemoveUserFromChatChannelGameServer* request )
{
   U32 gameInstanceId = request->gameInstanceId;
   const string& userUuid = request->userUuid; // user to remove

   PacketChatRemoveUserFromChatChannelGameServerResponse* response = new PacketChatRemoveUserFromChatChannelGameServerResponse;
   response->gameId = request->gameId;
   response->userUuid = userUuid;
   response->success = false;

   ChannelMapIterator  channelIter = FindChatChannel( request->gameId, 0 );
   if( channelIter == m_channelMap.end() )
   {
      string errorText = " RemoveUserFromChannel: Game server ";
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

   ChatChannel& channelMapData = channelIter->second;
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
   RemoveUserFromChannelAndWriteToDB( channelUuid, userUuid );

   response->success = false;
   PackageAndSendToOtherServer( response, request->gameInstanceId );
  
   // tell all clients that a user was added to the chat channel   
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

bool     ChatChannelManager::RemoveUserFromChannel( const string& channelUuid, const string& removedUserUuid )
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
      text += " but the channel does not exist in ChatChannelManager";
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
      text += " but the user does not exist in ChatChannelManager";
      m_chatServer->Log( text, 1 );
      //pair< UserMapIterator, bool > ins = m_userMap.insert( UserPair( userHash, UsersChatChannelList( userUuid ) ) );
      //userIter = ins.first;
   }

   bool success = RemoveUserFromChannelAndWriteToDB( channelUuid, removedUserUuid );
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
         RemoveChannelAndMarkChannelInDB( channelUuid );
      }
      else // notify everyone remaining
      {
         ChatChannel& channelMapData = channelIter->second;
         string channelName = channelIter->second.name;

         list< UserBasics >   ::iterator it = channelMapData.userBasics.begin();
         while( it != channelMapData.userBasics.end() )
         {
            string notifyUserUuid = it->userUuid;
            ChatUser* user = m_chatServer->GetUserByUuid( removedUserUuid );
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

void     ChatChannelManager::StoreUserInChannel( const string& channelUuid, const string& userUuid, const string userName )
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
   
   channelMapData.userBasics.push_back( UserBasics( userName, userUuid ) );

   //---------------------------------------------
   StoreUser( userUuid, userName );
   AddChannelToUser( userUuid, channelKeyLookup );
}

//---------------------------------------------------------------------

bool     ChatChannelManager::DeleteUserFromChannel( const string& channelUuid, const string& userUuid )
{
   stringhash  channelKeyLookup = GenerateUniqueHash( channelUuid );
   ChannelMapIterator iter = m_channelMap.find( channelKeyLookup );
   if( iter == m_channelMap.end() )
   {
      return false;
   }

   bool success = false;
   ChatChannel& channelMapData = iter->second;
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

   DeleteChannelFromUser( userUuid, channelKeyLookup );

   return true;
}

//---------------------------------------------------------------------

void     ChatChannelManager::StoreAllUsersInChannel( const string& channelUuid, const SerializedKeyValueVector< string >& usersAndIds, bool sendNotification )
{
   stringhash  channelKeyLookup = GenerateUniqueHash( channelUuid );
   ChannelMapIterator channelIter = m_channelMap.find( channelKeyLookup );
   if( channelIter == m_channelMap.end() )
   {
      assert( 0 );// wtf
   }

   ChatChannel& channelMapData = channelIter->second;

   SerializedKeyValueVector< string >::const_KVIterator userIt = usersAndIds.begin();
   while( userIt != usersAndIds.end() )
   {
      // first add the user to the channel info
      const string& userName = userIt->value;
      const string& userUuid = userIt->key;
      StoreUserInChannel( channelUuid, userUuid, userName );
      
      StoreUser( userUuid, userName );
      AddChannelToUser( userUuid, channelKeyLookup );

      //userInstance.userUuid = userUuid;// happens in c'tor

      if( sendNotification )
      {
         NotifyUserThatHeWasAddedToChannel( userUuid, channelMapData.name, channelUuid );
      }
      userIt ++;
   }
}

//---------------------------------------------------------------------

bool     ChatChannelManager::NotifyUserThatHeWasAddedToChannel( const string& userUuid, const string& channelName, const string& channelUuid )
{
   ChatUser* user = m_chatServer->GetUserByUuid( userUuid );
   if( user )
   {
      string userName = GetUserInfo( userUuid ).userName;
   // todo, change this UUID to the person who added these people
      return user->NotifyAddedToChannel( channelName, channelUuid, userName, userUuid );
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

         StoreUserInChannel( channelUuid, uuid, name );
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
   }

   return true;
}

//---------------------------------------------------------------------

bool     ChatChannelManager::HandleLoadAllUsersResult( PacketDbQueryResult* dbResult, ChatChannelDbJob& job )
{
   if( dbResult->successfulQuery == true )
   {
      SimpleUserTable              enigma( dbResult->bucket );
      SimpleUserTable::iterator    it = enigma.begin();
      while( it != enigma.end() )
      {
         SaveUserLoadResult( *it++ );
      }

      QueryAllUsersInAllChatChannels();
   }

   return true;
}

//---------------------------------------------------------------------

bool     ChatChannelManager::HandleSingleChannelLoad( PacketDbQueryResult* dbResult, ChatChannelDbJob& job )
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

void     ChatChannelManager::SaveUserLoadResult( SimpleUserTable::row row )
{
   //int      id =    boost::lexical_cast< int >( row[ TableSimpleUser::Column_id ] );
   string   name =         row[ TableSimpleUser::Column_name ];
   string   uuid =         row[ TableSimpleUser::Column_uuid ];
   
   StoreUser( uuid,  name );
}


//---------------------------------------------------------------------

void     ChatChannelManager::WriteChatToDb( const string& message, const string& senderUuid, const string& friendUuid, const string& channelUuid, U16 gameTurn, U32 connectionId )
{
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

   if( friendUuid.size() )
      m_numP2PChatsSent ++;
   else
      m_numChannelChatsSent ++;
}

//---------------------------------------------------------------------

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////