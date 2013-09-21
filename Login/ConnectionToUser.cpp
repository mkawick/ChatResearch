// ConnectionToUser.cpp

#include "ConnectionToUser.h"
#include "../NetworkCommon/Utils/TableWrapper.h"
#include "../NetworkCommon/Packets/DbPacket.h"
#include "../NetworkCommon/Packets/LoginPacket.h"
#include "../NetworkCommon/Packets/CheatPacket.h"
#include "../NetworkCommon/Packets/PacketFactory.h"
#include "../NetworkCommon/Utils/Utils.h"
#include <boost/lexical_cast.hpp>

#include "DiplodocusLogin.h"

//////////////////////////////////////////////////////////////////////////

ConnectionToUser:: ConnectionToUser( const string& name, const string& pword, const string& key ) : 
                     username( name ), 
                     passwordHash( pword ), 
                     loginKey( key ), 
                     status( LoginStatus_Pending ), 
                     isActive( true ), 
                     loggedOutTime( 0 ),
                     adminLevel( 0 ),
                     showWinLossRecord( true ),
                     marketingOptOut( false ),
                     showGenderProfile( false )
                     {}

//-----------------------------------------------------------------

void  ConnectionToUser::LoginResult( PacketDbQueryResult* dbResult )
{
   UserTable            enigma( dbResult->bucket );
   UserTable::row       row = *enigma.begin();
   id =                    row[ TableUser::Column_id ];
   username =              row[ TableUser::Column_name ];
   userUuid =              row[ TableUser::Column_uuid ];
   email =                 row[ TableUser::Column_email ];
   passwordHash =          row[ TableUser::Column_password_hash ];
   // note that we are using logout for our last login time.
   lastLoginTime =         row[ TableUser::Column_last_logout_time ];
   isActive =              boost::lexical_cast<bool>( row[ TableUser::Column_active] );

   status =                ConnectionToUser::LoginStatus_LoggedIn;

   gameProductId =         dbResult->serverLookup;
   connectionId =          0;

   if( loginKey.size() == 0 )
   {
      U32 hash = static_cast<U32>( GenerateUniqueHash( boost::lexical_cast< string >( dbResult->id ) + email ) );
      loginKey = GenerateUUID( GetCurrentMilliseconds() + hash );
   }
}

//-----------------------------------------------------------------

bool  ConnectionToUser::BeginLogout( bool wasDisconnectedByError )
{
   if( loggedOutTime ) /// we are already logging out. The gateway may send us multiple logouts so we simply have to ignore further attemps
      return false;

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

   //*************************** this is a hack. This should be based on user profiles.
   loginStatus->adminLevel = 1;
   
   //***************************

   if( isReloggedIn == false )
   {
      LoadUserProfile();
   }

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
}

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
}

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
               WriteProductToUserRecord( productUuid, purchaseEntry.number_price );
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

   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =           0;
   dbQuery->lookup =       DiplodocusLogin::QueryType_AddProductInfoToUser;
   dbQuery->meta =         loggedInGameProductName;
   dbQuery->serverLookup = 0;
   dbQuery->isFireAndForget = true;


   dbQuery->query = "INSERT INTO user_join_product VALUES( NULL, '";
   dbQuery->query += userUuid;
   dbQuery->query += "', '";
   dbQuery->query += productInfo.uuid;
   dbQuery->query += "', NULL, 0, 0, 0 )";

   userManager->AddQueryToOutput( dbQuery );
}

//---------------------------------------------------------------

void     ConnectionToUser:: WriteProductToUserRecord( const string& productFilterName, double pricePaid )
{
   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =           0;
   dbQuery->lookup =       DiplodocusLogin::QueryType_AddProductInfoToUser;
   dbQuery->meta =         productFilterName;
   dbQuery->serverLookup = 0;
   dbQuery->isFireAndForget = true;

   dbQuery->query = "INSERT INTO user_join_product VALUES( NULL, '%s', '%s', NULL, ";
   dbQuery->query += boost::lexical_cast< string >( pricePaid );
   dbQuery->query += ", 1, NULL)";

   dbQuery->escapedStrings.insert( userUuid );
   dbQuery->escapedStrings.insert( productFilterName );

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
      WriteProductToUserRecord( product.uuid, product.price );
   }

   return true;
}

bool     ConnectionToUser:: RequestProfile( const PacketRequestUserProfile* profileRequest )
{
   if( profileRequest->uuid == userUuid )
   {
      
      return false;
   }
   if( adminLevel < 1 )
   {
      userManager->SendErrorToClient( connectionId, PacketErrorReport::ErrorType_Cheat_BadPermissions );
   }
  // submit request for user profile with this connection id
   return false;
}

bool     ConnectionToUser:: UpdateProfile( const PacketUpdateUserProfile* updateProfileRequest )
{
   return false;
}

//////////////////////////////////////////////////////////////////////////
