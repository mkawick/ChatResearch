// ChatUser.h

#pragma once


class    DiplodocusChat;
class    ChatRoomManager;
//class    InvitationManager;

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

class ChatUser
{
public:
   ChatUser( U32 connectionId, U32 gatewayId );
   ~ChatUser();

   void           SetConnectionId( U32 connId ) { m_connectionId = connId; }
   U32            GetConnectionId() const { return m_connectionId; }

   void           SetGatewayId( U32 id ) { m_gatewayId = id; }
   U32            GetGatewayId() const { return m_gatewayId; }

   U32            GetUserId() const { return m_userId ; }
   string         GetUserName() const { return m_userName; }
   string         GetUuid() const { return m_uuid; }
   time_t         GetLoggedOutTime() const { return m_loggedOutTime; }

   void           Init( U32 userId, const string& name, const string& uuid, const string& lastLoginTime );

   void           LoggedIn();
   void           LoggedOut();

   static void    Set( DiplodocusChat* chat );
   static void    Set( ChatRoomManager* mgr );
   //static void    Set( InvitationManager* mgr );


   bool           HandleClientRequest( BasePacket* packet );
   bool           HandleDbResult( PacketDbQueryResult* packet );

   void           ChatReceived( const string& message, const string& senderUuid, const string& senderDisplayName, string groupUuid, string timeStamp, U32 userId );

   bool           Update();

   //--------------------------------------

   bool              SendErrorMessage( PacketErrorReport::ErrorType );

   bool              NotifyChannelAdded( const string& channelName, const string& channelUuid, bool wasSuccessful );
   bool              NotifyChannelMovedToInactive( const string& channelUuid, int numremoved, bool wasSuccessful );
   bool              NotifyChannelRemoved( const string& channelUuid, int numremoved );
   bool              NotifyAllChannelsLoaded( bool loaded );
   bool              NotifyUserStatusHasChanged( const string& userName, const string& userUuid, int statusChange );
   //bool              NotifyYouWereAddedToChannel( const string& channelUuid );

   bool              NotifyAddedToChannel( const string& channelName, const string& channelUuid, const string userName = "", const string userUuid = "" );
   //bool              NotifyAddedToChannel( const string& channelUuid, const string& userUuid, bool wasSuccessful );
   bool              NotifyRemovedFromChannel( const string& channelName, const string& channelUuid, bool wasSuccessful, string userUuid = "" );

   //--------------------------------------
private:
   bool              SendChat( const string& message, const string& userUuid, const string& channelUuid, U32 gameTurn );
   bool              EchoHandler();

   bool              SendMessageToClient( BasePacket* packet ) const;
   bool              RequestUserProfileInfo();
   void              RequestChatChannels();
   void              RequestAllBasicChatInfo();
   void              QueryChatChannelHistory( const string& channelUuid, int numRecords, int startingIndex, const string& startingTimestamp );
   void              LoadUserProfile( const PacketDbQueryResult * dbResult );
   void              SendChatChannelHistoryToClient( const PacketDbQueryResult * dbResult );

   void              QueryChatP2PHistory( const string& userUuid, int numRecords, int startingIndex, const string& startingTimestamp );
   void              SendChatp2pHistoryToClient( PacketDbQueryResult * dbResult );
   void              SendChatHistoryToClientCommon ( const DynamicDataBucket& bucket, const string& userUuid, const string& chatChannelUuid, const string& startingTimestamp, int startingIndex, int numExpected );

   void              RequestProfileInfo();
   void              GetAllChatHistroySinceLastLogin();
   void              SendChatHistorySinceLastLogin( const vector< MissedChatChannelEntry >& );
   void              StoreChatHistoryMissedSinceLastLogin( PacketDbQueryResult * dbResult );
   
   enum QueryType
   {
      QueryType_ChatChannelHistory,
      QueryType_ChatP2PHistory,
      QueryType_ChatHistoryMissedSinceLastLogin,
      QueryType_UserProfile
   };

private:

   int                        m_userId; // the db identity
   U32                        m_connectionId;
   U32                        m_gatewayId;

   bool                       m_isLoggedIn;
   time_t                     m_loggedOutTime;
   bool                       m_initialRequestForInfoSent;

   bool                       m_blockContactInvitations;
   bool                       m_blockGroupInvitations;


   string                     m_userName;
   string                     m_uuid;
   string                     m_lastLoginTime;

   static DiplodocusChat*     m_chatServer;
   static ChatRoomManager*    m_chatRoomManager;
   //static InvitationManager*  m_invitationManager;

private:
   ChatUser();
   ChatUser( const ChatUser& );
   ChatUser& operator = ( const ChatUser& );
};

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
