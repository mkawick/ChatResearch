// DiplodocusLogin.cpp

#include "DiplodocusLogin.h"
#include "../NetworkCommon/Utils/TableWrapper.h"
#include "../NetworkCommon/Utils/Utils.h"

#include "../NetworkCommon/Packets/ServerToServerPacket.h"
#include "../NetworkCommon/Packets/GamePacket.h"
#include "../NetworkCommon/Packets/DbPacket.h"
#include "../NetworkCommon/Packets/CheatPacket.h"
#include "../NetworkCommon/Packets/PacketFactory.h"

#include "../NetworkCommon/Logging/server_log.h"

#include <boost/lexical_cast.hpp>

#define _DEMO_13_Aug_2013


//////////////////////////////////////////////////////////

void  ConnectionToUser::AddProductFilterName( const string& text )
{
   bool found = false;
   vector< string >::iterator searchIt = productFilterNames.begin();
   while( searchIt != productFilterNames.end() )
   {
      if( *searchIt == text )
      {
         found = true;
         break;
      }
      searchIt++;
   }
   if( found == false )
   {
      productFilterNames.push_back( text );
   }
}

int   ConnectionToUser::FindProductFilterName( const string& text )
{
   vector< string >::iterator searchIt = productFilterNames.begin();
   while( searchIt != productFilterNames.end() )
   {
      if( *searchIt == text )
      {
         return ( searchIt - productFilterNames.begin() );
      }
      searchIt++;
   }
   return -1;
}

//////////////////////////////////////////////////////////

bool  CreateAccountResultsAggregator::HandleResult( const PacketDbQueryResult* dbResult )
{
   if( dbResult->id != m_connectionId ) // wrong result
      return false;

   m_numQueriesToAggregate--;

   // we really don't care about the results, but the error results are important
   switch( dbResult->lookup )
   {
   case DiplodocusLogin:: QueryType_LookupUserByUsernameOrEmail:
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
   case DiplodocusLogin:: QueryType_LookupTempUserByUsernameOrEmail:
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
   case DiplodocusLogin:: QueryType_LookupUserNameForInvalidName:
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

DiplodocusLogin:: DiplodocusLogin( const string& serverName, U32 serverId )  : Diplodocus< KhaanLogin >( serverName, serverId, 0, ServerType_Login ), m_isInitialized( false ), m_isInitializing( false )
{
   SetSleepTime( 30 );
   LogOpen();
   LogMessage( LOG_PRIO_INFO, "Login::Login server created" );
   cout << "Login::Login server created" << endl;
}


//---------------------------------------------------------------

void     DiplodocusLogin:: ServerWasIdentified( ChainedInterface* khaan )
{
   BasePacket* packet = NULL;
   PackageForServerIdentification( m_serverName, m_serverId, m_gameProductId, m_isGame, m_isControllerApp, true, m_isGateway, &packet );
   khaan->AddOutputChainData( packet, 0 );
   m_clientsNeedingUpdate.push_back( khaan->GetChainedId() );
}

//---------------------------------------------------------------

bool     DiplodocusLogin:: AddInputChainData( BasePacket* packet, U32 connectionId )
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
         PacketFactory factory;
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
         case PacketLogin::LoginType_ListOfPurchases:
            {
               PacketListOfUserPurchases* purchases = static_cast<PacketListOfUserPurchases*>( actualPacket );
               StoreUserPurchases( userConnectionId, purchases );
            }
            break;
         case PacketLogin::LoginType_RequestListOfPurchases:
            {
               PacketRequestListOfUserPurchases* purchaseRequest = static_cast<PacketRequestListOfUserPurchases*>( actualPacket );
               RequestListOfPurchases( userConnectionId, purchaseRequest );
            }
            break;
         
         factory.CleanupPacket( packet );
         /*delete wrapper;
         delete actualPacket;*/
         }
      }
      return true;
      case PacketType_Cheat:
         {
            PacketFactory factory;
            PacketCheat* cheat = static_cast<PacketCheat*>( actualPacket );
            HandleCheats( userConnectionId, cheat );
            factory.CleanupPacket( packet );
         }
         return true;
      
   }
   return false;
}

//---------------------------------------------------------------

bool     DiplodocusLogin:: AddQueryToOutput( PacketDbQuery* packet )
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

bool     DiplodocusLogin:: LogUserIn( const string& username, const string& password, const string& loginKey, U8 gameProductId, U32 connectionId )
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

      if( SuccessfulLogin( connectionId, true ) == true )
      {
         UpdateLastLoggedInTime( connectionId ); // update the user logged in time
      }
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

bool     DiplodocusLogin:: LogUserOut( U32 connectionId, bool wasDisconnectedByError )
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

void     DiplodocusLogin:: FinalizeLogout( U32 connectionId, bool wasDisconnectedByError )
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
                                    connection->loginKey,
                                    false, 
                                    wasDisconnectedByError );
      AddQueryToOutput( dbQuery );
   }
}

//---------------------------------------------------------------
//---------------------------------------------------------------

bool  DiplodocusLogin:: IsUserConnectionValid( U32 connectionId )
{
   Threading::MutexLock locker( m_inputChainListMutex );
   UserConnectionMapConstIterator it = m_userConnectionMap.find( connectionId );
   if( it != m_userConnectionMap.end() )
      return true;
   return false;
}

ConnectionToUser*     DiplodocusLogin:: GetUserConnection( U32 connectionId )
{
   Threading::MutexLock locker( m_inputChainListMutex );
   UserConnectionMapIterator it = m_userConnectionMap.find( connectionId );
   if( it != m_userConnectionMap.end() )
   {
      return &( it->second );
   }
   return NULL;
}

void    DiplodocusLogin:: ReinsertUserConnection( int oldIndex, int newIndex )
{
   Threading::MutexLock locker( m_inputChainListMutex );
    UserConnectionMapIterator it = m_userConnectionMap.find( oldIndex );
    if( it != m_userConnectionMap.end() )
    {
        m_userConnectionMap.insert( DiplodocusLogin:: UserConnectionPair( newIndex, it->second ) );
        m_userConnectionMap.erase( it );
    }
}

bool     DiplodocusLogin:: AddUserConnection( DiplodocusLogin:: UserConnectionPair pair )
{
   Threading::MutexLock locker( m_inputChainListMutex );
   m_userConnectionMap.insert( pair );
   return true;
}

bool     DiplodocusLogin:: RemoveUserConnection( U32 connectionId )
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

void     DiplodocusLogin:: RemoveOldConnections()
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

U32     DiplodocusLogin:: FindUserAlreadyInGame( const string& username, U8 gameProductId )
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

void     DiplodocusLogin:: TellUserThatAccountAlreadyMatched( const CreateAccountResultsAggregator* aggregator )
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

void DiplodocusLogin:: UpdateUserAccount( const CreateAccountResultsAggregator* aggregator )
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

void     DiplodocusLogin:: CreateNewPendingUserAccount( const CreateAccountResultsAggregator* aggregator, bool setGameKitHashToNUll )
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

void     DiplodocusLogin:: UpdatePendingUserRecord( const CreateAccountResultsAggregator* aggregator )
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

void     DiplodocusLogin:: CreateNewUserAccount( const CreateAccountResultsAggregator* aggregator, bool setGkHashTo0 )
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

bool        DiplodocusLogin:: CreateUserAccount( U32 connectionId, const string& email, const string& password, const string& username, const string& deviceAccountId, const string& deviceId, U8 languageId, U8 gameProductId )
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

int         DiplodocusLogin:: FindProductByName( const string& name )
{
   ProductList::iterator it = m_productList.begin();
   while( it != m_productList.end() )
   {
      if( it->filterName == name )   // NOTE: these names may vary
      {
         return ( it - m_productList.begin() );
      }
      it++;
   }
   return -1;
}

//---------------------------------------------------------------

// how can we trust these values?
bool        DiplodocusLogin:: StoreUserPurchases( U32 connectionId, const PacketListOfUserPurchases* deviceReportedPurchases )
{
   Threading::MutexLock locker( m_inputChainListMutex );
   ConnectionToUser* connection = GetUserConnection( connectionId );
   if( connection == NULL )
   {
      cout << "Error: could not find user by connection id" << endl;
      return false;
   }

   int numItems = deviceReportedPurchases->purchases.size();
   for( int i=0; i< numItems; i++ )
   {
      const PurchaseEntry& purchaseEntry = deviceReportedPurchases->purchases[i];
      int  originalProductNameIndex = FindProductByName( purchaseEntry.name );
      //----------------
      if( purchaseEntry.name.size() == 0 )// we can't do anything with this.
      {
         cout << "   ***Invalid product id...title: " << purchaseEntry.productStoreId << "   price: " << purchaseEntry.price <<  "   number price: " << purchaseEntry.number_price << endl;
      }
      else
      {
         // the order ot the next two lines matters a lot.
         int userProductIndex = connection->FindProductFilterName( purchaseEntry.name );
         connection->AddProductFilterName( purchaseEntry.name );// we're gonna save the name, regardless. The device told us about the purchase.

         //**  find the item in the user record and add it to the db if not **
         if( userProductIndex == -1 && originalProductNameIndex != -1 )// the user doesn't have the record, but the rest of the DB does.
         {
            const string& uuid = m_productList[originalProductNameIndex].uuid;
            WriteProductToUserRecord( uuid, connection->userUuid, purchaseEntry.number_price );
         }

         if( originalProductNameIndex == -1 )
         {
            ProductInfo pi;
            pi.name = purchaseEntry.name;
            pi.filterName = purchaseEntry.name;
            // productStoreId .. I don't know what to do with this.
            pi.price = purchaseEntry.number_price;

            connection->productsWaitingForInsertionToDb.push_back( pi );
            AddNewProductToDb( purchaseEntry );
         }
         cout << "   title: " << purchaseEntry.name << "   price: " << purchaseEntry.price <<  "   number price: " << purchaseEntry.number_price << endl;
      }
      
   }

   if( connection->productsWaitingForInsertionToDb.size() == 0 )
   {
      SendListOfUserProductsToAssetServer( connectionId );
   }
   return false;
}

//---------------------------------------------------------------

bool   DiplodocusLogin:: RequestListOfPurchases( U32 connectionId, const PacketRequestListOfUserPurchases* purchase )
{
   ConnectionToUser* connection = GetUserConnection( connectionId );
   if( connection == NULL || connection->status != ConnectionToUser::LoginStatus_LoggedIn )
   {
      Log( "Login server: major problem with successful login", 4 );
      return false;
   }

   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =     connectionId ;
   dbQuery->lookup = QueryType_GetProductListForUser;
   dbQuery->meta = "all";


   string queryString = "SELECT * FROM product";
   if( purchase->requestUserOnly == true )
   {
      dbQuery->meta = "user";
      queryString += " INNER JOIN user_join_product AS user_join_product ON product.uuid=user_join_product.product_id WHERE user_join_product.user_uuid='%s'";
      dbQuery->escapedStrings.insert( connection->userUuid );
   }
   dbQuery->query =  queryString;   

   return AddQueryToOutput( dbQuery );

}

//---------------------------------------------------------------

bool   DiplodocusLogin:: HandleCheats( U32 connectionId, const PacketCheat* cheat )
{
   //QueryType_GetProductListForUser
   return false;
}


//---------------------------------------------------------------

bool  DiplodocusLogin:: UpdateProductFilterName( int index, const string& newFilterName )
{
   m_productList[ index ].filterName = newFilterName;

   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =              0 ;
   dbQuery->lookup =          QueryType_UpdateProductFileInfo;
   dbQuery->isFireAndForget = true;// no result is needed

   string queryString = "UPDATE product SET filter_name='%s' WHERE uuid='%s'";
   dbQuery->escapedStrings.insert( newFilterName );
   dbQuery->escapedStrings.insert( m_productList[ index ].uuid );
   dbQuery->query =           queryString;

   return AddQueryToOutput( dbQuery );
}

//---------------------------------------------------------------

bool  DiplodocusLogin:: ForceUserLogoutAndBlock( U32 connectionId )
{
   SendErrorToClient( connectionId, PacketErrorReport::ErrorType_UserBadLogin );

   string                     username;
   string                     uuid;
   string                     lastLoginTime;
   string                     email;
   bool                       active = false;
   string                     passwordHash = "0";
   string                     userId = "0";
   string                     loginKey = "";
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
      loginKey =              connection->loginKey;
   }

   // now disconnect him/her
   PacketLoginToGateway* loginStatus = new PacketLoginToGateway();
   loginStatus->username = username;
   loginStatus->uuid = uuid;
   loginStatus->lastLogoutTime = GetDateInUTC();
   loginStatus->loginKey = loginKey;

   loginStatus->wasLoginSuccessful = false;

   SendPacketToGateway( loginStatus, connectionId );
   SendLoginStatusToOtherServers( username, uuid, connectionId, gameProductId, lastLoginTime, active, email, passwordHash, userId, loginKey, false, false );

   return true;
}

//------------------------------------------------------------------------------------------------

bool  DiplodocusLogin:: SendListOfGamesToGameServers( U32 connectionId, const KeyValueVector& kvArray )
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

bool  DiplodocusLogin:: UpdateLastLoggedInTime( U32 connectionId )
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

bool  DiplodocusLogin:: UpdateLastLoggedOutTime( U32 connectionId )
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

bool    DiplodocusLogin:: SuccessfulLogin( U32 connectionId, bool isReloggedIn )
{
   ConnectionToUser* connection = GetUserConnection( connectionId );
   if( connection == NULL || connection->status != ConnectionToUser::LoginStatus_LoggedIn )
   {
      Log( "Login server: major problem with successful login", 4 );
      return false;
   }

   connection->loggedOutTime = 0;// for relogin, we need this to be cleared.

   string   username =        connection->username;
   string   userUuid =        connection->userUuid;
   string   email =           connection->email;
   string   lastLoginTime =   connection->lastLoginTime;
   bool     active =          connection->active;
   string   passwordHash =    connection->passwordHash;
   string   userId =          connection->id;
   U8       gameProductId =   connection->gameProductId;
   string   loginKey =        connection->loginKey;

   connection->productFilterNames.clear();
   connection->productsWaitingForInsertionToDb.clear();

   if( loginKey.size() == 0 )
   {
      U32 hash = static_cast<U32>( GenerateUniqueHash( boost::lexical_cast< string >( connectionId ) + email ) );
      loginKey = GenerateUUID( GetCurrentMilliseconds() + hash );
      connection->loginKey = loginKey;
   }


   PacketLoginToGateway* loginStatus = new PacketLoginToGateway();
   if( connection != NULL )
   {
      loginStatus->username = username;
      loginStatus->uuid = userUuid;
      loginStatus->lastLogoutTime = lastLoginTime;
      loginStatus->loginKey = loginKey;
   }
   loginStatus->wasLoginSuccessful = true;


   SendPacketToGateway( loginStatus, connectionId );

   RequestListOfGames( connectionId, userUuid );

   RequestListOfProducts( connectionId, userUuid );

   //This is where we inform all of the games that the user is logged in.

   return SendLoginStatusToOtherServers( username, userUuid, connectionId, gameProductId, lastLoginTime, active, email, passwordHash, userId, loginKey, true, false );
}

//------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------

bool  DiplodocusLogin:: RequestListOfGames( U32 connectionId, const string& userUuid )
{
   return false;// not working this way anymore
   if( userUuid.size() && connectionId != 0 )
   {
      PacketDbQuery* dbQuery = new PacketDbQuery;
      dbQuery->id =     connectionId ;
      dbQuery->lookup = QueryType_UserListOfGame;

      string queryString = "SELECT game.uuid, game.name FROM game INNER JOIN user_join_game AS user_game ON game.uuid=user_game.game_uuid WHERE user_game.user_uuid = '%s'";
      dbQuery->escapedStrings.insert( userUuid );
      dbQuery->query =  queryString;

      return AddQueryToOutput( dbQuery );
   }
   return false;
}

bool  DiplodocusLogin:: RequestListOfProducts( U32 connectionId, const string& userUuid )
{
   //return false;// not working this way anymore
   if( userUuid.size() && connectionId != 0 )
   {
      PacketDbQuery* dbQuery = new PacketDbQuery;
      dbQuery->id =     connectionId ;
      dbQuery->lookup = QueryType_UserListOfUserProducts;

      string queryString = "SELECT product.product_id, filter_name FROM product INNER JOIN user_join_product AS user_join_product ON product.uuid=user_join_product.product_id WHERE user_join_product.user_uuid='%s'";
      dbQuery->query =  queryString;
      dbQuery->escapedStrings.insert( userUuid );

      return AddQueryToOutput( dbQuery );
   }
   return false;
}

//---------------------------------------------------------------

bool  DiplodocusLogin:: SendLoginStatusToOtherServers( const string& username, 
                                                     const string& userUuid, 
                                                     U32 connectionId, 
                                                     U8 gameProductId, 
                                                     const string& lastLoginTime, 
                                                     bool isActive, 
                                                     const string& email, 
                                                     const string& passwordHash, 
                                                     const string& userId, 
                                                     const string& loginKey,
                                                     bool isLoggedIn, 
                                                     bool wasDisconnectedByError )
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
         prepareForUser->loginKey = loginKey;

         packetToSend = prepareForUser;
         
      }
      else
      {
         PacketPrepareForUserLogout* logout = new PacketPrepareForUserLogout;
         logout->uuid = userUuid;
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

bool     DiplodocusLogin:: SendListOfUserProductsToOtherServers( const string& userUuid, U32 connectionId, const vector< string >& productNames )
{
   PacketListOfUserProductsS2S packet;

   

   return false;
}

//---------------------------------------------------------------

void     DiplodocusLogin:: UpdateUserRecord( CreateAccountResultsAggregator* aggregator )
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

bool     DiplodocusLogin:: AddOutputChainData( BasePacket* packet, U32 connectionId )
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

                  if( SuccessfulLogin( connectionId ) == true )
                  {
                     UpdateLastLoggedInTime( dbResult->id ); // update the user logged in time
                  }
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
            case QueryType_UserListOfUserProducts:
               {
                  if( dbResult->successfulQuery == false )
                  {
                     string str = "Query failed looking up a user products ";
                     str += connection->username;
                     str += ", uuid: ";
                     str += connection->userUuid;
                     Log( str, 4 );
                     ForceUserLogoutAndBlock( connectionId );
                     return false;
                  }

                  StoreListOfUsersProductsFromDB( connectionId, dbResult );
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
            case QueryType_LoadProductInfo:
               {
                  if( dbResult->successfulQuery == false )
                  {
                     string str = "Initialization failed: table does not exist ";
                     return false;
                  }
                  StoreAllProducts( dbResult );
               }
               break;
            case QueryType_GetSingleProductInfo:
               {
                  if( dbResult->successfulQuery == false )
                  {
                     string str = "Product not found ";
                     str += dbResult->meta;
                     Log( str, 4 );
                     return false;
                  }
                  StoreSingleProduct( dbResult );
               }
               break;
            case QueryType_GetProductListForUser:
               {
                  SendProductListResultToUser( connectionId, dbResult );
               }
               break;
         }
      }
   }
   return true;
}

//---------------------------------------------------------------

void     DiplodocusLogin:: StoreAllProducts( PacketDbQueryResult* dbResult )
{
   ProductTable            enigma( dbResult->bucket );

   ProductTable::iterator  it = enigma.begin();
   int numProducts = dbResult->bucket.bucket.size();
            
   while( it != enigma.end() )
   {
      ProductTable::row       row = *it++;

      ProductInfo productDefn;
      productDefn.productId  = boost::lexical_cast< int >( row[ TableProduct::Column_product_id ] );
      productDefn.uuid =                 row[ TableProduct::Column_uuid ];
      productDefn.name =                 row[ TableProduct::Column_name ];
      productDefn.filterName =           row[ TableProduct::Column_filter_name ];
      productDefn.Begindate =            row[ TableProduct::Column_begin_date ];
      string temp = row[ TableProduct::Column_product_type ];
      if( temp == "" )
         temp = "0";

      productDefn.productType  = boost::lexical_cast< int >( temp );
      

      m_productList.push_back( productDefn );
   }

   m_isInitialized = true;
}

//---------------------------------------------------------------

void     DiplodocusLogin:: StoreSingleProduct( PacketDbQueryResult* dbResult )
{
   ProductTable            enigma( dbResult->bucket );

   string filterName;
   string filterUuid;
   ProductTable::row row = *enigma.begin();
   //if( it != enigma.end() )
   {
      ProductInfo productDefn;
      productDefn.productId  = boost::lexical_cast< int >( row[ TableProduct::Column_product_id ] );
      productDefn.uuid =                 row[ TableProduct::Column_uuid ];
      productDefn.name =                 row[ TableProduct::Column_name ];
      productDefn.filterName =           row[ TableProduct::Column_filter_name ];
      productDefn.Begindate =            row[ TableProduct::Column_begin_date ];

      string temp = row[ TableProduct::Column_product_type ];
      if( temp == "" )
         temp = "0";
      productDefn.productType  = boost::lexical_cast< int >( temp );
      
      filterName = productDefn.filterName;
      filterUuid = productDefn.uuid;

      m_productList.push_back( productDefn );
   }

   // now that we've added the new product, see which users needed it.
   Threading::MutexLock locker( m_inputChainListMutex );
   UserConnectionMapIterator userIt = m_userConnectionMap.begin();
   while( userIt != m_userConnectionMap.end() )
   {
      vector< ProductInfo >& userProducts = userIt->second.productsWaitingForInsertionToDb;
      vector< ProductInfo >::iterator productIt = userProducts.begin();
      while( productIt != userProducts.end() )
      {
         if( productIt->filterName == filterName )
         {
            double price = productIt->price;
            WriteProductToUserRecord( filterUuid, userIt->second.userUuid, price );
            userProducts.erase( productIt );

            if( userProducts.size() == 0 )
            {
               SendListOfUserProductsToAssetServer( userIt->first );
            }
            break;
         }
         productIt ++;
      }


      userIt ++;
   }
   
}

//---------------------------------------------------------------

void     DiplodocusLogin:: SendProductListResultToUser( U32 connectionId, PacketDbQueryResult* dbResult )
{
   ConnectionToUser* connection = GetUserConnection( connectionId );
   if( connection )
   {
      ProductTable            enigma( dbResult->bucket );

      ProductTable::iterator  it = enigma.begin();
      int numProducts = dbResult->bucket.bucket.size();
               
      PacketListOfUserPurchases* purchasePacket = new PacketListOfUserPurchases();
      purchasePacket->isAllProducts = false;
      if( dbResult->meta == "all" )
         purchasePacket->isAllProducts = true;
      purchasePacket->platformId = 0;


      while( it != enigma.end() )
      {
         ProductTable::row       row = *it++;

         PurchaseEntry pe;
         pe.productStoreId = row[ TableProduct::Column_name ];
         pe.name = row[ TableProduct::Column_filter_name ];
         purchasePacket->purchases.push_back( pe );
        /* ProductInfo productDefn;
         productDefn.productId  = boost::lexical_cast< int >( row[ TableProduct::Column_product_id ] );
         productDefn.uuid =                 row[ TableProduct::Column_uuid ];
         productDefn.name =                 row[ TableProduct::Column_name ];
         productDefn.filterName =           row[ TableProduct::Column_filter_name ];
         productDefn.Begindate =            row[ TableProduct::Column_begin_date ];
         string temp = row[ TableProduct::Column_product_type ];
         if( temp == "" )
            temp = "0";

         productDefn.productType  = boost::lexical_cast< int >( temp );
         

         m_productList.push_back( productDefn );*/
      }

      SendPacketToGateway( purchasePacket, connectionId );
   }
   
}

//---------------------------------------------------------------

void     DiplodocusLogin:: WriteProductToUserRecord( const string& productFilterName, const string& uuid, double pricePaid )
{
   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =           0;
   dbQuery->lookup =       QueryType_AddProductInfoToUser;
   dbQuery->meta =         productFilterName;
   dbQuery->serverLookup = 0;
   dbQuery->isFireAndForget = true;

   dbQuery->query = "INSERT INTO user_join_product VALUES( NULL, '%s', '%s', NULL, ";
   dbQuery->query += boost::lexical_cast< string >( pricePaid );
   dbQuery->query += ", 1, NULL)";

   dbQuery->escapedStrings.insert( uuid );
   dbQuery->escapedStrings.insert( productFilterName );

   AddQueryToOutput( dbQuery );
}

//---------------------------------------------------------------

void     DiplodocusLogin:: StoreListOfUsersProductsFromDB( U32 connectionId, PacketDbQueryResult* dbResult )
{
   ConnectionToUser* connection = GetUserConnection( connectionId );
   if( connection )
   {
      bool  didFindGameProduct = false;
      int   loggedInGameProductId = connection->gameProductId;
      // verify that this product is owned by the player and if not, then add an entry

      KeyValueParser  enigma( dbResult->bucket );
      KeyValueParser::iterator      it = enigma.begin();
      int   numProducts = dbResult->bucket.bucket.size();
      while( it != enigma.end() )
      {
         KeyValueParser::row       row = *it++;

         int productId = boost::lexical_cast< int> ( row[ TableKeyValue::Column_key ] );
         if( loggedInGameProductId == productId )
         {
            didFindGameProduct = true;
         }
         connection->productFilterNames.push_back( row[ TableKeyValue::Column_value ] );
      }

      if( didFindGameProduct == false )
      {
         AddGameProductIdToUserProducts( connectionId );
      }
   }

   RequestListOfProductsFromClient( connectionId );
}

//---------------------------------------------------------------

void     DiplodocusLogin:: AddGameProductIdToUserProducts( U32 connectionId )
{
   ConnectionToUser* connection = GetUserConnection( connectionId );
   if( connection )
   {
      int   loggedInGameProductId = connection->gameProductId;
      const char* loggedInGameProductName = FindProductName( loggedInGameProductId );
      if( loggedInGameProductName == NULL )
      {
         cout << "Major error: user logging in with a product not identified" << endl;
         return;
      }

      ProductInfo productInfo;
      bool found = false;
      vector< ProductInfo >::iterator it = m_productList.begin();
      while( it != m_productList.end() )
      {
         if( it->productId == loggedInGameProductId )
         {
            found = true;
            productInfo = *it;
            break;
         }
         it++;
      }

      if( found == false )
      {
         cout << "Major error: user logging in with a product not in our list of loaded products" << endl;
         return;
      }

      PacketDbQuery* dbQuery = new PacketDbQuery;
      dbQuery->id =           0;
      dbQuery->lookup =       QueryType_AddProductInfoToUser;
      dbQuery->meta =         loggedInGameProductName;
      dbQuery->serverLookup = 0;
      dbQuery->isFireAndForget = true;


      dbQuery->query = "INSERT INTO user_join_product VALUES( NULL, '";
      dbQuery->query += connection->userUuid;
      dbQuery->query += "', '";
      dbQuery->query += productInfo.uuid;
      dbQuery->query += "', NULL, 0, 0, 0 )";

      AddQueryToOutput( dbQuery );
   }
}

//---------------------------------------------------------------

void     DiplodocusLogin:: RequestListOfProductsFromClient( U32 connectionId )
{
   PacketRequestListOfUserPurchases* purchaseRequest = new PacketRequestListOfUserPurchases;
   SendPacketToGateway( purchaseRequest, connectionId );
}

//---------------------------------------------------------------

void     DiplodocusLogin:: AddNewProductToDb( const PurchaseEntry& product )
{
   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =           0;
   dbQuery->lookup =       QueryType_AddProductInfo;
   dbQuery->meta =         product.name;
   dbQuery->serverLookup = 0;
   dbQuery->isFireAndForget = true;

   U32 hash = static_cast<U32>( GenerateUniqueHash( boost::lexical_cast< string >( product.name ) ) );
   string newUuid = GenerateUUID( hash );

   std::string lowercase_username = product.name; 
   std::transform( lowercase_username.begin(), lowercase_username.end(), lowercase_username.begin(), ::tolower );

   dbQuery->query = "INSERT INTO product VALUES( NULL, 0, '";// new products haven an id of 0
   dbQuery->query += newUuid;
   dbQuery->query += "', '%s', '%s', NULL, 0, NULL)";

   dbQuery->escapedStrings.insert( product.productStoreId );
   dbQuery->escapedStrings.insert( product.name );

   AddQueryToOutput( dbQuery );

   //--------------------------------------------------

   // pull back the results so that we have the index.
   dbQuery = new PacketDbQuery;
   dbQuery->id =           0;
   dbQuery->lookup =       QueryType_GetSingleProductInfo;
   dbQuery->meta =         product.name;
   dbQuery->serverLookup = 0;

   dbQuery->query = "SELECT * FROM product WHERE filter_name = '%s'";
   dbQuery->escapedStrings.insert( product.name );

   AddQueryToOutput( dbQuery );
}

//---------------------------------------------------------------

void     DiplodocusLogin:: SendListOfUserProductsToAssetServer( U32 connectionId )
{
   ConnectionToUser* connection = GetUserConnection( connectionId );
   if( connection )
   {
      PacketListOfUserProductsS2S* packet = new PacketListOfUserProductsS2S;
      packet->uuid = connection->userUuid;
      vector< string >::iterator it =  connection->productFilterNames.begin();
      while( it != connection->productFilterNames.end() )
      {
         packet->products.insert( *it++ );
      }


      BaseOutputContainer::iterator itOutputs = m_listOfOutputs.begin();
      while( itOutputs != m_listOfOutputs.end() )
      {
         ChainedInterface<BasePacket*>*  outputPtr = (*itOutputs).m_interface;

         if( outputPtr->AddOutputChainData( packet, m_chainId ) == true )
         {
            return;
         }
         itOutputs++;
      }

      delete packet;
   }
}

//---------------------------------------------------------------

void     DiplodocusLogin:: LoadInitializationData()
{
   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =           0;
   dbQuery->lookup =       QueryType_LoadProductInfo;
   dbQuery->meta =         "";
   dbQuery->serverLookup = 0;

   dbQuery->query = "SELECT * FROM product";

   AddQueryToOutput( dbQuery );
}

//---------------------------------------------------------------

int      DiplodocusLogin:: CallbackFunction()
{
   SendServerIdentification();

   if( m_isInitializing == false )
   {
      if( m_isInitialized == false )
      {
         LoadInitializationData();
         m_isInitializing = true;
      }
   }

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
