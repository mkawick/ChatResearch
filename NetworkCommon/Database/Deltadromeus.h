// Deltadromeus.h

#pragma once

#include <string>
#include <list>
#include <deque>
using namespace std;

#include "../DataTypes.h"
#include "../ChainedArchitecture/ChainedThread.h"

class    BasePacket;
struct st_mysql;

//typedef struct MYSQL DbPointer;
namespace Database 
{
   #define DbHandle st_mysql

   typedef U32             JobId;
   typedef std::list< std::string > ResultRow;
   typedef std::list< ResultRow >   ResultSet;

   //---------------------------------------------------------------

   class DbJobBase
   {
   public:
      DbJobBase( Database::JobId id, const std::string& query, U32 senderKey = 0, U32 senderIdentifier = 0 );
      virtual bool      SubmitQuery( DbHandle* connection, const string& dbName ) = 0;

      //------------------------------------------------------------------
      Database::JobId
                  GetJobId() const { return m_jobId; }
      U32         GetSenderId() const { return m_senderIdentifier; }
      U32         GetSenderKey() const { return m_senderKey; }
      U32         GetSenderLookup() const { return m_senderLookup; }
      U32         GetServerId() const { return m_serverId; }
      string      GetSenderMeta() const { return m_senderMeta; }
      bool        GetErrorCondition() const { return m_errorCondition; }

      void        SetSenderMeta( const string* meta ) { m_senderMeta = *meta; }
      void        SetSenderLookup( U32 lookup ) { m_senderLookup = lookup; }
      void        SetServerLookup( U32 serverId ) { m_serverId = serverId; }
      void        AddEscapedString( DbHandle* connection, const string& escapeMe );

      void        SetFireAndForget( bool fireAndForget = true ) { m_fireAndForget = fireAndForget; }
      bool        IsFireAndForget() const { return m_fireAndForget; }

      void        SetIsChainData( bool isChainData ) { m_isChainData = isChainData; }
      bool        IsChainData() const { return m_isChainData; }

      void        Cancel() ;// todo, this currently has no effect
      bool        IsCancelled() const { return m_cancelled; }
      // must invoke Resume for the job to start
           
      U32         GetTimeSubmitted() const { return m_timeSubmitted; }
      bool        HasStarted() const { return m_hasStarted; }
      bool        IsComplete() const { return m_isComplete; }
      bool        HasResultSet() const { return m_hasResultSet; }
      int         GetNumRows() const { return m_results.size(); }

      Database::ResultSet& 
                  GetResults() { return m_results; }
   protected:
      protected:
      Database::JobId         m_jobId;
      string                  m_query;
      U32                     m_senderIdentifier;
      U32                     m_senderKey;
      U32                     m_senderLookup;
      U32                     m_serverId;
      string                  m_senderMeta;

      U32                     m_timeSubmitted;
      bool                    m_hasStarted;
      bool                    m_hasResultSet;
      bool                    m_isComplete;
      bool                    m_cancelled;
      bool                    m_fireAndForget;
      bool                    m_isChainData;
      
      bool                    m_errorCondition;
      Database::ResultSet     m_results;
   };
   //---------------------------------------------------------------------------------------------------
   //---------------------------------------------------------------------------------------------------

   class    Deltadromeus : public Threading::CChainedThread< BasePacket* >
   {
   public:
      static const JobId      JobIdError = 0xffffffff;
   public:
      Deltadromeus();

      virtual void     SetConnectionInfo( const string& serverName, U16 port, const string& username,const string& password, const string& dbName );
      virtual bool     IsConnected() const { return m_isConnected; }

      //-----------------------------------------------
      virtual JobId    SendQuery( const string& query, int myId = 0, int senderReference = 0, const list<string>* stringsToEscape = NULL, bool isFireAndForget = false, bool isChainData = false, int extraLookupInfo = 0, string* meta = NULL, U32 serverId =0 );
      virtual bool     AddInputChainData( BasePacket* packet, U32 chainId );// only use this interface for chained processing

      bool     HasResults( JobId id ) const;
      bool     GetResults( JobId id, ResultSet& results );
      bool     IsComplete( JobId id ) const;
      bool     HasJobsInProgress() const { return m_jobsInProgress.size() > 0; }

      bool     Log( const char* text, int priority = 1 );

      //------------------------------------------------------------

   protected:
      virtual ~Deltadromeus();
      void     Disconnect();
      void     Connect();

      virtual int      CallbackFunction();
      bool     PutQueryResultInProperChain( DbJobBase* );
      JobId    CreateJobId();

      typedef deque< DbJobBase* >    JobQueue;
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
} // namespace Database