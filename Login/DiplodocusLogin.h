// DiplodocusLogin.h

#pragma once

#include "ProductInfo.h"
#include "ConnectionToUser.h"

#include "../NetworkCommon/NetworkIn/Diplodocus.h"
#include "KhaanLogin.h"
#include "../NetworkCommon/Database/QueryHandler.h"
#include "CreateAccountResultsAggregator.h"
#include <ctime>

class PacketDbQuery;
class PacketDbQueryResult;
class PacketCheat;
struct PurchaseEntry;

class PacketRequestListOfUserPurchases;
//class PacketRequestUserProfile;

//-----------------------------------------------------------------------------------------

/*
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
*/

//-----------------------------------------------------------------------------------------

class DiplodocusLogin : public Diplodocus< KhaanLogin >
{
public:
   enum QueryType 
   {
      QueryType_UserLoginInfo = 1,
      QueryType_UserProfile,
      QueryType_CreateBlankUserProfile,
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
      QueryType_GetProductListForUser,
      QueryType_RemoveAllProductInfoForUser,

      QueryType_AdminLoadUserProfile, // profile, etc
      QueryType_AdminLoadUserProducts
   };

public:
   DiplodocusLogin( const string& serverName, U32 serverId );
   void     ServerWasIdentified( ChainedInterface* khaan );
   void     AutoAddTheProductFromWhichYouLogin( bool addIt = false ) { m_autoAddProductFromWhichUsersLogin = addIt; }

   bool     AddInputChainData( BasePacket* packet, U32 connectionId );
   bool     AddOutputChainData( BasePacket* packet, U32 connectionId );

   bool     AddQueryToOutput( PacketDbQuery* query );

   //--------------------------------------
   //bool     LoadUserProfile( U32 connectionId );
   bool     RequestListOfGames( U32 connectionId, const string& userUuid );
   bool     RequestListOfProducts( U32 connectionId, const string& userUuid );

   int      FindProductByName( const string& name );
   void     AddNewProductToDb( const PurchaseEntry& product );   
   void     SendListOfUserProductsToAssetServer( U32 connectionId );

   bool     GetProductByIndex( int index, ProductInfo& returnPi );
   bool     GetProductByProductId( int productId, ProductInfo& returnPi );

private:

   int      CallbackFunction();
   void     RemoveOldConnections();

   void     LoadInitializationData();
   void     StoreAllProducts( PacketDbQueryResult* dbResult );
   void     StoreSingleProduct( PacketDbQueryResult* dbResult );

   
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

   //--------------------------------------------
   bool     HandleLoginResultFromDb( U32 connectionId, PacketDbQueryResult* dbResult );
   bool     HandleUserProfileFromDb( U32 connectionId, PacketDbQueryResult* dbResult );
   bool     SuccessfulLogin( U32 connectionId, bool isReloggedIn = false );
   
   bool     AddBlankUserProfile( U32 connectionId );
   bool     ForceUserLogoutAndBlock( U32 connectionId );
   bool     CreateAccount( const char* username, const char* emailAddress, const char* password, int userId, int gameId );
   bool     SendListOfGamesToGameServers( U32 connectionId, const KeyValueVector& kv_array );

   bool     UpdateLastLoggedOutTime( U32 connectionId );
   bool     UpdateLastLoggedInTime( U32 connectionId );

   //--------------------------------------------

   bool     SendLoginStatusToOtherServers( const string& username, const string& userUuid, U32 connectionId, U8 gameProductId, const string& lastLoginTime, bool isActive, const string& email, const string& passwordHash, const string& userId,  
                                                    const string& loginKey, bool isLoggedIn, bool wasDisconnectedByError );
   bool     SendListOfUserProductsToOtherServers( const string& userUuid, U32 connectionId, const vector< string >& productNames );

   bool     StoreUserPurchases( U32 connectionId, const PacketListOfUserPurchases* purchase );
   bool     RequestListOfPurchases( U32 connectionId, const PacketRequestListOfUserPurchases* purchase );
   bool     RequestProfile( U32 connectionId, const PacketRequestUserProfile* profileRequest );
   bool     UpdateProfile( U32 connectionId, const PacketUpdateUserProfile* profileRequest );

   bool     HandleCheats( U32 connectionId, const PacketCheat* cheat );
   
   void     StoreListOfUsersProductsFromDB( U32 connectionId, PacketDbQueryResult* dbResult );
   bool     UpdateProductFilterName( int index, const string& newFilterName );
   void     RequestListOfProductsFromClient( U32 connectionId );
   void     SendProductListResultToUser( U32 connectionId, PacketDbQueryResult* dbResult );

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
   bool                       m_autoAddProductFromWhichUsersLogin;

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