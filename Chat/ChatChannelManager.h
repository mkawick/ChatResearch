#pragma once

#include "../NetworkCommon/DataTypes.h"
#include "../NetworkCommon/ChainedArchitecture/ChainedInterface.h"
//#include "../NetworkCommon/ChainedArchitecture/Thread.h"

#include <set>

class UserConnection;
class PacketDbQueryResult;
//////////////////////////////////////////////////////////////////////////////////

struct ChatUser 
{
   string            userName;
   string            userUuid;
   UserConnection*   connection;
   list< stringhash > channels;
   bool              isOnline;
};

struct UserBasics
{
   UserBasics( const string& name, const string& uuid ) : userName( name ), userUuid( uuid ) {}
   string            userName;
   string            userUuid;
};

struct ChatChannel
{
   ChatChannel() : gameTurn( 0 ) {}

   int      recordId;
   string   name;
   string   uuid;
   bool     isActive;
   int      maxPlayers;
   string   channelDetails;
   U8       gameType;
   U32      gameId;
   string   createDate;

   vector< ChatUser >   admins;
   list< stringhash >   userUuidList;
   list< UserBasics >   userBasics;
   U16         gameTurn;
};

struct ChatChannelDbJob
{
   enum JobType
   {
      JobType_Create,
      JobType_Delete,
      JobType_LoadSingleChannel,
      JobType_LoadAllChannels,
      //JobType_LoadAllUsersInChannel,
      JobType_SelectAllChannelsForAUser,
      JobType_Exists,
      JobType_AddUser,
      JobType_RemoveUser,
      JobType_SelectAllChannelsToSendToAuth,
      JobType_SelectAllUsersToSendToAuth,
      JobType_FindChatters,
      JobType_AllUsersInChannel,

      JobType_CreateFromGameServer,
      JobType_MakeInactiveFromGameServer,
      JobType_AddUserFromGameServer,
      JobType_RemoveUserFromGameServer
   };

   string         name;
   string         uuid;
   stringhash     chatUserLookup;
   stringhash     authUserLookup;
   int            jobId;
   U32            serverId;
   JobType        jobType;
   StringBucket   stringBucket;
};

//--------------------------------------

class UserConnection;
class BasePacket;
class DiplodocusChat;
class PacketChatCreateChatChannelFromGameServer;
class PacketChatDeleteChatChannelFromGameServer;
class PacketChatAddUserToChatChannelGameServer;
class PacketChatRemoveUserFromChatChannelGameServer;

//////////////////////////////////////////////////////////////////////////////////

class ChatChannelManager : public ChainedInterface< BasePacket* >
{
private:
   enum UserStatusChange
   {
      UserStatusChange_Logout = 0,
      UserStatusChange_Login,      
      UserStatusChange_UserAddedToChannel,
      UserStatusChange_UserRemovedFromChannel,
   };
public:
   ChatChannelManager();
   ~ChatChannelManager();

   void  Init();

   static void       SetDiplodocusChat( DiplodocusChat* chat ) { m_chatServer = chat; }
   //void              SetConnectionId( U32 connectionId ) { m_connectionId = connectionId; }
   //U32               GetConnectionId() const { return m_connectionId; }

   void     UserLoggedIn( const string& username, const string& userUuid, UserConnection* connection );
   void     UserLoggedOut( const string& username, const string& userUuid, UserConnection* connection );

   void     UserSendsChatToChannel( const string& userUuid, const string& channelUuid, const string& message, U16 gameTurn ); 
   void     UserSendsChatToUser( const string& userUuid, const string& destinationUuid, const string& message ); 

   // user-based authenticated methods
   bool     CreateNewChannel( const string& channelName, const string& authUuid ); // permissions
   bool     DeleteChannel( const string& channelName, const string& authUuid );
   
   bool     InviteUserToChannel( const string& channelUuid, const string& userUuid, const string& authUuid ); // permissions
   //bool     AcceptInvitationToChannel( const string& channelUuid, const string& userUuid, const string& authUuid ); // permissions
   bool     AddUserToChannel( const string& channelUuid, const string& userUuid, const string& authUuid ); // permissions
   bool     RemoveUserFromChannel( const string& channelUuid, const string& userUuid, const string& authUuid ); // permissions

   string   FindChannel( const string& channelName ) const; 
   void     LoadAllChannels( const string& authUuid );
   void     LoadSingleChannel( string uuid, int gameId, int chatChannelId );

   void     RequestChatChannelList( const string& authUuid, bool isFullList );
   void     RequestUsersList( const string& authUuid, bool isFullList );
   bool     RequestChatters( const string& channelUuid, const string& authUuid );
   bool     RequestAllUsersInChatChannel( const string& channelUuid, bool fullList, const string& authUuid );

   // other servers requests
   bool     CreateNewChannel( const PacketChatCreateChatChannelFromGameServer* pPacket );
   bool     DeleteChannel( const PacketChatDeleteChatChannelFromGameServer* pPacket );
   
   bool     DeleteChannel( const string& channelName, const U32 serverId );
   
   bool     AddUserToChannel( const PacketChatAddUserToChatChannelGameServer* pPacket );
   bool     RemoveUserFromChannel( const PacketChatRemoveUserFromChatChannelGameServer* pPacket );
   bool     AdvanceGameTurn( const string& channelUuid, U32 serverId );
   void     RequestChatChannelList( U32 serverId );

   bool     MoveChannelToDepricated( const string& channelUuid, const string& userUuid, const U32 serverId );
   bool     GetListOfGameBasedchannels( const U32 serverId );

   bool     GetChatChannels( const string& uuid, ChannelKeyValue& listOfChannels );


   //-------------------------------------------------------

   bool              AddInputChainData( BasePacket* packet );// usually a query
   void              Update();

protected:

   void  PackageAndSendToDiplodocusChat( BasePacket* packet, U32 serverId );

   //-------------------------------------------------------

   typedef map< stringhash, ChatChannel >    ChannelMap;
   typedef ChannelMap::iterator              ChannelMapIterator;
   typedef pair< stringhash, ChatChannel >   ChannelMapPair;

   typedef map< stringhash, ChatUser >       UserUuidMap;
   typedef UserUuidMap::iterator             UserUuidMapIterator;
   typedef pair< stringhash, ChatUser >      UserUuidPair;

   typedef std::set< stringhash >            UserUuidSet;

   typedef list< ChatChannelDbJob >          DbJobList;
   typedef DbJobList::iterator               DbJobIterator;

   void              AddUserToChannelInternal( stringhash hashedUserUuid, const string& channelUuid, const string& channelName, UserUuidSet& otherUsersToNotify );
   void              AddChatchannel( int id, const string& channelName, const string& channelUuid, bool isActive, int maxPlayers, int gameType, int gameId, const string& createDate );
   void              AddAllUsersToChannel( const string& channelUuid, const SerializedKeyValueVector< string >& usersAndIds );
   void              AddUserToChannel( const string& channelUuid, const UserBasics& ub );
   bool              AddChatChannelToStorage( const string& channelUuid, const string& channelName );
   void              AssembleListOfUsersToNotifyForAllChannelsForUser( stringhash userUuidHash, stringhash channelUuid, const string& userUuid, UserUuidSet& otherUsersToNotify );
   void              RemoveUserFromChannelInternal( stringhash hashedUserUuid, stringhash channelUuid, const string& userUuide, UserUuidSet& otherUsersToNotify );
   bool              RemoveChatChannel( const string& channelUuid );
   void              WriteChatToDb( const string& message, const string& senderUuid, const string& friendUuid, const string& channelUuid, U16 gameTurn, U32 connectionId );
   bool              ProcessPacket( BasePacket* packet );

   void              InformUsersAboutUserStatusChange( stringhash userHashLookup, const UserUuidSet& otherUsersToNotify, UserStatusChange status );

   bool              FindDbJob( int jobId, DbJobList& listOfJobs, DbJobIterator& iter );
   bool              FinishJob( PacketDbQueryResult* result, ChatChannelDbJob& job );
   int               DbQueryAndPacket( const string& channelName, const string& channelUuid, U32 serverId, const string& authUuid, stringhash chatUserLookup, const string& queryString, ChatChannelDbJob::JobType jobType, bool isFandF, list< string >* sanitizedStrings = NULL );

   bool              FinishAddingChatChannelFromGameServer( PacketDbQueryResult* dbResult, ChatChannelDbJob& job );
   bool              FinishAddingAddingUserToChatchannelFromGameServer( PacketDbQueryResult* dbResult, ChatChannelDbJob& job );
   bool              FinishAddingRemovingUserFromChatchannelFromGameServer( PacketDbQueryResult* dbResult, ChatChannelDbJob& job );
   //-------------------------------------------------------

   //U32                              m_connectionId;
   ChannelMap                       m_channelMap;
   UserUuidMap                      m_userUuidMap;// a quick reverse lookup for when a user logs out or sends a chat.
   deque< stringhash >              m_channelsNeedingQueryInfo;

   deque< BasePacket* >             m_packetsIn;

   static DiplodocusChat*           m_chatServer;
   Threading::Mutex                 m_mutex, m_jobMutex;
   

   int               AddDbJob( const string& channelName, const string& channelUuid, U32 serverId, stringhash chatUserLookup, stringhash authUserLookup, ChatChannelDbJob::JobType type );
   bool              CreateAndSendUserListJob( const string& queryString, const string& channelName, const string& channelUuid, U32 serverId, const string& requestorUuid, stringhash chatUserLookup, ChatChannelDbJob::JobType type, list< string >* sanitizedStrings = NULL );
   int                              m_dbJobLookupId;
   DbJobList                        m_pendingDbJobs;
   
};

//////////////////////////////////////////////////////////////////////////////////