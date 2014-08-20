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

      ChatType_RenameChatChannel,
      ChatType_RenameChatChannelResponse,

      ChatType_UpdateProfile,

      ChatType_Friend
   };
public:
   PacketChatToServer( int packet_type = PacketType_Chat, int packet_sub_type = ChatType_ChatToServer ) : BasePacket( packet_type, packet_sub_type ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   BoundedString140 message;
   UuidString     channelUuid;
   UuidString     userUuid;
   U16            gameTurn;
};

///////////////////////////////////////////////////////////////


class ChannelInfo
{
public:
   ChannelInfo() {}
   ChannelInfo( const string& name, const string& uuid, int gameProductId, int _gameId, int _numNewChats, bool active) : 
               channelName( name ), channelUuid( uuid ), gameProduct( gameProductId ), gameId( _gameId ), numNewChats( _numNewChats ), isActive( active ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   BoundedString80   channelName;
   UuidString        channelUuid;
   int               gameProduct;
   int               gameId;
   int               numNewChats;
   bool              isActive;
};

typedef SerializedKeyValueVector< ChannelInfo > ChannelKeyValue;


class ChannelInfoFullList : public ChannelInfo
{
public:
   ChannelInfoFullList() : ChannelInfo() {}
   ChannelInfoFullList( const string& name, const string& uuid, int gameProductId, int _gameId, int _numNewChats, bool active ) : 
   ChannelInfo( name, uuid, gameProductId, _gameId, _numNewChats,  active ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   SerializedKeyValueVector< string >   userList;
};

typedef SerializedKeyValueVector< ChannelInfoFullList > ChannelFullKeyValue;

///////////////////////////////////////////////////////////////

class PacketChatChannelList : public PacketUserInfo
{
public:
   PacketChatChannelList( int packet_type = PacketType_UserInfo, int packet_sub_type = InfoType_ChatChannelList ) : PacketUserInfo( packet_type, packet_sub_type ){  }
   ~PacketChatChannelList();

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   SerializedKeyValueVector< ChannelInfoFullList >  channelList;
};


///////////////////////////////////////////////////////////////
/*
class PacketChangeChatChannel : public PacketChatToServer
{
public:
   PacketChangeChatChannel( int packet_type = PacketType_Chat, int packet_sub_type = ChatType_ChangeChatChannel ) : PacketChatToServer( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   string   chatChannelUuid;
};

///////////////////////////////////////////////////////////////

class PacketChangeChatChannelToClient : public BasePacket
{
public:
   PacketChangeChatChannelToClient( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_ChangeChatChannelToClient ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   string   userName;
   string   chatChannel;
};*/

///////////////////////////////////////////////////////////////

class PacketChatToClient : public PacketChatToServer 
{
public:
   PacketChatToClient( int packet_type = PacketType_Chat, int packet_sub_type = ChatType_ChatToClient ) : 
      PacketChatToServer( packet_type, packet_sub_type ), userTempId( 0 ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   BoundedString80   userName;
   TimeString        timeStamp;
   U32               userTempId;
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

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   U32               statusChange;
   UuidString        uuid;
   BoundedString80   userName;
   UuidString        chatChannelUuid;
};

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

class PacketChatChannelListToClient : public PacketChatToServer 
{
public:
   PacketChatChannelListToClient( int packet_type = PacketType_Chat, int packet_sub_type = ChatType_SendListOfChannelsToClient ) : PacketChatToServer( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   vector< string >  chatChannel;
};


///////////////////////////////////////////////////////////////

class PacketChatUserAddedToChatChannelFromGameServer : public BasePacket
{
public:
   PacketChatUserAddedToChatChannelFromGameServer( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_UserAddedToChatChannelFromGameServer ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   BoundedString80         gameName;
   U32                     gameId;
   UuidString              channelUuid;
   SerializedKeyValueVector< string >   userList;
};


///////////////////////////////////////////////////////////////

class PacketChatHistoryRequest : public BasePacket
{
public:
   PacketChatHistoryRequest( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_RequestHistory ) : BasePacket( packet_type, packet_sub_type ), numRecords( 20 ), startingIndex( 0 ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   UuidString     chatChannelUuid;
   UuidString     userUuid;
   FixedString32  startingTimestamp;
   int            numRecords;
   int            startingIndex;
};

///////////////////////////////////////////////////////////////

struct ChatEntry
{
   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   BoundedString80   userName;
   UuidString        useruuid;
   U32               userTempId;
   BoundedString140  message;
   TimeString        timestamp;   
   U16               gameTurn;
};

///////////////////////////////////////////////////////////////

class PacketChatHistoryResult : public BasePacket
{
public:
   PacketChatHistoryResult( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_RequestHistoryResult ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   UuidString  chatChannelUuid;
   UuidString  userUuid;
   string         startingTimestamp;
   int            startingIndex;

   SerializedVector< ChatEntry > chat;
};

///////////////////////////////////////////////////////////////

class PacketChatMissedHistoryRequest : public BasePacket
{
public:
   PacketChatMissedHistoryRequest( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_RequestHistorySinceLastLogin ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion ) { return BasePacket::SerializeIn( data, bufferOffset, minorVersion ); }
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const { return BasePacket::SerializeOut( data, bufferOffset, minorVersion ); }
};

///////////////////////////////////////////////////////////////

struct MissedChatChannelEntry
{
   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   UuidString  senderUuid;
   UuidString  chatChannelUuid;
   string   message;
   //U16      numMessages;
   bool     isGamechannel;
   U32      senderTempId;
};

class PacketChatMissedHistoryResult : public BasePacket
{
public:
   PacketChatMissedHistoryResult( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_RequestHistorySinceLastLoginResponse ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   SerializedVector< MissedChatChannelEntry > history;
};

///////////////////////////////////////////////////////////////

class PacketChatCreateChatChannel : public BasePacket
{
public:
   PacketChatCreateChatChannel( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_CreateChatChannel ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   string   name;
   // rules?
};

///////////////////////////////////////////////////////////////

class PacketChatCreateChatChannelResponse : public BasePacket
{
public:
   PacketChatCreateChatChannelResponse( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_CreateChatChannelResponse ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   string         name;
   UuidString  uuid;
   bool        successfullyCreated;
};


///////////////////////////////////////////////////////////////

class PacketChatCreateChatChannelFromGameServer : public BasePacket
{
public:
   PacketChatCreateChatChannelFromGameServer( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_CreateChatChannelFromGameServer ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   string         gameName;
   U32            gameId;
   StringBucket   userUuidList;
};


///////////////////////////////////////////////////////////////

class PacketChatCreateChatChannelFromGameServerResponse : public BasePacket
{
public:
   PacketChatCreateChatChannelFromGameServerResponse( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_CreateChatChannelFromGameServerResponse ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   U32            gameId;
   UuidString  channelUuid;
   bool           success;
};

///////////////////////////////////////////////////////////////

class PacketChatDeleteChatChannel : public BasePacket
{
public:
   PacketChatDeleteChatChannel( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_DeleteChatChannel ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   UuidString   uuid;
};

///////////////////////////////////////////////////////////////

class PacketChatDeleteChatChannelResponse : public BasePacket
{
public:
   PacketChatDeleteChatChannelResponse( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_DeleteChatChannelResponse ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   UuidString  uuid;
   bool     successfullyDeleted;
   int      numUsersRemoved;
};

///////////////////////////////////////////////////////////////

class PacketChatDeleteChatChannelFromGameServer : public BasePacket
{
public:
   PacketChatDeleteChatChannelFromGameServer( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_DeleteChatChannelFromGameServer ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   //string   uuid;
   U32            gameId;
   string         gameName;
};

///////////////////////////////////////////////////////////////

class PacketChatDeleteChatChannelFromGameServerResponse : public BasePacket
{
public:
   PacketChatDeleteChatChannelFromGameServerResponse( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_DeleteChatChannelFromGameServerResponse ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

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

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   UuidString  chatChannelUuid;
   UuidString  userUuid;
};

///////////////////////////////////////////////////////////////

class PacketChatInviteUserToChatChannelResponse : public BasePacket
{
public:
   PacketChatInviteUserToChatChannelResponse( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_InviteUserToChatChannelResponse ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   UuidString  chatChannelUuid;
   UuidString  userUuid;
   bool        success;
};

///////////////////////////////////////////////////////////////

class PacketChatAddUserToChatChannel : public BasePacket
{
public:
   PacketChatAddUserToChatChannel( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_AddUserToChatChannel ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   UuidString  chatChannelUuid;
   UuidString  userUuid;
};

///////////////////////////////////////////////////////////////

class PacketChatAddUserToChatChannelResponse : public BasePacket
{
public:
   PacketChatAddUserToChatChannelResponse( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_AddUserToChatChannelResponse ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   BoundedString80   channelName; // only valuable sometimes.
   UuidString        channelUuid;
   UuidString        userUuid;
   BoundedString80   userName;
   bool              success;
};

///////////////////////////////////////////////////////////////

class PacketChatAddUserToChatChannelGameServer : public BasePacket
{
public:
   PacketChatAddUserToChatChannelGameServer( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_AddUserToChatChannelGameServer ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   BoundedString80   gameName;
   U32               gameId;
   UuidString        userUuid;
};

///////////////////////////////////////////////////////////////

class PacketChatAddUserToChatChannelGameServerResponse : public BasePacket
{
public:
   PacketChatAddUserToChatChannelGameServerResponse( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_AddUserToChatChannelGameServerResponse ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   UuidString  chatChannelUuid;
   UuidString  userUuid;
   U32            gameId;
   bool           success;
};

///////////////////////////////////////////////////////////////

class PacketChatRemoveUserFromChatChannel : public BasePacket
{
public:
   PacketChatRemoveUserFromChatChannel( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_RemoveUserFromChatChannel ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   UuidString  chatChannelUuid;
   UuidString  userUuid;
};

///////////////////////////////////////////////////////////////

class PacketChatRemoveUserFromChatChannelResponse : public BasePacket
{
public:
   PacketChatRemoveUserFromChatChannelResponse( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_RemoveUserFromChatChannelResponse ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   UuidString  chatChannelUuid;
   UuidString  userUuid;
   bool           success;
};

///////////////////////////////////////////////////////////////

class PacketChatRemoveUserFromChatChannelGameServer : public BasePacket
{
public:
   PacketChatRemoveUserFromChatChannelGameServer( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_RemoveUserFromChatChannelGameServer ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   BoundedString80   gameName;
   U32               gameId;
   UuidString        userUuid;
};

///////////////////////////////////////////////////////////////

class PacketChatRemoveUserFromChatChannelGameServerResponse : public BasePacket
{
public:
   PacketChatRemoveUserFromChatChannelGameServerResponse( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_RemoveUserFromChatChannelGameServerResponse ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   UuidString     chatChannelUuid;
   UuidString     userUuid;
   U32            gameId;
   bool           success;
};

///////////////////////////////////////////////////////////////

class PacketChatAdminRequestChatChannelList : public BasePacket
{
public:
   PacketChatAdminRequestChatChannelList( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_AdminRequestChatChannelList ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   bool  isFullList; // if true, we go to the db, otherwise, we return the loaded list
};

///////////////////////////////////////////////////////////////

class PacketChatAdminRequestChatChannelListResponse : public BasePacket
{
public:
   PacketChatAdminRequestChatChannelListResponse( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_AdminRequestChatChannelListResponse ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   //SerializedKeyValueVector< string >   chatChannels; // group details?
   SerializedKeyValueVector< ChannelInfo >  chatChannels;
};

///////////////////////////////////////////////////////////////

class PacketChatAdminRequestUsersList : public BasePacket
{
public:
   PacketChatAdminRequestUsersList( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_AdminRequestUsersList ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   bool  isFullList; // if true, we go to the db, otherwise, we return the loaded list
};

///////////////////////////////////////////////////////////////

class PacketChatAdminRequestUsersListResponse : public BasePacket
{
public:
   PacketChatAdminRequestUsersListResponse( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_AdminRequestUsersListResponse ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   SerializedKeyValueVector< string >   users; // user details?
};

///////////////////////////////////////////////////////////////

class PacketChatRequestChatters : public BasePacket
{
public:
   PacketChatRequestChatters( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_RequestChatters ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   UuidString  chatChannelUuid;
};

///////////////////////////////////////////////////////////////

class PacketChatRequestChattersResponse : public BasePacket
{
public:
   PacketChatRequestChattersResponse( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_RequestChattersResponse ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   UuidString                       chatChannelUuid;
   SerializedKeyValueVector< string >  userList;
};

///////////////////////////////////////////////////////////////

class PacketChatEnableFiltering : public BasePacket
{
public:
   PacketChatEnableFiltering( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_EnableDisableFiltering ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   bool  isEnabled;
};

///////////////////////////////////////////////////////////////

class PacketChatEnableFilteringResponse : public BasePacket
{
public:
   PacketChatEnableFilteringResponse( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_EnableDisableFilteringResponse ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   bool  isEnabled;
   bool  success;
};

///////////////////////////////////////////////////////////////

class PacketChatListAllMembersInChatChannel : public BasePacket
{
public:
   PacketChatListAllMembersInChatChannel( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_ListAllMembersInChatChannel ) : BasePacket( packet_type, packet_sub_type ) {  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   UuidString  chatChannelUuid;
};

///////////////////////////////////////////////////////////////

class PacketChatListAllMembersInChatChannelResponse : public BasePacket
{
public:
   PacketChatListAllMembersInChatChannelResponse( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_ListAllMembersInChatChannelResponse ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   UuidString                       chatChannelUuid;
   SerializedKeyValueVector< string >  userList;
};

///////////////////////////////////////////////////////////////

class PacketChatAdminLoadAllChannels : public BasePacket
{
public:
   PacketChatAdminLoadAllChannels( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_AdminLoadAllChannels ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion ) { return BasePacket::SerializeIn( data, bufferOffset, minorVersion ); }
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const { return BasePacket::SerializeOut( data, bufferOffset, minorVersion ); }
};

///////////////////////////////////////////////////////////////

class PacketChatAdminLoadAllChannelsResponse : public BasePacket
{
public:
   PacketChatAdminLoadAllChannelsResponse( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_AdminLoadAllChannelsResponse ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   bool  success;
};

///////////////////////////////////////////////////////////////

class PacketChatRenameChannel : public BasePacket
{
public:
   PacketChatRenameChannel( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_RenameChatChannel ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;
   
   UuidString        channelUuid;
   BoundedString80   newName;
};

///////////////////////////////////////////////////////////////

class PacketChatRenameChannelResponse : public BasePacket
{
public:
   PacketChatRenameChannelResponse( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_RenameChatChannelResponse ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   bool              success;
   UuidString        channelUuid;
   BoundedString80   newName;
};

///////////////////////////////////////////////////////////////

class PacketChat_UserProfileChange : public BasePacket
{
public:
   PacketChat_UserProfileChange( int packet_type = PacketType_Chat, int packet_sub_type = PacketChatToServer::ChatType_UpdateProfile ) : BasePacket( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   bool     blockChannelInvites;
};

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

