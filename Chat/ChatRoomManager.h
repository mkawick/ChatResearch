#pragma once

#include <map>
#include <deque>
#include <list>
using namespace std;

#include "ChatRoom.h"
#include "ChatRoomDbJob.h"
#include "UsersChatRoomList.h"

#include "../NetworkCommon/Packets/ChatPacket.h"
#include "../NetworkCommon/Utils/TableWrapper.h"
#include "../NetworkCommon/Invitations/InvitationManager.h"

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

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

class ChatRoomManager : public GroupLookupInterface
{
public:
   ChatRoomManager();
   ~ChatRoomManager();

   void           Init();

   void           SetDbIdentifier( U32 dbIdentifier ) { m_dbIdentifier = dbIdentifier; }
   U32            GetDbIdentifier() const { return m_dbIdentifier; }

   static void    Set( InvitationManager* svr ) { m_invitationServer = svr; }

   static void    Set( DiplodocusChat* svr ) { m_chatServer = svr; }
   int            GetNumChannelChatsSent() const { return m_numChannelChatsSent; }
   int            GetNumP2PChatsSent() const { return m_numP2PChatsSent; }
   int            GetNumChangesToChatRoom() const { return m_numChangesToChatRoom; }
   void           ClearStats();

   bool           Update();
   bool           AddInboundPacket( BasePacket* packet ); // not thread safe
   bool           HandleDbResult( PacketDbQueryResult* packet );

   bool           CreateNewRoom( const string& channelName, const string& userUuid );
   bool           GetGroupName( const string& groupUuid, string& name ) const;
   bool           GetChatRooms( const string& userUuid, ChannelFullKeyValue& availableChannels );
   U32            GetUserId( const string& userUuid ) const;
   string         GetUserName( const string& uuid ) const;
   string         GetUserUuid( const U32 userId ) const;


   bool           CreateNewRoom( const PacketChatCreateChatChannelFromGameServer* pPacket );
   bool           DeleteRoom( const PacketChatDeleteChatChannelFromGameServer* request );
   bool           DeleteRoom( const string& chanelUuid, const string& userUuid );

   bool           RenameChatRoom( const string& channelUuid, const string& newName, const string& userUuid, string& oldName );// returns old name

   bool           SetUserPreferences( const string& userUuid, bool blockContactInvitations, bool blockGroupInvitations );
   bool           AddUserToRoom( const PacketChatAddUserToChatChannelGameServer* packet );
   bool           AddUserToRoom( const string& channelUuid, const string& userUuid, const string& requesterUuid );

   bool           RemoveUserFromRoom( const PacketChatRemoveUserFromChatChannelGameServer* packet );
   bool           RemoveUserFromRoom( const string& channelUuid, const string& userUuid );

   bool           UserHasLoggedIn( const string& userUUid );
   bool           UserHasLoggedOut( const string& userUUid );
   bool           UserSendP2PChat( const string& senderUuid, const string& receiverUuid, const string& message );
   bool           UserSendsChatToChannel( const string& senderUuid, const string& channelUuid, const string& message, U16 gameTurn );

   bool           RequestChatRoomInfo( const PacketChatListAllMembersInChatChannel* packet, U32 connectionId );// users + pending?
   //bool           UserAddsSelfToRoom( const string& channelUuid, const string& addedUserUuid );
      
   bool           UserAddsSelfToGroup( const string& channelUuid, const string& addedUserUuid );
   bool           GetUserInfo( const string& userUuid, UsersChatRoomList& ) const;
   //bool           IsRoomValid( const string& channelUuid ) const;
   bool           IsGroupValid( const string& inviteGroup ) const;
private:

   //---------------------------------------------------
   typedef list< ChatRoomDbJob >             DbJobList;
   typedef DbJobList::iterator               DbJobIterator;

   typedef map< stringhash, ChatRoom >       ChannelMap;
   typedef ChannelMap::iterator              ChannelMapIterator;
   typedef ChannelMap::const_iterator        ChannelMapConstIterator;
   typedef pair< stringhash, ChatRoom >      ChannelMapPair;

   typedef map< stringhash, UsersChatRoomList >    UserMap;
   typedef UserMap::iterator                       UserMapIterator;
   typedef UserMap::const_iterator                 UserMapConstIterator;
   typedef pair< stringhash, UsersChatRoomList >   UserPair;
   //---------------------------------------------------

   void           ProcessIncomingPacket( BasePacket* );
   bool           ProcessDbResult( PacketDbQueryResult* packet, ChatRoomDbJob& job );

   // lots of utility functions
   int            AddDbJob( const string& debugString, const string& lookupString, U32 serverId, stringhash chatUserLookup, stringhash authUserLookup, ChatRoomDbJob::JobType type );
   PacketDbQuery* DbQueryFactory( const string& queryString, bool isFandF = false );
   bool           SaveQueryDetails( PacketDbQuery* dbQuery, const string& channelUuid, const string& authUuid, stringhash chatUserLookup, ChatRoomDbJob::JobType jobType, U32 serverId = 0, const string& debugString = "debug_string" );
   bool           AddSanitizedStrings( PacketDbQuery* dbQuery, list< string >& sanitizedStrings );
   //bool           AddSanitizedStrings( PacketDbQuery* dbQuery, int num, ... );
   bool           AddCustomData( PacketDbQuery* dbQuery, void* data );
   bool           Send( PacketDbQuery* dbQuery );
   void           PackageAndSendToOtherServer( BasePacket* packet, U32 serverId );
   string         CreateUniqueChatRoomId();


   bool           FindDbJob( int jobId, list< ChatRoomDbJob >& listOfJobs, DbJobIterator& iter );
   void           AddChatRoom( int id, const string& channelName, const string& channelUuid, bool isActive, int maxPlayers, int gameType, U32 gameInstanceId, const string& createDate );
   void           AddMetaData( PacketDbQueryResult* dbQuery, void* myData );
   int            NewDbId();
   ChannelMapIterator   FindChatRoom( U32 gameInstanceId, U8 gameType );
   string         FindChannelByGameId( U32 gameInstanceId );

   void           StoreUserInChannel( const string& channelUuid, const string& userUuid, const string username );
   bool           DeleteUserFromChannel( const string& channelUuid, const string& userUuid );
   void           QueryAllChatUsers( int startIndex, int numToFetch );
   void           QueryAddNewChatUsers();
   void           StoreAllUsersInChannel( const string& channelUuid, const SerializedKeyValueVector< string >& usersAndIds, bool sendNotification = false );
   bool           NotifyUserThatHeWasAddedToChannel( const string& userUuid, const string& channelName, const string& channelUuid ); 
   U32            QueryAllUsersInAllChatRooms();

   //void           HandleChatChannelCreateResult( PacketDbQueryResult* dbResult, ChatChannelDbJob& job );
   string         CreateNewRoom( const string& channelName, const string userUuid, U32 serverId, U8 gameType, U32 gameInstanceId );
   bool           DeleteRoom( const string& chanelUuid );
   bool           SendMessageToClient( BasePacket* packet, U32 connectionId, U32 gatewayId ) const;
   
   bool           LoadSingleRoom( const string& channelUuid );

   bool           HandleLoadAllRoomsResult( PacketDbQueryResult* dbResult, ChatRoomDbJob& job );
   bool           HandleLoadAllUsersResult( PacketDbQueryResult* dbResult, ChatRoomDbJob& job );
   bool           HandleLoadNewUsersResult( PacketDbQueryResult* dbResult, ChatRoomDbJob& job );
   
   bool           HandleSingleRoomLoad( PacketDbQueryResult* dbResult, ChatRoomDbJob& job );
   void           SaveChatRoomLoadResult( ChatChannelTable::row row );
   void           SaveUserLoadResult( SimpleUserTable::row row );
   bool           HandleAllUsersInAllChannelsResult( PacketDbQueryResult* dbResult, ChatRoomDbJob& job );

   void           StoreUser( const string& userUuid, const string& userName );
   void           StoreUser( const string& userUuid, U32 userId, const string& userName, bool blockContactInvites, bool blockGroupInvites );

   void           AddRoomToUser( const string& userUuid, stringhash channelHash );
   void           DeleteRoomFromUser( const string& userUuid, stringhash channelHash );
   bool           AddUserToRoomAndWriteToDB( const string& channelUuid, const string& addedUserUuid, const string& addedUserName );
   bool           RemoveUserFromRoomAndWriteToDB( const string& channelUuid, const string& removedUserUuid );
   bool           RemoveRoomAndMarkRoomInDB( const string& channelUuid );
   bool           RemoveAnyRelatedInvitationsAndInformUsers( const string& channelUuid );
   
   //-----------------------------------------------------

   // query functions
   bool     RequestAllUsersInChatRoom( const string& channelUuid, bool fullList, const string& authUuid );
   bool     QueryAllUsersInChatRoom( const string& channelUuid );
   void     WriteChatToDb( const string& message, const string& senderUuid, const string& friendUuid, const string& channelUuid, U16 gameTurn, U32 connectionId );
   void     MarkFriendDateLastChat( const string& senderUuid, const string& friendUuid );
   void     MarkChatChannelDateLastChat( const string& channelUuid );

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
   int                           m_numUsersToLoadPerQueryForInitialLoad;
   int                           m_offsetIndex_QueryForInitialLoad;
   bool                          m_isPullingAllUsersAndChannels;
   
   int                           m_numChannelChatsSent;
   int                           m_numP2PChatsSent;
   int                           m_numChangesToChatRoom;
   U32                           m_dbIdentifier;

   static InvitationManager*     m_invitationServer;
   time_t                        m_timestampSearchNewUserAccounts;
   static const U32              timeoutSearchNewUserAccounts = 30;
};

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
