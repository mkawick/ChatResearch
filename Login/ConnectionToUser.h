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
class PacketRequestListOfUserPurchases;
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

   //------------------------------------------------

   void     LoginResult( PacketDbQueryResult* dbResult );
   bool     BeginLogout( bool wasDisconnectedByError );
   bool     FinalizeLogout();

   //------------------------------------------------
   
   void     AddProductFilterName( const string& text );
   int      FindProductFilterName( const string& text ); 

   //------------------------------------------------

   bool     UpdateLastLoggedInTime();
   bool     UpdateLastLoggedOutTime();
   bool     SuccessfulLogin( U32 connectionId, bool isReloggedIn );
   //bool     LoadUserProfile( U32 whoseProfileIsLoaded = 0 );
   //bool     AddBlankUserProfile();
   bool     RequestListOfGames( const string& userUuid );
   bool     RequestListOfProducts( const string& userUuid );
   bool     HandleGameReportedListOfPurchases( const PacketRequestListOfUserPurchases* purchase );
   bool     AddPurchase( const PacketRequestListOfUserPurchases* purchase );
   //bool     HandleUserProfileFromDb( PacketDbQueryResult* dbResult );
   bool     StoreUserPurchases( const PacketListOfUserPurchases* deviceReportedPurchases );

   void     AddCurrentlyLoggedInProductToUserPurchases();
   void     WriteProductToUserRecord( const string& productFilterName, double pricePaid );
   void     WriteProductToUserRecord( const string& userUuid, const string& productFilterName, double pricePaid, float numPurchased, bool providedByAdmin, string adminNotes );
   void     StoreListOfUsersProductsFromDB( PacketDbQueryResult* dbResult, bool shouldAddLoggedInProduct );

   bool     RequestProfile( const PacketRequestUserProfile* profileRequest );
   bool     UpdateProfile( const PacketUpdateUserProfile* updateProfileRequest );

   //----------------------------------

   bool     HandleCheats( const PacketCheat* cheat );

   //----------------------------------

   string                  id;
   string                  username;
   string                  passwordHash;
   string                  email;
   string                  userUuid;
   string                  loginKey;
   string                  lastLoginTime;

   LoginStatus             status;
   U8                      gameProductId;
   U32                     connectionId;
   
   time_t                  loggedOutTime;

   int                     languageId;
   bool                    isActive;
   int                     adminLevel;
   bool                    showWinLossRecord;
   bool                    marketingOptOut;
   bool                    showGenderProfile;
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
   

protected:
   ConnectionToUser() {};
   

   static DiplodocusLogin* userManager;
   typedef pair< string, ConnectionToUser> UserConnectionPair;
   map< string, ConnectionToUser> adminUserData; // if you are logged in as an admin, you may be querying other users. use hashed user_name

   bool     HandleCheat_RemoveAll( const string& command );
   bool     HandleCheat_AddProduct( const string& command );

   bool     StoreUserInfo( PacketDbQueryResult* dbResult );
   void     SaveUserSettings( UserPlusProfileTable& enigma, U8 gameProductId );
   void     SaveUpdatedProfile( const PacketUpdateUserProfile* profileUpdate, int adminLevelOfCaller, bool writeToDB );
   void     PackUserProfileRequestAndSendToClient( U32 connectionId );
   
};

//////////////////////////////////////////////////////////////////////////
