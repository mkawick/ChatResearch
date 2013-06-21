// Deltadromeus.cpp

// for these includes to work, you must have these in your path:
// C:\Program Files (x86)\MySQL\mysql_connector_cpp\include
// C:\Program Files (x86)\MySQL\mysql_connector_c\include
// C:\Program Files (x86)\MySQL\mysql_connector_cpp\lib
// C:\Program Files (x86)\MySQL\mysql_connector_c\lib
// these can be obtained from the Oracle site. Search for MySql connector/c and connector/c++

// this setup is based on this:
// http://www.nitecon.com/tutorials-articles/develop/cpp/c-mysql-beginner-tutorial/


#include <boost/type_traits.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <iostream>
using namespace std;

#include "../Platform.h"
#if PLATFORM == PLATFORM_WINDOWS
#pragma warning( disable:4996)
#pragma comment( lib, "libmysql.lib" )
//#include <winsock2.h>
#endif

#include "../Packets/DbPacket.h"

#include <my_global.h> // Include this file first to avoid problems
#include <mysql.h> // MySQL Include File

#include <memory.h>
#include "../DataTypes.h"

#include "../ChainedArchitecture/Thread.h"

// the following define allows us to generalize our header such that external users do not know the underlying tech
#define DbHandle st_mysql

//#include <stl_vector.h>

#include "Deltadromeus.h"


void  LogEverything( const char* text )
{
   //cout << text << endl;
}

//---------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------

struct DbJob
{
public:
   DbJob( Deltadromeus::JobId id, const std::string& query, U32 senderKey = 0, U32 senderIdentifier = 0 ) : 
      m_jobId( id ), 
      m_query( query ), 
      m_senderIdentifier( senderIdentifier ), 
      m_senderKey( senderKey ),
      m_senderLookup( 0 ),
      m_serverId( 0 ),
      m_hasStarted( false ),
      m_hasResultSet( false ),
      m_isComplete( false ),
      m_cancelled( false ),
      m_fireAndForget( false ),
      m_isChainData( false ),
      m_errorCondition( false )
      {
         if( m_query.size() < 12 )// about the minimum that a query can be "select * from a"
         {
            m_isComplete = true;
            m_errorCondition = true;
         }
      }

   //-----------------------------------------------------------

   Deltadromeus::JobId
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
   void        AddEscapedString( MYSQL* connection, const string& escapeMe );

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

   Deltadromeus::ResultSet& 
               GetResults() { return m_results; }

   bool        SubmitQuery( MYSQL* connection, const string& dbName );
   //bool        SubmitQueryAndPrintResults( MYSQL* connection );

protected:
   Deltadromeus::JobId     m_jobId;
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
   Deltadromeus::ResultSet m_results;
};

//---------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------

Deltadromeus::Deltadromeus() : Threading::CChainedThread< BasePacket* >(),
   m_isConnected( false ),
   m_port( 0 ),
   m_DbConnection( NULL )
{
   SetSleepTime( 65 );// only check for db needs once in a while
}

Deltadromeus::~Deltadromeus()
{
   Disconnect();
}

//------------------------------------------------------------

void     Deltadromeus::SetConnectionInfo( const string& serverName, U16 port, const string& username,const string& password, const string& dbName )
{
   Disconnect();
   
   m_serverName = serverName;
   m_port = port;
   m_username = username;
   m_password = password;
   m_dbName = dbName;

   Connect();
}

//------------------------------------------------------------

Deltadromeus::JobId    
   Deltadromeus::SendQuery( const string& query, int myId, int senderReference, const list<string>* stringsToEscape, bool isFireAndForget, bool isChainData, int extraLookupInfo, string* meta, U32 serverId )
{
   if( m_isConnected == false )
      return JobIdError;

   JobId    jobId = CreateJobId();
   DbJob* job = new DbJob( jobId, query, senderReference, myId );// yes we are tracking the jobId twice
   job->SetIsChainData( isChainData );
   job->SetSenderLookup( extraLookupInfo );

   if( meta )
   {
      job->SetSenderMeta( meta );
   }
   job->SetServerLookup( serverId );
   if( stringsToEscape && stringsToEscape->size() )
   {
      list<string>::const_iterator it = stringsToEscape->begin();
      while( it != stringsToEscape->end () )
      {
         job->AddEscapedString( m_DbConnection, *it++ );
      }
   }

   job->SetFireAndForget( isFireAndForget );

   m_mutex.lock();
   m_jobsInProgress.push_back( job );
   m_mutex.unlock();   

   return jobId;
}

//------------------------------------------------------------

bool     Deltadromeus::AddInputChainData( BasePacket* packet, U32 senderId )
{
   if( packet->packetType == PacketType_DbQuery )
   {
      PacketDbQuery* dbPacket = static_cast< PacketDbQuery* >( packet );
      SendQuery( dbPacket->query, senderId, dbPacket->id, &(dbPacket->escapedStrings.bucket), dbPacket->isFireAndForget, true, dbPacket->lookup, &dbPacket->meta, dbPacket->serverLookup );
      delete packet;
      return true;
   }

   return false;
}

//------------------------------------------------------------

bool     Deltadromeus::GetResults( JobId id, ResultSet& results )
{
   Threading::MutexLock Lock( m_mutex );
   //int num = m_jobsComplete.size();

   deque< DbJob* >::iterator it = m_jobsComplete.begin();
   while( it != m_jobsComplete.end() )
   {
      DbJob* job = *it;
      if( job->GetJobId() == id )
      {
         results = job->GetResults();
         delete job;
         m_jobsComplete.erase( it );
         return true;
      }
      it++;
   }

   return false;
}

//------------------------------------------------------------

bool     Deltadromeus::HasResults( JobId id ) const 
{ 
   int num = m_jobsComplete.size();
   for( int i=0; i<num; i++ )
   {
      DbJob* job = m_jobsComplete[i];
      if( job->GetJobId() == id )
      {
         return job->HasResultSet();
      }
   }
   return false; 
}

//------------------------------------------------------------

bool     Deltadromeus::IsComplete( JobId id ) const
{
   int num = m_jobsComplete.size();
   for( int i=0; i<num; i++ )
   {
      DbJob* job = m_jobsComplete[i];
      if( job->GetJobId() == id )
      {
         if( job->IsComplete() )
            return true;
         else
            return true;
      }
   }
   return false;
}

//------------------------------------------------------------

bool     Deltadromeus::Log( const char* text, int priority )
{
   string str = "INSERT INTO log values( NULL, '";
   str += text;
   str += "', ";

   ostringstream Convert;
   Convert << priority;
   str += Convert.str();

   str += ", NULL, 'sql_test_server' )";
   SendQuery( str, -1, true );

   return true;
}

//------------------------------------------------------------

void     Deltadromeus::Disconnect()
{
   m_isConnected = false;

   m_mutex.lock();

   if( m_jobsInProgress.size() )
   {
      DbJob* currentJob = m_jobsInProgress.front();
      if( currentJob->HasStarted() == true )
      {
         while( currentJob->IsComplete() == false )// wait fo this to finish
         {
            Sleep( 20 );
         }
      }

      while( m_jobsInProgress.size() > 0 )
      {
         DbJob* jobPtr = m_jobsInProgress.front();
         m_jobsInProgress.pop_front();
         delete jobPtr;
      }
   }
   while( m_jobsComplete.size() > 0 )
   {
      DbJob* jobPtr = m_jobsComplete.front();
      m_jobsComplete.pop_front();
      delete jobPtr;
   }

   m_mutex.unlock();

   if( m_DbConnection )
   {
      mysql_close( m_DbConnection );   // Close and shutdown 
   }
}

//------------------------------------------------------------

void     Deltadromeus::Connect()
{
   Pause();

   m_isConnected = false;

   m_DbConnection = mysql_init( NULL ); // Initialise the instance
   // This If is irrelevant and you don't need to show it. I kept it in for Fault Testing.
   if( !m_DbConnection )    // If instance didn't initialize say so and exit with fault.
   {
      std::cout << stderr << "MySQL Initialization Failed" << endl;
      return;
   }
   // Now we will actually connect to the specific database.

   m_DbConnection = mysql_real_connect( m_DbConnection, m_serverName.c_str(), m_username.c_str(), m_password.c_str(), m_dbName.c_str(), m_port, NULL, 0 );

   if( m_DbConnection )    // If instance didn't initialize say so and exit with fault.
   {
      m_isConnected = true;
      cout << "Successful login to the DB, IP: " << m_serverName << ":" << m_port << " with schema " << m_dbName <<  " using  user=" << m_username << endl;
   }
   else
   {
      cout << "Failed to login to the DB, IP: " << m_serverName << ":" << m_port << " with schema " << m_dbName <<  " using  user=" << m_username << endl;
   }
   

   Resume();
}

//------------------------------------------------------------

int     Deltadromeus::CallbackFunction()
{
   if( m_isConnected == false )
      return 0;

   LogEverything( "DB update" );

   if( m_jobsInProgress.size() )
   {
      DbJob* currentJob = m_jobsInProgress.front();
      if( currentJob->HasStarted() == false )
      {
         LogEverything( "DB submit query " );
         currentJob->SubmitQuery( m_DbConnection, m_dbName );
      }
      else if( currentJob->IsComplete() == true )
      {
         LogEverything( "DB query is complete " );
         if( currentJob->IsFireAndForget() == true )
         {
            LogEverything( "DB query is f&f " );
            delete currentJob;
         }
         else if( PutQueryResultInProperChain( currentJob ) == false )
         {
            LogEverything( "DB query result stored " );
            m_jobsComplete.push_back( currentJob );
         }
         m_mutex.lock();
         m_jobsInProgress.pop_front();
         m_mutex.unlock(); 

         // now start the next job
         if( m_jobsInProgress.size() > 0 )
         {
            LogEverything( "DB has more jobs to start " );
            DbJob* currentJob = m_jobsInProgress.front();
            currentJob->SubmitQuery( m_DbConnection, m_dbName );
         }
         else
         {
            LogEverything( "DB queue is empty" );
         }
      }
   } 

   return 1;
}

//------------------------------------------------------------

bool     Deltadromeus::PutQueryResultInProperChain( DbJob* testJob )
{
   if( testJob->IsChainData() == false )
      return false;

   U32 matchingId = testJob->GetSenderId();

   if( matchingId != (U32)-1 )
   {
      Threading::MutexLock    locker( m_inputChainListMutex );
      ChainLinkIteratorType it = m_listOfInputs.begin();
      while( it != m_listOfInputs.end() )
      {
         //ChainLink& link = (*it);
         ChainedInterface< BasePacket*>*	chainObj = (*it).m_interface;

         if( chainObj->GetChainedId() == matchingId )
         {
            PacketDbQueryResult* resultPacket = new PacketDbQueryResult;
            resultPacket->id = testJob->GetSenderKey();
            resultPacket->lookup = testJob->GetSenderLookup();
            resultPacket->serverLookup = testJob->GetServerId();
            resultPacket->meta = testJob->GetSenderMeta();
            resultPacket->successfulQuery = testJob->GetErrorCondition() == false;

            if( resultPacket->successfulQuery == true )
            {
               //**************************************************************************************
               resultPacket->bucket = testJob->GetResults();// <<<<< is very slow for large result sets.
               //**************************************************************************************
            }
            
            bool result = chainObj->AddOutputChainData( resultPacket, GetChainedId() );

            if( result == true )
            {
               delete testJob;
               return true;
            }
         }
         it++;
      }
   }

   return false;
}

//------------------------------------------------------------

Deltadromeus::JobId    
   Deltadromeus::CreateJobId()
{
   U8 buffer[4];
   for( int i=0; i<4; i++ )
   {
      buffer[i] = rand() % 255;
   }
   JobId jobId;
   memcpy( &jobId, buffer, sizeof( jobId ) );
   return jobId;
}

//------------------------------------------------------------

void  DbJob::AddEscapedString( MYSQL* connection, const string& escapeMe )
{
   if( escapeMe.size() == 0 )
   {
      boost::replace_first( m_query, "%s", "" );// much, much better than sprintf
      return;
   }

   char escapedVersion[512];

   mysql_real_escape_string( connection, escapedVersion, escapeMe.c_str(), escapeMe.size() );
   if( strlen( escapedVersion ) == 0 )
      return;

   if( strlen( escapedVersion ) > 0 )
   {
      boost::replace_first( m_query, "%s", escapedVersion);// much, much better than sprintf
   }
   else
   {
      boost::replace_first( m_query, "%s", "" );// much, much better than sprintf
   }
}

//------------------------------------------------------------

bool  DbJob::SubmitQuery( MYSQL* connection, const string& dbName )
{
   if( m_hasStarted == true )
      return false;

   m_hasStarted = true;

   //mysql_select_db( connection, dbName.c_str() );
   int result = mysql_query( connection, m_query.c_str() );
   
   if( result != 0 )
   {
      m_errorCondition = true;
      const char* errorText = mysql_error( connection );
      cout << "DB Error: " << errorText << " on query " << m_query << endl;
   }
   else
   {
      MYSQL_RES* res_set = mysql_store_result( connection ); // Receive the result and store it in res_set
      if( res_set )
      {
         //unsigned long long numrows = mysql_num_rows( res_set ); // Create the count to print all rows
         unsigned int numcolumns = mysql_num_fields( res_set );

         MYSQL_ROW row;  // Assign variable for rows. 

         // This while is to store all rows and not just the first row found,
         while ( ( row = mysql_fetch_row( res_set ) ) != NULL )
         {
            Deltadromeus::ResultRow oneRow;
            for( unsigned int i=0; i<numcolumns; i++ )
            {
               if( row[i] )
               {
                  oneRow.push_back( row[i] );
               }
               else
               {
                  oneRow.push_back( "NULL" );
               }
            }

            m_results.push_back( oneRow );
            m_hasResultSet = true;
         }
      }
      else
      {
         m_hasResultSet = false;
      }
   }

   m_isComplete = true;

   return m_errorCondition;
}

//------------------------------------------------------------

void  DbJob::Cancel() 
{
   m_cancelled = true;
}

//------------------------------------------------------------
//------------------------------------------------------------
