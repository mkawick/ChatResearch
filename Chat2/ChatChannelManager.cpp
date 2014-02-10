// ChatChannelManager.cpp

#include <assert.h>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
using boost::format;

#include "ChatChannelManager.h"
#include "../NetworkCommon/Packets/BasePacket.h"
#include "../NetworkCommon/Packets/ChatPacket.h"
#include "../NetworkCommon/Packets/DbPacket.h"
#include "../NetworkCommon/Packets/PacketFactory.h"
#include "DiplodocusChat.h"

DiplodocusChat*        ChatChannelManager::m_chatServer;

// ChatChannelManagerNeedsUpdate

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

ChatChannelManager::ChatChannelManager(): m_dbIdTracker( 0 )
{
   //m_inboundPackets.reserve( 120 );// absolutely arbitrary.. it seems big enough and log(n) means that this will never be reallocated much
}

//---------------------------------------------------------------

ChatChannelManager::~ChatChannelManager()
{
   m_inboundPackets.clear();
}

//---------------------------------------------------------------

void     ChatChannelManager::Init()
{
   string queryString = "SELECT * FROM chat_channel";
   U32 serverId = 0;
}

//---------------------------------------------------------------

void     ChatChannelManager::Update()
{
   deque< BasePacket* >::iterator it = m_inboundPackets.begin();
   while( it != m_inboundPackets.end() )
   {
      BasePacket* packet = *it++;
      ProcessIncomingPacket( packet );
   }

   m_inboundPackets.clear();

   //----------------------------

   deque< PacketDbQueryResult* >::iterator itDb = m_dbResults.begin();
   while( itDb != m_dbResults.end() )
   {
      PacketDbQueryResult* packet = *itDb++;
      ProcessDbResult( packet );
   }

   m_dbResults.clear();
}

//---------------------------------------------------------------

bool     ChatChannelManager::AddInboundPacket( BasePacket* packet ) // not thread safe.. obviously
{
   m_inboundPackets.push_back( packet );
   return true;
}

//---------------------------------------------------------------

bool     ChatChannelManager::HandleDbResult( PacketDbQueryResult* packet ) // not thread safe.. obviously
{
   m_dbResults.push_back( packet );
   return true;
}

//---------------------------------------------------------------

void     ChatChannelManager::ProcessIncomingPacket( BasePacket* packet )// should mostly be database 
{
   PacketFactory factory;
   factory.CleanupPacket( packet );
}

//---------------------------------------------------------------

bool     ChatChannelManager::ProcessDbResult( PacketDbQueryResult* packet )
{
   return true;
}

//---------------------------------------------------------------------

int     ChatChannelManager::AddDbJob( const string& debugString, const string& lookupString, U32 serverId, stringhash chatUserLookup, stringhash authUserLookup, ChatChannelDbJob::JobType type )
{
   ChatChannelDbJob job;
   job.debugString = debugString;
   job.lookupString = lookupString;
   job.chatUserLookup = chatUserLookup;
   job.authUserLookup = authUserLookup;
   job.jobType = type;
   job.jobId = NewDbId();
   job.serverId = serverId;

   m_jobsPending.push_back( job );

   return job.jobId;
}

//---------------------------------------------------------------

int      ChatChannelManager::DbQueryAndPacket( U32 serverId, 
                                              const string& authUuid, 
                                              stringhash chatUserLookup, 
                                              const string& queryString, 
                                              ChatChannelDbJob::JobType jobType, 
                                              bool isFandF, 
                                              list< string >* sanitizedStrings )
{
   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id = ChatChannelUniqueId;
   dbQuery->lookup = AddDbJob( channelName, channelUuid, serverId, chatUserLookup, GenerateUniqueHash( authUuid ), jobType );
   dbQuery->meta = authUuid;
   dbQuery->isFireAndForget = isFandF;
   dbQuery->query = queryString;

   if( sanitizedStrings )
   {
      list< string >::iterator it = sanitizedStrings->begin();
      while( it != sanitizedStrings->end() )
      {
         dbQuery->escapedStrings.insert( *it++ );
      }
   }
   int queryId = dbQuery->lookup;

   if( m_chatServer->AddQueryToOutput( dbQuery, ChatChannelUniqueId, true ) == false )
   {
      cout << "ChatChannelManager:: Query packet rejected" << endl;
      
      delete dbQuery;
   }

   return queryId;
}

//---------------------------------------------------------------

void     ChatChannelManager::AddMetaData( PacketDbQueryResult* dbQuery, void* myData )
{
   assert( dbQuery->customData == NULL );
   dbQuery->customData = myData;
}

//---------------------------------------------------------------

int      ChatChannelManager::NewDbId()
{
   if( ++m_dbIdTracker < 0 ) // wrap around
      m_dbIdTracker = 1;
   return m_dbIdTracker;
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////