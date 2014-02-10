// ChatUser.h

#pragma once


class DiplodocusChat;
class ChatChannelManager;

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

class ChatUser
{
public:
   ChatUser( U32 connectionId );
   ~ChatUser();

   void           SetConnectionId( U32 connId ) { m_connectionId = connId; }
   U32            GetConnectionId() const { return m_connectionId; }
   U32            GetUserId() const { return m_userId ; }
   string         GetUserName() const { return m_userName; }
   string         GetUuid() const { return m_uuid; }
   time_t         GetLoggedOutTime() const { return m_loggedOutTime; }

   void           Init( U32 userId, const string& name, const string& uuid, const string& lastLoginTime );

   void           LoggedIn() { m_isLoggedIn = true; }
   void           LoggedOut();

   static void    Set( DiplodocusChat* chat );
   static void    Set( ChatChannelManager* mgr );


   bool           HandleDbResult( PacketDbQueryResult* packet );
   //--------------------------------------
private:
   
   enum QueryType
   {
      QueryType_UserLoginInfo =     1<<0,
      QueryType_UserFriendsList =   1<<1,
      QueryType_UserChannelList =   1<<2,
      QueryType_ChatChannelHistory =1<<3,
      QueryType_ChatP2PHistory =    1<<4,
      QueryType_ChatHistoryMissedSinceLastLogin = 1<<5,
      QueryType_Last =              1<<6,
      QueryType_All =               QueryType_Last - 1 // tricky math here.. pay attention
   };

private:

   int                        m_userId; // the db identity
   U32                        m_connectionId;
   U32                        m_pendingQueries;

   bool                       m_isLoggedIn;
   time_t                     m_loggedOutTime;

  /* bool                       m_isUserDbLookupPending;
   bool                       m_userFriendsComplete;
   bool                       m_userChannelsComplete;
   bool                       m_badConnection;*/


   string                     m_userName;
   string                     m_uuid;
   string                     m_lastLoginTime;

   static DiplodocusChat*     m_chatServer;
   static ChatChannelManager* m_chatChannelManager;

private:
   ChatUser();
   ChatUser( const ChatUser& );
   ChatUser& operator = ( const ChatUser& );
};

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
