// FruitadensClientChat.h

#pragma once
#include "../NetworkCommon/ChainedArchitecture/Thread.h"
#include "../NetworkCommon/Packets/BasePacket.h"
#include "../NetworkCommon/Packets/ChatPacket.h"

#include "../NetworkCommon/Packets/PacketFactory.h"
#include "../NetworkCommon/Packets/GamePacket.h"

#include "../NetworkCommon/NetworkOut/Fruitadens.h"
#include "../NetworkCommon/NetworkOut/Pyroraptor.h"

class FruitadensClientChat : public Fruitadens
{
   enum { MaxBufferBytes = 1024 };
public:
   FruitadensClientChat() : Fruitadens( "client chat" ), m_selectedGame( 0 ), m_numBytesSent( 0 ) {}

   bool     FinalFixup();
   string   GetUsername() const { return m_username; }
   string   GetCurrentChannel() const { return m_currentChannel; }

   // remember to wrap all of these in Gateway wrappers.
   bool     Login( const string& username, const string& password );
   bool     Logout();
   bool     RequestFriends();
   bool     RequestGroups();
   bool     ChangeChannel( const string& channel );
   bool     ChangeGame( const string& gameName );// either name or shortname

   string   FindChatChannel( const string& name ) const;
   string   FindChatChannelFromUuid( const string& uuid ) const;
   string   FindFriend( const string& name ) const;
   string   FindFriendFromUuid( const string& uuid ) const;

   U32      FindGame( const string& name ) const;
   string   FindGameNameFromGameId( U32 id ) const;

   // admin for chat
   bool     CreateChatChannel( const string& chatChannelName );
   bool     DeleteChatChannel( const string& chatChannelUuid );
   bool     InviteUserToChat( const string& userUuid, const string& chatChannelUuid );
   bool     AddUserToChat( const string& userUuid, const string& chatChannelUuid );
   bool     RemoveUserFromChat( const string& userUuid, const string& chatChannelUuid );
   bool     RequestChatHistory( const string& chatChannelUuid );
   bool     RequestAllChatChannels( const string& chatChannelUuid );
   bool     RequestChatters( const string& chatChannelUuid );
   bool     EnableDisableFiltering( const string& chatChannelUuid, bool enable = true );
   bool     RequestMembers( const string& chatChannelUuid );
   bool     LoadAllChannels();

   bool     RequestFullListOfChatChannels( bool isFullList = true );
   bool     RequestFullListOfUsers( bool isFullList = true );

   void     DumpFriends();
   void     DumpChatChannels();
   void     DumpGames();
   bool	   SendMessage( const string& message );

   bool     CreateGame( const string& gameName );
   bool     AddUserToGame( const string& channel, const string& gameUuid );
   bool     LoadListOfGames();
   bool     DeleteGame( const string& gameUuid );
   bool     GameEcho( int numBytes );
  /* string   FindGame( const string& name );
   string   FindGameByUuid( const string& name );*/

   bool     SendRawData( U8* data, int len );

protected:

   typedef SerializedKeyValueVector< string >      UserNameKeyValue;
   typedef SerializedKeyValueVector< ChannelInfo > ChannelKeyValue;
   typedef vector< PacketGameIdentification >      GameList;

   
   void     DumpListOfUsers( const string& channelUuid, const SerializedKeyValueVector< string >& users );
   void     DumpListOfGames( const GameList& gameList );
   void     AddChatChannelsToList( const ChannelKeyValue& channels );
   void     AddFriendsToList( const UserNameKeyValue& users );

   bool     RequestMissedHistoryRequest();
   string   FindChannel( const string& uuid );

   bool     SerializePacketOut( const BasePacket* packet );

   int      ProcessInputFunction();
   void     HandlePacketIn( BasePacket* packetIn );

   
   KeyValueVector    m_friends;
   ChannelKeyValue   m_groups;
   KeyValueVector    m_availableGames;
   GameList          m_gameList;

   Pyroraptor*       m_pyro;
   DWORD             m_beginTime, m_endTime;
   string            m_username, m_attemptedUsername;
   string            m_uuid;
   string            m_currentChannel;
   U32               m_selectedGame;

   U8                m_comparisonBuffer[ MaxBufferBytes ];
   int               m_numBytesSent;
};