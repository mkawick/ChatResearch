#pragma once

#include <map>
#include <deque>
#include <list>
using namespace std;

#include "ChatChannel.h"
#include "ChatChannelDbJob.h"
#include "UsersChatChannelList.h"

#include "../NetworkCommon/Packets/ChatPacket.h"
#include "../NetworkCommon/Utils/TableWrapper.h"

//------------------------------------------------------

class DiplodocusChat;
class ChatUser;
class BasePacket;
class PacketDbQuery;
class PacketDbQueryResult;

class PacketChatCreateChatChannelFromGameServer;
class PacketChatDeleteChatChannelFromGameServer;
class PacketChatAddUserToChatChannelGameServer;
class PacketChatRemoveUserFromChatChannelGameServer;


//////////////////////////////////////////////////////////////

class TableUserWithChatPreferences
{
public:
   enum Columns
   {
      Column_name,
      Column_uuid,
      Column_id,
      Column_block_contact_invites,
      Column_block_group_invites,
      Column_end
   };
   static const char* const column_names[];
};

typedef Enigmosaurus <TableUserWithChatPreferences> UserWithChatPreferencesTable;

class TableUserWithChatPreferencesJoinChatChannel
{
public:
   enum Columns
   {
      Column_name,
      Column_uuid,
      Column_id,
      Column_block_contact_invites,
      Column_block_group_invites,
      Column_channel_uuid,
      Column_end
   };
   static const char* const column_names[];
};

typedef Enigmosaurus <TableUserWithChatPreferencesJoinChatChannel> UserWithPreferencesJoinChatChannelTable;


// The chat channel manager maintains a list of all chat channels, adds new ones, and removes old ones
// if a user sends a chat to a channel, the ChatUser simply passes the request straight onto the chat channel manager.
// Even user-to-user chat is handled this way.
/*
The flow works like this.

Client requests to send a chat to another user identified by uuid.
   Packet->DiplodocusChat
   Find sender in user-list and push packet. Request update.
   During update... ChatUser->HandleRequest...
   ChatUser finds the friend (verification)
   ChatUser tells ChatChannelManager::SendP2PChat
   ChatChannelManager requests destination user from DiplodocusChat
   If user is found, send a chat immediately.
   ChatChannelManager writes chat to db
*/

static const int ChatChannelManagerUniqueId = 0;
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

class ChatChannelManager
{
public:
   ChatChannelManager();
   ~ChatChannelManager();

   void           Init();

   static void    Set( DiplodocusChat* svr ) { m_chatServer = svr; }
   int            GetNumChannelChatsSent() const { return m_numChannelChatsSent; }
   int            GetNumP2PChatsSent() const { return m_numP2PChatsSent; }
   int            GetNumChangesToChatChannel() const { return m_numChangesToChatChannel; }
   void           ClearStats();

   bool           Update();
   bool           AddInboundPacket( BasePacket* packet ); // not thread safe
   bool           HandleDbResult( PacketDbQueryResult* packet );

   bool           CreateNewChannel( const string& channelName, const string& userUuid );
   bool           GetChatChannels( const string& userUuid, ChannelFullKeyValue& availableChannels );


   bool           CreateNewChannel( const PacketChatCreateChatChannelFromGameServer* pPacket );
   bool           DeleteChannel( const PacketChatDeleteChatChannelFromGameServer* request );
   bool           DeleteChannel( const string& chanelUuid, const string& userUuid );

   bool           RenameChatChannel( const string& channelUuid, const string& newName, const string& userUuid, string& oldName );// returns old name

   bool           AddUserToChannel( const PacketChatAddUserToChatChannelGameServer* packet );
   bool           AddUserToChannel( const string& channelUuid, const string& userUuid, const string& requesterUuid );

   bool           RemoveUserFromChannel( const PacketChatRemoveUserFromChatChannelGameServer* packet );
   bool           RemoveUserFromChannel( const string& channelUuid, const string& userUuid );

   bool           UserHasLoggedIn( const string& userUUid );
   bool           UserHasLoggedOut( const string& userUUid );
   bool           UserSendP2PChat( const string& senderUuid, const string& receiverUuid, const string& message );
   bool           UserSendsChatToChannel( const string& senderUuid, const string& channelUuid, const string& message, U16 gameTurn );

private:

   //---------------------------------------------------
   typedef list< ChatChannelDbJob >          DbJobList;
   typedef DbJobList::iterator               DbJobIterator;

   typedef map< stringhash, ChatChannel >    ChannelMap;
   typedef ChannelMap::iterator              ChannelMapIterator;
   typedef pair< stringhash, ChatChannel >   ChannelMapPair;

   typedef map< stringhash, UsersChatChannelList >    UserMap;
   typedef UserMap::iterator                          UserMapIterator;
   typedef pair< stringhash, UsersChatChannelList >   UserPair;
   //---------------------------------------------------

   void           ProcessIncomingPacket( BasePacket* );
   bool           ProcessDbResult( PacketDbQueryResult* packet, ChatChannelDbJob& job );

   // lots of utility functions
   int            AddDbJob( const string& debugString, const string& lookupString, U32 serverId, stringhash chatUserLookup, stringhash authUserLookup, ChatChannelDbJob::JobType type );
   PacketDbQuery* DbQueryFactory( const string& queryString, bool isFandF = false );
   bool           SaveQueryDetails( PacketDbQuery* dbQuery, const string& channelUuid, const string& authUuid, stringhash chatUserLookup, ChatChannelDbJob::JobType jobType, U32 serverId = 0, const string& debugString = "debug_string" );
   bool           AddSanitizedStrings( PacketDbQuery* dbQuery, list< string >& sanitizedStrings );
   //bool           AddSanitizedStrings( PacketDbQuery* dbQuery, int num, ... );
   bool           AddCustomData( PacketDbQuery* dbQuery, void* data );
   bool           Send( PacketDbQuery* dbQuery );
   void           PackageAndSendToOtherServer( BasePacket* packet, U32 serverId );
   string         CreateUniqueChatChannelId();


   bool           FindDbJob( int jobId, list< ChatChannelDbJob >& listOfJobs, DbJobIterator& iter );
   void           AddChatchannel( int id, const string& channelName, const string& channelUuid, bool isActive, int maxPlayers, int gameType, U32 gameInstanceId, const string& createDate );
   void           AddMetaData( PacketDbQueryResult* dbQuery, void* myData );
   int            NewDbId();
   ChannelMapIterator   FindChatChannel( U32 gameInstanceId, U8 gameType );
   string         FindChannelByGameId( U32 gameInstanceId );

   void           StoreUserInChannel( const string& channelUuid, const string& userUuid, const string username, bool blockContactInvites, bool blockGroupInvites );
   bool           DeleteUserFromChannel( const string& channelUuid, const string& userUuid );
   void           StoreAllUsersInChannel( const string& channelUuid, const SerializedKeyValueVector< UserBasics >& usersAndIds, bool sendNotification = false );
   bool           NotifyUserThatHeWasAddedToChannel( const string& userUuid, const string& channelName, const string& channelUuid ); 
   U32            QueryAllUsersInAllChatChannels();

   //void           HandleChatChannelCreateResult( PacketDbQueryResult* dbResult, ChatChannelDbJob& job );
   string         CreateNewChannel( const string& channelName, const string userUuid, U32 serverId, U8 gameType, U32 gameInstanceId );
   bool           DeleteChannel( const string& chanelUuid );
   
   bool           LoadSingleChannel( const string& channelUuid );

   bool           HandleLoadAllChannelsResult( PacketDbQueryResult* dbResult, ChatChannelDbJob& job );
   bool           HandleLoadAllUsersResult( PacketDbQueryResult* dbResult, ChatChannelDbJob& job );
   bool           HandleSingleChannelLoad( PacketDbQueryResult* dbResult, ChatChannelDbJob& job );
   void           SaveChatChannelLoadResult( ChatChannelTable::row row );
   void           SaveUserLoadResult( UserWithChatPreferencesTable::row row );
   bool           HandleAllUsersInAllChannelsResult( PacketDbQueryResult* dbResult, ChatChannelDbJob& job );

   void           StoreUser( const string& userUuid, const string& userName, bool blockContactInvites, bool blockGroupInvites );
   UsersChatChannelList&   GetUserInfo( const string& userUuid );
   void           AddChannelToUser( const string& userUuid, stringhash channelHash );
   void           DeleteChannelFromUser( const string& userUuid, stringhash channelHash );
   bool           AddUserToChannelAndWriteToDB( const string& channelUuid, const string& addedUserUuid, const string& addedUserName, bool blockContactInvites, bool blockGroupInvites );
   bool           RemoveUserFromChannelAndWriteToDB( const string& channelUuid, const string& removedUserUuid );
   bool           RemoveChannelAndMarkChannelInDB( const string& channelUuid );
   //-----------------------------------------------------

   // query functions
   bool     RequestAllUsersInChatChannel( const string& channelUuid, bool fullList, const string& authUuid );
   bool     QueryAllUsersInChatChannel( const string& channelUuid );
   void     WriteChatToDb( const string& message, const string& senderUuid, const string& friendUuid, const string& channelUuid, U16 gameTurn, U32 connectionId );

   //-----------------------------------------------------

   static DiplodocusChat*        m_chatServer;
   string                        m_systemNotificationUuid;
   string                        m_adminNotificationUuid;
   string                        m_techSupportNotificationUuid;

   deque< BasePacket* >          m_inboundPackets;
   deque< PacketDbQueryResult* > m_dbResults;

   DbJobList                     m_jobsPending;
   ChannelMap                    m_channelMap;
   UserMap                       m_userMap;

   bool                          m_isInitialized;

   int                           m_dbIdTracker;
   time_t                        m_initializationTimeStamp;
   int                           m_numChannelsToLoad;
   bool                          m_isPullingAllUsersAndChannels;
   
   int                           m_numChannelChatsSent;
   int                           m_numP2PChatsSent;
   int                           m_numChangesToChatChannel;
   
};

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
