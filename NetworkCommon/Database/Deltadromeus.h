// Deltadromeus.h

#pragma once

#include <string>
#include <list>
#include <deque>
using namespace std;

#include "../DataTypes.h"
#include "../ChainedArchitecture/ChainedThread.h"

class    BasePacket;
struct   DbJob;// forward declare... see cpp

//typedef struct MYSQL DbPointer;
struct   DbHandle;

//---------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------

class    Deltadromeus : public Threading::CChainedThread< BasePacket* >
{
public:
   typedef U32             JobId;
   static const JobId      JobIdError = 0xffffffff;

   typedef std::list< std::string > ResultRow;
   typedef std::list< ResultRow >   ResultSet;

public:
   Deltadromeus();

   void     SetConnectionInfo( const string& serverName, U16 port, const string& username,const string& password, const string& dbName );
   bool     IsConnected() const { return m_isConnected; }

   //-----------------------------------------------
   JobId    SendQuery( const string& query, int myId = 0, int senderReference = 0, const list<string>* stringsToEscape = NULL, bool isFireAndForget = false, bool isChainData = false, int extraLookupInfo = 0, string* meta = NULL, U32 serverId =0 );
   bool     AddInputChainData( BasePacket* packet, U32 chainId );// only use this interface for chained processing

   bool     HasResults( JobId id ) const;
   bool     GetResults( JobId id, ResultSet& results );
   bool     IsComplete( JobId id ) const;
   bool     HasJobsInProgress() const { return m_jobsInProgress.size() > 0; }

   bool     Log( const char* text, int priority = 1 );

   //------------------------------------------------------------

protected:
   ~Deltadromeus();
   void     Disconnect();
   void     Connect();

   int      CallbackFunction();
   bool     PutQueryResultInProperChain( DbJob* );
   JobId    CreateJobId();

   typedef deque< DbJob* >    JobQueue;
   typedef JobQueue::iterator JobQueueIterator;

   JobQueue                m_jobsInProgress;
   JobQueue                m_jobsComplete;

   bool                    m_isConnected;
   string                  m_serverName;
   U16                     m_port;
   string                  m_username;
   string                  m_password;
   string                  m_dbName;

   DbHandle*               m_DbConnection;
};

//---------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------
