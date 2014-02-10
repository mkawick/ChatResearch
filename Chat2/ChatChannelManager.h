#pragma once

#include <map>
#include <deque>
using namespace std;

#include "ChatChannelDbJob.h"

class DiplodocusChat;
class ChatUser;
class BasePacket;
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

static const int ChatChannelUniqueId = 0;
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

class ChatChannelManager
{
public:
   ChatChannelManager();
   ~ChatChannelManager();

   void           Init();

   static void    Set( DiplodocusChat* svr ) { m_chatServer = svr; }

   void           Update();
   bool           AddInboundPacket( BasePacket* packet ); // not thread safe
   bool           HandleDbResult( PacketDbQueryResult* packet );

private:

   void           ProcessIncomingPacket( BasePacket* );
   bool           ProcessDbResult( PacketDbQueryResult* packet );

   //bool           FindDbJob( int jobId, DbJobList& listOfJobs, DbJobIterator& iter );
   //bool           FinishJob( PacketDbQueryResult* result, ChatChannelDbJob& job );
   int            AddDbJob( const string& debugString, const string& lookupString, U32 serverId, stringhash chatUserLookup, stringhash authUserLookup, ChatChannelDbJob::JobType type );
   int            DbQueryAndPacket( U32 serverId, const string& authUuid, stringhash chatUserLookup, const string& queryString, ChatChannelDbJob::JobType jobType, bool isFandF, list< string >* sanitizedStrings = NULL );
   void           AddMetaData( PacketDbQueryResult* dbQuery, void* myData );
   int            NewDbId();


   static DiplodocusChat*        m_chatServer;

   deque< BasePacket* >          m_inboundPackets;
   deque< PacketDbQueryResult* > m_dbResults;

   //list
   vector< ChatChannelDbJob >    m_jobsPending;

   int                           m_dbIdTracker;
   // we need several maps.
   //static ChatChannelManager;
};

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
