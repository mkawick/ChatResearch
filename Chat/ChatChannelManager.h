#pragma once

#include "../NetworkCommon/DataTypes.h"
#include "../NetworkCommon/ChainedArchitecture/ChainedInterface.h"

#include <set>

class UserConnection;
class PacketDbQueryResult;
//////////////////////////////////////////////////////////////////////////////////

struct ChatUser 
{
   string            username;
   string            userUuid;
   UserConnection*   connection;
   list< stringhash > channels;
   bool              isOnline;
};

struct ChatChannel
{
   ChatChannel() : gameTurn( 0 ) {}

   string   name;
   string   uuid;

   vector< ChatUser >   admins;
   list< stringhash >   userUuidList;
   U16         gameTurn;
};

struct ChatChannelDbJob
{
   enum JobType
   {
      JobType_Create,
      JobType_Delete,
      JobType_LoadAllChannels,
      JobType_SelectAllChannelsForAUser,
      JobType_Exists,
      JobType_AddUser,
      JobType_RemoveUser,
      JobType_SelectAllChannelsToSendToAuth,
      JobType_SelectAllUsersToSendToAuth,
      JobType_FindChatters,
      JobType_AllUsersInChannel
   };

   string      name;
   string      uuid;
   stringhash  chatUserLookup;
   stringhash  authUserLookup;
   int         jobId;
   U32         serverId;
   JobType     jobType;
};

//--------------------------------------

class UserConnection;
class BasePacket;
class DiplodocusChat;
class PacketChatCreateChatChannelFromGameServer;
class PacketChatAddUserToChatChannelGameServer;
class PacketChatRemoveUserFromChatChannelGameServer;

//////////////////////////////////////////////////////////////////////////////////

class ChatChannelManager : public ChainedInterface< BasePacket* >
{
private:
   enum UserStatusChange
   {
      UserStatusChange_Login,
      UserStatusChange_Logout,
      UserStatusChange_UserAddedToChannel,
      UserStatusChange_UserRemovedFromChannel,
   };
public:
   ChatChannelManager();
   ~ChatChannelManager();

   static void       SetDiplodocusChat( DiplodocusChat* chat ) { m_chatServer = chat; }
   //void              SetConnectionId( U32 connectionId ) { m_connectionId = connectionId; }
   //U32               GetConnectionId() const { return m_connectionId; }

   void     UserLoggedIn( const string& username, const string& userUuid, UserConnection* connection );
   void     UserLoggedOut( const string& username, const string& userUuid, UserConnection* connection );

   void     UserSendsChatToChannel( const string& userUuid, const string& channelUuid, const string& message ); 
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

   void     RequestChatChannelList( const string& authUuid, bool isFullList );
   void     RequestUsersList( const string& authUuid, bool isFullList );
   bool     RequestChatters( const string& channelUuid, const string& authUuid );
   bool     RequestAllUsersInChatChannel( const string& channelUuid, bool fullList, const string& authUuid );

   // other servers requests
   bool     CreateNewChannel( const PacketChatCreateChatChannelFromGameServer* pPacket );
   bool     DeleteChannel( const string& channelName, const U32 serverId );
   
   bool     AddUserToChannel( const PacketChatAddUserToChatChannelGameServer* pPacket );
   bool     RemoveUserFromChannel( const PacketChatRemoveUserFromChatChannelGameServer* pPacket );
   bool     AdvanceGameTurn( const string& channelUuid, U32 serverId );
   void     RequestChatChannelList( U32 serverId );

   bool     MoveChannelToDepricated( const string& channelUuid, const string& userUuid, const U32 serverId );
   bool     GetListOfGameBasedchannels( const U32 serverId );


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
   bool              AddChatChannel( const string& channelUuid, const string& channelName );
   void              RemoveUserFromChannelInternal( stringhash hashedUserUuid, stringhash channelUuid, const string& userUuide, UserUuidSet& otherUsersToNotify );
   bool              RemoveChatChannel( const string& channelUuid );
   void              WriteChatToDb( const string& message, const string& senderUuid, const string& friendUuid, const string& channelUuid, U16 gameTurn, U32 connectionId );
   bool              ProcessPacket( BasePacket* packet );

   void              InformUsersAboutUserStatusChange( stringhash userHashLookup, const UserUuidSet& otherUsersToNotify, UserStatusChange status );

   bool              FindDbJob( int jobId, DbJobList& listOfJobs, DbJobIterator& iter );
   bool              FinishJob( PacketDbQueryResult* result, ChatChannelDbJob& job );
   void              DbQueryAndPacket( const string& channelName, const string& channelUuid, U32 serverId, const string& authUuid, stringhash chatUserLookup, const string& queryString, ChatChannelDbJob::JobType jobType, bool isFandF );

   //-------------------------------------------------------

   //U32                              m_connectionId;
   ChannelMap                       m_channelMap;
   UserUuidMap                      m_userUuidMap;// a quick reverse lookup for when a user logs out or sends a chat.
   deque< stringhash >              m_channelsNeedingQueryInfo;

   deque< BasePacket* >             m_packetsIn;

   static DiplodocusChat*           m_chatServer;
   

   int               AddDbJob( const string& channelName, const string& channelUuid, U32 serverId, stringhash chatUserLookup, stringhash authUserLookup, ChatChannelDbJob::JobType type );
   bool              CreateAndSendUserListJob( const string& queryString, const string& channelName, const string& channelUuid, U32 serverId, const string& requestorUuid, stringhash chatUserLookup, ChatChannelDbJob::JobType type );
   int                              m_dbJobLookupId;
   DbJobList                        m_pendingDbJobs;
   
};

//////////////////////////////////////////////////////////////////////////////////