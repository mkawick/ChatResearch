// ConnectionToUser.h
#pragma once

#include "../NetworkCommon/DataTypes.h"
#include "../NetworkCommon/Utils/TableWrapper.h"
#include "../NetworkCommon/Packets/LoginPacket.h"
#include "ProductInfo.h"
#include <string>
#include <vector>
#include <map>
using namespace std; 

class PacketDbQueryResult;
class DiplodocusLogin;
class PacketListOfUserPurchasesRequest;
class PacketCheat;

//////////////////////////////////////////////////////////////////////////

struct ConnectionToUser
{
   enum LoginStatus
   {
      LoginStatus_Pending,
      LoginStatus_Invalid,
      LoginStatus_LoggedIn,
      LoginStatus_Hacker
   };

   ConnectionToUser( const string& name, const string& pword, const string& key );
   void     SetManager( DiplodocusLogin* manager )  { m_userManager = manager; }
   void     ClearLoggingOutStatus() { m_loggedOutTime = 0; }
   bool     CanContinueLogginIn() const { return m_isActive && ( m_isReadyToBeCleanedUp == false ); }

   time_t   GetLoginTime() const { return m_loginTime; }

   LoginStatus GetLoginStatus() const { return status; }
   void     IncreaseLoginAttemptCount() { ++m_loginAttemptCount; }
   int      GetLoginAttemptCount() const { return m_loginAttemptCount; }

   void     SetGatewayId( U32 gatewayId ) { m_gatewayId = gatewayId; }
   U32      GetGatewayId() const { return m_gatewayId; }

   //------------------------------------------------

   void     LoginResult( PacketDbQueryResult* dbResult );
   bool     BeginLogout( bool wasDisconnectedByError );
   bool     FinalizeLogout();
   
   bool     StoreProductInfo( PacketDbQueryResult* dbResult );
   bool     IsReadyToBeCleanedUp() const { return m_isReadyToBeCleanedUp; }

   //------------------------------------------------
   
   void     AddProductVendorUuid( string text );
   int      FindProductVendorUuid( string text ); 

   //------------------------------------------------

   bool     UpdateLastLoggedInTime();
   bool     UpdateLastLoggedOutTime();
   bool     SuccessfulLoginFinished( U32 connectionId, bool isReloggedIn );
   void     UpdateConnectionId( U32 connectionId );

   bool     RequestListOfGames( const string& userUuid );
   bool     RequestListOfPurchases( const string& userUuid );
   bool     HandleRequestForListOfPurchases( const PacketListOfUserPurchasesRequest* purchase );
   bool     AddPurchase( const PacketAddPurchaseEntry* purchase );
   bool     StoreUserPurchases( const PacketListOfUserAggregatePurchases* deviceReportedPurchases );
   bool     CanProductBePurchasedMultipleTimes( const ProductInfo& productInfo );

   void     AddCurrentlyLoggedInProductToUserPurchases();
   void     WriteProductToUserRecord( const string& productVendorUuid, double pricePaid );
   void     WriteProductToUserRecord( const string& userUuid, const string& productUuid, double pricePaid, float numPurchased, string adminId, string adminNotes );
   void     StoreListOfUsersProductsFromDB( PacketDbQueryResult* dbResult, bool shouldAddLoggedInProduct );

   bool     RequestProfile( const PacketRequestUserProfile* profileRequest );
   bool     RequestOthersProfile( const PacketRequestOtherUserProfile* profileRequest );
   void     RequestProfile( const string& email, const string& uuid, const string& name, bool asAdmin, bool isFullProfile );
   bool     UpdateProfile( const PacketUpdateUserProfile* updateProfileRequest );
   bool     UpdateProfile( const PacketUpdateSelfProfile* updateProfileRequest );
   void     AddItemToProductTable( const PurchaseEntryExtended& purchaseEntry );
   bool     HandleAdminRequestUserProfile( const PacketDbQueryResult* dbResult );
   bool     StoreOffProductInUserRecord ( int userManagerIndex, const string& productUuid, float numPurchased );
   void     AddConversionProductsToUserPurchases( const ProductInfo& productInfo );

   bool     LoginResult( const PacketDbQueryResult* dbResult );

   bool     EchoHandler();

   //----------------------------------

   bool     HandleCheats( const PacketCheat* cheat );

   //----------------------------------

   string                  m_id;
   string                  m_userName;
   string                  m_passwordHash;
   string                  m_email;
   string                  m_userUuid;
   string                  m_loginKey;
   string                  m_lastLoginTime;
   string                  m_lastLogoutTime;
   string                  m_userMotto;
   int                     m_avatarIcon;
   int                     m_loginAttemptCount;

   LoginStatus             status;
   U8                      m_gameProductId;
   U32                     m_connectionId;
   
   time_t                  m_loginTime;
   time_t                  m_loggedOutTime;
   bool                    m_isLoggingOut;
   bool                    m_isReadyToBeCleanedUp;
   U32                     m_gatewayId;

   int                     m_timeZone;
   int                     m_languageId;
   int                     m_adminLevel;
   bool                    m_isActive;
   bool                    m_showWinLossRecord;
   bool                    m_marketingOptOut;
   bool                    m_showGenderProfile;
   bool                    m_displayOnlineStatusToOtherUsers;
   bool                    m_blockContactInvitations;
   bool                    m_blockGroupInvitations;

   bool                    m_isSavingUserProfile;
   bool                    m_hasRequestedPurchasesFromClient;
   /*
   address1
   address2
   city
   provence
   mail_code
   country
   screen_name
   gender
   mber_avatar
   home_phone
   alt_phone
   time_zone
   */
   
   vector< string >        productVendorUuids;
   vector< ProductInfo >   productsWaitingForInsertionToDb;
   map< U32, ProductBrief >  productsOwned;
   

protected:
   ConnectionToUser() {};
   

   
   static DiplodocusLogin*    m_userManager;

   typedef map< string, ConnectionToUser>    UserConnectionMap;
   typedef pair< string, ConnectionToUser>   UserConnectionPair;
   typedef UserConnectionMap::iterator       UserConnectionMapIterator;

   UserConnectionMap adminUserData; // if you are logged in as an admin, you may be querying other users. use hashed user_name

   bool     HandleCheat_RemoveAll( const string& command );
   bool     HandleCheat_AddProduct( const string& command );

   void     CopyUserSettings( UserPlusProfileTable& enigma, U8 gameProductId );
   void     SaveUpdatedProfile( const PacketUpdateUserProfile* profileUpdate, int adminLevelOfCaller, bool writeToDB );

   void     WriteUserBasicsToAccount();
   void     WriteUserProfile();
   void     PackUserProfileRequestAndSendToClient( U32 connectionId, U32 gatewayId = 0 );
   template < typename type >
   void     PackUserSettings( type* response );
   void     PackOtherUserProfileRequestAndSendToClient( U32 connectionId );

   void     ClearAllProductsOwned();
   bool     AddToProductsOwned( int productDbId, const string& productName, const string& productUuid, float quantity, const string& vendorUuid );
   void     SendListOfProductsToClientAndAsset( U32 connectionId );
   void     SendListOfOwnedProductsToClient( U32 connectionId );
   void     TellContactServerToReloadUserProfile();
   void     RequestListOfProductsFromClient();

   UserConnectionMapIterator FindUser( const string& email, const string& userUuid, const string& userName );
   
};

//////////////////////////////////////////////////////////////////////////
