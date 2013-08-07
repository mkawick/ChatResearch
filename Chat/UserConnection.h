// UserConnection.h

#pragma once

// todo, make the includes correct

class DiplodocusChat;// foward decl
class ChatChannelManager;



///////////////////////////////////////////////////////////////////

class UserConnection
{
   enum QueryType // move to deltadrmoeus?
   {
      QueryType_UserLoginInfo = 1,
      QueryType_UserFriendsList,
      QueryType_UserChannelList,
      QueryType_ChatLog,
      QueryType_ChatChannelHistory,
      QueryType_ChatP2PHistory,
      QueryType_ChatHistoryMissedSinceLastLogin
   };
public:
   typedef SerializedKeyValueVector< ChannelInfo > ChannelKeyValue;

public:
   UserConnection( U32 connectionId );

   static void       SetChatManager( ChatChannelManager* manager ) { m_chatChannelManager = manager; }
   static void       SetDiplodocusChat( DiplodocusChat* chat ) { m_chatServer = chat; }


   U32               GetConnectionId() const { return m_connectionId; }
   const string&     GetName() const { return m_username; }
   const string&     GetUuid() const { return m_uuid; }
   const string&     GetLoginKey() const { return m_loginKey; }

   void              SetupFromLogin( U32 userId, const string& name, const string& uuid, const string& loginKey, const string& lastLoginTime );

   const string&     GetCurrentChatGroup() const { return m_currentChannel; }
   //void              SetCurrentChatGroup( const string& channel );
   bool              SetChatChannel( const string& groupUuid );

   const ChannelKeyValue& GetGroups() const { return availableChannels; }
   const KeyValueVector& GetFriends() const { return availableFriends; }

   //-------------------------------------------------------------------------

   bool              SendChat( const string& message, const string& senderUuid, const string& senderDisplayName, string groupUuid );

   //-------------------------------------------------------------------------

   bool              AddInputChainData( BasePacket* packet, U32 connectionId ) ;
   
   void              CheckForProperChatServerPointer();
   void              UpdateInwardPacketList();
   void              UpdateOutwardPacketList();

   void              Update();

   bool              HandleInvalidSetup();
   bool              IsLoggedIn() const;

   //-------------------------------------------------------------------------

   bool              NotifyChannelAdded( const string& channelName, const string& channelUuid, bool wasSuccessful );
   bool              NotifyChannelMovedToInactive( const string& channelUuid, int numremoved, bool wasSuccessful );
   bool              NotifyChannelRemoved( const string& channelUuid, int numremoved );
   bool              NotifyAllChannelsLoaded( bool loaded );
   bool              NotifyUserStatusHasChanged( const string& userName, const string& userUuid, int statusChange );

   bool              NotifyAddedToChannel( const string& channelUuid, const string& userUuid, bool wasSuccessful );
   bool              NotifyRemovedFromChannel( const string& channelName, const string& channelUuid, bool wasSuccessful );

   bool              SendListOfAllChatChannels( SerializedKeyValueVector< ChannelInfo >& usersAndIds );
   bool              SendListOfAllUsers( SerializedKeyValueVector< string >& usersAndIds );
   bool              SendListOfChatters( const string& channelUuid, SerializedKeyValueVector< string >& usersAndIds );
   bool              SendListOfAllUsersInChatChannel( const string& channelUuid, SerializedKeyValueVector< string >& usersAndIds );

protected:
   bool              ProcessPacket( BasePacket* packet );
   bool              HandleDbQueryResult( BasePacket* packet );
   bool              SendListOfFriendsToGateway();
   bool              SendListOfChannelsToGateway();

   bool              PrepInitialLogin();
   bool              RequestExtraUserInfo();
   bool              RequestFriends();
   bool              RequestChatChannels();
   void              GetChatChannelDigest( const string& groupUuid, int numRecords, int startingIndex );
   void              GetChatP2PDigest( const string& groupUuid, int numRecords, int startingIndex );
   void              GetAllChatHistroySinceLastLogin();

   bool              InformUserOfSuccessfulLogin();

   bool              SendPacketToGateway( BasePacket* packet ) const;

   bool              SendLoginStatus( bool wasSuccessful );

   //-------------------------------------------------------------------------

   bool              SendChatOut( const string& message, const string& userUuid, const string& channelUuid );

   //-------------------------------------------------------------------------

   int                        m_userDbId;
   U32                        m_connectionId;

   bool                       m_isUserDbLookupPending;
   bool                       m_userFriendsComplete;
   bool                       m_userChannelsComplete;
   bool                       m_badConnection;


   string                     m_username;
   string                     m_uuid;
   string                     m_loginKey;

   string                     m_currentChannel;// eventually replace with a uuid for the group
   string                     m_currentFriendUuid;// eventually replace with a uuid for the group

   string                     m_lastLoginTime;

   // todo, change these to structures
   ChannelKeyValue            availableChannels; // the key is the uuid and the value is the user name
   KeyValueVector             availableFriends; // the key is the uuid and the value is the user name

   deque< BasePacket* >       m_packetsIn;

   static DiplodocusChat*     m_chatServer;

   static ChatChannelManager* m_chatChannelManager;
};
