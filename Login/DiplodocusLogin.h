// DiplodocusLogin.h

#pragma once

#include "../NetworkCommon/NetworkIn/Diplodocus.h"
#include "KhaanLogin.h"
#include "../NetworkCommon/Database/QueryHandler.h"
#include <ctime>

class PacketDbQuery;
class PacketDbQueryResult;
class PacketCheat;

//-----------------------------------------------------------------------------------------

struct ProductInfo
{
   ProductInfo() : productId( 0 ), productType( 0 ) {}
   int      productId;
   string   uuid;
   string   name;
   string   filterName;
   string   Begindate;
   int      productType;

   double   price;
};

//-----------------------------------------------------------------------------------------

struct ConnectionToUser
{
   enum LoginStatus
   {
      LoginStatus_Pending,
      LoginStatus_Invalid,
      LoginStatus_LoggedIn,
      LoginStatus_Hacker
   };

   ConnectionToUser( const string& name, const string& pword, const string& key ) : username( name ), passwordHash( pword ), loginKey( key ), status( LoginStatus_Pending ), active( true ), loggedOutTime( 0 ) {}
   string               id;
   string               username;
   string               passwordHash;
   string               email;
   string               userUuid;
   string               loginKey;
   string               lastLoginTime;
   //U32                  

   vector< string >     productFilterNames;
   vector< ProductInfo > productsWaitingForInsertionToDb;
   LoginStatus          status;
   U8                   gameProductId;
   bool                 active;
   time_t               loggedOutTime;

   void  AddProductFilterName( const string& text );
   int   FindProductFilterName( const string& text ); 
};

//---------------------------------------------------------------

class CreateAccountResultsAggregator
{
public:
   enum     MatchingRecord
   {
      MatchingRecord_None,
      MatchingRecord_Name = 1,
      MatchingRecord_Email = 2,
      MatchingRecord_Gamekithash = 4
   };

   CreateAccountResultsAggregator( U32 _connectionId, const string& email, const string& _password, const string& _username, const string& _gamekitHashId, 
                                 const string& _deviceId, U8 _languageId, U8 _gameProductId, int numQueriesToComplete = 3 ) : 
         m_numQueriesToAggregate( numQueriesToComplete ), 
         m_numUserRecordsMatching( 0 ), 
         m_numPendingUserRecordsMatching( 0 ),
         m_whichRecordMatched( MatchingRecord_None ),
         m_connectionId( _connectionId ), 
         m_useremail( email ), 
         m_password( _password ), 
         m_username( _username ), 
         m_gamekitHashId( _gamekitHashId ), 
         m_deviceId( _deviceId ), 
         m_gameProductId( _gameProductId ),
         m_languageId( _languageId ),
         m_userNameIsInvalid( false ),
         m_userRecordMatchingGKHash( 0 ),
         m_userRecordMatchingName( 0 ),
         m_userRecordMatchingEmail( 0 ),
         m_userRecordMatchingActive( 0 ),
         m_pendingUserRecordMatchingGKHash( 0 ),
         m_pendingUserRecordMatchingName( 0 ),
         m_pendingUserRecordMatchingEmail( 0 )
         {}

   bool     HandleResult( const PacketDbQueryResult* dbResult );
   U32      GetConnectionId() const { return m_connectionId; }
   bool     IsComplete() const { if( m_numQueriesToAggregate == 0 ) return true; return false; }
   bool     HasFoundAnyMatchingRecords() const;
   bool     HasMatchingGamekitHash() const { if( m_whichRecordMatched == MatchingRecord_Gamekithash ) return true; return false; }

   bool     GetMatchingRecordType( MatchingRecord recordType ) const { return (m_whichRecordMatched & recordType )? true: false; }
   //U32      Get

   // decision making
   bool     IsDuplicateRecord() const;
   bool     ShouldUpdateUserRecord() const;
   bool     ShouldUpdatePendingUserRecord() const;
   bool     ShouldInsertNewUserRecord() const;

   bool     IsMatching_GKHashRecord_DifferentFrom_UserEmail( const string& testEmail ) const;
   
public:
   int            m_numQueriesToAggregate; // when it drops to 0, we are done.
   int            m_numUserRecordsMatching;
   int            m_numPendingUserRecordsMatching;
   U32            m_connectionId;
   string         m_useremail;
   string         m_password;
   string         m_username;
   string         m_gamekitHashId;
   string         m_deviceId;
   U8             m_gameProductId;
   U8             m_languageId;
   bool           m_userNameIsInvalid;
   
public:// we don't need accessors for all these
   U32            m_whichRecordMatched;

   U32            m_userRecordMatchingGKHash;
   U32            m_userRecordMatchingName;
   U32            m_userRecordMatchingEmail;
   U32            m_userRecordMatchingActive;

   U32            m_pendingUserRecordMatchingGKHash;
   U32            m_pendingUserRecordMatchingName;
   U32            m_pendingUserRecordMatchingEmail;

   string         m_emailForMatchingRecord_GamekitHashId;
};

//-----------------------------------------------------------------------------------------

enum LanguageList // corresponds to the db-language table
{
   LanguageList_english = 1,
   LanguageList_spanish,
   LanguageList_french,
   LanguageList_german,
   LanguageList_italian,
   LanguageList_portuguese,
   LanguageList_russian,
   LanguageList_japanese,
   LanguageList_chinese
};

//-----------------------------------------------------------------------------------------

class DiplodocusLogin : public Diplodocus< KhaanLogin >
{
public:
   enum QueryType 
   {
      QueryType_UserLoginInfo = 1,
      QueryType_UpdateLastLoggedInTime,
      QueryType_UpdateLastLoggedOutTime,
      QueryType_UserListOfGame,
      QueryType_UserListOfUserProducts,

      QueryType_LookupUserNameForInvalidName,
      QueryType_LookupUserByUsernameOrEmail,
      QueryType_LookupTempUserByUsernameOrEmail,
      QueryType_UpdateUseraccount,
      QueryType_CreateUseraccount,
      QueryType_UpdatePendingUseraccount,
      QueryType_CreatePendingUseraccount,


      QueryType_LoadProductInfo,
      QueryType_AddProductInfo,
      QueryType_GetSingleProductInfo,
      QueryType_UpdateProductFileInfo,
      QueryType_AddProductInfoToUser,
      QueryType_GetProductListForUser
   };

public:
   DiplodocusLogin( const string& serverName, U32 serverId );
   void     ServerWasIdentified( ChainedInterface* khaan );

   bool     AddInputChainData( BasePacket* packet, U32 connectionId );
   bool     AddOutputChainData( BasePacket* packet, U32 connectionId );

private:

   int      CallbackFunction();
   void     RemoveOldConnections();

   void     LoadInitializationData();
   void     StoreAllProducts( PacketDbQueryResult* dbResult );
   void     StoreSingleProduct( PacketDbQueryResult* dbResult );

   bool     AddQueryToOutput( PacketDbQuery* query );
   bool     LogUserIn( const string& username, const string& password, const string& loginKey, U8 gameProductId, U32 connectionId );
   bool     LogUserOut( U32 connectionId, bool wasDisconnectedByError );
   bool     CreateUserAccount( U32 connectionId, const string& email, const string& password, const string& username, const string& deviceAccountId, const string& deviceId, U8 languageId, U8 gameProductId );
   U32      FindUserAlreadyInGame( const string& username, U8 gameProductId );

   void     TellUserThatAccountAlreadyMatched( const CreateAccountResultsAggregator* aggregator );
   void     CreateNewUserAccount( const CreateAccountResultsAggregator* aggregator, bool setGkHashTo0 );
   void     UpdateUserAccount( const CreateAccountResultsAggregator* aggregator );
   void     UpdatePendingUserRecord( const CreateAccountResultsAggregator* aggregator );
   void     CreateNewPendingUserAccount( const CreateAccountResultsAggregator* aggregator, bool setGameKitHashToNUll = false );
   void     UpdateUserRecord( CreateAccountResultsAggregator* aggregator );
   //void     UpdateUserRecordToMatchingGamekitHash( const CreateAccountResultsAggregator* aggregator );

   bool     SuccessfulLogin( U32 connectionId, bool isReloggedIn = false );
   bool     ForceUserLogoutAndBlock( U32 connectionId );
   bool     CreateAccount( const char* username, const char* emailAddress, const char* password, int userId, int gameId );
   bool     RequestListOfGames( U32 connectionId, const string& userUuid );
   bool     RequestListOfProducts( U32 connectionId, const string& userUuid );
   bool     SendListOfGamesToGameServers( U32 connectionId, const KeyValueVector& kv_array );

   bool     UpdateLastLoggedOutTime( U32 connectionId );
   bool     UpdateLastLoggedInTime( U32 connectionId );

   bool     SendLoginStatusToOtherServers( const string& username, const string& userUuid, U32 connectionId, U8 gameProductId, const string& lastLoginTime, bool isActive, const string& email, const string& passwordHash, const string& userId,  
                                                    const string& loginKey, bool isLoggedIn, bool wasDisconnectedByError );
   bool     SendListOfUserProductsToOtherServers( const string& userUuid, U32 connectionId, const vector< string >& productNames );

   bool     StoreUserPurchases( U32 connectionId, const PacketListOfUserPurchases* purchase );
   bool     RequestListOfPurchases( U32 connectionId, const PacketRequestListOfUserPurchases* purchase );
   bool     HandleCheats( U32 connectionId, const PacketCheat* cheat );
   
   void     WriteProductToUserRecord( const string& productFilterName, const string& uuid, double pricePaid );
   void     StoreListOfUsersProductsFromDB( U32 connectionId, PacketDbQueryResult* dbResult );
   void     AddGameProductIdToUserProducts( U32 connectionId );
   void     SendListOfUserProductsToAssetServer( U32 connectionId );
   void     AddNewProductToDb( const PurchaseEntry& product );
   bool     UpdateProductFilterName( int index, const string& newFilterName );
   void     RequestListOfProductsFromClient( U32 connectionId );
   void     SendProductListResultToUser( U32 connectionId, PacketDbQueryResult* dbResult );
   int      FindProductByName( const string& name );

   //bool     SendPacketToGateway( BasePacket*, U32 connectionId );
   //bool     SendErrorToClient( U32 connectionId, PacketErrorReport::ErrorType );

   //---------------------------------------------------------------
   typedef map< U32, ConnectionToUser >      UserConnectionMap;
   typedef pair< U32, ConnectionToUser >     UserConnectionPair;
   typedef UserConnectionMap::iterator       UserConnectionMapIterator;
   typedef UserConnectionMap::const_iterator UserConnectionMapConstIterator;

   typedef map< stringhash, vector< string > >    StringTableLookup;
   typedef pair< stringhash, vector< string > >   StringTableLookupPair;

   typedef map< U32, CreateAccountResultsAggregator* >   UserCreateAccountMap;
   typedef pair< U32, CreateAccountResultsAggregator*>   UserCreateAccountPair;
   typedef UserCreateAccountMap::iterator                UserCreateAccountIterator;

   typedef vector< ProductInfo >             ProductList;

   UserConnectionMap          m_userConnectionMap;
   UserCreateAccountMap       m_userAccountCreationMap;
   bool                       m_isInitialized, m_isInitializing;

   bool                       IsUserConnectionValid( U32 id );
   ConnectionToUser*          GetUserConnection( U32 id );
   void                       ReinsertUserConnection( int oldIndex, int newIndex );
   bool                       AddUserConnection( UserConnectionPair );
   bool                       RemoveUserConnection( U32 id );

   ProductList                m_productList;

   void                       FinalizeLogout( U32 connectionId, bool wasDisconnectedByError );

};

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------