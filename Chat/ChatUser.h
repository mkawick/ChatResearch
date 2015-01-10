// ChatUser.h

#pragma once

#include "../NetworkCommon/UserAccount/UserAccountCommon.h"

class    DiplodocusChat;
class    ChatRoomManager;
//class    InvitationManager;

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

class ChatUser : public UserLoginBase 
{
public:
   ChatUser();
   ~ChatUser();

   //void           SetConnectionId( U32 connId ) { m_connectionId = connId; }
   //U32            GetConnectionId() const { return m_connectionId; }

   //void           SetGatewayId( U32 id ) { m_gatewayId = id; }
   //U32            GetGatewayId() const { return m_gatewayId; }

   //U32            GetUserId() const { return m_userId ; }
   //string         GetUserName() const { return m_userName; }
   //string         GetUuid() const { return m_uuid; }
   //time_t         GetLoggedOutTime() const { return m_loggedOutTime; }

   void             Init();
   //void           Init( U32 userId, const string& name, const string& uuid, const string& lastLoginTime );

   //void           LoggedIn();
   //void           LoggedOut();

   static void    Set( DiplodocusChat* chat );
   static void    Set( ChatRoomManager* mgr );
   //static void    Set( InvitationManager* mgr );


   bool           HandleClientRequest( const BasePacket* packet, U32 connectionId );
   bool           HandleDbResult( const PacketDbQueryResult* packet );

   void           ChatReceived( const string& message, const string& senderUuid, const string& senderDisplayName, string groupUuid, string timeStamp, U32 userId );

   void           Update();

   //--------------------------------------

   bool              SendErrorMessage( U32 connectionId, U32 gatewayId, PacketErrorReport::ErrorType );

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
   bool              SendChat( const string& message, const string& userUuid, const string& channelUuid, U32 gameTurn, U32 connectionId );
   bool              EchoHandler( U32 connectionId );

   bool              SendMessageToClient( BasePacket* packet, U32 connectionId, U32 gatewayId = 0 ) const;
   bool              RequestUserProfileInfo();
   void              RequestChatChannels( U32 connectionId = 0 );
   void              RequestAllBasicChatInfo();
   void              QueryChatChannelHistory( const string& channelUuid, int numRecords, int startingIndex, const string& startingTimestamp, U32 connectionId );
   void              LoadUserProfile( const PacketDbQueryResult * dbResult, U32 connectionId );
   void              SendChatChannelHistoryToClient( const PacketDbQueryResult * dbResult, U32 connectionId );

   void              MarkChatChannelLastReadDate( const string& chatChannelUuid, U32 connectionId );
   void              MarkP2PLastReadDate( const string& friendUuid, U32 connectionId );
   void              MarkFriendLastReadDateFinish( const PacketDbQueryResult * dbResult, U32 connectionId );
   
   void              QueryChatP2PHistory( const string& userUuid, int numRecords, int startingIndex, const string& startingTimestamp, U32 connectionId );
   void              SendChatp2pHistoryToClient( const PacketDbQueryResult * dbResult, U32 connectionId );

   void              SendChatHistoryToClientCommonSetup( const DynamicDataBucket& bucket, const string& userUuid, const string& chatChannelUuid, const string& startingTimestamp, int startingIndex, int numExpected );
   void              SendChatHistoryToClientCommon ( const DynamicDataBucket& bucket, const string& userUuid, const string& chatChannelUuid, const string& startingTimestamp, int startingIndex, int numExpected, U32 connectionId, U32 gatewayId );

   void              RequestProfileInfo();
   void              GetAllChatHistroySinceLastLogin();
   void              SendChatHistorySinceLastLogin( const vector< MissedChatChannelEntry >& );
   void              StoreChatHistoryMissedSinceLastLogin( const PacketDbQueryResult * dbResult, U32 connectionId );
   
   void              ManageChatHistoryUsers( const PacketDbQueryResult * dbResult, U32 connectionId );
   void              ManageChatHistoryChannels( const PacketDbQueryResult * dbResult, U32 connectionId );
   

   enum QueryType
   {
      QueryType_ChatChannelHistory,
      QueryType_ChatP2PHistory,
      QueryType_ManageChatHistoryUsers,
      QueryType_ManageChatHistoryChannels,
      QueryType_StoreChatHistoryMissedSinceLastLogin,
      QueryType_UpdateLastReadDate,
      QueryType_UserProfile,
      QueryType_LookupUserIdToMarkAsRead
   };

private:

   //int                        m_userId; // the db identity
   //U32                        m_connectionId;
   //U32                        m_gatewayId;

   bool                       m_isLoggedIn;
   time_t                     m_loggedOutTime;
   bool                       m_initialRequestForInfoSent;

   bool                       m_blockContactInvitations;
   bool                       m_blockGroupInvitations;


   //string                     m_userName;
   //string                     m_uuid;
   string                     m_lastLoginTime;

   static DiplodocusChat*     m_chatServer;
   static ChatRoomManager*    m_chatRoomManager;
   //static InvitationManager*  m_invitationManager;

private:
   //ChatUser();
   //ChatUser( const ChatUser& );
   //ChatUser& operator = ( const ChatUser& );
};

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
