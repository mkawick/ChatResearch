#pragma once

#include <map>
#include <deque>
#include <list>
using namespace std;

#include "ChatChannel.h"
#include "ChatChannelDbJob.h"

class DiplodocusChat;
class ChatUser;
class BasePacket;
class PacketDbQuery;
class PacketDbQueryResult;


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

static const int ChatChannelManagerUniqueId = 0;
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

class ChatChannelManager
{
public:
   ChatChannelManager();
   ~ChatChannelManager();

   void           Init();

   static void    Set( DiplodocusChat* svr ) { m_chatServer = svr; }

   bool           Update();
   bool           AddInboundPacket( BasePacket* packet ); // not thread safe
   bool           HandleDbResult( PacketDbQueryResult* packet );

private:

   //---------------------------------------------------
   typedef list< ChatChannelDbJob >        DbJobList;
   typedef DbJobList::iterator               DbJobIterator;

   typedef map< stringhash, ChatChannel >    ChannelMap;
   typedef ChannelMap::iterator              ChannelMapIterator;
   typedef pair< stringhash, ChatChannel >   ChannelMapPair;
   //---------------------------------------------------

   void           ProcessIncomingPacket( BasePacket* );
   bool           ProcessDbResult( PacketDbQueryResult* packet, ChatChannelDbJob& job );

   //bool           FindDbJob( int jobId, DbJobList& listOfJobs, DbJobIterator& iter );
   //bool           FinishJob( PacketDbQueryResult* result, ChatChannelDbJob& job );

   // lots of utility functions
   int            AddDbJob( const string& debugString, const string& lookupString, U32 serverId, stringhash chatUserLookup, stringhash authUserLookup, ChatChannelDbJob::JobType type );
   PacketDbQuery* DbQueryFactory( const string& queryString, bool isFandF = false );
   bool           SaveQueryDetails( PacketDbQuery* dbQuery, const string& channelUuid, const string& authUuid, stringhash chatUserLookup, ChatChannelDbJob::JobType jobType, U32 serverId = 0, const string& debugString = "debug_string" );
   bool           AddSanitizedStrings( PacketDbQuery* dbQuery, list< string >& sanitizedStrings );
   bool           AddCustomData( PacketDbQuery* dbQuery, void* data );
   bool           Send( PacketDbQuery* dbQuery );


   bool           FindDbJob( int jobId, list< ChatChannelDbJob >& listOfJobs, DbJobIterator& iter );
   void           AddChatchannel( int id, const string& channelName, const string& channelUuid, bool isActive, int maxPlayers, int gameType, int gameId, const string& createDate );
   void           AddMetaData( PacketDbQueryResult* dbQuery, void* myData );
   int            NewDbId();

   void           AddUserToChannel( const string& channelUuid, const UserBasics& ub );
   void           AddAllUsersToChannel( const string& channelUuid, const SerializedKeyValueVector< string >& usersAndIds );
   bool           GetChatChannels( const string& uuid, ChannelKeyValue& availableChannels );

   //-----------------------------------------------------

   // query fundtions
   bool     RequestAllUsersInChatChannel( const string& channelUuid, bool fullList, const string& authUuid );
   bool     QueryAllUsersInChatChannel( const string& channelUuid );

   //-----------------------------------------------------

   static DiplodocusChat*        m_chatServer;

   deque< BasePacket* >          m_inboundPackets;
   deque< PacketDbQueryResult* > m_dbResults;

   DbJobList                     m_jobsPending;
   ChannelMap                    m_channelMap;

   bool                          m_isInitialized;

   int                           m_dbIdTracker;
   // we need several maps.
   //static ChatChannelManager;
};

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
