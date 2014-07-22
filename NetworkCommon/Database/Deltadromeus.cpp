// Deltadromeus.cpp

// for these includes to work, you must have these in your path:
// C:\Program Files (x86)\MySQL\mysql_connector_cpp\include
// C:\Program Files (x86)\MySQL\mysql_connector_c\include
// C:\Program Files (x86)\MySQL\mysql_connector_cpp\lib
// C:\Program Files (x86)\MySQL\mysql_connector_c\lib
// these can be obtained from the Oracle site. Search for MySql connector/c and connector/c++

// this setup is based on this:
// http://www.nitecon.com/tutorials-articles/develop/cpp/c-mysql-beginner-tutorial/

#include "../ServerConstants.h"

#if PLATFORM == PLATFORM_WINDOWS
#pragma warning( disable:4996 )
//#pragma comment( lib, "libmysql.lib" )
#endif

#include "../Utils/CommandLineParser.h"

#include <boost/type_traits.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <memory.h>

#include "../Logging/server_log.h"
#include "../DataTypes.h"
#include "../Packets/DbPacket.h"
#include "Deltadromeus.h"

#if PLATFORM != PLATFORM_WINDOWS
#undef closesocket
#endif

#include <mysql/mysql.h> // MySQL Include File
#include <mysql/errmsg.h> // MySQL Include File

using namespace Database;
using namespace std;

void  LogEverything( const char* text )
{
   //cout << text << endl;
   //LogMessage(LOG_PRIO_ERR, "MySQL Initialization Failed\n");//
}

//---------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------

DbJobBase :: DbJobBase( Database::JobId id, const std::string& query, U32 senderKey, U32 senderIdentifier ) : 
      m_jobId( id ), 
      m_query( query ), 
      m_senderIdentifier( senderIdentifier ), 
      m_senderKey( senderKey ),
      m_senderLookup( 0 ),
      m_serverId( 0 ),
      m_insertId( 0 ),
      m_numAttemptsToResend( 0 ),
      m_customData( NULL ),
      m_hasStarted( false ),
      m_hasResultSet( false ),
      m_isComplete( false ),
      m_cancelled( false ),
      m_fireAndForget( false ),
      m_isChainData( false ),
      m_errorCondition( false ),
      m_errorConnectionNeedsToBeReset( false )
{
   if( m_query.size() < 12 )// about the minimum that a query can be "select * from a"
   {
      m_isComplete = true;
      m_errorCondition = true;
   }
}

//---------------------------------------------------------------------------------------------------

struct DbJob : public DbJobBase
{
public:
   DbJob( Database::JobId id, const std::string& query, U32 senderKey = 0, U32 senderIdentifier = 0 ) : 
      DbJobBase( id, query, senderKey, senderIdentifier ) {}

   //-----------------------------------------------------------

   bool        SubmitQuery( DbHandle* connection, const string& dbName );
};

//------------------------------------------------------------

void  DbJobBase::AddEscapedString( DbHandle* connection, const string& escapeMe )
{
   if( escapeMe.size() == 0 )
   {
      boost::replace_first( m_query, "%s", "" );// much, much better than sprintf
      return;
   }

   // in the overwhelmin majority of cases, 512 is more than enough.
   // But occassionally...
   const int localBufferLen  = 512;
   int stringLen = static_cast< U32 >( escapeMe.size() );
   int len = stringLen;
   char localBuffer[ localBufferLen ];
   char* escapedVersion = localBuffer;
   if( len > localBufferLen )
   {
      stringLen = len + localBufferLen*4;// this can be very long
      escapedVersion = new char [stringLen]; // add a little extra
   }

   // be VERY CAREFUL on the last param here.... it's the size of the source string
   mysql_real_escape_string( connection, escapedVersion, escapeMe.c_str(), escapeMe.size() );
   size_t escapedLen = strlen( escapedVersion );
   if( escapedLen == 0 )
      return;

   if( escapedLen > 0 )
   {
      if( escapedLen > localBufferLen )
      {
         const string searchString = "%s";
         const size_t searchStringLen = searchString.size();
         const char* ptr = strstr( m_query.c_str(), searchString.c_str() );
         if( ptr != NULL )
         {
            boost::replace_first( m_query, "%s", escapedVersion );
         }
      }
      else // this works fine for smaller strings
      {
         boost::replace_first( m_query, "%s", escapedVersion );
      }
   }
   else
   {
      boost::replace_first( m_query, "%s", "" );// much, much better than sprintf
   }

   if( escapedVersion != localBuffer )
   {
      delete [] escapedVersion;
   }
}

//------------------------------------------------------------

bool  DbJob::SubmitQuery( MYSQL* connection, const string& dbName )
{
   if( m_hasStarted == true )
      return false;

   m_errorCondition = false;
   m_hasStarted = true;

   //mysql_select_db( connection, dbName.c_str() );
   int result = mysql_query( connection, m_query.c_str() );
   
   if( result != 0 )
   {
      m_errorCondition = true;
      const char* errorText = mysql_error( connection );
      cout << "DB Error: " << errorText << " on query " << m_query << endl;

      U32 errorCode = mysql_errno( connection );
      cout << "DB Error code: " << errorCode << endl;
      if( errorCode == CR_SERVER_LOST ||
         errorCode == CR_CONNECTION_ERROR ||
         errorCode == CR_SERVER_GONE_ERROR )
      {
         m_errorConnectionNeedsToBeReset = true; // verified
      }
   }
   else
   {
      MYSQL_RES* res_set = mysql_store_result( connection ); // Receive the result and store it in res_set
      m_insertId = static_cast<U32> ( mysql_insert_id( connection ) );// just grab it, even if not an insert
      if( res_set )
      {
         //unsigned long long numrows = mysql_num_rows( res_set ); // Create the count to print all rows
         unsigned int numcolumns = mysql_num_fields( res_set );

         MYSQL_ROW row;  // Assign variable for rows. 

         // This while is to store all rows and not just the first row found,
         while ( ( row = mysql_fetch_row( res_set ) ) != NULL )
         {
            if( *row == NULL )
               continue;

            Database::ResultRow oneRow;
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
         
         mysql_free_result( res_set );
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

void  DbJobBase::Cancel() 
{
   m_cancelled = true;
}

//---------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------

Deltadromeus::Deltadromeus() : Threading::CChainedThread< BasePacket* >(),
   m_dbConnectionTypeBitField( 0 ),
   m_isConnected( false ),
   m_needsReconnection( true ),
   m_port( 0 ),
   m_DbConnection( NULL )
   
{
   /// this could be simpler... but I want to make this easier to maintain
   for( int i=0; i<DbConnectionType_Count; i++ )
   {
       m_dbConnectionTypeBitField |= (1<<i);
   }
   SetSleepTime( 65 );// only check for db needs once in a while
}

Deltadromeus::~Deltadromeus()
{
   Disconnect();
}

//------------------------------------------------------------

void     Deltadromeus::SetConnectionInfo( const string& serverName, U16 port, const string& username,const string& password, const string& dbSchema )
{
   Disconnect();
   
   m_serverName = serverName;
   m_port = port;
   m_username = username;
   m_password = password;
   m_dbSchema = dbSchema;

   Connect();
}

void     Deltadromeus::GetConnectInfo( string& serverName, U16& port, string& username, string& password, string& dbName )  const
{
   serverName = m_serverName;
   port = m_port;
   username = m_username;
   password = m_password;
   dbName = m_dbSchema;
}

//------------------------------------------------------------

Database::JobId    
   Deltadromeus::SendQuery( const string& query, 
                            int myId, 
                            int senderReference, 
                            const list<string>* stringsToEscape, 
                            bool isFireAndForget, 
                            bool isChainData, 
                            int extraLookupInfo, 
                            string* meta, 
                            U32 serverId, 
                            void* customData )
{
   if( m_isConnected == false )
   {
      if( m_needsReconnection == true )
      {
         Connect();
      }
      if( m_isConnected == false )
         return JobIdError;
   }

   if( query.size() == 0 )
      return JobIdError;

   JobId    jobId = CreateJobId();
   DbJobBase* job = new DbJob( jobId, query, senderReference, myId );// yes we are tracking the jobId twice
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

   job->SetCustomData( customData );

   m_mutex.lock();
   m_jobsInProgress.push_back( job );
   m_mutex.unlock();   
   Resume();

   return jobId;
}

//------------------------------------------------------------

bool     Deltadromeus::AddInputChainData( BasePacket* packet, U32 senderId )
{
   if( packet->packetType == PacketType_DbQuery )
   {
      PacketDbQuery* dbPacket = static_cast< PacketDbQuery* >( packet );
      Database::JobId id = SendQuery( dbPacket->query, senderId, dbPacket->id, &(dbPacket->escapedStrings.bucket), dbPacket->isFireAndForget, true, dbPacket->lookup, &dbPacket->meta, dbPacket->serverLookup, dbPacket->customData );
      id = id;// remove warning.
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

   deque< DbJobBase* >::iterator it = m_jobsComplete.begin();
   while( it != m_jobsComplete.end() )
   {
      DbJobBase* job = *it;
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
   int num = static_cast<int>( m_jobsComplete.size() );
   for( int i=0; i<num; i++ )
   {
      const DbJobBase* job = m_jobsComplete[i];
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
   int num = static_cast<int>( m_jobsComplete.size() );
   for( int i=0; i<num; i++ )
   {
      const DbJobBase* job = m_jobsComplete[i];
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
   Resume();

   m_mutex.lock();

   if( m_jobsInProgress.size() )
   {
      DbJobBase* currentJob = m_jobsInProgress.front();
      if( currentJob->HasStarted() == true )
      {
         while( currentJob->IsComplete() == false )// wait fo this to finish
         {
            Sleep( 20 );
         }
      }

      while( m_jobsInProgress.size() > 0 )
      {
         DbJobBase* jobPtr = m_jobsInProgress.front();
         m_jobsInProgress.pop_front();
         delete jobPtr;
      }
   }
   while( m_jobsComplete.size() > 0 )
   {
      DbJobBase* jobPtr = m_jobsComplete.front();
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
      LogMessage(LOG_PRIO_ERR, "MySQL Initialization Failed\n");
      return;
   }
   int zeroMeansSuccess = mysql_options( m_DbConnection, MYSQL_SET_CHARSET_NAME, "utf8" );
   zeroMeansSuccess = mysql_options( m_DbConnection, MYSQL_INIT_COMMAND, "SET NAMES utf8"); 

   // Now we will actually connect to the specific database.

   m_DbConnection = mysql_real_connect( m_DbConnection, m_serverName.c_str(), m_username.c_str(), m_password.c_str(), m_dbSchema.c_str(), m_port, NULL, 0 );

   string output = " login to the DB, IP: ";
   output += m_serverName;
   output += ":";
   output += boost::lexical_cast<string> ( m_port );
   output += " with schema ";
   output += m_dbSchema;
   output += " using  user=";
   output += m_username;

   if( m_DbConnection )    // If instance didn't initialize say so and exit with fault.
   {
      m_isConnected = true;
      m_needsReconnection = false;
      cout << "Successful" << output << endl;
      //LogMessage(LOG_PRIO_ERR, "Successful%s\n", output.c_str());

      //zeroMeansSuccess = mysql_set_character_set( m_DbConnection, "utf8" );

      my_bool reconnect = true;
      zeroMeansSuccess = mysql_options( m_DbConnection, MYSQL_OPT_RECONNECT, &reconnect );
   }
   else
   {
      cout << "Failed" << output << endl;
      LogMessage(LOG_PRIO_ERR, "Failed%s\n", output.c_str());
   }
   

   Resume();

   
}

//------------------------------------------------------------

int     Deltadromeus::CallbackFunction()
{
   if( m_isConnected == false )
   {
      if( m_needsReconnection == true )
      {
         Connect();
      }
      return 0;
   }

   LogEverything( "DB update" );

   m_mutex.lock();
   int numRemaining = m_jobsInProgress.size();
   DbJobBase* currentJob = NULL;
   if( numRemaining )
      currentJob = m_jobsInProgress.front();// might be invalid
   m_mutex.unlock(); 

   if( currentJob != NULL )
   {
      if( currentJob->HasStarted() == false )
      {
         LogEverything( "DB submit query " );
         currentJob->SubmitQuery( m_DbConnection, m_dbSchema );
      }
      else if( currentJob->IsComplete() == true )
      {
         if( currentJob->GetErrorCondition() == true && currentJob->GetIsConnectionBad() )
         {
            m_isConnected = false;
            m_needsReconnection = true;
            cout << "-> Bad connection leads to a reconnect and resetting the state of the query." << endl;
            Connect();
            currentJob->ResetErrorState();// keep trying until the connection resets.
            currentJob->ResetToResubmitSameQuery();
         }
         else
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
               if( currentJob->GetNumAttemptsToResend() < 5 )
               {
                  currentJob->IncrementAttemptsToSend();
                  m_jobsComplete.push_back( currentJob );
               }
               else
               {
                  cout << "Result set rejected" << endl;
                  delete currentJob;// we gave the sender every opportunity to accept
               }
            }
            m_mutex.lock();
            m_jobsInProgress.pop_front();
            currentJob = NULL;
            numRemaining = m_jobsInProgress.size();
            if( numRemaining )
               currentJob = m_jobsInProgress.front();// might be invalid
            m_mutex.unlock(); 

            // now start the next job
            if( numRemaining > 0 )
            {
               LogEverything( "DB has more jobs to start " ); 
               currentJob->SubmitQuery( m_DbConnection, m_dbSchema );
            }
            else
            {
               Pause();
               LogEverything( "DB queue is empty" );
            }
         }
      }
   } 

   return 1;
}

//------------------------------------------------------------

bool     Deltadromeus::PutQueryResultInProperChain( DbJobBase* testJob )
{
   if( testJob->IsChainData() == false )
      return false;

   U32 matchingId = testJob->GetSenderId();

   if( matchingId != (U32)-1 )
   {
      Threading::MutexLock    locker( m_inputChainListMutex );
      ChainLinkIteratorType it = m_listOfInputs.begin();
      U32 chainedId = GetChainedId();
      while( it != m_listOfInputs.end() )
      {
         //ChainLink& link = (*it);
         ChainType*	chainObj = static_cast< ChainType*> ( (*it).m_interface );

         if( chainObj->GetChainedId() == matchingId )
         {
            PacketDbQueryResult* resultPacket = new PacketDbQueryResult;
            resultPacket->id =                  testJob->GetSenderKey();
            resultPacket->lookup =              testJob->GetSenderLookup();
            resultPacket->serverLookup =        testJob->GetServerId();
            resultPacket->meta =                testJob->GetSenderMeta();
            resultPacket->successfulQuery =     ( testJob->GetErrorCondition() == false );
            resultPacket->customData =          testJob->GetCustomData();
            resultPacket->insertId =            testJob->GetInsertId();

            if( resultPacket->successfulQuery == true )
            {
               //**************************************************************************************
               resultPacket->bucket = testJob->GetResults();// <<<<< is very slow for large result sets.
               //**************************************************************************************
            }

            if( chainObj->AddOutputChainData( resultPacket, chainedId ) == true )
            {
               delete testJob;
               return true;
            }
            else
            {
               resultPacket->bucket.bucket.clear();
               delete resultPacket;
            }
         }
         it++;
      }
   }

   return false;
}

//------------------------------------------------------------

Database::JobId    
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
//------------------------------------------------------------

bool  Database::ParseCommandLineIntoConnectionInfo( CommandLineParser& parser, vector< DbConnectionInfo >& listOfConnections )
{
   vector< string > databaseConfiguration;
   if( parser.FindValue( "databaselist", databaseConfiguration ) == false )
   {
      if( parser.FindValue( "dblist", databaseConfiguration ) == false )
      {
         cout << "Multiple dbs were not listed. " << endl;
         return false;
      }
   }

   int NumFields = 6;
   U32 bitTrackingForDbConnections = 0;
   cout << "databases found = " << endl << "[ " << endl; 
   vector< string >::iterator it = databaseConfiguration.begin();
   while( it != databaseConfiguration.end() )
   {
      string str = *it++;
      vector< string > values;
      if( parser.SeparateStringIntoValues( str, values, NumFields ) == true )
      {
         string dbConnectionType = values[0];
         const string& dbIpAddress = values[1];
         const string& dbPort = values[2];
         const string& dbUsername = values[3];
         const string& dbPassword = values[4];
         const string& dbSchema = values[5];

         cout << boost::format("%s: %15s:%s ={ %6s - %-6s }-schema=%s")  % dbConnectionType % dbIpAddress % dbPort % dbUsername % dbPassword % dbSchema << endl;

         
         int port = 0;
         bool success = false;
         try
         {
            port = boost::lexical_cast<int>( dbPort );
            success = true;
         } 
         catch( boost::bad_lexical_cast const& ) 
         {
             cout << "Error: input string was not valid" << endl;
         }
         std::transform( dbConnectionType.begin(), dbConnectionType.end(), dbConnectionType.begin(), ::tolower);

         U32 dbType = Database::Deltadromeus::DbConnectionType_none;         
         if( dbConnectionType == "game" || dbConnectionType.find( "game" ) != string::npos )// you can stick strings together "usergame" or "user|game"
         {
            dbType |= (U32)Database::Deltadromeus::DbConnectionType_GameData;
            if( dbType & bitTrackingForDbConnections )
            {
               cout << "Error: duplicate connection types" << endl;
               return false;
            }
         }
         if( dbConnectionType == "user" || dbConnectionType.find( "user" ) != string::npos )
         {
            dbType |= Database::Deltadromeus::DbConnectionType_UserData;
            if( dbType & bitTrackingForDbConnections )
            {
               cout << "Error: duplicate connection types" << endl;
               return false;
            }
         }
         if( dbConnectionType == "stat" || dbConnectionType.find( "stat" ) != string::npos )
         {
            dbType |= Database::Deltadromeus::DbConnectionType_StatData;
            if( dbType & bitTrackingForDbConnections )
            {
               cout << "Error: duplicate connection types" << endl;
               return false;
            }
         }
         if( dbConnectionType == "all" )
         {
            dbType = Database::Deltadromeus::DbConnectionType_All;
            if( dbType & bitTrackingForDbConnections )
            {
               cout << "Error: duplicate connection types" << endl;
               return false;
            }
         }
         if( dbType == Database::Deltadromeus::DbConnectionType_none )
            success = false;
         else
            success = true;

         if( success )
         {
            bitTrackingForDbConnections |= dbType;
            listOfConnections.push_back( DbConnectionInfo( dbIpAddress, port, dbUsername, dbPassword, dbSchema, dbType ) );
         }
      }
      else
      {
         cout << "Not enough database configuration for this game:" << str << endl;
      }
   }
   cout << "]" << endl;

   return true;
}
//------------------------------------------------------------
