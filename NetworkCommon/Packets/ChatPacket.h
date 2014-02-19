// ChatPacket.h
#pragma once

class PacketChatToServer : public BasePacket 
{
public:
   enum ChatType
   {
      ChatType_ChatToServer,
      ChatType_ChatToClient,
      ChatType_EchoToServer,
      ChatType_EchoToClient,

      ChatType_ChangeChatChannel,
      ChatType_ChangeChatChannelToClient,
      ChatType_SendListOfChannelsToClient,
      ChatType_UserAddedToChatChannelFromGameServer,

      ChatType_UserChatStatusChange,// sent to each member of each group of which a user logs in

      ChatType_RequestHistory,
      ChatType_RequestHistoryResult,
      ChatType_RequestHistorySinceLastLogin,
      ChatType_RequestHistorySinceLastLoginResponse,

      ChatType_CreateChatChannel,
      ChatType_CreateChatChannelResponse,
      ChatType_CreateChatChannelFromGameServer,
      ChatType_CreateChatChannelFromGameServerResponse,

      ChatType_DeleteChatChannel,
      ChatType_DeleteChatChannelResponse,
      ChatType_DeleteChatChannelFromGameServer,
      ChatType_DeleteChatChannelFromGameServerResponse,
      ChatType_InviteUserToChatChannel,
      ChatType_InviteUserToChatChannelResponse,
      
      ChatType_AddUserToChatChannel,
      ChatType_AddUserToChatChannelResponse,
      ChatType_AddUserToChatChannelGameServer,
      ChatType_AddUserToChatChannelGameServerResponse,

      ChatType_RemoveUserFromChatChannel,
      ChatType_RemoveUserFromChatChannelResponse,
      ChatType_RemoveUserFromChatChannelGameServer,
      ChatType_RemoveUserFromChatChannelGameServerResponse,
      
      ChatType_RequestChatters,
      ChatType_RequestChattersResponse,
      ChatType_EnableDisableFiltering,
      ChatType_EnableDisableFilteringResponse,
      
      ChatType_ListAllMembersInChatChannel,
      ChatType_ListAllMembersInChatChannelResponse,// groupUuid, groupName, {uuid, name, isLoggedIn}

      ChatType_AdminLoadAllChannels,
      ChatType_AdminLoadAllChannelsResponse,
      ChatType_AdminRequestChatChannelList,
      ChatType_AdminRequestChatChannelListResponse,
      ChatType_AdminRequestUsersList,
      ChatType_AdminRequestUsersListResponse,

      ChatType_S2SSendNotificationToUser,
      ChatType_SendNotificationToUser,

      ChatType_Friend
   };
public:
   PacketChatToServer( int packet_type = PacketType_Chat, int packet_sub_type = ChatType_ChatToServer ) : BasePacket( packet_type, packet_sub_type ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   message;
   string   channelUuid;
   string   userUuid;
   U16      gameTurn;
};

///////////////////////////////////////////////////////////////
/*
class PacketChangeChatChannel : public PacketChatToServer
{
public:
   PacketChangeChatChannel( int packet_type = PacketType_Chat, int packet_sub_type = ChatType_ChangeChatChannel ) : PacketChatToServer( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   chatChannelUuid;
};

///////////////////////////////////////////////////////////////

class PacketChangeChatChannelToClient : public BasePacket
{
public:
   PacketChangeChatChannelToClient( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_ChangeChatChannelToClient ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   userName;
   string   chatChannel;
};*/

///////////////////////////////////////////////////////////////

class PacketChatToClient : public PacketChatToServer 
{
public:
   PacketChatToClient( int packet_type = PacketType_Chat, int packet_sub_type = ChatType_ChatToClient ) : PacketChatToServer( packet_type, packet_sub_type ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   userName;
   string   timeStamp;
};

///////////////////////////////////////////////////////////////

class PacketChat_EchoToServer : public BasePacket
{
public:
   PacketChat_EchoToServer(): BasePacket( PacketType_Chat, PacketChatToServer::ChatType_EchoToServer ) {}
};

///////////////////////////////////////////////////////////////

class PacketChat_EchoToClient : public BasePacket
{
public:
   PacketChat_EchoToClient(): BasePacket( PacketType_Chat, PacketChatToServer::ChatType_EchoToClient ) {}
};

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

class PacketChatUserStatusChangeBase : public BasePacket 
{
public:
   PacketChatUserStatusChangeBase( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_UserChatStatusChange ) : BasePacket( packet_type, packet_sub_type ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   U32      statusChange;
   string   uuid;
   string   userName;
   string   chatChannelUuid;
};

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

class PacketChatChannelListToClient : public PacketChatToServer 
{
public:
   PacketChatChannelListToClient( int packet_type = PacketType_Chat, int packet_sub_type = ChatType_SendListOfChannelsToClient ) : PacketChatToServer( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   vector< string >  chatChannel;
};


///////////////////////////////////////////////////////////////

class PacketChatUserAddedToChatChannelFromGameServer : public BasePacket
{
public:
   PacketChatUserAddedToChatChannelFromGameServer( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_UserAddedToChatChannelFromGameServer ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   gameName;
   U32      gameId;
   string   channelUuid;
   SerializedKeyValueVector< string >   userList;
};


///////////////////////////////////////////////////////////////

class PacketChatHistoryRequest : public BasePacket
{
public:
   PacketChatHistoryRequest( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_RequestHistory ) : BasePacket( packet_type, packet_sub_type ), numRecords( 20 ), startingIndex( 0 ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   chatChannelUuid;
   string   userUuid;
   int      numRecords;
   int      startingIndex;
};

///////////////////////////////////////////////////////////////

struct ChatEntry
{
   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   userName;
   string   useruuid;
   string   message;
   string   timestamp;
   U16      gameTurn;
};

class PacketChatHistoryResult : public BasePacket
{
public:
   PacketChatHistoryResult( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_RequestHistoryResult ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   chatChannelUuid;
   string   userUuid;
   SerializedVector< ChatEntry > chat;
};

///////////////////////////////////////////////////////////////

class PacketChatMissedHistoryRequest : public BasePacket
{
public:
   PacketChatMissedHistoryRequest( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_RequestHistorySinceLastLogin ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset ) { return BasePacket::SerializeIn( data, bufferOffset ); }
   bool  SerializeOut( U8* data, int& bufferOffset ) const { return BasePacket::SerializeOut( data, bufferOffset ); }
};

///////////////////////////////////////////////////////////////
/*
struct FullChatChannelEntry
{
   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   message;
   string   senderUuid;
   string   chatChannelUuid;
   string   timeStamp;
   U16      gameTurn;
   
};

class PacketChatMissedHistoryResult : public BasePacket
{
public:
   PacketChatMissedHistoryResult( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_RequestHistorySinceLastLoginResponse ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   SerializedVector< FullChatChannelEntry > history;
};*/


struct MissedChatChannelEntry
{
   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   senderUuid;
   string   chatChannelUuid;
   U16      numMessages;
   bool     isGamechannel;
};

class PacketChatMissedHistoryResult : public BasePacket
{
public:
   PacketChatMissedHistoryResult( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_RequestHistorySinceLastLoginResponse ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   SerializedVector< MissedChatChannelEntry > history;
};

///////////////////////////////////////////////////////////////

class PacketChatCreateChatChannel : public BasePacket
{
public:
   PacketChatCreateChatChannel( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_CreateChatChannel ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   name;
   // rules?
};

///////////////////////////////////////////////////////////////

class PacketChatCreateChatChannelResponse : public BasePacket
{
public:
   PacketChatCreateChatChannelResponse( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_CreateChatChannelResponse ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   name;
   string   uuid;
   bool     successfullyCreated;
};


///////////////////////////////////////////////////////////////

class PacketChatCreateChatChannelFromGameServer : public BasePacket
{
public:
   PacketChatCreateChatChannelFromGameServer( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_CreateChatChannelFromGameServer ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string         gameName;
   U32            gameId;
   StringBucket   userUuidList;
};


///////////////////////////////////////////////////////////////

class PacketChatCreateChatChannelFromGameServerResponse : public BasePacket
{
public:
   PacketChatCreateChatChannelFromGameServerResponse( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_CreateChatChannelFromGameServerResponse ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   U32            gameId;
   string         channelUuid;
   bool           success;
};

///////////////////////////////////////////////////////////////

class PacketChatDeleteChatChannel : public BasePacket
{
public:
   PacketChatDeleteChatChannel( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_DeleteChatChannel ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   uuid;
};

///////////////////////////////////////////////////////////////

class PacketChatDeleteChatChannelResponse : public BasePacket
{
public:
   PacketChatDeleteChatChannelResponse( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_DeleteChatChannelResponse ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   uuid;
   bool     successfullyDeleted;
   int      numUsersRemoved;
};

///////////////////////////////////////////////////////////////

class PacketChatDeleteChatChannelFromGameServer : public BasePacket
{
public:
   PacketChatDeleteChatChannelFromGameServer( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_DeleteChatChannelFromGameServer ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   //string   uuid;
   U32            gameId;
   string         gameName;
};

///////////////////////////////////////////////////////////////

class PacketChatDeleteChatChannelFromGameServerResponse : public BasePacket
{
public:
   PacketChatDeleteChatChannelFromGameServerResponse( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_DeleteChatChannelFromGameServerResponse ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   U32            gameId;
   string         gameName;
   bool           successfullyDeleted;
   int            numUsersRemoved;
};

///////////////////////////////////////////////////////////////

class PacketChatInviteUserToChatChannel : public BasePacket
{
public:
   PacketChatInviteUserToChatChannel( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_InviteUserToChatChannel ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   chatChannelUuid;
   string   userUuid;
};

///////////////////////////////////////////////////////////////

class PacketChatInviteUserToChatChannelResponse : public BasePacket
{
public:
   PacketChatInviteUserToChatChannelResponse( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_InviteUserToChatChannelResponse ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   chatChannelUuid;
   string   userUuid;
   bool     success;
};

///////////////////////////////////////////////////////////////

class PacketChatAddUserToChatChannel : public BasePacket
{
public:
   PacketChatAddUserToChatChannel( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_AddUserToChatChannel ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   chatChannelUuid;
   string   userUuid;
};

///////////////////////////////////////////////////////////////

class PacketChatAddUserToChatChannelResponse : public BasePacket
{
public:
   PacketChatAddUserToChatChannelResponse( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_AddUserToChatChannelResponse ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   chatChannelUuid;
   string   userUuid;
   bool     success;
};

///////////////////////////////////////////////////////////////

class PacketChatAddUserToChatChannelGameServer : public BasePacket
{
public:
   PacketChatAddUserToChatChannelGameServer( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_AddUserToChatChannelGameServer ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   gameName;
   U32      gameId;
   string   userUuid;
};

///////////////////////////////////////////////////////////////

class PacketChatAddUserToChatChannelGameServerResponse : public BasePacket
{
public:
   PacketChatAddUserToChatChannelGameServerResponse( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_AddUserToChatChannelGameServerResponse ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   chatChannelUuid;
   string   userUuid;
   U32      gameId;
   bool     success;
};

///////////////////////////////////////////////////////////////

class PacketChatRemoveUserFromChatChannel : public BasePacket
{
public:
   PacketChatRemoveUserFromChatChannel( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_RemoveUserFromChatChannel ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   chatChannelUuid;
   string   userUuid;
};

///////////////////////////////////////////////////////////////

class PacketChatRemoveUserFromChatChannelResponse : public BasePacket
{
public:
   PacketChatRemoveUserFromChatChannelResponse( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_RemoveUserFromChatChannelResponse ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   chatChannelUuid;
   string   userUuid;
   bool     success;
};

///////////////////////////////////////////////////////////////

class PacketChatRemoveUserFromChatChannelGameServer : public BasePacket
{
public:
   PacketChatRemoveUserFromChatChannelGameServer( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_RemoveUserFromChatChannelGameServer ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   gameName;
   U32      gameId;
   string   userUuid;
};

///////////////////////////////////////////////////////////////

class PacketChatRemoveUserFromChatChannelGameServerResponse : public BasePacket
{
public:
   PacketChatRemoveUserFromChatChannelGameServerResponse( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_RemoveUserFromChatChannelGameServerResponse ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   chatChannelUuid;
   string   userUuid;
   U32      gameId;
   bool     success;
};

///////////////////////////////////////////////////////////////

class PacketChatAdminRequestChatChannelList : public BasePacket
{
public:
   PacketChatAdminRequestChatChannelList( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_AdminRequestChatChannelList ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   bool  isFullList; // if true, we go to the db, otherwise, we return the loaded list
};

///////////////////////////////////////////////////////////////

class PacketChatAdminRequestChatChannelListResponse : public BasePacket
{
public:
   PacketChatAdminRequestChatChannelListResponse( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_AdminRequestChatChannelListResponse ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   //SerializedKeyValueVector< string >   chatChannels; // group details?
   SerializedKeyValueVector< ChannelInfo >  chatChannels;
};

///////////////////////////////////////////////////////////////

class PacketChatAdminRequestUsersList : public BasePacket
{
public:
   PacketChatAdminRequestUsersList( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_AdminRequestUsersList ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   bool  isFullList; // if true, we go to the db, otherwise, we return the loaded list
};

///////////////////////////////////////////////////////////////

class PacketChatAdminRequestUsersListResponse : public BasePacket
{
public:
   PacketChatAdminRequestUsersListResponse( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_AdminRequestUsersListResponse ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   SerializedKeyValueVector< string >   users; // user details?
};

///////////////////////////////////////////////////////////////

class PacketChatRequestChatters : public BasePacket
{
public:
   PacketChatRequestChatters( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_RequestChatters ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string                     chatChannelUuid;
};

///////////////////////////////////////////////////////////////

class PacketChatRequestChattersResponse : public BasePacket
{
public:
   PacketChatRequestChattersResponse( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_RequestChattersResponse ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string                     chatChannelUuid;
   SerializedKeyValueVector< string >   userList;
};

///////////////////////////////////////////////////////////////

class PacketChatEnableFiltering : public BasePacket
{
public:
   PacketChatEnableFiltering( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_EnableDisableFiltering ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   bool  isEnabled;
};

///////////////////////////////////////////////////////////////

class PacketChatEnableFilteringResponse : public BasePacket
{
public:
   PacketChatEnableFilteringResponse( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_EnableDisableFilteringResponse ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   bool  isEnabled;
   bool  success;
};

///////////////////////////////////////////////////////////////

class PacketChatListAllMembersInChatChannel : public BasePacket
{
public:
   PacketChatListAllMembersInChatChannel( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_ListAllMembersInChatChannel ) : BasePacket( packet_type, packet_sub_type ), fullList( false ) {  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   bool        fullList;
   string      chatChannelUuid;
};

///////////////////////////////////////////////////////////////

class PacketChatListAllMembersInChatChannelResponse : public BasePacket
{
public:
   PacketChatListAllMembersInChatChannelResponse( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_ListAllMembersInChatChannelResponse ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string                     chatChannelUuid;
   SerializedKeyValueVector< string >   userList;
};

///////////////////////////////////////////////////////////////

class PacketChatAdminLoadAllChannels : public BasePacket
{
public:
   PacketChatAdminLoadAllChannels( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_AdminLoadAllChannels ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset ) { return BasePacket::SerializeIn( data, bufferOffset ); }
   bool  SerializeOut( U8* data, int& bufferOffset ) const { return BasePacket::SerializeOut( data, bufferOffset ); }
};

///////////////////////////////////////////////////////////////

class PacketChatAdminLoadAllChannelsResponse : public BasePacket
{
public:
   PacketChatAdminLoadAllChannelsResponse( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_AdminLoadAllChannelsResponse ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   bool  success;
};


///////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
/*
class PacketChatFriend : public BasePacket // can both directions
{
public:
   PacketChatFriend() : BasePacket( PacketType_Chat, PacketChatToServer::ChatType_Friend ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   userName;
   string   uuid;
};

///////////////////////////////////////////////////////////////

class PacketFriendsList : public BasePacket // can both directions
{
public:
   PacketFriendsList() : BasePacket( PacketType_FriendsList ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   int                  numFriends;
   PacketChatFriend*    listOfUsers;
};*/