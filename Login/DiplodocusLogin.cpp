// DiplodocusLogin.cpp

#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
using boost::format;

#include "../NetworkCommon/Utils/TableWrapper.h"
#include "../NetworkCommon/Utils/Utils.h"

#include "../NetworkCommon/Database/StringLookup.h"

#include "../NetworkCommon/Packets/ServerToServerPacket.h"
#include "../NetworkCommon/Packets/GamePacket.h"
#include "../NetworkCommon/Packets/DbPacket.h"
#include "../NetworkCommon/Packets/CheatPacket.h"
#include "../NetworkCommon/Packets/LoginPacket.h"
#include "../NetworkCommon/Packets/PacketFactory.h"
#include "../NetworkCommon/Utils/CommandLineParser.h"

#include "../NetworkCommon/Logging/server_log.h"

#include <boost/lexical_cast.hpp>


#include "DiplodocusLogin.h"
#include "FruitadensLogin.h"

#define _DEMO_13_Aug_2013


//////////////////////////////////////////////////////////

DiplodocusLogin:: DiplodocusLogin( const string& serverName, U32 serverId )  : 
                  Queryer(),
                  StatTrackingConnections(),
                  Diplodocus< KhaanLogin >( serverName, serverId, 0, ServerType_Login ), 
                  m_isInitialized( false ), 
                  m_isInitializing( false ),
                  m_autoAddProductFromWhichUsersLogin( true ),
                  m_numRelogins( 0 ),
                  m_numFailedLogins( 0 ),
                  m_numSuccessfulLogins( 0 ),
                  m_totalUserLoginSeconds( 0 ),
                  m_totalNumLogouts( 0 )
{
   SetSleepTime( 19 );
   LogOpen();
   LogMessage( LOG_PRIO_INFO, "Login::Login server created" );
   cout << "Login::Login server created" << endl;

   vector< string > stringCategories;
   stringCategories.push_back( string( "product" ) );
   stringCategories.push_back( string( "sale" ) );
   m_stringLookup = new StringLookup( QueryType_ProductStringLookup, this, stringCategories );

   time( &m_timestampHourlyStatServerStatisics );
   m_timestampDailyStatServerStatisics = m_timestampHourlyStatServerStatisics;

   m_timestampHourlyStatServerStatisics = ZeroOutMinutes( m_timestampHourlyStatServerStatisics );
   m_timestampDailyStatServerStatisics = ZeroOutHours( m_timestampDailyStatServerStatisics );
}


//---------------------------------------------------------------

void     DiplodocusLogin:: ServerWasIdentified( IChainedInterface* khaan )
{
   BasePacket* packet = NULL;
   PackageForServerIdentification( m_serverName, m_localIpAddress, m_serverId, m_listeningPort, m_gameProductId, m_isGame, m_isControllerApp, true, m_isGateway, &packet );
   ChainedType* localKhaan = static_cast< ChainedType* >( khaan );
   localKhaan->AddOutputChainData( packet, 0 );
   m_clientsNeedingUpdate.push_back( localKhaan->GetChainedId() );
}

//---------------------------------------------------------------

bool     DiplodocusLogin:: AddInputChainData( BasePacket* packet, U32 connectionId )
{
   // if packet is a login or a logout packet we'll handle it, otherwise.. no deal.
   // all packets coming in should be from the gateway only 

   //LogMessage( LOG_PRIO_INFO, "Login::Data in" );

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
               LogUserIn( login->userName, login->password, login->loginKey, login->gameProductId, userConnectionId );
            }
            break;
         case PacketLogin::LoginType_Logout:
            {
               PacketLogout* logout = static_cast<PacketLogout*>( actualPacket );
               //UpdateLastLoggedOutTime( userConnectionId );
               LogUserOut( userConnectionId, logout->wasDisconnectedByError );
            }
            break;
         case PacketLogin::LoginType_CreateAccount:
            {
               PacketCreateAccount* createAccount = static_cast<PacketCreateAccount*>( actualPacket );
               CreateUserAccount( userConnectionId, createAccount->userEmail, createAccount->password, createAccount->userName, createAccount->deviceAccountId, createAccount->deviceId, createAccount->languageId, createAccount->gameProductId );
            }
            break;
         case PacketLogin::LoginType_ListOfAggregatePurchases:
            {
               PacketListOfUserAggregatePurchases* purchases = static_cast<PacketListOfUserAggregatePurchases*>( actualPacket );
               StoreUserPurchases( userConnectionId, purchases );
            }
            break;
         case PacketLogin::LoginType_RequestListOfPurchases:
            {
               PacketListOfUserPurchasesRequest* purchaseRequest = static_cast<PacketListOfUserPurchasesRequest*>( actualPacket );
               RequestListOfPurchases( userConnectionId, purchaseRequest );
            }
            break;
         case PacketLogin::LoginType_AddPurchaseEntry:
            {
               PacketAddPurchaseEntry* addPurchase = static_cast<PacketAddPurchaseEntry*>( actualPacket );
               AddPurchase( userConnectionId, addPurchase );
            }
            break;
         case PacketLogin::LoginType_RequestUserProfile:
            {
               PacketRequestUserProfile* profileRequest = static_cast<PacketRequestUserProfile*>( actualPacket );
               RequestProfile( userConnectionId, profileRequest );
            }
            break;
         case PacketLogin::LoginType_UpdateUserProfile:
            {
               PacketUpdateUserProfile* updateProfileRequest = static_cast<PacketUpdateUserProfile*>( actualPacket );
               UpdateProfile( userConnectionId, updateProfileRequest );
            }
            break;
         case PacketLogin::LoginType_RequestListOfProducts:
            {
               PacketRequestListOfProducts* purchaseRequest = static_cast<PacketRequestListOfProducts*>( actualPacket );
               HandleRequestListOfProducts( userConnectionId, purchaseRequest );
            }
            break;

         case PacketLogin::LoginType_RequestOtherUserProfile:
            {
               PacketRequestOtherUserProfile* profileRequest = static_cast<PacketRequestOtherUserProfile*>( actualPacket );
               RequestOthersProfile( userConnectionId, profileRequest );
            }
            break;
         case PacketLogin::LoginType_UpdateSelfProfile:
            {
               PacketUpdateSelfProfile* updateProfileRequest = static_cast<PacketUpdateSelfProfile*>( actualPacket );
               UpdateProfile( userConnectionId, updateProfileRequest );
            }
            break;
         case PacketLogin::LoginType_EchoToServer:
            {
               EchoHandler( userConnectionId );
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
      ChainType* outputPtr = static_cast< ChainType*> ( (*itOutputs).m_interface );
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

bool     DiplodocusLogin:: LogUserIn( const string& userName, const string& password, const string& loginKey, U8 gameProductId, U32 connectionId )
{
   cout << endl << "***********************" << endl;
   cout << "attempt to login user: "<< userName << ", pwHash:" << password << " for game id=" << (int) gameProductId << " and conn: " << connectionId << endl;
   cout << "***********************" << endl;
   if( IsUserConnectionValid( connectionId ) )
   {
      // should we boot this user for hacking? Or is it bad code?
      Log( "Second attempt was made at login", 4 );
      //InformUserOfSuccessfulLogout();// someone is hacking our server
      ForceUserLogoutAndBlock( connectionId );
   }

   if( userName.size() == 0 )
   {
      // should we boot this user for hacking? Or is it bad code?
      Log( "invalid attempt at login: userName was empty", 4 );
      return false;
   }

   // Before we add the user, let's verify that s/he isn't already logged in with a different connectionId. 

   U32 oldConnectionId = FindUserAlreadyInGame( userName, gameProductId );
   if( oldConnectionId != 0 )
   {
      // should we boot this user for hacking? Or is it bad code?
      Log( "Second login from the same product attempt was made", 4 );
      Log( userName.c_str(), 4 );
      ForceUserLogoutAndBlock( oldConnectionId );// force disconnect
      //return false;

      Log( "Reconnect user", 1 );

      ReinsertUserConnection( oldConnectionId, connectionId );// and we remove the old connection

      ConnectionToUser* connection = GetUserConnection( connectionId );

      if( connection )
      {
         connection->ClearLoggingOutStatus();
         if( connection->CanContinueLogginIn() == true )
         {
            const bool isLoggedIn = true; 
            const bool wasDisconnectedByError = false;

            SendLoginStatusTo_Non_GameServers( connection->m_userName, 
                                             connection->m_userUuid, 
                                             connectionId, 
                                             gameProductId, 
                                             connection->m_lastLoginTime, 
                                             connection->m_isActive, 
                                             connection->m_email, 
                                             connection->m_passwordHash, 
                                             connection->m_id, 
                                             connection->m_loginKey, 
                                             connection->m_languageId, 
                                             isLoggedIn, 
                                             wasDisconnectedByError );

            if( connection->SuccessfulLoginFinished( connectionId, true ) == true )
            {
               m_uniqueUsers.insert( connection->m_userUuid );
               UpdateLastLoggedInTime( connectionId ); // update the user logged in time
            }
            SendLoginStatusTo_GameServers( connection->m_userName, 
                                             connection->m_userUuid, 
                                             connectionId, 
                                             gameProductId, 
                                             connection->m_lastLoginTime, 
                                             connection->m_isActive, 
                                             connection->m_email, 
                                             connection->m_passwordHash, 
                                             connection->m_id, 
                                             connection->m_loginKey, 
                                             connection->m_languageId, 
                                             isLoggedIn, 
                                             wasDisconnectedByError );
            m_numSuccessfulLogins++;
            m_numRelogins ++;
         }
         else
         {
            connection->UpdateConnectionId( connectionId );
            m_numFailedLogins ++;
          /*  ForceUserLogoutAndBlock( connectionId );*/
            return LoadUserAccount( userName, password, loginKey, gameProductId, connectionId );
         }
      }
      else
      {
         cout << "Error: could not find user by connectionId after verifying that s/he exists" << endl;
      }
   }
   else
   {
      return LoadUserAccount( userName, password, loginKey, gameProductId, connectionId );
   }

   return false;
}

//---------------------------------------------------------------

bool  DiplodocusLogin:: LoadUserAccount( const string& userName, const string& password, const string& loginKey, U8 gameProductId, U32 connectionId )
{
   ConnectionToUser conn( userName, password, loginKey );
   conn.m_gameProductId = gameProductId;
   conn.m_connectionId = connectionId;
   AddUserConnection( UserConnectionPair( connectionId, conn ) );

   //*********************************************************************************
   // perhaps some validation here is in order like is this user valid based on the key
   //*********************************************************************************

   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =           connectionId;
   dbQuery->lookup =       QueryType_UserLoginInfo;
   dbQuery->meta =         userName;
   dbQuery->serverLookup = gameProductId;

   dbQuery->query = "SELECT * FROM users JOIN user_profile ON users.user_id=user_profile.user_id WHERE users.user_email='%s' AND users.user_pw_hash='%s'";
   dbQuery->escapedStrings.insert( userName );
   dbQuery->escapedStrings.insert( password );

   return AddQueryToOutput( dbQuery );
}

//---------------------------------------------------------------

bool     DiplodocusLogin:: HandleUserLoginResult( U32 connectionId, const PacketDbQueryResult* dbResult )
{
   ConnectionToUser* connection = GetUserConnection( connectionId );
   if( connection )
   {
      if( dbResult->successfulQuery == false || dbResult->GetBucket().size() == 0 )
      {
         connection->ClearLoggingOutStatus();
         Log( "Connect user: Cannot continue logging in", 1 );
         Log( "User record not fouond", 1 );
         return false;
      }
      // ---------------------------------
      // lots happening here
      connection->LoginResult( dbResult );
      // ---------------------------------

      if( connection->CanContinueLogginIn() == false )
      {
         connection->ClearLoggingOutStatus();
         Log( "Connect user: Cannot continue logging in", 1 );
         return false;
      }

      if( dbResult->successfulQuery == true && dbResult->GetBucket().size() > 0 )
      {
         const bool isLoggedIn = true; 
         const bool wasDisconnectedByError = false;

        /* SendLoginStatusToOtherServers( connection->m_userName, 
                                       connection->m_userUuid, 
                                       connectionId, 
                                       connection->m_gameProductId, 
                                       connection->m_lastLoginTime, 
                                       connection->m_isActive, 
                                       connection->m_email, 
                                       connection->m_passwordHash, 
                                       connection->m_id,
                                       connection->m_loginKey, 
                                       connection->m_languageId, 
                                       isLoggedIn, 
                                       wasDisconnectedByError);*/

         SendLoginStatusTo_Non_GameServers( connection->m_userName, 
                                             connection->m_userUuid, 
                                             connectionId, 
                                             connection->m_gameProductId, 
                                             connection->m_lastLoginTime, 
                                             connection->m_isActive, 
                                             connection->m_email, 
                                             connection->m_passwordHash, 
                                             connection->m_id, 
                                             connection->m_loginKey, 
                                             connection->m_languageId, 
                                             isLoggedIn, 
                                             wasDisconnectedByError );

         cout << "--------------------------------------------" << endl;
         cout << "User login: " << connection->m_userName << endl;
         cout << "User uuid: " << connection->m_userUuid << endl;
         cout << "lastLoginTime = " << connection->m_lastLoginTime << endl;
         cout << "--------------------------------------------" << endl;
         if( connection->SuccessfulLoginFinished( connectionId, false ) == true )
         {
            Log( "User connection success", 1 );
            m_uniqueUsers.insert( connection->m_userUuid );

            UpdateLastLoggedInTime( dbResult->id ); // update the user logged in time
            SendLoginStatusTo_GameServers( connection->m_userName, 
                                             connection->m_userUuid, 
                                             connectionId, 
                                             connection->m_gameProductId, 
                                             connection->m_lastLoginTime, 
                                             connection->m_isActive, 
                                             connection->m_email, 
                                             connection->m_passwordHash, 
                                             connection->m_id, 
                                             connection->m_loginKey, 
                                             connection->m_languageId, 
                                             isLoggedIn, 
                                             wasDisconnectedByError );
            return true;
         }
         else
         {
            Log( "User connection failure", 1 );
         }         
      }
   }

   return false;
}

//---------------------------------------------------------------

bool     DiplodocusLogin:: HandleAdminRequestUserProfile( U32 connectionId, const PacketDbQueryResult* dbResult )
{
   ConnectionToUser* connection = GetUserConnection( connectionId );
   if( connection )
   {
      return connection->HandleAdminRequestUserProfile( dbResult );
   }

   return true;// i doesn't matter
}

//---------------------------------------------------------------

bool     DiplodocusLogin:: LogUserOut( U32 connectionId, bool wasDisconnectedByError )
{
   ConnectionToUser* connection = GetUserConnection( connectionId );
   if( connection )
   {
      return connection->BeginLogout( wasDisconnectedByError );
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
      const bool isLoggedIn = false; 
      bool result = connection->FinalizeLogout();
      result = result;
      time_t currentTime;
      time( &currentTime );

      m_totalUserLoginSeconds += static_cast<int>( difftime( currentTime, connection->GetLoginTime() ) );
      m_totalNumLogouts ++; 

      SendLoginStatusToOtherServers( connection->m_userName, 
                                    connection->m_userUuid, 
                                    connectionId, 
                                    connection->m_gameProductId, 
                                    connection->m_lastLoginTime, 
                                    connection->m_isActive, 
                                    connection->m_email, 
                                    connection->m_passwordHash, 
                                    connection->m_id, 
                                    connection->m_loginKey,
                                    connection->m_languageId, 
                                    isLoggedIn, 
                                    wasDisconnectedByError );
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
   pair.second.SetManager( this );
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
      if( temp->second.IsReadyToBeCleanedUp() == true && // do not remove in the middle of logging out
         temp->second.m_loggedOutTime != 0 )
      {
         const int normalExpireTime = 3; // seconds
         if( difftime( testTimer, temp->second.m_loggedOutTime ) >= normalExpireTime )
         {
            FinalizeLogout( temp->first, false );
            m_userConnectionMap.erase( temp );
         }
      }
   }
}

//---------------------------------------------------------------

U32     DiplodocusLogin:: FindUserAlreadyInGame( const string& userName, U8 gameProductId )
{
   Threading::MutexLock locker( m_mutex );

   UserConnectionMapIterator it = m_userConnectionMap.begin();
   while( it != m_userConnectionMap.end() )
   {
      UserConnectionPair pairObj = *it++;
      ConnectionToUser& conn = pairObj.second;
      if( conn.m_gameProductId == gameProductId && // optimized for simplest test first
         
         ( conn.m_email == userName || conn.m_userName == userName ) )// we use these interchangably right now.
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

bool        DiplodocusLogin:: CreateUserAccount( U32 connectionId, const string& email, const string& password, const string& userName, const string& deviceAccountId, const string& deviceId, U8 languageId, U8 gameProductId )
{
   if( IsUserConnectionValid( connectionId ) )
   {
      string str = "ERROR: Login server, user attempted to create a second account while logged in, userName: ";
      str += userName;
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

   std::string lowercase_username = userName; 
   std::transform( lowercase_username.begin(), lowercase_username.end(), lowercase_username.begin(), ::tolower );
   std::string lowercase_useremail = email; 
   std::transform( lowercase_useremail.begin(), lowercase_useremail.end(), lowercase_useremail.begin(), ::tolower );

   std::size_t found = lowercase_username.find( "playdek" );
   if (found!=std::string::npos)
   {
      SendErrorToClient( connectionId, PacketErrorReport::ErrorType_CreateFailed_DisallowedUsername );
      return false;
   }

   CreateAccountResultsAggregator* aggregator = new CreateAccountResultsAggregator( connectionId, lowercase_useremail, password, userName, deviceAccountId, deviceId, languageId, gameProductId ); 
   LockMutex();
   m_userAccountCreationMap.insert( UserCreateAccountPair( connectionId, aggregator ) );
   UnlockMutex();

   U64 passwordHash = 0;
   ConvertFromString( password, passwordHash );
   
   string queryInvalidUserName = "SELECT id FROM invalid_username WHERE user_name_match='%s'";
   string queryUsers = "SELECT * FROM users WHERE user_name='%s' OR user_email='%s' OR user_gamekit_hash='%s'";
   string queryTempUsers = "SELECT * FROM user_temp_new_user WHERE user_name='%s' OR user_email='%s' OR user_gamekit_hash='%s'";

   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =           connectionId;
   dbQuery->lookup =       QueryType_LookupUserNameForInvalidName;
   dbQuery->meta =         userName;
   dbQuery->serverLookup = gameProductId;

   dbQuery->query = queryInvalidUserName;
   dbQuery->escapedStrings.insert( userName );
   AddQueryToOutput( dbQuery );

   dbQuery = new PacketDbQuery;
   dbQuery->id =           connectionId;
   dbQuery->lookup =       QueryType_LookupUserByUsernameOrEmail;
   dbQuery->meta =         userName;
   dbQuery->serverLookup = gameProductId;

   dbQuery->query = queryUsers;
   dbQuery->escapedStrings.insert( userName );
   dbQuery->escapedStrings.insert( email );
   dbQuery->escapedStrings.insert( deviceAccountId );
   AddQueryToOutput( dbQuery );

   dbQuery = new PacketDbQuery;
   dbQuery->id =           connectionId;
   dbQuery->lookup =       QueryType_LookupTempUserByUsernameOrEmail;
   dbQuery->meta =         userName;
   dbQuery->serverLookup = gameProductId;

   dbQuery->query = queryTempUsers;
   dbQuery->escapedStrings.insert( userName );
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
	     int index = static_cast< int >( it - m_productList.begin() );
         return index;
      }
      it++;
   }
   return -1;
}


bool        DiplodocusLogin:: FindProductByLookupName( const string& lookupName, ProductInfo& productDefn )
{
   ProductList::iterator it = m_productList.begin();
   while( it != m_productList.end() )
   {
      if( it->lookupName == lookupName )   // NOTE: these names may vary
      {
	      productDefn = *it;
         return true;
      }
      it++;
   }
   return false;
}

bool        DiplodocusLogin:: FindProductByUuid( const string& uuid, ProductInfo& returnPi  )
{
   vector< ProductInfo >::iterator it = m_productList.begin();
   while( it != m_productList.end() )
   {
      if( it->uuid == uuid )
      {
         returnPi = *it;
         return true;
      }
      it++;
   }

   return false;
}

bool        DiplodocusLogin:: GetProductByIndex( int index, ProductInfo& returnPi )
{
   if( index > (int) m_productList.size() )
   {
      return false;
   }
   returnPi = m_productList[ index] ;
   return true;
}

bool        DiplodocusLogin:: GetProductByProductId( int productId, ProductInfo& returnPi  )
{
   vector< ProductInfo >::iterator it = m_productList.begin();
   while( it != m_productList.end() )
   {
      if( it->productId == productId )
      {
         returnPi = *it;
         return true;
      }
      it++;
   }

   return false;
}

//---------------------------------------------------------------

// how can we trust these values?
bool        DiplodocusLogin:: StoreUserPurchases( U32 connectionId, const PacketListOfUserAggregatePurchases* deviceReportedPurchases )
{
   Threading::MutexLock locker( m_inputChainListMutex );
   ConnectionToUser* connection = GetUserConnection( connectionId );
   if( connection == NULL )
   {
      cout << "Error: could not find user by connection id" << endl;
      return false;
   }

   if( connection->StoreUserPurchases( deviceReportedPurchases ) == false )
      return false;

   if( connection->productsWaitingForInsertionToDb.size() == 0 )
   {
      SendListOfUserProductsToAssetServer( connectionId );
      // SendListOfUserProductsToOtherServers
   }
   return false;
}

//---------------------------------------------------------------

bool   DiplodocusLogin:: RequestListOfPurchases( U32 connectionId, const PacketListOfUserPurchasesRequest* purchase )
{
   ConnectionToUser* connection = GetUserConnection( connectionId );
   if( connection == NULL || connection->status != ConnectionToUser::LoginStatus_LoggedIn )
   {
      Log( "Login server.RequestListOfPurchases: major problem logged in user", 4 );
      return false;
   }

   return connection->HandleRequestForListOfPurchases( purchase );

}

//---------------------------------------------------------------

bool   DiplodocusLogin:: AddPurchase( U32 connectionId, const PacketAddPurchaseEntry* purchase )
{
   ConnectionToUser* connection = GetUserConnection( connectionId );
   if( connection == NULL || connection->status != ConnectionToUser::LoginStatus_LoggedIn )
   {
      Log( "Login server.AddPurchase: major problem logged in user", 4 );
      return false;
   }

   return connection->AddPurchase( purchase );

}

//---------------------------------------------------------------

bool     DiplodocusLogin:: RequestProfile( U32 connectionId, const PacketRequestUserProfile* profileRequest )
{
   ConnectionToUser* connection = GetUserConnection( connectionId );
   if( connection == NULL || connection->status != ConnectionToUser::LoginStatus_LoggedIn )
   {
      Log( "Login server.RequestProfile: major problem logged in user", 4 );
      return false;
   }

   return connection->RequestProfile( profileRequest );
}

//---------------------------------------------------------------

bool     DiplodocusLogin:: UpdateProfile( U32 connectionId, const PacketUpdateUserProfile* profileRequest )
{
   ConnectionToUser* connection = GetUserConnection( connectionId );
   if( connection == NULL || connection->status != ConnectionToUser::LoginStatus_LoggedIn )
   {
      Log( "Login server.UpdateProfile: major problem logged in user", 4 );
      return false;
   }

   return connection->UpdateProfile( profileRequest );
}

//---------------------------------------------------------------

bool     DiplodocusLogin:: UpdateProfile( U32 connectionId, const PacketUpdateSelfProfile* profileRequest )
{
   ConnectionToUser* connection = GetUserConnection( connectionId );
   if( connection == NULL || connection->status != ConnectionToUser::LoginStatus_LoggedIn )
   {
      Log( "Login server.UpdateProfile: major problem logged in user", 4 );
      return false;
   }

   return connection->UpdateProfile( profileRequest );
}

//---------------------------------------------------------------

bool     DiplodocusLogin:: HandleRequestListOfProducts( U32 connectionId, PacketRequestListOfProducts* purchaseRequest )
{
   ConnectionToUser* connection = GetUserConnection( connectionId );
   if( connection == NULL || connection->status != ConnectionToUser::LoginStatus_LoggedIn )
   {
      Log( "Login server.HandleRequestListOfProducts: major problem logged in user", 4 );
      return false;
   }

   PacketRequestListOfProductsResponse* response = new PacketRequestListOfProductsResponse();
   response->platformId = purchaseRequest->platformId;
   ProductList::iterator it = m_productList.begin();
   while( it != m_productList.end() )
   {
      const ProductInfo& pi = *it ++;
      ProductBriefPacketed brief;
      brief.uuid = pi.uuid;
      brief.filterName = pi.filterName;
      brief.quantity = pi.quantity;
      response->products.push_back( brief );
   }

   SendPacketToGateway( response, connectionId );
   return true;
}

//---------------------------------------------------------------

bool  DiplodocusLogin:: RequestOthersProfile( U32 connectionId, const PacketRequestOtherUserProfile* profileRequest )
{
   ConnectionToUser* connection = GetUserConnection( connectionId );
   if( connection == NULL || connection->status != ConnectionToUser::LoginStatus_LoggedIn )
   {
      Log( "Login server.RequestProfile: major problem logged in user", 4 );
      return false;
   }

   return connection->RequestOthersProfile( profileRequest );
}


//---------------------------------------------------------------

bool     DiplodocusLogin::EchoHandler( U32 connectionId )
{
   ConnectionToUser* connection = GetUserConnection( connectionId );
   if( connection == NULL || connection->status != ConnectionToUser::LoginStatus_LoggedIn )
   {
      //Log( "Login server.RequestProfile: major problem logged in user", 4 );
      cout << "echo failed to find user" << endl;
      return false;
   }

   return connection->EchoHandler();
}


//---------------------------------------------------------------

ConnectionToUser*     DiplodocusLogin:: GetLoadedUserConnectionByUuid(const string & uuid )
{
   Threading::MutexLock locker( m_inputChainListMutex );
   UserConnectionMapIterator it = m_userConnectionMap.begin();
   while( it != m_userConnectionMap.end() )
   {
      if( it->second.m_userUuid == uuid )
         return &it->second;
      it++;
   }

   return NULL;
}; 

//---------------------------------------------------------------

bool   DiplodocusLogin:: HandleCheats( U32 connectionId, const PacketCheat* cheat )
{
   ConnectionToUser* connection = GetUserConnection( connectionId );
   if( connection == NULL || connection->status != ConnectionToUser::LoginStatus_LoggedIn )
   {
      Log( "Login server: major problem with cheats... user not set properly", 4 );
      SendErrorToClient( connectionId, PacketErrorReport::ErrorType_Cheat_BadPermissions );
      return false;
   }
   
   // 
   return connection->HandleCheats( cheat );
}

//---------------------------------------------------------------

bool     DiplodocusLogin:: UpdateProductFilterName( int index, const string& newFilterName )
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

   string                     userName;
   string                     uuid;
   string                     lastLoginTime;
   string                     email;
   bool                       active = false;
   string                     passwordHash = "0";
   string                     userId = "0";
   string                     loginKey = "";
   int                        languageId = 1;
   U8 gameProductId = 0;

   ConnectionToUser* connection = GetUserConnection( connectionId );
   if( connection )
   {
      userName =              connection->m_userName;
      uuid =                  connection->m_userUuid;
      lastLoginTime =         connection->m_lastLoginTime;
      connection->status =    ConnectionToUser::LoginStatus_Invalid;
      active =                connection->m_isActive;
      passwordHash =          connection->m_passwordHash;
      userId =                connection->m_id;
      email =                 connection->m_email;
      gameProductId =         connection->m_gameProductId;
      loginKey =              connection->m_loginKey;
      languageId =            connection->m_languageId;
   }

   // now disconnect him/her
   PacketLoginToGateway* loginStatus = new PacketLoginToGateway();
   loginStatus->userName = userName;
   loginStatus->uuid = uuid;
   loginStatus->lastLogoutTime = GetDateInUTC();
   loginStatus->loginKey = loginKey;
   loginStatus->languageId = connection->m_languageId;

   loginStatus->wasLoginSuccessful = false;
   loginStatus->adminLevel = 0;
   
   SendPacketToGateway( loginStatus, connectionId );
   const bool isLoggedIn = false; 
   const bool wasDisconnectedByError = false;

   SendLoginStatusToOtherServers( userName, uuid, connectionId, gameProductId, lastLoginTime, active, email, passwordHash, userId, loginKey, languageId, isLoggedIn, wasDisconnectedByError );

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
         ChainType* outputPtr = static_cast< ChainType*> ( (*itOutputs).m_interface );

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
      return connection->UpdateLastLoggedInTime();
   }
   return false;
}


//------------------------------------------------------------------------------------------------

bool  DiplodocusLogin:: UpdateLastLoggedOutTime( U32 connectionId )
{
   ConnectionToUser* connection = GetUserConnection( connectionId );
   if( connection )
   {
      return connection->UpdateLastLoggedOutTime();
   }

   return false;
}

//------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------
/*
bool     DiplodocusLogin:: HandleUserProfileFromDb( U32 connectionId, PacketDbQueryResult* dbResult )
{
   ConnectionToUser* connection = GetUserConnection( connectionId );
   if( connection == NULL )
   {
      Log( "Login server: major problem with successful login", 4 );
      return false;
   }

   if( connection->HandleUserProfileFromDb( dbResult ) == true )
   {
      string   userName =        connection->userName;
      string   userUuid =        connection->userUuid;
      string   email =           connection->email;
      string   lastLoginTime =   connection->lastLoginTime;
      bool     active =          connection->isActive;
      string   passwordHash =    connection->passwordHash;
      string   userId =          connection->id;
      U8       gameProductId =   connection->gameProductId;
      string   loginKey =        connection->loginKey;

      return SendLoginStatusToOtherServers( userName, userUuid, connectionId, gameProductId, lastLoginTime, active, email, passwordHash, userId, loginKey, true, false );
   }

   else return false;
}*/

//------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------
bool  DiplodocusLogin:: SendLoginStatus(  ChainType*  destinationServerPtr,
                                          const string& userName, 
                                          const string& userUuid, 
                                          U32 connectionId, 
                                          U8 gameProductId, 
                                          const string& lastLoginTime, 
                                          bool isActive, 
                                          const string& email, 
                                          const string& passwordHash, 
                                          const string& userId, 
                                          const string& loginKey,
                                          int languageId, 
                                          bool isLoggedIn, 
                                          bool wasDisconnectedByError )
{
   if( destinationServerPtr == NULL )
      return false;

   BasePacket* packetToSend = NULL;
   if( isLoggedIn )
   {
      PacketPrepareForUserLogin* prepareForUser = new PacketPrepareForUserLogin;
      prepareForUser->connectionId = connectionId;
      prepareForUser->userName = userName;
      prepareForUser->uuid = userUuid;
      prepareForUser->lastLoginTime = lastLoginTime;
      prepareForUser->gameProductId = gameProductId;

      prepareForUser->active = isActive;
      prepareForUser->email= email;
      prepareForUser->userId = boost::lexical_cast<U32>( userId );
      prepareForUser->password = passwordHash;
      prepareForUser->loginKey = loginKey;
      prepareForUser->languageId = languageId;

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

   if( destinationServerPtr->AddOutputChainData( packetToSend, m_chainId ) == false )
   {
      delete packetToSend;
   }

   return true;
}

//---------------------------------------------------------------

bool  DiplodocusLogin:: SendLoginStatusToOtherServers( const string& userName, 
                                                     const string& userUuid, 
                                                     U32 connectionId, 
                                                     U8 gameProductId, 
                                                     const string& lastLoginTime, 
                                                     bool isActive, 
                                                     const string& email, 
                                                     const string& passwordHash, 
                                                     const string& userId, 
                                                     const string& loginKey,
                                                     int languageId, 
                                                     bool isLoggedIn, 
                                                     bool wasDisconnectedByError )
{
   // send this to every other listening server
   BaseOutputContainer::iterator itOutputs = m_listOfOutputs.begin();
   //cout << "SendLoginStatusToOtherServers" << endl;
   while( itOutputs != m_listOfOutputs.end() )
   {
      ChainType*  outputPtr = static_cast< ChainType*> ( (*itOutputs).m_interface );
      itOutputs++;

      FruitadensLogin* login = static_cast< FruitadensLogin* >( outputPtr ); 
      SendLoginStatus( outputPtr, 
                        userName, 
                       userUuid, 
                       connectionId, 
                       gameProductId, 
                       lastLoginTime, 
                       isActive, 
                       email, 
                       passwordHash, 
                       userId, 
                       loginKey,
                       languageId, 
                       isLoggedIn, 
                       wasDisconnectedByError );
   }

   if( isLoggedIn )
   {
      m_numSuccessfulLogins++;
   }

   return true;
}

//---------------------------------------------------------------

bool  DiplodocusLogin:: SendLoginStatusTo_Non_GameServers( const string& userName, 
                                                     const string& userUuid, 
                                                     U32 connectionId, 
                                                     U8 gameProductId, 
                                                     const string& lastLoginTime, 
                                                     bool isActive, 
                                                     const string& email, 
                                                     const string& passwordHash, 
                                                     const string& userId, 
                                                     const string& loginKey,
                                                     int languageId, 
                                                     bool isLoggedIn, 
                                                     bool wasDisconnectedByError )
{
   // send this to every other listening server
   BaseOutputContainer::iterator itOutputs = m_listOfOutputs.begin();
   cout << "SendLoginStatusTo_Non_GameServers for user: " << userName << endl;
   while( itOutputs != m_listOfOutputs.end() )
   {
      ChainType*  outputPtr = static_cast< ChainType*> ( (*itOutputs).m_interface );
      itOutputs++;

      FruitadensLogin* login = static_cast< FruitadensLogin* >( outputPtr );      
      if( login->IsGameServer() == true )
      {
         continue;
      }
      SendLoginStatus( outputPtr, 
                        userName, 
                       userUuid, 
                       connectionId, 
                       gameProductId, 
                       lastLoginTime, 
                       isActive, 
                       email, 
                       passwordHash, 
                       userId, 
                       loginKey,
                       languageId, 
                       isLoggedIn, 
                       wasDisconnectedByError );
   }

   return true;
}

bool  DiplodocusLogin:: SendLoginStatusTo_GameServers( const string& userName, 
                                                     const string& userUuid, 
                                                     U32 connectionId, 
                                                     U8 gameProductId, 
                                                     const string& lastLoginTime, 
                                                     bool isActive, 
                                                     const string& email, 
                                                     const string& passwordHash, 
                                                     const string& userId, 
                                                     const string& loginKey,
                                                     int languageId, 
                                                     bool isLoggedIn, 
                                                     bool wasDisconnectedByError )
{
   // send this to every other listening server
   BaseOutputContainer::iterator itOutputs = m_listOfOutputs.begin();
   cout << "SendLoginStatusTo_GameServers for user: " << userName << endl;
   while( itOutputs != m_listOfOutputs.end() )
   {
      ChainType*  outputPtr = static_cast< ChainType*> ( (*itOutputs).m_interface );
      itOutputs++;

      FruitadensLogin* login = static_cast< FruitadensLogin* >( outputPtr );      
      if( login->IsGameServer() == false )
      {
         continue;
      }
      SendLoginStatus( outputPtr, 
                        userName, 
                       userUuid, 
                       connectionId, 
                       gameProductId, 
                       lastLoginTime, 
                       isActive, 
                       email, 
                       passwordHash, 
                       userId, 
                       loginKey,
                       languageId, 
                       isLoggedIn, 
                       wasDisconnectedByError );
   }

   return true;
}


//---------------------------------------------------------------
/*
bool     DiplodocusLogin:: SendListOfUserProductsToOtherServers( const string& userUuid, U32 connectionId, const vector< string >& productNames )
{
   ConnectionToUser* connection = GetUserConnection( connectionId );
   if( connection )
   {
      ////--------------
      fix me
//------------------------
      PacketListOfUserProductsS2S* packet = new PacketListOfUserProductsS2S;

      vector< string >::const_iterator it = productNames.begin();
      while( it != productNames.end() )
      {
         packet->products.insert( *it++ ) ;
      }
      packet->uuid = userUuid;

      BaseOutputContainer::iterator itOutputs = m_listOfOutputs.begin();
      while( itOutputs != m_listOfOutputs.end() )
      {
         ChainType*  outputPtr = static_cast< ChainType*> ( (*itOutputs).m_interface );

         if( outputPtr->AddOutputChainData( packet, m_chainId ) == true )
         {
            return true;
         }
         itOutputs++;
      }

      delete packet;   
   }

   return false;
}*/


//---------------------------------------------------------------

bool     DiplodocusLogin:: SendPacketToOtherServer( BasePacket* packet, U32 connectionId )
{
   ConnectionToUser* connection = GetUserConnection( connectionId );
   if( connection )
   {
      BaseOutputContainer::iterator itOutputs = m_listOfOutputs.begin();
      while( itOutputs != m_listOfOutputs.end() )
      {
         ChainType*  outputPtr = static_cast< ChainType*> ( (*itOutputs).m_interface );

         if( outputPtr->AddOutputChainData( packet, m_chainId ) == true )
         {
            // this will be cleaned up much later
            //PacketFactory factory;
            //factory.CleanupPacket( packet );  
            return true;
         }
         itOutputs++;
      }
   }

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

// data going out can go only a few directions
// coming from the DB, we can have a result or possibly a different packet meant for a single chat UserConnection
// otherwise, coming from a UserConnection, to go out, it will already be packaged as a Gateway Wrapper and then 
// we simply send it on.
bool     DiplodocusLogin::AddOutputChainData( BasePacket* packet, U32 connectionId ) 
{
   if( packet->packetType == PacketType_ServerJobWrapper )
   {
      return HandlePacketToOtherServer( packet, connectionId );
   }

   if( packet->packetType == PacketType_DbQuery )
   {
      Threading::MutexLock locker( m_mutex );
      if( packet->packetSubType == BasePacketDbQuery::QueryType_Result )
      {
         PacketDbQueryResult* result = static_cast<PacketDbQueryResult*>( packet );
         m_dbQueries.push_back( result );
         if( result->customData != NULL )
            cout << "AddOutputChainData: Non-null custom data " << endl;
      }
      return true;
   }
   
   return false;
}


//---------------------------------------------------------------

void     DiplodocusLogin::UpdateDbResults()
{
   PacketFactory factory;

   m_mutex.lock();
   deque< PacketDbQueryResult* > tempQueue = m_dbQueries;
   m_dbQueries.clear();
   m_mutex.unlock();

   deque< PacketDbQueryResult* >::iterator it = tempQueue.begin();
   while( it != tempQueue.end() )
   {
      PacketDbQueryResult* result = *it++;
      if( result->customData != NULL )
            cout << "UpdateDbResults: Non-null custom data " << endl;
      BasePacket* packet = static_cast<BasePacket*>( result );

      HandleDbResult( result );
      factory.CleanupPacket( packet );
   }
}

//---------------------------------------------------------------

bool     DiplodocusLogin:: HandleDbResult( PacketDbQueryResult* dbResult )
{
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
         Threading::MutexLock locker( m_inputChainListMutex );
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
         str = "was apparently requested or at least it was not stored properly: userName was :";
         str += dbResult->meta;
         Log( str, 4 );
         return false;
      }
   }
   bool  wasHandled = false;
   if( m_stringLookup->HandleResult( dbResult ) == true )
   {
      wasHandled = true;
   }
   else
   {
      switch( dbResult->lookup )
      {
         cout << "Db query type:"<< dbResult->lookup << ", success=" << dbResult->successfulQuery << endl;

         case QueryType_UserLoginInfo:
            {
               if( HandleUserLoginResult( connectionId, dbResult ) == false )
               {
                  SendErrorToClient( connectionId, PacketErrorReport::ErrorType_UserBadLogin );  
                  string str = "User not valid and db query failed, userName: ";
                  str += connection->m_userName;
                  str += ", uuid: ";
                  str += connection->m_userUuid;
                  Log( str, 4 );
                  ForceUserLogoutAndBlock( connectionId );
                  wasHandled = false;
               }
               else
               {
                  wasHandled = true;
               }
            }
            break;
         case QueryType_AdminRequestUserProfile:
            {
               // in some weird circustance, we could end up in an infinite loop here.
              /* if( dbResult->successfulQuery == false || dbResult->GetBucket().size() == 0 )// no records found
               {
                  connection->AddBlankUserProfile();
               }
               else
               {
                  HandleUserProfileFromDb( connectionId, dbResult );
               }*/
               HandleAdminRequestUserProfile( connectionId, dbResult );
               wasHandled = true;
            }
            break;
         case QueryType_UserListOfGame:
            {
               if( dbResult->successfulQuery == false || dbResult->GetBucket().size() == 0 )// no records found
               {
                  string str = "List of games not valid db query failed, userName: ";
                  str += connection->m_userName;
                  str += ", uuid: ";
                  str += connection->m_userUuid;
                  Log( str, 4 );
                  ForceUserLogoutAndBlock( connectionId );
                  wasHandled = false;
               }
               else
               {
                  wasHandled = true;
               
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
            }
            break;
         case QueryType_UserListOfUserProducts:
            {
               if( dbResult->successfulQuery == false || dbResult->GetBucket().size() == 0 )
               {
                  string str = "Query failed looking up a user products ";
                  str += connection->m_userName;
                  str += ", uuid: ";
                  str += connection->m_userUuid;
                  Log( str, 4 );
                  ForceUserLogoutAndBlock( connectionId );
                  wasHandled = false;
               }
               else
               {
                  StoreListOfUsersProductsFromDB( connectionId, dbResult );
                  wasHandled = true;
               }
            }
            break;
         case QueryType_LookupUserByUsernameOrEmail:// these should never happen since these are handled elsewhere
         case QueryType_LookupTempUserByUsernameOrEmail:
         case QueryType_LookupUserNameForInvalidName:
            {
               if( dbResult->successfulQuery == false || dbResult->GetBucket().size() == 0 )
               {
                  string str = "Query failed looking up a user ";
                  str += connection->m_userName;
                  str += ", uuid: ";
                  str += connection->m_userUuid;
                  Log( str, 4 );
                  ForceUserLogoutAndBlock( connectionId );
                  wasHandled = false;
               }
               else
               {
                  wasHandled = true;
               }
            }
            break;
         case QueryType_LoadProductInfo:
            {
               if( dbResult->successfulQuery == false || dbResult->GetBucket().size() == 0 )
               {
                  string str = "Initialization failed: table does not exist ";
                  wasHandled = false;
               }
               else
               {
                  StoreAllProducts( dbResult );
                  wasHandled = true;
               }
            }
            break;
         case QueryType_GetSingleProductInfo:
            {
               if( dbResult->successfulQuery == false ||
                  dbResult->GetBucket().size() == 0 )
               {
                  string str = "Product not found ";
                  str += dbResult->meta;
                  Log( str, 4 );
                  wasHandled = false;
               }
               else
               {
                  StoreSingleProduct( dbResult );
                  wasHandled = true;
               }
            }
            break;
         case QueryType_GetProductListForUser:
            {
               SendListOfPurchasesToUser( connectionId, dbResult );
               wasHandled = true;
            }
            break;
         }
      }
      return wasHandled;
}

//---------------------------------------------------------------

void     DiplodocusLogin:: StoreAllProducts( const PacketDbQueryResult* dbResult )
{
   ProductTable            enigma( dbResult->bucket );

   ProductTable::iterator  it = enigma.begin();
  // int numProducts = static_cast< int >( dbResult->bucket.size() );          
   while( it != enigma.end() )
   {
      ProductTable::row       row = *it++;

      ProductInfo productDefn;
      productDefn.productId  = boost::lexical_cast< int >( row[ TableProduct::Column_product_id ] );
      productDefn.uuid =                 row[ TableProduct::Column_uuid ];
      productDefn.name =                 row[ TableProduct::Column_name ];
      productDefn.filterName =           row[ TableProduct::Column_filter_name ];
      productDefn.Begindate =            row[ TableProduct::Column_begin_date ];
      productDefn.lookupName =           row[ TableProduct::Column_name_string ];
      string temp = row[ TableProduct::Column_product_type ];
      if( temp == "" )
         temp = "0";

      productDefn.productType  = boost::lexical_cast< int >( temp );
      

      m_productList.push_back( productDefn );
   }

   m_isInitialized = true;
}

//---------------------------------------------------------------

void     DiplodocusLogin:: StoreSingleProduct( const PacketDbQueryResult* dbResult )
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
      productDefn.lookupName =           row[ TableProduct::Column_name_string ];

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
            userIt->second.WriteProductToUserRecord( filterUuid, price );
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

void     DiplodocusLogin:: SendListOfPurchasesToUser( U32 connectionId, PacketDbQueryResult* dbResult )
{
   ConnectionToUser* connection = GetUserConnection( connectionId );
   if( connection )
   {
      connection->StoreProductInfo( dbResult );
   }
}

//---------------------------------------------------------------

void     DiplodocusLogin:: StoreListOfUsersProductsFromDB( U32 connectionId, PacketDbQueryResult* dbResult )
{
   ConnectionToUser* connection = GetUserConnection( connectionId );
   if( connection )
   {
      connection->StoreListOfUsersProductsFromDB( dbResult, m_autoAddProductFromWhichUsersLogin );
   }

   RequestListOfProductsFromClient( connectionId );
}

//---------------------------------------------------------------

void     DiplodocusLogin:: RequestListOfProductsFromClient( U32 connectionId )
{
   PacketListOfUserPurchasesRequest* purchaseRequest = new PacketListOfUserPurchasesRequest;
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

   U32 hash = static_cast<U32>( GenerateUniqueHash( product.name ) );
   string newUuid = GenerateUUID( hash );

   std::string lowercase_username = product.name; 
   std::transform( lowercase_username.begin(), lowercase_username.end(), lowercase_username.begin(), ::tolower );

   dbQuery->query = "INSERT INTO product VALUES( DEFAULT, 0, '";// new products haven an id of 0
   dbQuery->query += newUuid;
   dbQuery->query += "', '%s', '%s', NOW(), 0, DEFAULT, DEFAULT, DEFAULT)";

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
      packet->uuid = connection->m_userUuid;
      vector< string >::iterator it =  connection->productFilterNames.begin();
      while( it != connection->productFilterNames.end() )
      {
         ProductInfo productDefn;
         if( FindProductByLookupName( *it++, productDefn ) == true )
         {
            packet->products.insert( productDefn.filterName );
         }
      }


      BaseOutputContainer::iterator itOutputs = m_listOfOutputs.begin();
      while( itOutputs != m_listOfOutputs.end() )
      {
         ChainType*  outputPtr = static_cast< ChainType*> ( (*itOutputs).m_interface );

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


//-----------------------------------------------------------------------------------------

void     DiplodocusLogin::TrackCountStats( StatTracking stat, float value, int sub_category )
{
   StatTrackingConnections::TrackCountStats( m_serverName, m_serverId, stat, value, sub_category );
}

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------

void     DiplodocusLogin::RunHourlyStats()
{
   /*if( m_userConnectionMap.size() == 0 )
      return;*/

   time_t currentTime;
   time( &currentTime );

   if( difftime( currentTime, m_timestampHourlyStatServerStatisics ) >= timeoutHourlyStatisics ) 
   {
      m_timestampHourlyStatServerStatisics = ZeroOutMinutes( currentTime );

      //--------------------------------
      float numConnections = static_cast<float>( m_userConnectionMap.size() );
      float totalNumSeconds = 0;

      UserConnectionMapIterator nextIt = m_userConnectionMap.begin();
      while( nextIt != m_userConnectionMap.end() )
      {
         const ConnectionToUser& user = nextIt->second;
         nextIt++;
         time_t loginTime = user.GetLoginTime();
         if( loginTime == 0 )
            loginTime = currentTime;

         totalNumSeconds += static_cast<float>( difftime( currentTime, loginTime ) );
      }

      totalNumSeconds += m_totalUserLoginSeconds;
      numConnections += m_totalNumLogouts;

      float averageNumSeconds = totalNumSeconds / numConnections;
      TrackCountStats( StatTracking_UserAverageTimeOnline, averageNumSeconds, 0 );
      TrackCountStats( StatTracking_UserTotalTimeOnline, totalNumSeconds, 0 );
      TrackCountStats( StatTracking_NumUsersOnline, numConnections, 0 );

      // if we decide to track these
      m_numFailedLogins = 0;
      m_numRelogins = 0;
      m_numSuccessfulLogins = 0;
      m_totalUserLoginSeconds = 0;
      m_totalNumLogouts = 0;
   }
}

//---------------------------------------------------------------

void     DiplodocusLogin::RunDailyStats()
{
   time_t currentTime;
   time( &currentTime );
   if( difftime( currentTime, m_timestampDailyStatServerStatisics ) >= timeoutDailyStatisics ) 
   {
      m_timestampDailyStatServerStatisics = ZeroOutHours( currentTime );

      float numUniqueUsers = static_cast< float >( m_uniqueUsers.size() );
      if( numUniqueUsers == 0 )
         return;

      ClearOutUniqueUsersNotLoggedIn();

      TrackCountStats( StatTracking_UniquesUsersPerDay, numUniqueUsers, 0 );
   }
}

//---------------------------------------------------------------

void     DiplodocusLogin::ClearOutUniqueUsersNotLoggedIn()
{
   // clear out
   set< string >::iterator it = m_uniqueUsers.begin();
   while( it != m_uniqueUsers.end() )
   {
      set< string >::iterator currentIt = it++;
      UserConnectionMapIterator userIt = m_userConnectionMap.begin();
      bool found = false;
      while( userIt != m_userConnectionMap.end() )
      {
         const ConnectionToUser& user = userIt->second;
         userIt++;
         if( m_uniqueUsers.find( user.m_email ) != m_uniqueUsers.end() )
         {
            found = true;
            break;
         }
      }
      if( found == false )
      {
         m_uniqueUsers.erase( currentIt );
      }
   }
}
//---------------------------------------------------------------

int      DiplodocusLogin:: CallbackFunction()
{
   CommonUpdate();

   if( m_isInitializing == false )
   {
      if( m_isInitialized == false )
      {
         LoadInitializationData();
         m_isInitializing = true;
      }
   }

   time_t currentTime;
   time( &currentTime );
   m_stringLookup->Update( currentTime );

 /*  m_mutex.lock();
   ChainLinkIteratorType itInputs = m_listOfInputs.begin();
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

   UpdatePendingGatewayPackets();

   RunHourlyStats();
   RunDailyStats();

   UpdateDbResults();

   StatTrackingConnections::SendStatsToStatServer( m_listOfOutputs, m_serverName, m_serverId, m_serverType );

   return 1;
}

//---------------------------------------------------------------
//////////////////////////////////////////////////////////
