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
   void     SetManager( DiplodocusLogin* manager )  { userManager = manager; }
   void     ClearLoggingOutStatus() { loggedOutTime = 0; }
   bool     CanContinueLogginIn() const { return isActive; }

   time_t   GetLoginTime() const { return m_loginTime; }

   //------------------------------------------------

   void     LoginResult( PacketDbQueryResult* dbResult );
   bool     BeginLogout( bool wasDisconnectedByError );
   bool     FinalizeLogout();
   
   bool     StoreProductInfo( PacketDbQueryResult* dbResult );
   bool     IsReadyToBeCleanedUp() const { return isReadyToBeCleanedUp; }

   //------------------------------------------------
   
   void     AddProductFilterName( const string& text );
   int      FindProductFilterName( const string& text ); 

   //------------------------------------------------

   bool     UpdateLastLoggedInTime();
   bool     UpdateLastLoggedOutTime();
   bool     SuccessfulLogin( U32 connectionId, bool isReloggedIn );
   void     UpdateConnectionId( U32 connectionId );

   bool     RequestListOfGames( const string& userUuid );
   bool     RequestListOfPurchases( const string& userUuid );
   bool     HandleRequestForListOfPurchases( const PacketListOfUserPurchasesRequest* purchase );
   bool     AddPurchase( const PacketAddPurchaseEntry* purchase );
   bool     StoreUserPurchases( const PacketListOfUserAggregatePurchases* deviceReportedPurchases );

   void     AddCurrentlyLoggedInProductToUserPurchases();
   void     WriteProductToUserRecord( const string& productFilterName, double pricePaid );
   void     WriteProductToUserRecord( const string& userUuid, const string& productUuid, double pricePaid, float numPurchased, string adminId, string adminNotes );
   void     StoreListOfUsersProductsFromDB( PacketDbQueryResult* dbResult, bool shouldAddLoggedInProduct );

   bool     RequestProfile( const PacketRequestUserProfile* profileRequest );
   bool     RequestOthersProfile( const PacketRequestOtherUserProfile* profileRequest );
   void     RequestProfile( const string& email, const string& uuid, const string& name, bool asAdmin );
   bool     UpdateProfile( const PacketUpdateUserProfile* updateProfileRequest );
   bool     UpdateProfile( const PacketUpdateSelfProfile* updateProfileRequest );
   bool     HandleAdminRequestUserProfile( PacketDbQueryResult* dbResult );

   bool     EchoHandler();

   //----------------------------------

   bool     HandleCheats( const PacketCheat* cheat );

   //----------------------------------

   string                  id;
   string                  m_userName;
   string                  passwordHash;
   string                  m_email;
   string                  m_userUuid;
   string                  loginKey;
   string                  lastLoginTime;
   string                  lastLogoutTime;
   int                     avatarIcon;

   LoginStatus             status;
   U8                      gameProductId;
   U32                     connectionId;
   
   time_t                  m_loginTime;
   time_t                  loggedOutTime;
   bool                    isLoggingOut;
   bool                    isReadyToBeCleanedUp;

   int                     timeZone;
   int                     m_languageId;
   int                     adminLevel;
   bool                    isActive;
   bool                    showWinLossRecord;
   bool                    marketingOptOut;
   bool                    showGenderProfile;
   bool                    m_isSavingUserProfile;
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
   
   vector< string >        productFilterNames;
   vector< ProductInfo >   productsWaitingForInsertionToDb;
   map< U32, ProductBrief >  productsOwned;
   

protected:
   ConnectionToUser() {};
   

   
   static DiplodocusLogin* userManager;

   typedef map< string, ConnectionToUser>    UserConnectionMap;
   typedef pair< string, ConnectionToUser>   UserConnectionPair;
   typedef UserConnectionMap::iterator       UserConnectionMapIterator;

   UserConnectionMap adminUserData; // if you are logged in as an admin, you may be querying other users. use hashed user_name

   bool     HandleCheat_RemoveAll( const string& command );
   bool     HandleCheat_AddProduct( const string& command );

   bool     StoreUserInfo( PacketDbQueryResult* dbResult );

   void     SaveUserSettings( UserPlusProfileTable& enigma, U8 gameProductId );
   void     SaveUpdatedProfile( const PacketUpdateUserProfile* profileUpdate, int adminLevelOfCaller, bool writeToDB );
   void     PackUserProfileRequestAndSendToClient( U32 connectionId );
   void     PackOtherUserProfileRequestAndSendToClient( U32 connectionId );

   void     ClearAllProductsOwned();
   void     AddToProductsOwned( int productDbId, const string& productName, const string& productUuid, float quantity );
   void     SendListOfOwnedProductsToClient( U32 connectionId );

   UserConnectionMapIterator FindUser( const string& email, const string& userUuid, const string& userName );
   
};

//////////////////////////////////////////////////////////////////////////
