// DiplodocusLogin.h

#pragma once

#include "ProductInfo.h"
#include "ConnectionToUser.h"

#include "../NetworkCommon/NetworkIn/Diplodocus.h"
#include "../NetworkCommon/Stat/StatTrackingConnections.h"
#include "../NetworkCommon/Database/QueryHandler.h"

#include "KhaanLogin.h"
#include "CreateAccountResultsAggregator.h"

#include <ctime>

#include <set>
using namespace std;

class PacketDbQuery;
class PacketDbQueryResult;
class PacketCheat;
struct PurchaseEntry;

class PacketListOfUserPurchasesRequest;
class StringLookup;
class PacketLoginFromGateway;
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

class DiplodocusLogin : public Queryer, public Diplodocus< KhaanLogin >, public StatTrackingConnections
{
public:
   enum QueryType 
   {
      QueryType_UserLoginInfo = 1,
      QueryType_UserLogout,
      QueryType_AdminRequestUserProfile,
      QueryType_UpdateUserProfile,
      QueryType_UpdateSelfProfile,
      QueryType_UpdateUsers,
      QueryType_CreateBlankUserProfile,
      QueryType_UpdateLastLoggedInTime,
      QueryType_UpdateLastLoggedOutTime,
      QueryType_UserListOfGame,
      QueryType_UserListOfUserProducts,

      QueryType_LookupUserNameForInvalidName,
      QueryType_LookupUserByUsernameOrEmail,
      QueryType_LookupTempUserByUsernameOrEmail,
      QueryType_LookupUserByGkHash,
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
      QueryType_AdminLoadUserProducts,

      QueryType_ProductStringLookup
   };
   enum 
   {
      ProductNotFound = -1
   };

public: 
   typedef Diplodocus< KhaanLogin > ChainedType;

public:
   DiplodocusLogin( const string& serverName, U32 serverId );

   const char*    GetClassName() const { return "DiplodocusLogin"; }

   void           PrintPacketTypes( bool printingOn = true );
   void           PrintFunctionNames( bool printingOn = true );

   void           ServerWasIdentified( IChainedInterface* khaan );
   //void           OutputConnected( IChainedInterface * );
   void           AutoAddTheProductFromWhichYouLogin( bool addIt = false ) { m_autoAddProductFromWhichUsersLogin = addIt; }

   bool           AddInputChainData( BasePacket* packet, U32 connectionId );
   bool           AddOutputChainData( BasePacket* packet, U32 connectionId );

   bool           AddQueryToOutput( PacketDbQuery* query );

   //--------------------------------------
   //bool     LoadUserProfile( U32 connectionId );
   bool           RequestListOfGames( U32 connectionId, const string& userUuid );
   bool           RequestListOfProducts( U32 connectionId, const string& userUuid );

   int            FindProductByName( const string& name );
   bool           FindProductByLookupName( const string& lookupName, ProductInfo& productDefn );
   void           AddNewProductToDb( const PurchaseEntry& product );   
   void           SendListOfUserProductsToAssetServer( U32 connectionId );

   ConnectionToUser* GetLoadedUserConnectionByUuid(const string & uuid );
   bool           FindProductByUuid( const string& uuid, ProductInfo& returnPi  );
   int            FindProductByVendorUuid( const string& vendorUuid );
   bool           GetProductByIndex( int index, ProductInfo& returnPi );
   bool           GetProductByProductId( int productId, ProductInfo& returnPi );

   StringLookup*  GetStringLookup() { return m_stringLookup; }
   bool           SendPacketToOtherServer( BasePacket* packet, U32 connectionId );

   bool           IsPrintingFunctionNames() const { return m_printFunctionNames; }
   bool           IsPrintingVerbose() const { return m_printPacketTypes; }
   //--------------------------------------

private:

   int      CallbackFunction();
   void     RemoveOldConnections();

   void     LoadInitializationData();
   void     StoreAllProducts( const PacketDbQueryResult* dbResult );
   void     StoreSingleProduct( const PacketDbQueryResult* dbResult );

   
   bool     LogUserIn( const PacketLoginFromGateway* packet, U32 connectionId ); //const string& username, const string& password, const string& loginKey, U8 gameProductId, U32 connectionId, U32 gatewayId );
   bool     LogUserOut( U32 connectionId, bool wasDisconnectedByError );
   bool     LoadUserAccount( const string& userName, const string& password, const string& loginKey, U8 gameProductId, U32 connectionId, U32 gatewayId );
   bool     SetupQueryForLogin( const string& userName, const string& password, U8 gameProductId, U32 connectionId );
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
   bool     HandlePacketFromOtherServer( BasePacket* packet, U32 connectionId );
   bool     HandleUserLoginResult( U32 connectionId, const PacketDbQueryResult* dbResult );
   bool     HandleAdminRequestUserProfile( U32 connectionId, const PacketDbQueryResult* dbResult );
   //bool     HandleUserProfileFromDb( U32 connectionId, PacketDbQueryResult* dbResult );
   bool     SuccessfulLogin( U32 connectionId, bool isReloggedIn = false );
   
   bool     AddBlankUserProfile( U32 connectionId );
   bool     ForceUserLogoutAndBlock( U32 connectionId );
   bool     CreateAccount( const char* username, const char* emailAddress, const char* password, int userId, int gameId );
   bool     SendListOfGamesToGameServers( U32 connectionId, const KeyValueVector& kv_array );

   bool     UpdateLastLoggedOutTime( U32 connectionId );
   bool     UpdateLastLoggedInTime( U32 connectionId );

   
   //bool     SendListOfUserProductsToOtherServers( const string& userUuid, U32 connectionId, const vector< string >& productNames );

   bool     StoreUserPurchases( U32 connectionId, const PacketListOfUserAggregatePurchases* purchase );
   bool     RequestListOfPurchases( U32 connectionId, const PacketListOfUserPurchasesRequest* purchase );
   bool     RequestListOfPurchasesUpdate( const PacketListOfUserPurchasesUpdated* userInventory );
   bool     AddPurchase( U32 userConnectionId, const PacketAddPurchaseEntry* addPurchase );
   bool     RequestProfile( U32 connectionId, const PacketRequestUserProfile* profileRequest );
   bool     UpdateProfile( U32 connectionId, const PacketUpdateUserProfile* profileRequest );
   bool     UpdateProfile( U32 connectionId, const PacketUpdateSelfProfile* profileRequest );
   bool     HandleRequestListOfProducts( U32 connectionId, PacketRequestListOfProducts* purchaseRequest );
   bool     RequestOthersProfile( U32 connectionId, const PacketRequestOtherUserProfile* profileRequest );
   bool     ThrottleUser( U32 userConnectionId, const PacketLoginDebugThrottleUserPackets* throttleRequest );
   bool     EchoHandler( U32 connectionId );
   
   bool     HandleCheats( U32 connectionId, const PacketCheat* cheat );
   
   void     StoreListOfUsersProductsFromDB( U32 connectionId, PacketDbQueryResult* dbResult );
   bool     UpdateProductFilterName( int index, string newFilterName );
   
   void     SendListOfPurchasesToUser( U32 connectionId, PacketDbQueryResult* dbResult );

   //---------------------------------------------------------------

   void     TrackCountStats( StatTracking stat, float value, int sub_category );
   void     RunHourlyStats();
   void     RunDailyStats();
   void     ClearOutUniqueUsersNotLoggedIn();

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
   StringLookup*              m_stringLookup;

   deque< PacketDbQueryResult* >    m_dbQueries;

   // stat tracking
   time_t                     m_timestampHourlyStatServerStatisics;
   static const U32           timeoutHourlyStatisics = 60*60;
   time_t                     m_timestampDailyStatServerStatisics;
   static const U32           timeoutDailyStatisics = timeoutHourlyStatisics*24;

   int                        m_numRelogins;
   int                        m_numFailedLogins;
   int                        m_numSuccessfulLogins;
   int                        m_totalUserLoginSeconds;
   int                        m_totalNumLogouts;
   set< string >              m_uniqueUsers;
   bool                       m_printPacketTypes;
   bool                       m_printFunctionNames;

private:
   //--------------------------------------------

   void     FinalizeLogout( U32 connectionId, bool wasDisconnectedByError );
   void     UpdateDbResults();
   bool     HandleDbResult( PacketDbQueryResult* dbResult );

   bool     SendLoginStatusToOtherServers( const string& username, 
                                             const string& userUuid, 
                                             U32 connectionId, 
                                             U8 gameProductId, 
                                             const string& lastLoginTime, 
                                             bool isActive, 
                                             const string& email, 
                                             const string& passwordHash, 
                                             const string& userId,  
                                             const string& loginKey, 
                                             int langaugeId, 
                                             bool isLoggedIn, 
                                             bool wasDisconnectedByError,
                                             U32 gatewayId);
   bool     SendLoginStatus(  ChainType*  destinationServerPtr,
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
                              bool wasDisconnectedByError,
                              U32 gatewayId);
   bool     SendLoginStatusTo_Non_GameServers( const string& userName, 
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
                                                     bool wasDisconnectedByError,
                                                     U32 gatewayId );
   bool     SendLoginStatusTo_GameServers( const string& userName, 
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
                                            bool wasDisconnectedByError,
                                            U32 gatewayId );
};

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------