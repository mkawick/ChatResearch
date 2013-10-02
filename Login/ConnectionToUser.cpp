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

string CreateLookupKey( const string& email, const string& userUuid, const string& userName )
{
   if( email.size() && userUuid.size() && userName.size() )
   {
      return email + userUuid + userName;
   }
   return string();
}

//////////////////////////////////////////////////////////////////////////

ConnectionToUser:: ConnectionToUser( const string& name, const string& pword, const string& key ) : 
                     userName( name ), 
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

bool  ConnectionToUser::HandleAdminRequestUserProfile( PacketDbQueryResult* dbResult )
{
   string lookupKey = dbResult->meta;
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
         UserPlusProfileTable             enigma( dbResult->bucket );
         ConnectionToUser& conn = it->second;
         conn.SaveUserSettings( enigma, 0 );
         conn.PackUserProfileRequestAndSendToClient( connectionId );
         RequestListOfPurchases( conn.userUuid );
      }
      else
      {
         adminUserData.erase( it );
      }
   }
   else
   {
      assert( 0 );// we submitted a query and got an invalid result
      return false;
   }
   return true;
}

//-----------------------------------------------------------------

bool  ConnectionToUser::StoreUserInfo( PacketDbQueryResult* dbResult )
{
   if( dbResult->successfulQuery == false || dbResult->bucket.bucket.size() == 0 )// no records found
   {
      return false;
   }

   UserPlusProfileTable             enigma( dbResult->bucket );
   UserPlusProfileTable::row        row = *enigma.begin();
   string lookup_email;
   if( dbResult->successfulQuery == true )
   {
      lookup_email = row[ TableUserPlusProfile::Column_email ];
   }

   SaveUserSettings( enigma, dbResult->serverLookup );
   connectionId =                   dbResult->id;
   if( loginKey.size() == 0 )
   {
      U32 hash = static_cast<U32>( GenerateUniqueHash( boost::lexical_cast< string >( dbResult->id ) + email ) );
      loginKey = GenerateUUID( GetCurrentMilliseconds() + hash );
   }

   return true;
}

//-----------------------------------------------------------------

bool  ConnectionToUser::StoreProductInfo( PacketDbQueryResult* dbResult )
{
   /*int numProducts = dbResult->bucket.bucket.size();
               
      PacketListOfUserPurchases* purchasePacket = new PacketListOfUserPurchases();
      purchasePacket->isAllProducts = false;
      if( dbResult->meta == "all" )
         purchasePacket->isAllProducts = true;
      purchasePacket->platformId = 0;

      if( purchasePacket->isAllProducts == true )
      {
         ProductTable            enigma( dbResult->bucket );
         ProductTable::iterator  it = enigma.begin();
         while( it != enigma.end() )
         {
            ProductTable::row       row = *it++;

            PurchaseEntry pe;
            pe.productStoreId =     row[ TableProduct::Column_name ];
            pe.name =               row[ TableProduct::Column_filter_name ];
            purchasePacket->purchases.push_back( pe );
         }
      }
      else
      {
         
      }

      SendPacketToGateway( purchasePacket, connectionId );
      */
 /*  ProductJoinUserProductTable            enigma( dbResult->bucket );
   ProductJoinUserProductTable::iterator  it = enigma.begin();
   while( it != enigma.end() )
   {
      ProductJoinUserProductTable::row       row = *it++;


      int id =          boost::lexical_cast< int >  ( row[ TableProductJoinUserProduct::Column_id ] );

      string name =           row[ TableProductJoinUserProduct::Column_filter_name ];
      string uuid =           row[ TableProductJoinUserProduct::Column_uuid ];
      float quantity =  boost::lexical_cast< float >( row[ TableProductJoinUserProduct::Column_num_purchased ] );

      AddToProductsOwned( id, name, uuid, quantity );
   }*/

   return true;
}

//-----------------------------------------------------------------

void  ConnectionToUser::SaveUserSettings( UserPlusProfileTable& enigma, U8 productId )
{
   UserPlusProfileTable::row row = *enigma.begin();

   id =                             row[ TableUserPlusProfile::Column_id ];
   userName =                       row[ TableUserPlusProfile::Column_name ];
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
      if( profileUpdate->userName.size() > 0 )
      {
         userName =                       profileUpdate->userName;
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
      dbQuery->escapedStrings.insert( userName );
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
      logout->userName =            userName;// just for loggin purposes
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
   if( userName.size() )
   {
      loginStatus->userName = userName;
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

  // RequestListOfGames( userUuid );

   RequestListOfPurchases( userUuid );

   //This is where we inform all of the games that the user is logged in.

   return true;//SendLoginStatusToOtherServers( userName, userUuid, connectionId, gameProductId, lastLoginTime, active, email, passwordHash, userId, loginKey, true, false );
}

//---------------------------------------------------------------
/*
bool  ConnectionToUser:: RequestListOfGames( const string& userUuid )
{
   return false;// not working this way anymore
}*/

//---------------------------------------------------------------

bool     ConnectionToUser:: RequestListOfPurchases( const string& user_uuid )
{
  /* if( userUuid == user_uuid )
   {
      return PackageListOfProducts();
   }*/

   if( userUuid.size() && connectionId != 0 )
   {
      PacketDbQuery* dbQuery = new PacketDbQuery;
      dbQuery->id =     connectionId ;
      dbQuery->lookup = DiplodocusLogin::QueryType_UserListOfUserProducts;
      dbQuery->meta = user_uuid;

      string queryString = "SELECT product.product_id, filter_name, product.uuid, num_purchased FROM product INNER JOIN user_join_product AS user_join_product ON product.uuid=user_join_product.product_id WHERE user_join_product.user_uuid='%s'";
      dbQuery->query =  queryString;
      dbQuery->escapedStrings.insert( user_uuid );

      return userManager->AddQueryToOutput( dbQuery );
   }
   return false;
}

//---------------------------------------------------------------

bool     ConnectionToUser:: HandleRequestForListOfPurchases( const PacketListOfUserPurchasesRequest* purchase )
{
   //FindUser( "", purchase->userUuid, "" );
   UserConnectionMapIterator  it = FindUser( "", purchase->userUuid, "" );
   if( it == adminUserData.end() )
   {
      userManager->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_Cheat_BadUserLookup_TryLoadingUserFirst );
         
      return false;
   }

   if( purchase->userUuid == userUuid )
   {
      SendListOfProductsToClient( connectionId );
   }

   return true;
}

//---------------------------------------------------------------

void     ConnectionToUser:: SendListOfProductsToClient( U32 connectionId )
{
   PacketListOfUserAggregatePurchases* purchases = new PacketListOfUserAggregatePurchases;

   map< U32, ProductBrief >::iterator it = productsOwned.begin();
   while( it != productsOwned.end() )
   {
      const ProductBrief& pb = it->second;
      it++;

      if( pb.quantity == 0 )
         continue;

      PurchaseEntry pe;
      pe.name = pb.filterName;
      pe.quantity = pb.quantity;
      pe.productStoreId = pb.uuid;

      purchases->purchases.push_back( pe );
   }

   userManager->SendPacketToGateway( purchases, connectionId );
}

//---------------------------------------------------------------

bool     ConnectionToUser:: AddPurchase( const PacketAddPurchaseEntry* purchase )
{
   if( adminLevel > 0 )
   {
      ProductInfo productInfo;
      string productUuid = purchase->productUuid;
      bool success = userManager->FindProductByUuid( productUuid, productInfo );
      if( success == false )
      {
         userManager->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_Cheat_ProductUnknown );
         return false;
      }

      if( purchase->userUuid == this->userUuid ||
          purchase->userEmail == this->email ||
          purchase->userName == this->userName
         )// giving to self for admins
      {
         double price = 0.0;
         float numToGive = static_cast<float>( purchase->quantity );
         WriteProductToUserRecord( userUuid, productUuid, price, numToGive, userUuid, "add purchase entry to self by admin" );
         AddToProductsOwned( productInfo.productId, productInfo.filterName, productUuid, numToGive );
         SendListOfProductsToClient( connectionId );
         return true;
      }

      string userUuid =    purchase->userUuid;
      string userEmail =   purchase->userEmail;
      string userName =    purchase->userName;

      UserConnectionMapIterator  it = FindUser( userEmail, userUuid, userName );
      if( it == adminUserData.end() )
      {
         userManager->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_Cheat_BadUserLookup );
         return true;
      }
      else
      {
         double price = 0.0;
         float numToGive = static_cast<float>( purchase->quantity );
         WriteProductToUserRecord( it->second.userUuid, productUuid, price, numToGive, userUuid, "add purchase entry by admin" );

         it->second.AddToProductsOwned( productInfo.productId, productInfo.filterName, productUuid, numToGive );
         it->second.SendListOfProductsToClient( connectionId );

         ConnectionToUser* loadedConnection = userManager->GetLoadedUserConnectionByUuid( userUuid );
         if( loadedConnection )
         {
            loadedConnection->AddToProductsOwned( productInfo.productId, productInfo.filterName, productInfo.uuid, 1 );
            loadedConnection->SendListOfProductsToClient( connectionId );
         }
         return true;
      }
   }
   else
   {
      userManager->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_Cheat_BadPermissions );
   }
   return false;
}


//---------------------------------------------------------------

bool     ConnectionToUser:: StoreUserPurchases( const PacketListOfUserAggregatePurchases* deviceReportedPurchases )// only works for self
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
         // the order of the next two lines matters a lot.
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
               WriteProductToUserRecord( userUuid, productUuid, purchaseEntry.number_price, purchaseEntry.quantity, "", "" );
               AddToProductsOwned( productInfo.productId, productInfo.filterName, productUuid, purchaseEntry.quantity );
            }
         }

         if( originalProductNameIndex == -1 )
         {
            ProductInfo pi;
            pi.name = purchaseEntry.name;
            pi.filterName = purchaseEntry.name;
            // productStoreId .. I don't know what to do with this.
            pi.price = purchaseEntry.number_price;
            pi.quantity = purchaseEntry.quantity;

            productsWaitingForInsertionToDb.push_back( pi );
            //productsOwned.insert( pi );
            userManager->AddNewProductToDb( purchaseEntry );
         }
         cout << "   title: " << purchaseEntry.name << "   price: " << purchaseEntry.price <<  "   number price: " << purchaseEntry.number_price << endl;
      }
      
   }

   return true;
}

//---------------------------------------------------------------

void     ConnectionToUser:: AddCurrentlyLoggedInProductToUserPurchases()// only works for self
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

   WriteProductToUserRecord( userUuid, productInfo.uuid, 0.0, 1, "user", "default by login" );
   float numToGive = 1;
   AddToProductsOwned( productInfo.productId, productInfo.filterName, productInfo.uuid, numToGive );
}

//---------------------------------------------------------------

void     ConnectionToUser:: WriteProductToUserRecord( const string& productFilterName, double pricePaid )
{
   int userProductIndex = FindProductFilterName( productFilterName );
   //**  find the item in the user record and add it to the db if not **
   if( userProductIndex == -1 )// the user doesn't have the record, but the rest of the DB does.
   {
      userManager->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_Cheat_ProductUnknown ); 
      return;
   }

   ProductInfo productInfo;
   bool result = userManager->GetProductByIndex( userProductIndex, productInfo );
   if( result == true )
   {
      WriteProductToUserRecord( userUuid, productInfo.uuid, pricePaid, 1, this->userUuid, "new product reported by user login" );
      AddToProductsOwned( productInfo.productId, productInfo.filterName, productInfo.uuid, 1 );
   }
   else
   {
      userManager->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_Cheat_ProductUnknown ); 
   }
}

//---------------------------------------------------------------

void     ConnectionToUser:: WriteProductToUserRecord( const string& userUuid, const string& productUuid, double pricePaid, float numPurchased, string adminId, string adminNotes )
{
   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =           0;
   dbQuery->lookup =       DiplodocusLogin::QueryType_AddProductInfoToUser;
   dbQuery->meta =         productUuid;
   dbQuery->serverLookup = 0;
   dbQuery->isFireAndForget = true;

   dbQuery->query = "INSERT INTO user_join_product VALUES( DEFAULT, '%s', '%s',DEFAULT,";
   dbQuery->query += boost::lexical_cast< string >( pricePaid );
   dbQuery->query += ",1,";
   dbQuery->query += boost::lexical_cast< string >( numPurchased );
   dbQuery->query += ",'%s','%s',NULL)";

   dbQuery->escapedStrings.insert( userUuid );
   dbQuery->escapedStrings.insert( productUuid );
   dbQuery->escapedStrings.insert( adminId );
   dbQuery->escapedStrings.insert( adminNotes );

   userManager->AddQueryToOutput( dbQuery );

   userManager->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_Status, PacketErrorReport::StatusSubtype_ProductAdded );
}


//---------------------------------------------------------------

void     ConnectionToUser:: StoreListOfUsersProductsFromDB( PacketDbQueryResult* dbResult, bool shouldAddLoggedInProduct )
{
   bool  didFindGameProduct = false;

   ConnectionToUser* userWhoGetsProducts = this;
   ConnectionToUser* loadedConnection = NULL;
   bool loadedForSelf = true;
   if( dbResult->meta.size() && dbResult->meta != this->userUuid )
   {
      UserConnectionMapIterator  it = FindUser( "", dbResult->meta, "" );
      if( it != adminUserData.end() )
      {
         userWhoGetsProducts = &it->second;
      }
      loadedConnection = userManager->GetLoadedUserConnectionByUuid( userUuid );
      loadedForSelf = false;
   }

   // verify that this product is owned by the player and if not, then add an entry
   userWhoGetsProducts->ClearAllProductsOwned();
   UserOwnedProductSimpleTable  enigma( dbResult->bucket );
   UserOwnedProductSimpleTable::iterator      it = enigma.begin();
   int   numProducts = dbResult->bucket.bucket.size();
   while( it != enigma.end() )
   {
      UserOwnedProductSimpleTable::row       row = *it++;

      int productId =   boost::lexical_cast< int>  (  row[ TableUserOwnedProductSimple::Column_product_id ] );
      string filterName =                             row[ TableUserOwnedProductSimple::Column_product_name ];
      string productUuid =                            row[ TableUserOwnedProductSimple::Column_product_uuid ];
      float  quantity = boost::lexical_cast< float> ( row[ TableUserOwnedProductSimple::Column_quantity ] );

      if( gameProductId == productId )
      {
         didFindGameProduct = true;
      }
      userWhoGetsProducts->AddToProductsOwned( productId, filterName, productUuid, quantity );
         
      if( loadedConnection != NULL )
      {
         loadedConnection->AddToProductsOwned( productId, filterName, productUuid, quantity );
      }
   }

   if( didFindGameProduct == false && shouldAddLoggedInProduct == true && loadedForSelf == true )
   {
      AddCurrentlyLoggedInProductToUserPurchases();
   }

   userWhoGetsProducts->SendListOfProductsToClient( connectionId );
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

   ProductInfo productInfo;
   bool result = userManager->GetProductByIndex( productIndex, productInfo );
   if( result == true )
   {
      //WriteProductToUserRecord( userUuid, product.uuid, product.price );
      WriteProductToUserRecord( userUuid, productInfo.uuid, 0.0, 1, userUuid, "added by cheat" );
      AddToProductsOwned( productInfo.productId, productInfo.filterName, productInfo.uuid, 1 );
      ConnectionToUser* loadedConnection = userManager->GetLoadedUserConnectionByUuid( userUuid );
      if( loadedConnection )
      {
         loadedConnection->AddToProductsOwned( productInfo.productId, productInfo.filterName, productInfo.uuid, 1 );
         loadedConnection->SendListOfProductsToClient( connectionId );
      }
   }

   return true;
}

//-----------------------------------------------------------------

void     ConnectionToUser:: PackUserProfileRequestAndSendToClient( U32 connectionId )
{
   PacketRequestUserProfileResponse* response = new PacketRequestUserProfileResponse;
   // string userName, string email, string userUuid, string lastLoginTime, string loggedOutTime, int adminLevel, bool isActive, bool showWinLossRecord, bool marketingOptOut, bool showGenderProfile
   response->userName =          userName;
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

void     ConnectionToUser:: ClearAllProductsOwned()
{
   productsOwned.clear();
   productFilterNames.clear();
}

//-----------------------------------------------------------------

void     ConnectionToUser:: AddToProductsOwned( int productDbId, const string& filterName, const string& productUuid, float quantity )
{
   map< U32, ProductBrief >::iterator it = productsOwned.find( productDbId );
   if( it != productsOwned.end() )
   {
      it->second.quantity += quantity;
   }
   else
   {
      ProductBrief brief;
      brief.productDbId = productDbId;
      brief.filterName = filterName;
      brief.uuid = productUuid;
      brief.quantity = quantity;
      productsOwned.insert( pair< U32, ProductBrief > ( productDbId, brief ) );

      AddProductFilterName( filterName );
   }
}

//-----------------------------------------------------------------

bool     ConnectionToUser:: RequestProfile( const PacketRequestUserProfile* profileRequest )
{
   if( profileRequest->uuid == userUuid || profileRequest->userName == userName || profileRequest->userEmail == email )
   {
      PackUserProfileRequestAndSendToClient( connectionId );
      return true;
   }
   if( adminLevel < 1 )
   {
      userManager->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_Cheat_BadPermissions );
      return false;
   }

   UserConnectionMapIterator  it = FindUser( profileRequest->userEmail, profileRequest->uuid, profileRequest->userName );
   if( it != adminUserData.end() )
   {
      it->second.PackUserProfileRequestAndSendToClient( connectionId );
      return true;
   }

   string requestId = CreateLookupKey( profileRequest->userEmail, profileRequest->uuid, profileRequest->userName );
  // submit request for user profile with this connection id
   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =           connectionId;
   dbQuery->lookup =       DiplodocusLogin::QueryType_AdminRequestUserProfile;
   dbQuery->meta =         boost::lexical_cast<string>( GenerateUniqueHash( requestId ) );
   dbQuery->serverLookup = gameProductId;

   string temp = "SELECT * FROM users JOIN user_profile ON users.user_id=user_profile.user_id WHERE users.user_email='%s' OR users.uuid='%s' OR users.user_name='%s' LIMIT 1";
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
    
  /* string requestId = CreateLookupKey( updateProfileRequest->email, updateProfileRequest->userUuid, updateProfileRequest->userName );
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
      if( updateProfileRequest->userName.size() != 0 && it->second.userName == updateProfileRequest->userName )
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
   }*/
   UserConnectionMapIterator  it = FindUser( updateProfileRequest->email, updateProfileRequest->userUuid, updateProfileRequest->userName );
   if( it != adminUserData.end() )
   {
      it->second.SaveUpdatedProfile( updateProfileRequest, adminLevel,true );
      return true;
   }

   userManager->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_Cheat_BadUserLookup_TryLoadingUserFirst );

   return false;
}

//-----------------------------------------------------------------

ConnectionToUser::UserConnectionMapIterator  
ConnectionToUser:: FindUser( const string& email, const string& userUuid, const string& userName )
{
   string requestId = CreateLookupKey( email, userUuid, userName );
   if( requestId.size() != 0 )
   {
      map< string, ConnectionToUser> ::iterator it = adminUserData.find( requestId );// just use the existing data.
      if( it != adminUserData.end() )
      {
         return it;
      }
   }
   map< string, ConnectionToUser> ::iterator it = adminUserData.begin();
   while( it != adminUserData.end() )
   {
      if( userName.size() != 0 && it->second.userName == userName )
      {
         return it;
      }
      if( userUuid.size() != 0 && it->second.userUuid == userUuid )
      {
         return it;
      }
      if( email.size() != 0 && it->second.email == email )
      {
         return it;
      }
      it++;
   }

   return adminUserData.end();
}

//////////////////////////////////////////////////////////////////////////
