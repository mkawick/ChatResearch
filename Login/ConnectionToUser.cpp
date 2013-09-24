// ConnectionToUser.cpp

#include "ConnectionToUser.h"

#include "../NetworkCommon/Packets/DbPacket.h"
#include "../NetworkCommon/Packets/LoginPacket.h"
#include "../NetworkCommon/Packets/CheatPacket.h"
#include "../NetworkCommon/Packets/PacketFactory.h"
#include "../NetworkCommon/Utils/Utils.h"
#include <boost/lexical_cast.hpp>

#include "DiplodocusLogin.h"

//////////////////////////////////////////////////////////////////////////

DiplodocusLogin* ConnectionToUser::userManager = NULL;

//////////////////////////////////////////////////////////////////////////

ConnectionToUser:: ConnectionToUser( const string& name, const string& pword, const string& key ) : 
                     username( name ), 
                     passwordHash( pword ), 
                     loginKey( key ), 
                     status( LoginStatus_Pending ), 
                     isActive( true ), 
                     loggedOutTime( 0 ),
                     adminLevel( 0 ),
                     languageId( 1 ),
                     showWinLossRecord( true ),
                     marketingOptOut( false ),
                     showGenderProfile( false )
                     {}

//-----------------------------------------------------------------

void  ConnectionToUser::LoginResult( PacketDbQueryResult* dbResult )
{
   StoreUserInfo( dbResult );
}

//-----------------------------------------------------------------

bool  ConnectionToUser::StoreUserInfo( PacketDbQueryResult* dbResult )
{
   string lookupKey = dbResult->meta;

   UserPlusProfileTable             enigma( dbResult->bucket );
   UserPlusProfileTable::row        row = *enigma.begin();
   string lookup_email;
   if( dbResult->successfulQuery == true )
   {
      lookup_email = row[ TableUserPlusProfile::Column_email ];
   }

   if( lookupKey == username && dbResult->id == connectionId )//&& lookupkey)
   {
      if( dbResult->successfulQuery == false || dbResult->bucket.bucket.size() == 0 )// no records found
      {
         return false;
      }

      SaveUserSettings( enigma, dbResult->serverLookup );
      connectionId =                   dbResult->id;
      if( loginKey.size() == 0 )
      {
         U32 hash = static_cast<U32>( GenerateUniqueHash( boost::lexical_cast< string >( dbResult->id ) + email ) );
         loginKey = GenerateUUID( GetCurrentMilliseconds() + hash );
      }
   }
   else
   {
      map< string, ConnectionToUser>:: iterator it = adminUserData.find( lookupKey );
      if( it != adminUserData.end() )
      {
         if( dbResult->bucket.bucket.size() == 0 )// no records found
         {
            userManager->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_Cheat_BadUserLookup );
            return false;
         }
         if( dbResult->successfulQuery == true )
         {
            ConnectionToUser& conn = it->second;
            conn.SaveUserSettings( enigma, 0 );
            conn.PackUserProfileRequestAndSendToClient( connectionId );
         }
         else
         {
            adminUserData.erase( it );
         }
      }
      else
      {
         assert( 0 );// we submitted a query and got an invalid result
      }
   }

   return true;
}

//-----------------------------------------------------------------

void  ConnectionToUser::SaveUserSettings( UserPlusProfileTable& enigma, U8 productId )
{
   UserPlusProfileTable::row row = *enigma.begin();

   id =                             row[ TableUserPlusProfile::Column_id ];
   username =                       row[ TableUserPlusProfile::Column_name ];
   userUuid =                       row[ TableUserPlusProfile::Column_uuid ];
   email =                          row[ TableUserPlusProfile::Column_email ];
   passwordHash =                   row[ TableUserPlusProfile::Column_password_hash ];
   
   lastLoginTime =                  row[ TableUserPlusProfile::Column_last_logout_time ]; // note that we are using logout for our last login time.
   loggedOutTime =                  0;

   isActive =                       boost::lexical_cast<bool>( row[ TableUserPlusProfile::Column_active] );

   if( row[ TableUserPlusProfile::Column_language_id] != "NULL" )
   {
      languageId =                     boost::lexical_cast<int>( row[ TableUserPlusProfile::Column_language_id] );
   }

   adminLevel =                     boost::lexical_cast< int > ( row[ TableUserPlusProfile::Column_admin_level] );
   marketingOptOut =                boost::lexical_cast< bool >( row[ TableUserPlusProfile::Column_marketing_opt_out] );
   showWinLossRecord =              boost::lexical_cast< bool >( row[ TableUserPlusProfile::Column_show_win_loss_record] );
   showGenderProfile =              boost::lexical_cast< bool >( row[ TableUserPlusProfile::Column_show_gender_profile] );

   gameProductId =                  productId;
}

//-----------------------------------------------------------------

void  ConnectionToUser::SaveUpdatedProfile( const PacketUpdateUserProfile* profileUpdate, int adminLevelOfCaller, bool writeToDB )
{
   if( adminLevelOfCaller > 0 ) // only admins can change a user's name etc.
   {
      // some fields may be blank
      if( profileUpdate->username.size() > 0 )
      {
         username =                       profileUpdate->username;
      }
      //userUuid =                       profileUpdate->userUuid;
      if( profileUpdate->email.size() > 0 )
      {
         email =                          profileUpdate->email;
      }
      if( profileUpdate->passwordHash.size() > 0 )
      {
         passwordHash =                   profileUpdate->passwordHash;
      }
   }
   
   //lastLoginTime =                  profileUpdate->lastLoginTime;
   //loggedOutTime =                  GetDateFromString( profileUpdate->loggedOutTime.c_str() );

   isActive =                       profileUpdate->isActive;
   languageId =                     profileUpdate->languageId;

   adminLevel =                     profileUpdate->adminLevel;
   marketingOptOut =                profileUpdate->marketingOptOut;
   showWinLossRecord =              profileUpdate->showWinLossRecord;
   showGenderProfile =              profileUpdate->showGenderProfile;

   ConnectionToUser* loadedConnection = userManager->GetLoadedUserConnectionByUuid( userUuid );
   if( loadedConnection != NULL )
   {
      loadedConnection->SaveUpdatedProfile( profileUpdate, adminLevelOfCaller, false );// we will write if needed... no pass thru required
   }

   if( writeToDB )
   {
      PacketDbQuery* dbQuery = new PacketDbQuery;
      dbQuery->id =           connectionId;
      dbQuery->lookup =       DiplodocusLogin::QueryType_UpdateUsers; 
      dbQuery->meta =         "";
      dbQuery->serverLookup = gameProductId;
      dbQuery->isFireAndForget = true;

      string query = "UPDATE playdek.users SET user_name='%s',user_email='%s',user_pw_hash='%s',active=";
      query += boost::lexical_cast<string>( isActive?1:0 );
      query += ",language_id=";
      query += boost::lexical_cast<string>( languageId );
      query += "  WHERE user_id=";
      query += boost::lexical_cast<string>( id );
      dbQuery->query = query;
      dbQuery->escapedStrings.insert( username );
      dbQuery->escapedStrings.insert( email );
      dbQuery->escapedStrings.insert( passwordHash );

      userManager->AddQueryToOutput( dbQuery );

      //---------------------------------------
      dbQuery = new PacketDbQuery;
      dbQuery->id =           connectionId;
      dbQuery->lookup =       DiplodocusLogin::QueryType_UpdateUserProfile;
      dbQuery->meta =         "";
      dbQuery->serverLookup = gameProductId;
      dbQuery->isFireAndForget = true;

      query = "UPDATE playdek.user_profile SET admin_level=";
      query += boost::lexical_cast<string>( adminLevel );
      query += ",marketing_opt_out=";
      query += boost::lexical_cast<string>( marketingOptOut?1:0 );
      query += ",show_win_loss_record=";
      query += boost::lexical_cast<string>( showWinLossRecord?1:0 );
      query += ",show_profile_gender=";
      query += boost::lexical_cast<string>( showGenderProfile?1:0 );
      dbQuery->query = query;

      userManager->AddQueryToOutput( dbQuery );
   }
   PackUserProfileRequestAndSendToClient( connectionId );
}

//-----------------------------------------------------------------

bool  ConnectionToUser::BeginLogout( bool wasDisconnectedByError )
{
   if( loggedOutTime ) /// we are already logging out. The gateway may send us multiple logouts so we simply have to ignore further attemps
      return false;

   status = LoginStatus_Invalid;

   time( &loggedOutTime ); // time stamp this guy
   if( wasDisconnectedByError )
   {
      return FinalizeLogout();
   }
   if( wasDisconnectedByError == false )
   {
      PacketLogoutToClient* logout = new PacketLogoutToClient();
      logout->username =            username;// just for loggin purposes
      logout->uuid =                userUuid;
      return userManager->SendPacketToGateway( logout, connectionId );
   }

   return false;
}

//-----------------------------------------------------------------

bool  ConnectionToUser::FinalizeLogout()
{
   if( userUuid.size() == 0 )// this should never happen, but being careful never hurts.
      return false;

   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =              connectionId;
   dbQuery->lookup =          DiplodocusLogin::QueryType_UserLoginInfo;
   dbQuery->isFireAndForget = true;// no result is needed
   
   string queryString = "UPDATE users AS user SET user.last_logout_timestamp=CURRENT_TIMESTAMP WHERE uuid = '";
   queryString +=             userUuid;
   queryString += "'";
   dbQuery->query =           queryString;

   //userManager->SendPacketToGateway( logout, connectionId );
   return userManager->AddQueryToOutput( dbQuery );
}

//-----------------------------------------------------------------

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

//-----------------------------------------------------------------

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

//-----------------------------------------------------------------

bool  ConnectionToUser::UpdateLastLoggedInTime()
{
   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =              connectionId ;
   dbQuery->lookup =          DiplodocusLogin::QueryType_UpdateLastLoggedInTime;
   dbQuery->isFireAndForget = true;// no result is needed

   string queryString = "UPDATE users AS user SET user.last_login_timestamp=CURRENT_TIMESTAMP WHERE uuid = '";
   queryString +=             userUuid;
   queryString += "'";
   dbQuery->query =           queryString;

   return userManager->AddQueryToOutput( dbQuery );
}

//-----------------------------------------------------------------

bool  ConnectionToUser::UpdateLastLoggedOutTime()
{
   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =              connectionId ;
   dbQuery->lookup =          DiplodocusLogin::QueryType_UpdateLastLoggedOutTime;
   dbQuery->isFireAndForget = true;// no result is needed

   string queryString = "UPDATE users AS user SET user.last_logout_timestamp=CURRENT_TIMESTAMP WHERE uuid = '";
   queryString += userUuid;
   queryString += "'";
   dbQuery->query =           queryString;

   return userManager->AddQueryToOutput( dbQuery );
}
                   
//-----------------------------------------------------------------

bool    ConnectionToUser:: SuccessfulLogin( U32 connectId, bool isReloggedIn )
{
   loggedOutTime = 0;// for relogin, we need this to be cleared.
   connectionId =          connectId;

   productFilterNames.clear();
   productsWaitingForInsertionToDb.clear();

   PacketLoginToGateway* loginStatus = new PacketLoginToGateway();
   if( username.size() )
   {
      loginStatus->username = username;
      loginStatus->uuid = userUuid;
      loginStatus->lastLogoutTime = lastLoginTime;
      loginStatus->loginKey = loginKey;
   }
   loginStatus->wasLoginSuccessful = true;
   loginStatus->adminLevel = adminLevel;

  /* if( isReloggedIn == false )
   {
      LoadUserProfile();
   }*/

   status = LoginStatus_LoggedIn;

   userManager->SendPacketToGateway( loginStatus, connectionId );

   if( loginStatus->adminLevel > 0 )
   {
      userManager->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_Login, PacketErrorReport::StatusSubtype_UserIsAdminAccount );
   }

   RequestListOfGames( userUuid );

   RequestListOfProducts( userUuid );

   //This is where we inform all of the games that the user is logged in.

   return true;//SendLoginStatusToOtherServers( username, userUuid, connectionId, gameProductId, lastLoginTime, active, email, passwordHash, userId, loginKey, true, false );
}

//------------------------------------------------------------------------------------------------
/*
bool     ConnectionToUser:: LoadUserProfile( U32 whoseProfileIsLoaded )
{
   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =           connectionId;
   dbQuery->meta =         "";
   dbQuery->lookup =       DiplodocusLogin::QueryType_UserProfile;
   dbQuery->serverLookup = whoseProfileIsLoaded;

   dbQuery->query = "SELECT * FROM user_profile WHERE user_id='%s'";

   dbQuery->escapedStrings.insert( id );

   return userManager->AddQueryToOutput( dbQuery );
}

//---------------------------------------------------------------

bool     ConnectionToUser:: AddBlankUserProfile()
{
   // insert into user_profile values( 32, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL )
   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =           connectionId;
   dbQuery->meta =         "";
   dbQuery->lookup =       DiplodocusLogin::QueryType_CreateBlankUserProfile;
   dbQuery->serverLookup = connectionId;
   dbQuery->isFireAndForget = true;

   dbQuery->query = "insert into user_profile values( '";
   dbQuery->query += boost::lexical_cast<string>( id );
   dbQuery->query += "', DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT )";

   bool success = userManager->AddQueryToOutput( dbQuery );

   LoadUserProfile();

   return success;
}*/

//---------------------------------------------------------------

bool  ConnectionToUser:: RequestListOfGames( const string& userUuid )
{
   return false;// not working this way anymore
   /*if( userUuid.size() && connectionId != 0 )
   {
      PacketDbQuery* dbQuery = new PacketDbQuery;
      dbQuery->id =     connectionId ;
      dbQuery->lookup = QueryType_UserListOfGame;

      string queryString = "SELECT game.uuid, game.name FROM game INNER JOIN user_join_game AS user_game ON game.uuid=user_game.game_uuid WHERE user_game.user_uuid = '%s'";
      dbQuery->escapedStrings.insert( userUuid );
      dbQuery->query =  queryString;

      return userManager->AddQueryToOutput( dbQuery );
   }
   return false;*/
}

//---------------------------------------------------------------

bool     ConnectionToUser:: RequestListOfProducts( const string& userUuid )
{
   //return false;// not working this way anymore
   if( userUuid.size() && connectionId != 0 )
   {
      PacketDbQuery* dbQuery = new PacketDbQuery;
      dbQuery->id =     connectionId ;
      dbQuery->lookup = DiplodocusLogin::QueryType_UserListOfUserProducts;

      string queryString = "SELECT product.product_id, filter_name FROM product INNER JOIN user_join_product AS user_join_product ON product.uuid=user_join_product.product_id WHERE user_join_product.user_uuid='%s'";
      dbQuery->query =  queryString;
      dbQuery->escapedStrings.insert( userUuid );

      return userManager->AddQueryToOutput( dbQuery );
   }
   return false;
}

//---------------------------------------------------------------

bool     ConnectionToUser:: HandleGameReportedListOfPurchases( const PacketRequestListOfUserPurchases* purchase )
{
   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =     connectionId ;
   dbQuery->lookup = DiplodocusLogin::QueryType_GetProductListForUser;
   dbQuery->meta = "all";


   string queryString = "SELECT * FROM product";
   if( purchase->requestUserOnly == true )
   {
      dbQuery->meta = "user";
      queryString += " INNER JOIN user_join_product AS user_join_product ON product.uuid=user_join_product.product_id WHERE user_join_product.user_uuid='%s'";
      dbQuery->escapedStrings.insert( userUuid );
   }
   dbQuery->query =  queryString;   

   return userManager->AddQueryToOutput( dbQuery );
}

//---------------------------------------------------------------

bool     ConnectionToUser:: AddPurchase( const PacketRequestListOfUserPurchases* purchase )
{
}

//---------------------------------------------------------------
/*
bool     ConnectionToUser:: HandleUserProfileFromDb( PacketDbQueryResult* dbResult )
{
   UserProfileTable            enigma( dbResult->bucket );

   UserProfileTable::iterator  it = enigma.begin();
            
   if( it != enigma.end() )
   {
      UserProfileTable::row       row = *it++;

      adminLevel =         boost::lexical_cast< int > ( row[ TableUserProfile::Column_admin_level] );
      marketingOptOut =    boost::lexical_cast< bool >( row[ TableUserProfile::Column_marketing_opt_out] );
      showWinLossRecord =  boost::lexical_cast< bool >( row[ TableUserProfile::Column_show_win_loss_record] );
      showGenderProfile =  boost::lexical_cast< bool >( row[ TableUserProfile::Column_show_gender_profile] );
   }
   else
   {
      return false;
   }

   return true;
}*/

//---------------------------------------------------------------

bool     ConnectionToUser:: StoreUserPurchases( const PacketListOfUserPurchases* deviceReportedPurchases )
{
   int numItems = deviceReportedPurchases->purchases.size();
   for( int i=0; i< numItems; i++ )
   {
      const PurchaseEntry& purchaseEntry = deviceReportedPurchases->purchases[i];
      int  originalProductNameIndex = userManager->FindProductByName( purchaseEntry.name );
      //----------------
      if( purchaseEntry.name.size() == 0 )// we can't do anything with this.
      {
         cout << "   ***Invalid product id...title: " << purchaseEntry.productStoreId << "   price: " << purchaseEntry.price <<  "   number price: " << purchaseEntry.number_price << endl;
      }
      else
      {
         // the order ot the next two lines matters a lot.
         int userProductIndex = FindProductFilterName( purchaseEntry.name );
         AddProductFilterName( purchaseEntry.name );// we're gonna save the name, regardless. The device told us about the purchase.

         //**  find the item in the user record and add it to the db if not **
         if( userProductIndex == -1 && originalProductNameIndex != -1 )// the user doesn't have the record, but the rest of the DB does.
         {
            ProductInfo productInfo;
            bool result = userManager->GetProductByIndex( originalProductNameIndex, productInfo );
            if( result == true )
            {
               const string& productUuid = productInfo.uuid;
               //WriteProductToUserRecord( productUuid, purchaseEntry.number_price );
               WriteProductToUserRecord( userUuid, productUuid, purchaseEntry.number_price, purchaseEntry.quantity, false, "" );
            }
         }

         if( originalProductNameIndex == -1 )
         {
            ProductInfo pi;
            pi.name = purchaseEntry.name;
            pi.filterName = purchaseEntry.name;
            // productStoreId .. I don't know what to do with this.
            pi.price = purchaseEntry.number_price;

            productsWaitingForInsertionToDb.push_back( pi );
            userManager->AddNewProductToDb( purchaseEntry );
         }
         cout << "   title: " << purchaseEntry.name << "   price: " << purchaseEntry.price <<  "   number price: " << purchaseEntry.number_price << endl;
      }
      
   }

   return true;
}

//---------------------------------------------------------------

void     ConnectionToUser:: AddCurrentlyLoggedInProductToUserPurchases()
{
   const char* loggedInGameProductName = FindProductName( gameProductId );
   if( loggedInGameProductName == NULL )
   {
      cout << "Major error: user logging in with a product not identified" << endl;
      userManager->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_Login_CannotAddCurrentProductToUser ); 
      
      return;
   }

   ProductInfo productInfo;
   bool found = userManager->GetProductByProductId( gameProductId, productInfo );
   if( found == false )
   {
      cout << "Major error: user logging in with a product not in our list of loaded products" << endl;
      return;
   }

   WriteProductToUserRecord( userUuid, productInfo.uuid, 0.0, 1, 1, "default by login" );
}

//---------------------------------------------------------------

void     ConnectionToUser:: WriteProductToUserRecord( const string& productFilterName, double pricePaid )
{
   WriteProductToUserRecord( userUuid, productFilterName, pricePaid, 1, 1, "new product reported by user login" );
}

//---------------------------------------------------------------

void     ConnectionToUser:: WriteProductToUserRecord( const string& userUuid, const string& productFilterName, double pricePaid, float numPurchased, bool providedByAdmin, string adminNotes )
{
   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =           0;
   dbQuery->lookup =       DiplodocusLogin::QueryType_AddProductInfoToUser;
   dbQuery->meta =         productFilterName;
   dbQuery->serverLookup = 0;
   dbQuery->isFireAndForget = true;

   dbQuery->query = "INSERT INTO user_join_product VALUES( DEFAULT, '%s', '%s', DEFAULT, ";
   dbQuery->query += boost::lexical_cast< string >( pricePaid );
   dbQuery->query += ", 1, 1,";
   dbQuery->query += boost::lexical_cast< string >( providedByAdmin? 1:0 );
   dbQuery->query += ", '%s', NULL)";

   dbQuery->escapedStrings.insert( userUuid );
   dbQuery->escapedStrings.insert( productFilterName );
   dbQuery->escapedStrings.insert( adminNotes );

   userManager->AddQueryToOutput( dbQuery );

   userManager->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_Status, PacketErrorReport::StatusSubtype_ProductAdded );
}


//---------------------------------------------------------------

void     ConnectionToUser:: StoreListOfUsersProductsFromDB( PacketDbQueryResult* dbResult, bool shouldAddLoggedInProduct )
{
   bool  didFindGameProduct = false;

   // verify that this product is owned by the player and if not, then add an entry
   KeyValueParser  enigma( dbResult->bucket );
   KeyValueParser::iterator      it = enigma.begin();
   int   numProducts = dbResult->bucket.bucket.size();
   while( it != enigma.end() )
   {
      KeyValueParser::row       row = *it++;

      int productId = boost::lexical_cast< int> ( row[ TableKeyValue::Column_key ] );
      if( gameProductId == productId )
      {
         didFindGameProduct = true;
      }
      productFilterNames.push_back( row[ TableKeyValue::Column_value ] );
   }

   if( didFindGameProduct == false && shouldAddLoggedInProduct == true )
   {
      AddCurrentlyLoggedInProductToUserPurchases();
   }
}

//---------------------------------------------------------------

bool     ConnectionToUser:: HandleCheats( const PacketCheat* cheat )
{
   cout << endl << "---- Cheat received ----" << endl;
   if( adminLevel < 1 )
   {
      cout << "user admin level is too low" << endl;
      return false;
   }

   //QueryType_GetProductListForUser
   const char* cheat_text = cheat->cheat.c_str();

   std::vector<std::string> strings;
   split( cheat->cheat, strings );

   
   int count = 0;
   std::vector<std::string>::iterator it =  strings.begin();
   while( it != strings.end() )
   {
      cout << *it++ ;
      if( it != strings.end() )
         cout << ", ";
   }
   cout << endl;

   string command = *strings.begin();
   string param = *(strings.begin()+1);

   if( command == "remove_all" )
   {
      return HandleCheat_RemoveAll( param );
   }
   else if( command == "add_product" )
   {
      return HandleCheat_AddProduct( param );
   }
   else
   {
      return userManager->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_Cheat_UnrecognizedCommand );
   }
   // 
   return true;
}

//---------------------------------------------------------------

bool     ConnectionToUser:: HandleCheat_RemoveAll( const string& command )
{
   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =           0;
   dbQuery->lookup =       DiplodocusLogin::QueryType_RemoveAllProductInfoForUser;
   dbQuery->meta =         command;
   dbQuery->serverLookup = 0;
   dbQuery->isFireAndForget = true;

   dbQuery->query = "DELETE FROM user_join_product WHERE user_uuid='%s'";
   dbQuery->escapedStrings.insert( userUuid );

   userManager->AddQueryToOutput( dbQuery );

   userManager->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_Status, PacketErrorReport::StatusSubtype_AllProductsRemoved );
   return true;
}

//---------------------------------------------------------------

bool     ConnectionToUser:: HandleCheat_AddProduct( const string& productName )
{
   int productIndex = userManager->FindProductByName( productName );
   if( productIndex == -1 )
   {
      userManager->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_Cheat_ProductUnknown );
      return false;
   }

   ProductInfo product;
   bool result = userManager->GetProductByIndex( productIndex, product );
   if( result == true )
   {
      //WriteProductToUserRecord( userUuid, product.uuid, product.price );
      WriteProductToUserRecord( userUuid, productName, 0.0, 1, 1, "added by cheat" );
   }

   return true;
}

//-----------------------------------------------------------------

void     ConnectionToUser:: PackUserProfileRequestAndSendToClient( U32 connectionId )
{
   PacketRequestUserProfileResponse* response = new PacketRequestUserProfileResponse;
   // string username, string email, string userUuid, string lastLoginTime, string loggedOutTime, int adminLevel, bool isActive, bool showWinLossRecord, bool marketingOptOut, bool showGenderProfile
   response->username =          username;
   response->userUuid =          userUuid;
   response->email =             email;
   response->lastLoginTime =     lastLoginTime;
   response->loggedOutTime =     GetDateInUTC( loggedOutTime );

   response->adminLevel =        adminLevel;
   response->isActive =          isActive;
   response->showWinLossRecord = showWinLossRecord;
   response->marketingOptOut =   marketingOptOut;
   response->showGenderProfile = showGenderProfile;

   userManager->SendPacketToGateway( response, connectionId );
}

//-----------------------------------------------------------------

bool     ConnectionToUser:: RequestProfile( const PacketRequestUserProfile* profileRequest )
{
   if( profileRequest->uuid == userUuid )
   {
      PackUserProfileRequestAndSendToClient( connectionId );
      return true;
   }
   if( adminLevel < 1 )
   {
      userManager->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_Cheat_BadPermissions );
      return false;
   }

   string requestId = profileRequest->userEmail + profileRequest->uuid + profileRequest->userName;
   if( requestId.size() == 0 )
   {
      userManager->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_Cheat_BadUserLookup );
      return true;
   }
   else
   {
      map< string, ConnectionToUser> ::iterator it = adminUserData.find( requestId );// just use the existing data.
      if( it != adminUserData.end() )
      {
         it->second.PackUserProfileRequestAndSendToClient( connectionId );
         return true;
      }
      
   }

  // submit request for user profile with this connection id
   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =           connectionId;
   dbQuery->lookup =       DiplodocusLogin::QueryType_UserLoginInfo;
   dbQuery->meta =         boost::lexical_cast<string>( GenerateUniqueHash( requestId ) );
   dbQuery->serverLookup = gameProductId;

   string temp = "select * from users JOIN user_profile ON users.user_id=user_profile.user_id WHERE users.user_email='%s' OR users.uuid='%s' OR users.user_name='%s' LIMIT 1";
   dbQuery->escapedStrings.insert( profileRequest->userEmail );
   dbQuery->escapedStrings.insert( profileRequest->uuid );
   dbQuery->escapedStrings.insert( profileRequest->userName );
   dbQuery->query = temp;

   adminUserData.insert( UserConnectionPair( dbQuery->meta, ConnectionToUser() ) );
   userManager->AddQueryToOutput( dbQuery );

   return false;
}

//-----------------------------------------------------------------

bool     ConnectionToUser:: UpdateProfile( const PacketUpdateUserProfile* updateProfileRequest )
{
   if( updateProfileRequest->adminLevel >= adminLevel )
   {
      userManager->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_Cheat_BadPermissions );
      return false;
   }

   if( updateProfileRequest->userUuid == userUuid )
   {
      SaveUpdatedProfile( updateProfileRequest, adminLevel, true );
      return true;
   }
    
   string requestId = updateProfileRequest->email + updateProfileRequest->userUuid + updateProfileRequest->username;
   if( requestId.size() != 0 )
   {
      map< string, ConnectionToUser> ::iterator it = adminUserData.find( requestId );// just use the existing data.
      if( it != adminUserData.end() )
      {
         it->second.SaveUpdatedProfile( updateProfileRequest, adminLevel,true );
         return true;
      }
   }
   
   map< string, ConnectionToUser> ::iterator it = adminUserData.begin();
   while( it != adminUserData.end() )
   {
      if( updateProfileRequest->username.size() != 0 && it->second.username == updateProfileRequest->username )
      {
         it->second.SaveUpdatedProfile( updateProfileRequest, adminLevel,true );
         return true;
      }
      if( updateProfileRequest->userUuid.size() != 0 && it->second.userUuid == updateProfileRequest->userUuid )
      {
         it->second.SaveUpdatedProfile( updateProfileRequest, adminLevel,true );
         return true;
      }
      if( updateProfileRequest->email.size() != 0 && it->second.email == updateProfileRequest->email )
      {
         it->second.SaveUpdatedProfile( updateProfileRequest, adminLevel,true );
         return true;
      }
      it++;
   }

   userManager->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_Cheat_BadUserLookup_TryLoadingUserFirst );

   return false;
}

//////////////////////////////////////////////////////////////////////////
