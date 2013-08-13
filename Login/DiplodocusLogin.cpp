// DiplodocusLogin.cpp

#include "DiplodocusLogin.h"
#include "../NetworkCommon/Utils/TableWrapper.h"
#include "../NetworkCommon/Utils/Utils.h"

#include "../NetworkCommon/Packets/ServerToServerPacket.h"
#include "../NetworkCommon/Packets/GamePacket.h"
#include "../NetworkCommon/Packets/DbPacket.h"

#include "../NetworkCommon/Logging/server_log.h"

#include <boost/lexical_cast.hpp>

#define _DEMO_13_Aug_2013
//////////////////////////////////////////////////////////

bool  CreateAccountResultsAggregator::HandleResult( const PacketDbQueryResult* dbResult )
{
   if( dbResult->id != m_connectionId ) // wrong result
      return false;

   m_numQueriesToAggregate--;

   // we really don't care about the results, but the error results are important
   switch( dbResult->lookup )
   {
   case DiplodocusLogin::QueryType_LookupUserByUsernameOrEmail:
      {
         UserTable            enigma( dbResult->bucket );
         UserTable::iterator  it = enigma.begin();
         m_numUserRecordsMatching  = dbResult->bucket.bucket.size();
                  
         while( it != enigma.end() )
         {
            UserTable::row       row = *it++;

            string name =        row[ TableUser::Column_name ];
            string email =       row[ TableUser::Column_email ];
            string gamekit =     row[ TableUser::Column_user_gamekit_id_hash];
            string active =      row[ TableUser::Column_active];

            if( gamekit == m_gamekitHashId )
            {
               m_whichRecordMatched |= MatchingRecord_Gamekithash;
               m_userRecordMatchingGKHash = boost::lexical_cast< U32 > ( row[ TableUser::Column_id ] );
               m_emailForMatchingRecord_GamekitHashId = email;
            }
            if( name == m_username )
            {
               m_whichRecordMatched |= MatchingRecord_Name;
               m_userRecordMatchingName = boost::lexical_cast< U32 > ( row[ TableUser::Column_id ] );
            }
            if( email == m_useremail )
            {
               m_whichRecordMatched |= MatchingRecord_Email;
               m_userRecordMatchingEmail = boost::lexical_cast< U32 > ( row[ TableUser::Column_id ] );
            }
            if( active == "1" )
            {
               m_userRecordMatchingActive = boost::lexical_cast< U32 > ( row[ TableUser::Column_id ] );
            }
         }
      }
      return true;
   case DiplodocusLogin::QueryType_LookupTempUserByUsernameOrEmail:
      {
         NewUsersTable              enigma( dbResult->bucket );
         UserTable::iterator        it = enigma.begin();
         m_numPendingUserRecordsMatching  = dbResult->bucket.bucket.size();

         while( it != enigma.end() )
         {
            NewUsersTable::row  row = *it++;
            
            string name =              row[ TableUserTempNewUser::Column_name ];
            string gkHash =            row[ TableUserTempNewUser::Column_gamekit_hash ];
            string email =             row[ TableUserTempNewUser::Column_email ];
            if( name == m_username )
            {
               m_whichRecordMatched |= MatchingRecord_Name;
               m_pendingUserRecordMatchingName = boost::lexical_cast< U32 > ( row[ TableUser::Column_id ] );
            }
            if( email == m_useremail )
            {
               m_whichRecordMatched |= MatchingRecord_Email;
               m_pendingUserRecordMatchingEmail = boost::lexical_cast< U32 > ( row[ TableUser::Column_id ] );
            }
            if( gkHash == m_gamekitHashId )
            {
               m_whichRecordMatched |= MatchingRecord_Gamekithash;
               m_pendingUserRecordMatchingGKHash = boost::lexical_cast< U32 > ( row[ TableUser::Column_id ] );
            }
         }
      }
      return true;
   case DiplodocusLogin::QueryType_LookupUserNameForInvalidName:
      {
         if( dbResult->bucket.bucket.size() != 0 )
         {
            m_userNameIsInvalid = true;
         }
      }
      return true;
   }

   return false;
}

//---------------------------------------------

bool  CreateAccountResultsAggregator::IsDuplicateRecord() const
{
   if( m_userRecordMatchingEmail || m_userRecordMatchingName )
   {
      return true;
   }
   return false;
} 

//---------------------------------------------

bool     CreateAccountResultsAggregator::ShouldUpdateUserRecord() const 
{
   if( m_userRecordMatchingEmail || m_userRecordMatchingName ) 
   {
      return false;
   }

   if( m_userRecordMatchingGKHash > 0 ) // && m_userRecordMatchingGKHash == m_userRecordMatchingActive )
   {
      return true;
   }

   return false;
}

//---------------------------------------------

bool     CreateAccountResultsAggregator::ShouldUpdatePendingUserRecord() const 
{
   if( m_pendingUserRecordMatchingGKHash || m_pendingUserRecordMatchingEmail || m_pendingUserRecordMatchingName )
   {
      if( m_pendingUserRecordMatchingGKHash == m_pendingUserRecordMatchingEmail && // only when they match do we not update because they already have what the user needs
            m_pendingUserRecordMatchingGKHash == m_pendingUserRecordMatchingName )
         return false;

      return true; /// one is different so update
   }

   return false;
}

//---------------------------------------------

bool     CreateAccountResultsAggregator::HasFoundAnyMatchingRecords() const 
{ 
   if( m_numUserRecordsMatching > 0 || m_numPendingUserRecordsMatching > 0 ) 
      return true; 
   return false;
}  

//---------------------------------------------

bool     CreateAccountResultsAggregator::ShouldInsertNewUserRecord() const
{
   if( HasFoundAnyMatchingRecords() == false )
      return true;

   if( m_numUserRecordsMatching > 0 )
   {
      // if it possible that the gk hash matches but there is not an email set.. we want to create a new record and then update the existing record to clear the gk-hash
      if( m_userRecordMatchingGKHash && m_userRecordMatchingGKHash != m_userRecordMatchingEmail )
         return true;

      // in all other cases, do not create a new account.
      return false;
   }

   if( m_numPendingUserRecordsMatching )
   {
      return false;
     /* if( m_pendingUserRecordMatchingGKHash == m_pendingUserRecordMatchingEmail && // only when they match do we not update because they already have what the user needs
               m_pendingUserRecordMatchingGKHash == m_pendingUserRecordMatchingName )
            return false;*/

   }

   return false;
}

//---------------------------------------------

bool     CreateAccountResultsAggregator::IsMatching_GKHashRecord_DifferentFrom_UserEmail( const string& testEmailAddress ) const 
{
   // m_emailForMatchingRecord_GamekitHashId could be equal to NULL
   if( m_userRecordMatchingGKHash != 0 && 
      testEmailAddress != m_emailForMatchingRecord_GamekitHashId && 
      m_emailForMatchingRecord_GamekitHashId.size() && 
      m_userRecordMatchingGKHash != m_userRecordMatchingEmail )
      return true;

   return false;
}

//////////////////////////////////////////////////////////

DiplodocusLogin::DiplodocusLogin( const string& serverName, U32 serverId )  : Diplodocus< KhaanLogin >( serverName, serverId, 0, ServerType_Login )
{
   SetSleepTime( 30 );
   LogOpen();
   LogMessage( LOG_PRIO_INFO, "Login::Login server created" );
   cout << "Login::Login server created" << endl;
}

//---------------------------------------------------------------

bool     DiplodocusLogin::AddInputChainData( BasePacket* packet, U32 connectionId )
{
   // if packet is a login or a logout packet we'll handle it, otherwise.. no deal.
   // all packets coming in should be from the gateway only 

   LogMessage( LOG_PRIO_INFO, "Login::Data in" );

   //cout << "Login::Data in" << endl;

   if( packet->packetType != PacketType_GatewayWrapper )
   {
      string text = "Login server: received junk packets. Type: ";
      text += packet->packetType;
      Log( text, 4 );
      return false;
   }

   PacketGatewayWrapper* wrapper = static_cast< PacketGatewayWrapper * >( packet );
   U32   userConnectionId = wrapper->connectionId;
   BasePacket* actualPacket = wrapper->pPacket;

   switch( actualPacket->packetType )
   {
      case PacketType_Login:
      {
         switch( actualPacket->packetSubType )
         {
         case PacketLogin::LoginType_Login:
            {
               PacketLogin* login = static_cast<PacketLogin*>( actualPacket );
               LogUserIn( login->username, login->password, login->loginKey, login->gameProductId, userConnectionId );
            }
            break;
         case PacketLogin::LoginType_Logout:
            {
               PacketLogout* logout = static_cast<PacketLogout*>( actualPacket );
               UpdateLastLoggedOutTime( userConnectionId );
               LogUserOut( userConnectionId, logout->wasDisconnectedByError );
            }
            break;
         case PacketLogin::LoginType_CreateAccount:
            {
               PacketCreateAccount* createAccount = static_cast<PacketCreateAccount*>( actualPacket );
               CreateUserAccount( userConnectionId, createAccount->useremail, createAccount->password, createAccount->username, createAccount->deviceAccountId, createAccount->deviceId, createAccount->languageId, createAccount->gameProductId );
            }
            break;
         }
         delete wrapper;
         delete actualPacket;
         return true;
      }
   }
   return false;
}

//---------------------------------------------------------------

bool     DiplodocusLogin::AddQueryToOutput( PacketDbQuery* packet )
{
   ChainLinkIteratorType itOutputs = m_listOfOutputs.begin();
   while( itOutputs != m_listOfOutputs.end() )// only one output currently supported.
   {
      ChainedInterface<BasePacket*>* outputPtr = (*itOutputs).m_interface;
      if( outputPtr->AddInputChainData( packet, m_chainId ) == true )
      {
         return true;
      }
      itOutputs++;
   }

   delete packet;/// normally, we'd leave this up to the invoker to cleanup. 
   return false;
}

//---------------------------------------------------------------

bool     DiplodocusLogin::LogUserIn( const string& username, const string& password, const string& loginKey, U8 gameProductId, U32 connectionId )
{
   cout << endl << "***********************" << endl;
   cout << "attempt to login user: "<< username << ", pwHash:" << password << " for game id=" << (int) gameProductId << " and conn: " << connectionId << endl;
   cout << "***********************" << endl;
   if( IsUserConnectionValid( connectionId ) )
   {
      // should we boot this user for hacking? Or is it bad code?
      Log( "Second attempt was made at login", 4 );
      //InformUserOfSuccessfulLogout();// someone is hacking our server
      ForceUserLogoutAndBlock( connectionId );
   }

   if( username.size() == 0 )
   {
      // should we boot this user for hacking? Or is it bad code?
      Log( "invalid attempt at login: username was empty", 4 );
      return false;
   }

   // Before we add the user, let's verify that s/he isn't already logged in with a different connectionId. Storing this in a map
   // makes sense, but it's overkill for now.

   U32 oldConnectionId = FindUserAlreadyInGame( username, gameProductId );
   if( oldConnectionId != 0 )
   {
      // should we boot this user for hacking? Or is it bad code?
      /*Log( "Second login from the same product attempt was made", 4 );
      Log( username.c_str(), 4 );
      ForceUserLogoutAndBlock( connectionId );
      return false;*/

      ReinsertUserConnection( oldConnectionId, connectionId );// and we remove the old connection

      SuccessfulLogin( connectionId, true );
      UpdateLastLoggedInTime( connectionId ); // update the user logged in time
   }
   else
   {
      ConnectionToUser conn( username, password, loginKey );
      conn.gameProductId = gameProductId;
      AddUserConnection( UserConnectionPair( connectionId, conn ) );

      //*********************************************************************************
      // perhaps some validation here is in order like is this user valid based on the key
      //*********************************************************************************

      PacketDbQuery* dbQuery = new PacketDbQuery;
      dbQuery->id =           connectionId;
      dbQuery->lookup =       QueryType_UserLoginInfo;
      dbQuery->meta =         username;
      dbQuery->serverLookup = gameProductId;

      string queryString = "SELECT * FROM users as user WHERE user_email='%s' and user_pw_hash='" ;
      queryString += boost::lexical_cast< string >( password );
      queryString += "'";
      dbQuery->query = queryString;
      dbQuery->escapedStrings.insert( username );
      
      return AddQueryToOutput( dbQuery );
   }

   return false;
}

//---------------------------------------------------------------

bool     DiplodocusLogin::LogUserOut( U32 connectionId, bool wasDisconnectedByError )
{
   ConnectionToUser* connection = GetUserConnection( connectionId );
   if( connection )
   {
      if( connection->loggedOutTime ) /// we are already logging out. The gateway may send us multiple logouts so we simply have to ignore further attemps
         return false;

      time( &connection->loggedOutTime ); // time stamp this guy
      if( wasDisconnectedByError )
      {
         FinalizeLogout( connectionId, wasDisconnectedByError );
      }
      if( wasDisconnectedByError == false )
      {
         PacketLogoutToClient* logout = new PacketLogoutToClient();
         logout->username =            connection->username;// just for loggin purposes
         logout->uuid =                connection->userUuid;
         SendPacketToGateway( logout, connectionId );
      }
   }
   else
   {
      Log( "Attempt to log user out failed: user record not found", 1 );
      return false;
   }
   return true;
}

//---------------------------------------------------------------

void  DiplodocusLogin::FinalizeLogout( U32 connectionId, bool wasDisconnectedByError )
{
   ConnectionToUser* connection = GetUserConnection( connectionId );
   if( connection )
   {
      if( connection->userUuid.size() == 0 )// this should never happen, but being careful never hurts.
         return;

      PacketDbQuery* dbQuery = new PacketDbQuery;
      dbQuery->id =              connectionId;
      dbQuery->lookup =          QueryType_UserLoginInfo;
      dbQuery->isFireAndForget = true;// no result is needed
      
      string queryString = "UPDATE users AS user SET user.last_logout_timestamp=CURRENT_TIMESTAMP WHERE uuid = '";
      queryString +=             connection->userUuid;
      queryString += "'";
      dbQuery->query =           queryString;

      SendLoginStatusToOtherServers( connection->username, 
                                    connection->userUuid, 
                                    connectionId, 
                                    connection->gameProductId, 
                                    connection->lastLoginTime, 
                                    connection->active, 
                                    connection->email, 
                                    connection->passwordHash, 
                                    connection->id, 
                                    false, 
                                    wasDisconnectedByError );
      AddQueryToOutput( dbQuery );
   }
}

//---------------------------------------------------------------
//---------------------------------------------------------------

bool  DiplodocusLogin::IsUserConnectionValid( U32 connectionId )
{
   Threading::MutexLock locker( m_inputChainListMutex );
   UserConnectionMapConstIterator it = m_userConnectionMap.find( connectionId );
   if( it != m_userConnectionMap.end() )
      return true;
   return false;
}

ConnectionToUser*     DiplodocusLogin::GetUserConnection( U32 connectionId )
{
   Threading::MutexLock locker( m_inputChainListMutex );
   UserConnectionMapIterator it = m_userConnectionMap.find( connectionId );
   if( it != m_userConnectionMap.end() )
   {
      return &( it->second );
   }
   return NULL;
}

void    DiplodocusLogin::ReinsertUserConnection( int oldIndex, int newIndex )
{
   Threading::MutexLock locker( m_inputChainListMutex );
    UserConnectionMapIterator it = m_userConnectionMap.find( oldIndex );
    if( it != m_userConnectionMap.end() )
    {
        m_userConnectionMap.insert( DiplodocusLogin::UserConnectionPair( newIndex, it->second ) );
        m_userConnectionMap.erase( it );
    }
}

bool     DiplodocusLogin::AddUserConnection( DiplodocusLogin::UserConnectionPair pair )
{
   Threading::MutexLock locker( m_inputChainListMutex );
   m_userConnectionMap.insert( pair );
   return true;
}

bool     DiplodocusLogin::RemoveUserConnection( U32 connectionId )
{
   Threading::MutexLock locker( m_inputChainListMutex );
   UserConnectionMapIterator it = m_userConnectionMap.find( connectionId );
   if( it != m_userConnectionMap.end() )
   {
      m_userConnectionMap.erase( it );
      return true;
   }
   return false;
}

//---------------------------------------------------------------
//---------------------------------------------------------------

void     DiplodocusLogin::RemoveOldConnections()
{
   Threading::MutexLock    locker( m_inputChainListMutex );
   UserConnectionMapIterator it = m_userConnectionMap.begin();
   time_t testTimer;
   time( &testTimer );

   while( it != m_userConnectionMap.end() )
   {
      UserConnectionMapIterator temp = it++;
      if( temp->second.loggedOutTime )
      {
         const int normalExpireTime = 15; // seconds
         if( difftime( testTimer, temp->second.loggedOutTime ) >= normalExpireTime )
         {
            FinalizeLogout( temp->first, false );
            m_userConnectionMap.erase( temp );
         }
      }
   }
}

//---------------------------------------------------------------

U32     DiplodocusLogin::FindUserAlreadyInGame( const string& username, U8 gameProductId )
{
   Threading::MutexLock locker( m_mutex );

   UserConnectionMapIterator it = m_userConnectionMap.begin();
   while( it != m_userConnectionMap.end() )
   {
      UserConnectionPair pairObj = *it++;
      ConnectionToUser& conn = pairObj.second;
      if( conn.gameProductId == gameProductId && // optimized for simplest test first
         
         ( conn.email == username || conn.username == username ) )// we use these interchangably ight now.
      {
         return pairObj.first;
      }
   }
   return 0;
}


//---------------------------------------------------------------
//---------------------------------------------------------------

void  DiplodocusLogin::TellUserThatAccountAlreadyMatched( const CreateAccountResultsAggregator* aggregator )
{
   if( aggregator->GetMatchingRecordType( CreateAccountResultsAggregator::MatchingRecord_Name ) )
   {
      SendErrorToClient( aggregator->GetConnectionId(), PacketErrorReport::ErrorType_CreateFailed_DuplicateUsername );  // E_NETWORK_DUPLICATE_USERNAME
      return;
   }
   if( aggregator->GetMatchingRecordType( CreateAccountResultsAggregator::MatchingRecord_Email ) )
   {
      SendErrorToClient( aggregator->GetConnectionId(), PacketErrorReport::ErrorType_CreateFailed_DuplicateEmail );  // E_NETWORK_DUPLICATE_USERNAME
      return;
   }
}

//---------------------------------------------------------------

void DiplodocusLogin::UpdateUserAccount( const CreateAccountResultsAggregator* aggregator )
{
   U32 user_id = 0;
   if( aggregator->m_userRecordMatchingGKHash != 0 )
   {
      user_id = aggregator->m_userRecordMatchingGKHash;
   }
   else if( aggregator->m_userRecordMatchingEmail != 0 )
   {
      user_id = aggregator->m_userRecordMatchingEmail;
   }
   else
   {
      assert( 0 );
   }

   // Rule #1 
   // if the GK Hash matches but the email does not, then this may be two users sharing the same device. Zero out the
   // GK hash, and then create the new account.
   if( aggregator->IsMatching_GKHashRecord_DifferentFrom_UserEmail( aggregator->m_useremail ) )
   {
      bool  wasUpdated = false;
      if( aggregator->m_userRecordMatchingGKHash != 0 )
      {
         string query = "UPDATE users SET user_gamekit_hash='0', active='1' WHERE user_id='";
         query += boost::lexical_cast< string> ( user_id );
         query += "'";
         PacketDbQuery* dbQuery = new PacketDbQuery;
         dbQuery->id =           aggregator->m_connectionId;
         dbQuery->lookup =       QueryType_UpdateUseraccount;
         dbQuery->meta =         aggregator->m_username;
         dbQuery->serverLookup = aggregator->m_gameProductId;
         dbQuery->isFireAndForget = true;
         dbQuery->query = query;
         AddQueryToOutput( dbQuery );
         wasUpdated = true;
      }


      // now use all of this infor to create a new user record and reset the game_kit_hash
      if( aggregator->m_numPendingUserRecordsMatching == 0 )//  aggregator->ShouldUpdatePendingUserRecord() == false ) // we may already have this user update pending...
      {
         CreateNewPendingUserAccount( aggregator, true );
      }
      else if( wasUpdated )
      {
         SendErrorToClient( aggregator->m_connectionId, PacketErrorReport::ErrorType_CreateAccount_AccountUpdated );
      }
      return;
   }


   // Rule #2
   // user is probably updating his user name.
   
   string query = "UPDATE users SET user_name='%s', user_name_match='%s', user_pw_hash='%s', user_email='%s', user_gamekit_hash='%s', active='1' WHERE user_id=";

   query += boost::lexical_cast< string> ( user_id );

   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =           aggregator->m_connectionId;
   dbQuery->lookup =       QueryType_UpdateUseraccount;
   dbQuery->meta =         aggregator->m_username;
   dbQuery->serverLookup = aggregator->m_gameProductId;
   dbQuery->isFireAndForget = true;

   std::string lowercase_username = aggregator->m_username; 
   std::transform( lowercase_username.begin(), lowercase_username.end(), lowercase_username.begin(), ::tolower );

   dbQuery->query = query;
   dbQuery->escapedStrings.insert( aggregator->m_username );
   dbQuery->escapedStrings.insert( lowercase_username );
   dbQuery->escapedStrings.insert( aggregator->m_password );
   dbQuery->escapedStrings.insert( aggregator->m_useremail );
   dbQuery->escapedStrings.insert( aggregator->m_gamekitHashId );
   AddQueryToOutput( dbQuery );

   SendErrorToClient( aggregator->m_connectionId, PacketErrorReport::ErrorType_CreateAccount_AccountUpdated );
}

//---------------------------------------------------------------

void  DiplodocusLogin::CreateNewPendingUserAccount( const CreateAccountResultsAggregator* aggregator, bool setGameKitHashToNUll )
{
   string query = "INSERT INTO user_temp_new_user (user_name, user_name_match, user_pw_hash, user_email, user_gamekit_hash, game_id, language_id) "
                                 "VALUES ('%s', '%s', '%s', '%s', '%s', '%s', '%s')";

   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =           aggregator->m_connectionId;
   dbQuery->lookup =       QueryType_CreateUseraccount;
   dbQuery->meta =         aggregator->m_username;
   dbQuery->serverLookup = aggregator->m_gameProductId;
   dbQuery->isFireAndForget = true;

   std::string lowercase_username = aggregator->m_username; 
   std::transform( lowercase_username.begin(), lowercase_username.end(), lowercase_username.begin(), ::tolower );

   dbQuery->query = query;
   dbQuery->escapedStrings.insert( aggregator->m_username );
   dbQuery->escapedStrings.insert( lowercase_username );
   dbQuery->escapedStrings.insert( aggregator->m_password );
   dbQuery->escapedStrings.insert( aggregator->m_useremail );
   string gameKitHash( "0" );
   if( setGameKitHashToNUll == false )
   {
      gameKitHash  = aggregator->m_gamekitHashId;
   }

   dbQuery->escapedStrings.insert( gameKitHash );
   dbQuery->escapedStrings.insert( boost::lexical_cast< string >( static_cast< int > ( aggregator->m_gameProductId ) ) );
   dbQuery->escapedStrings.insert( boost::lexical_cast< string >( static_cast< int > ( aggregator->m_languageId ) ) );
   AddQueryToOutput( dbQuery );

   SendErrorToClient( aggregator->m_connectionId, PacketErrorReport::ErrorType_CreateAccount_Success );
}

//---------------------------------------------------------------

void  DiplodocusLogin::UpdatePendingUserRecord( const CreateAccountResultsAggregator* aggregator )
{
   string query = "UPDATE user_temp_new_user SET user_name='%s', user_name_match='%s', "
         "user_pw_hash='%s', user_email='%s', user_gamekit_hash='%s', game_id='%s', "
         "language_id='%s', was_email_sent='0', lookup_key=NULL, flagged_as_invalid='0' WHERE id='";
   query += boost::lexical_cast< U32 >( aggregator->m_pendingUserRecordMatchingEmail );
   query += "'";

   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =           aggregator->m_connectionId;
   dbQuery->lookup =       QueryType_CreateUseraccount;
   dbQuery->meta =         aggregator->m_username;
   dbQuery->serverLookup = aggregator->m_gameProductId;
   dbQuery->isFireAndForget = true;

   std::string lowercase_username = aggregator->m_username; 
   std::transform( lowercase_username.begin(), lowercase_username.end(), lowercase_username.begin(), ::tolower );

   dbQuery->query = query;

   dbQuery->escapedStrings.insert( aggregator->m_username );
   dbQuery->escapedStrings.insert( lowercase_username );
   dbQuery->escapedStrings.insert( aggregator->m_password );
   dbQuery->escapedStrings.insert( aggregator->m_useremail );
   dbQuery->escapedStrings.insert( aggregator->m_gamekitHashId );
   dbQuery->escapedStrings.insert( boost::lexical_cast< string >( static_cast< int > ( aggregator->m_gameProductId ) ) );
   dbQuery->escapedStrings.insert( boost::lexical_cast< string >( static_cast< int > ( aggregator->m_languageId ) ) );

   AddQueryToOutput( dbQuery );

   SendErrorToClient( aggregator->m_connectionId, PacketErrorReport::ErrorType_CreateAccount_AccountUpdated );
}

//---------------------------------------------------------------

void  DiplodocusLogin::CreateNewUserAccount( const CreateAccountResultsAggregator* aggregator, bool setGkHashTo0 )
{
   U32 hash = static_cast<U32>( GenerateUniqueHash( boost::lexical_cast< string >( aggregator->GetConnectionId() ) + aggregator->m_useremail ) );

   string newUuid = GenerateUUID( GetCurrentMilliseconds() + hash );

   string query = "INSERT INTO users (user_name, user_name_match, user_pw_hash, user_email, user_gamekit_hash, active, language_id, uuid ) "
                                 "VALUES ('%s', '%s', '%s', '%s', '%s', '1', '%s','";
   query += newUuid;
   query += "')";

   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =           aggregator->m_connectionId;
   dbQuery->lookup =       QueryType_CreateUseraccount;
   dbQuery->meta =         aggregator->m_username;
   dbQuery->serverLookup = aggregator->m_gameProductId;
   dbQuery->isFireAndForget = true;

   std::string lowercase_username = aggregator->m_username; 
   std::transform( lowercase_username.begin(), lowercase_username.end(), lowercase_username.begin(), ::tolower );

   dbQuery->query = query;
   dbQuery->escapedStrings.insert( aggregator->m_username );
   dbQuery->escapedStrings.insert( lowercase_username );
   dbQuery->escapedStrings.insert( aggregator->m_password );
   dbQuery->escapedStrings.insert( aggregator->m_useremail );

   string gkHash( "0" );
   if( setGkHashTo0 == false )
   {
      gkHash = aggregator->m_gamekitHashId;
   }
   dbQuery->escapedStrings.insert( gkHash );
   dbQuery->escapedStrings.insert( boost::lexical_cast< string >( static_cast< int > ( aggregator->m_languageId ) ) );
   dbQuery->escapedStrings.insert( boost::lexical_cast< string >( static_cast< int > ( aggregator->m_gameProductId ) ) );

   AddQueryToOutput( dbQuery );

   SendErrorToClient( aggregator->m_connectionId, PacketErrorReport::ErrorType_CreateAccount_Success );
}


//---------------------------------------------------------------

bool  DiplodocusLogin::CreateUserAccount( U32 connectionId, const string& email, const string& password, const string& username, const string& deviceAccountId, const string& deviceId, U8 languageId, U8 gameProductId )
{
   if( IsUserConnectionValid( connectionId ) )
   {
      string str = "ERROR: Login server, user attempted to create a second account while logged in, username: ";
      str += username;
      str += ", email: ";
      str += email;
      Log( str, 4 );

      Log( "--- shutting down user", 4 );

      ForceUserLogoutAndBlock( connectionId );
      return false;
   }

   if( password.size() < 6 )
   {
      SendErrorToClient( connectionId, PacketErrorReport::ErrorType_CreateFailed_BadPassword );
      return false;
   }
   if( email.size() < 3 ) // currently user name can be null
   {
      SendErrorToClient( connectionId, PacketErrorReport::ErrorType_CreateFailed_DisallowedUsername );
      return false;
   }

   U64 gameKitHash = 0;
   ConvertFromString( deviceAccountId, gameKitHash );
   LogMessage(LOG_PRIO_INFO, "        email=%s, GC ID=%llu\n", email.c_str(), gameKitHash );

   std::string lowercase_username = username; 
   std::transform( lowercase_username.begin(), lowercase_username.end(), lowercase_username.begin(), ::tolower );
   std::string lowercase_useremail = email; 
   std::transform( lowercase_useremail.begin(), lowercase_useremail.end(), lowercase_useremail.begin(), ::tolower );

   std::size_t found = lowercase_username.find( "playdek" );
   if (found!=std::string::npos)
   {
      SendErrorToClient( connectionId, PacketErrorReport::ErrorType_CreateFailed_DisallowedUsername );
      return false;
   }

   CreateAccountResultsAggregator* aggregator = new CreateAccountResultsAggregator( connectionId, lowercase_useremail, password, username, deviceAccountId, deviceId, languageId, gameProductId ); 
   m_userAccountCreationMap.insert( UserCreateAccountPair( connectionId, aggregator ) );

   U64 passwordHash = 0;
   ConvertFromString( password, passwordHash );
   
   string queryInvalidUserName = "SELECT id FROM invalid_username WHERE user_name_match='%s'";
   string queryUsers = "SELECT * FROM users WHERE user_name='%s' OR user_email='%s' OR user_gamekit_hash='%s'";
   string queryTempUsers = "SELECT * FROM user_temp_new_user WHERE user_name='%s' OR user_email='%s' OR user_gamekit_hash='%s'";

   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =           connectionId;
   dbQuery->lookup =       QueryType_LookupUserNameForInvalidName;
   dbQuery->meta =         username;
   dbQuery->serverLookup = gameProductId;

   dbQuery->query = queryInvalidUserName;
   dbQuery->escapedStrings.insert( username );
   AddQueryToOutput( dbQuery );

   dbQuery = new PacketDbQuery;
   dbQuery->id =           connectionId;
   dbQuery->lookup =       QueryType_LookupUserByUsernameOrEmail;
   dbQuery->meta =         username;
   dbQuery->serverLookup = gameProductId;

   dbQuery->query = queryUsers;
   dbQuery->escapedStrings.insert( username );
   dbQuery->escapedStrings.insert( email );
   dbQuery->escapedStrings.insert( deviceAccountId );
   AddQueryToOutput( dbQuery );

   dbQuery = new PacketDbQuery;
   dbQuery->id =           connectionId;
   dbQuery->lookup =       QueryType_LookupTempUserByUsernameOrEmail;
   dbQuery->meta =         username;
   dbQuery->serverLookup = gameProductId;

   dbQuery->query = queryTempUsers;
   dbQuery->escapedStrings.insert( username );
   dbQuery->escapedStrings.insert( email );
   dbQuery->escapedStrings.insert( deviceAccountId );
   AddQueryToOutput( dbQuery );
   
   return false;

}

//---------------------------------------------------------------

bool  DiplodocusLogin::ForceUserLogoutAndBlock( U32 connectionId )
{
   SendErrorToClient( connectionId, PacketErrorReport::ErrorType_UserBadLogin );

   string                     username;
   string                     uuid;
   string                     lastLoginTime;
   string                     email;
   bool                       active = false;
   string                     passwordHash = "0";
   string                     userId = "0";
   U8 gameProductId = 0;

   ConnectionToUser* connection = GetUserConnection( connectionId );
   if( connection )
   {
      username =              connection->username;
      uuid =                  connection->userUuid;
      lastLoginTime =         connection->lastLoginTime;
      connection->status =    ConnectionToUser::LoginStatus_Invalid;
      active =                connection->active;
      passwordHash =          connection->passwordHash;
      userId =                connection->id;
      email =                 connection->email;
      gameProductId =         connection->gameProductId;
   }

   // now disconnect him/her
   PacketLoginToGateway* loginStatus = new PacketLoginToGateway();
   loginStatus->username = username;
   loginStatus->uuid = uuid;
   loginStatus->lastLogoutTime = GetDateInUTC();

   loginStatus->wasLoginSuccessful = false;

   SendPacketToGateway( loginStatus, connectionId );
   SendLoginStatusToOtherServers( username, uuid, connectionId, gameProductId, lastLoginTime, active, email, passwordHash, userId, false, false );

   return true;
}

//------------------------------------------------------------------------------------------------

bool  DiplodocusLogin::SendListOfGamesToGameServers( U32 connectionId, const KeyValueVector& kvArray )
{
   if( IsUserConnectionValid( connectionId ) ) // user may have disconnected waiting for the db.
   {
      BaseOutputContainer::iterator itOutputs = m_listOfOutputs.begin();
      while( itOutputs != m_listOfOutputs.end() )
      {
         ChainedInterface<BasePacket*>* outputPtr = (*itOutputs).m_interface;

         PacketListOfGames* packetToSend = new PacketListOfGames;
         packetToSend->games = kvArray;// potentially costly.
         packetToSend->connectionId = connectionId;

         if( outputPtr->AddOutputChainData( packetToSend, m_chainId ) == false )
         {
            delete packetToSend;
         }
         itOutputs++;
      }
   }

   return false;
}

//------------------------------------------------------------------------------------------------

bool  DiplodocusLogin::UpdateLastLoggedInTime( U32 connectionId )
{
   ConnectionToUser* connection = GetUserConnection( connectionId );
   if( connection )
   {
      PacketDbQuery* dbQuery = new PacketDbQuery;
      dbQuery->id =              connectionId ;
      dbQuery->lookup =          QueryType_UpdateLastLoggedInTime;
      dbQuery->isFireAndForget = true;// no result is needed

      string queryString = "UPDATE users AS user SET user.last_login_timestamp=CURRENT_TIMESTAMP WHERE uuid = '";
      queryString +=             connection->userUuid;
      queryString += "'";
      dbQuery->query =           queryString;

      return AddQueryToOutput( dbQuery );
   }
   return false;
}


//------------------------------------------------------------------------------------------------

bool  DiplodocusLogin::UpdateLastLoggedOutTime( U32 connectionId )
{
   ConnectionToUser* connection = GetUserConnection( connectionId );
   if( connection )
   {
      PacketDbQuery* dbQuery = new PacketDbQuery;
      dbQuery->id =              connectionId ;
      dbQuery->lookup =          QueryType_UpdateLastLoggedOutTime;
      dbQuery->isFireAndForget = true;// no result is needed

      string queryString = "UPDATE users AS user SET user.last_logout_timestamp=CURRENT_TIMESTAMP WHERE uuid = '";
      queryString += connection->userUuid;
      queryString += "'";
      dbQuery->query =           queryString;

      return AddQueryToOutput( dbQuery );
   }
   return false;
}

//------------------------------------------------------------------------------------------------

bool    DiplodocusLogin::SuccessfulLogin( U32 connectionId, bool isReloggedIn )
{
   ConnectionToUser* connection = GetUserConnection( connectionId );
   if( connection == NULL )
   {
      Log( "Login server: major problem with successful login", 4 );
      return false;
   }

   connection->loggedOutTime = 0;// for relogin, we need this to be cleared.

   string  username =        connection->username;
   string  userUuid =        connection->userUuid;
   string  email =           connection->email;
   string  lastLoginTime =   connection->lastLoginTime;
   bool    active =          connection->active;
   string  passwordHash =    connection->passwordHash;
   string  userId =          connection->id;
   U8 gameProductId =        connection->gameProductId;


   PacketLoginToGateway* loginStatus = new PacketLoginToGateway();
   if( connection != NULL )
   {
      loginStatus->username = username;
      loginStatus->uuid = userUuid;
      loginStatus->lastLogoutTime = lastLoginTime;
   }
   loginStatus->wasLoginSuccessful = true;


   SendPacketToGateway( loginStatus, connectionId );

   RequestListOfGames( connectionId, userUuid );

   //This is where we inform all of the games that the user is logged in.

   return SendLoginStatusToOtherServers( username, userUuid, connectionId, gameProductId, lastLoginTime, active, email, passwordHash, userId, true, false );
}

//------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------

bool  DiplodocusLogin::RequestListOfGames( U32 connectionId, const string& userUuid )
{
   return false;// not working this way anymore
   if( userUuid.size() && connectionId != 0 )
   {
      PacketDbQuery* dbQuery = new PacketDbQuery;
      dbQuery->id =     connectionId ;
      dbQuery->lookup = QueryType_UserListOfGame;

      string queryString = "SELECT game.uuid, game.name FROM game INNER JOIN user_join_game AS user_game ON game.uuid=user_game.game_uuid WHERE user_game.user_uuid = '";
      queryString += userUuid;
      queryString += "'";
      dbQuery->query =  queryString;

      return AddQueryToOutput( dbQuery );
   }
   return false;
}

//---------------------------------------------------------------

bool  DiplodocusLogin::SendLoginStatusToOtherServers( const string& username, const string& userUuid, U32 connectionId, U8 gameProductId, const string& lastLoginTime, bool isActive, const string& email, const string& passwordHash, const string& userId, bool isLoggedIn, bool wasDisconnectedByError )
{
   // send this to every other listening server
   BaseOutputContainer::iterator itOutputs = m_listOfOutputs.begin();
   while( itOutputs != m_listOfOutputs.end() )
   {
      ChainedInterface<BasePacket*>*  outputPtr = (*itOutputs).m_interface;

      BasePacket* packetToSend = NULL;
      if( isLoggedIn )
      {
         PacketPrepareForUserLogin* prepareForUser = new PacketPrepareForUserLogin;
         prepareForUser->connectionId = connectionId;
         prepareForUser->username = username;
         prepareForUser->uuid = userUuid;
         prepareForUser->lastLoginTime = lastLoginTime;
         prepareForUser->gameProductId = gameProductId;

         prepareForUser->active = isActive;
         prepareForUser->email= email;
         prepareForUser->userId = boost::lexical_cast<U32>( userId );
         prepareForUser->password = passwordHash;

         packetToSend = prepareForUser;
         
      }
      else
      {
         PacketPrepareForUserLogout* logout = new PacketPrepareForUserLogout;
         logout->connectionId = connectionId;
         logout->wasDisconnectedByError = wasDisconnectedByError;

         packetToSend = logout;
      }

      if( outputPtr->AddOutputChainData( packetToSend, m_chainId ) == false )
      {
         delete packetToSend;
      }
      itOutputs++;
   }

   return true;
}

//---------------------------------------------------------------

void  DiplodocusLogin::UpdateUserRecord( CreateAccountResultsAggregator* aggregator )
{
   // any duplicates should simply report back to the user that this account email or user id is already taken
   if( aggregator->IsDuplicateRecord() && aggregator->ShouldUpdatePendingUserRecord() == false )
   {
      TellUserThatAccountAlreadyMatched( aggregator );
   }
   else if( aggregator->ShouldUpdateUserRecord() )
   {
      UpdateUserAccount( aggregator );
   }
   else if( aggregator->ShouldUpdatePendingUserRecord() )
   {
      UpdatePendingUserRecord( aggregator );
   }
   else if( aggregator->ShouldInsertNewUserRecord() )
   {
      //
#ifdef _DEMO_13_Aug_2013
      CreateNewUserAccount( aggregator, true );
#else
      CreateNewPendingUserAccount( aggregator );
#endif
   }
   else
   {
      SendErrorToClient( aggregator->GetConnectionId(), PacketErrorReport::ErrorType_CreateFailed_UserCreateAccountPending );
   }
}

//---------------------------------------------------------------

bool     DiplodocusLogin::AddOutputChainData( BasePacket* packet, U32 connectionId )
{
   // this should be a DB Query Response only. Lookup the appropriate gateway connection and push the login result back out.
   // If 
   if( packet->packetType == PacketType_DbQuery )
   {
      if( packet->packetSubType == BasePacketDbQuery::QueryType_Result )
      {
         PacketDbQueryResult* dbResult = static_cast<PacketDbQueryResult*>( packet );
         U32 connectionId = dbResult->id;

         // new user accounts are not going to be part of the normal login.
         UserCreateAccountIterator createIt = m_userAccountCreationMap.find( connectionId );
         if( createIt != m_userAccountCreationMap.end () )
         {
            CreateAccountResultsAggregator* aggregator = createIt->second;
            aggregator->HandleResult( dbResult );
            if( aggregator->IsComplete() )
            {
               UpdateUserRecord( aggregator );
               delete aggregator;
               m_userAccountCreationMap.erase( createIt );
            }
            return true;
         }

         ConnectionToUser* connection = NULL; 
         if( connectionId != 0 )
         {
            connection = GetUserConnection( connectionId );
            if( connection == NULL )
            {
               string str = "Login server: Something seriously wrong where the db query came back from the server but no record.. ";
               Log( str, 4 );
               str = "was apparently requested or at least it was not stored properly: username was :";
               str += dbResult->meta;
               Log( str, 4 );
               return false;
            }
         }
         switch( dbResult->lookup )
         {
            cout << "Db query type:"<< dbResult->lookup << ", success=" << dbResult->successfulQuery << endl;
            case QueryType_UserLoginInfo:
               {
                  if( dbResult->successfulQuery == false || dbResult->bucket.bucket.size() == 0 )// no records found
                  {
                     //assert( 0 );// begin teardown. Inform gateway that user is not available. Gateway will teardown the connection
                     // and send a reply to this game instance.
                     SendErrorToClient( connectionId, PacketErrorReport::ErrorType_UserBadLogin );  
                     string str = "User not valid and db query failed, username: ";
                     str += connection->username;
                     str += ", uuid: ";
                     str += connection->userUuid;
                     Log( str, 4 );
                     ForceUserLogoutAndBlock( connectionId );
                     return false;
                  }
                  
                  UserTable            enigma( dbResult->bucket );
                  UserTable::row       row = *enigma.begin();
                  string id =               row[ TableUser::Column_id ];
                  string name =             row[ TableUser::Column_name ];
                  string uuid =             row[ TableUser::Column_uuid ];
                  string email =            row[ TableUser::Column_email ];
                  string passwordHash =     row[ TableUser::Column_password_hash ];
                  // note that we are using logout for our last login time.
                  string lastLoginTime =    row[ TableUser::Column_last_logout_time ];
                  string isActive =         row[ TableUser::Column_active];

                  connection->username = name;
                  connection->userUuid = uuid;
                  connection->status = ConnectionToUser::LoginStatus_LoggedIn;
                  connection->lastLoginTime = lastLoginTime;
                  connection->email = email;
                  connection->id =   id;
                  connection->passwordHash = passwordHash;
                  connection->active = boost::lexical_cast<bool>( isActive );

                  connection->gameProductId = dbResult->serverLookup;

                  SuccessfulLogin( connectionId );
                  UpdateLastLoggedInTime( dbResult->id ); // update the user logged in time
               }
               break;
            case QueryType_UserListOfGame:
               {
                  if( dbResult->successfulQuery == false || dbResult->bucket.bucket.size() == 0 )// no records found
                  {
                     string str = "List of games not valid db query failed, username: ";
                     str += connection->username;
                     str += ", uuid: ";
                     str += connection->userUuid;
                     Log( str, 4 );
                     ForceUserLogoutAndBlock( connectionId );
                     return false;
                  }

                  KeyValueVector             key_value_array;

                  SimpleGameTable            enigma( dbResult->bucket );
                  SimpleGameTable::iterator it = enigma.begin();
                  
                  while( it != enigma.end() )
                  {
                     SimpleGameTable::row       row = *it++;
                     string name =              row[ SimpleGame::Column_name ];
                     string uuid =              row[ SimpleGame::Column_uuid ];

                     key_value_array.push_back( KeyValueString ( uuid, name ) );
                  }

                  SendListOfGamesToGameServers( connectionId, key_value_array );
               }
               break;
            case QueryType_LookupUserByUsernameOrEmail:// these should never happen since these are handled elsewhere
            case QueryType_LookupTempUserByUsernameOrEmail:
            case QueryType_LookupUserNameForInvalidName:
               {
                  if( dbResult->successfulQuery == false )
                  {
                     string str = "Query failed looking up a user ";
                     str += connection->username;
                     str += ", uuid: ";
                     str += connection->userUuid;
                     Log( str, 4 );
                     ForceUserLogoutAndBlock( connectionId );
                     return false;
                  }
               }
               break;
         }
      }
   }
   return true;
}

//---------------------------------------------------------------

int      DiplodocusLogin::CallbackFunction()
{
   SendServerIdentification();

 /*  m_mutex.lock();
  /* ChainLinkIteratorType itInputs = m_listOfInputs.begin();
   while( itInputs != m_listOfInputs.end() )// only one output currently supported.
   {
      ChainedInterface<BasePacket*>*  inputPtr = itInputs->m_interface;
      InputChainType* connection = static_cast< InputChainType* >( inputPtr );
      connection->Update();
      itInputs++;
   }
   

   m_mutex.unlock();*/

   UpdateAllConnections();

   RemoveOldConnections();

   return 1;
}

//---------------------------------------------------------------
//////////////////////////////////////////////////////////
