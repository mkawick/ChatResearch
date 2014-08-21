// ConnectionToUser.cpp

#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
using boost::format;

#include "ConnectionToUser.h"

#include "../NetworkCommon/Packets/DbPacket.h"
#include "../NetworkCommon/Packets/LoginPacket.h"
#include "../NetworkCommon/Packets/CheatPacket.h"
#include "../NetworkCommon/Packets/PacketFactory.h"
#include "../NetworkCommon/Database/StringLookup.h"

#include "../NetworkCommon/Utils/CommandLineParser.h"
#include "../NetworkCommon/Utils/Utils.h"
#include <boost/lexical_cast.hpp>

#include "DiplodocusLogin.h"
#include "ProductInfo.h"

//////////////////////////////////////////////////////////////////////////

DiplodocusLogin* ConnectionToUser::m_userManager = NULL;

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
                     m_userName( name ), 
                     m_passwordHash( pword ), 
                     m_loginKey( key ), 
                     m_avatarIcon( 0 ),
                     m_loginAttemptCount( 0 ),
                     status( LoginStatus_Pending ), 
                     m_gameProductId( 0 ),
                     m_connectionId( 0 ),
                     m_loginTime( 0 ),
                     m_loggedOutTime( 0 ),
                     m_isLoggingOut( false ),
                     m_isReadyToBeCleanedUp( false ),
                     m_gatewayId( 0 ),
                     m_timeZone( 0 ),
                     m_languageId( 1 ),
                     m_adminLevel( 0 ),
                     m_isActive( true ), 
                     m_showWinLossRecord( true ),
                     m_marketingOptOut( false ),
                     m_showGenderProfile( false ),
                     m_isSavingUserProfile( false ),
                     m_hasRequestedPurchasesFromClient( false ),
                     m_displayOnlineStatusToOtherUsers( false ),
                     m_blockContactInvitations( false ),
                     m_blockGroupInvitations( false )
                     {}

//-----------------------------------------------------------------

bool  ConnectionToUser::HandleAdminRequestUserProfile( const PacketDbQueryResult* dbResult )
{
   std::auto_ptr<bool> isFullProfile ( static_cast< bool* >( dbResult->customData ) );
   string lookupKey = dbResult->meta;
   map< string, ConnectionToUser>:: iterator it = adminUserData.find( lookupKey );
   if( it != adminUserData.end() )
   {
      if( dbResult->GetBucket().size() == 0 )// no records found
      {
         m_userManager->SendErrorToClient( m_connectionId, PacketErrorReport::ErrorType_Cheat_BadUserLookup );
         return false;
      }
      if( dbResult->successfulQuery == true )
      {
         UserPlusProfileTable             enigma( dbResult->bucket );
         ConnectionToUser& conn = it->second;
         conn.CopyUserSettings( enigma, 0 );
         if( dbResult->serverLookup != 0 )
         {
            conn.PackUserProfileRequestAndSendToClient( m_connectionId );
         }
         if( *isFullProfile )
         {
            RequestListOfPurchases( conn.m_userUuid );
         }
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

bool  ConnectionToUser::EchoHandler()
{
   cout << " Echo " << endl;
   PacketLogin_EchoToClient* echo = new PacketLogin_EchoToClient;
   m_userManager->SendPacketToGateway( echo, m_connectionId );
   return true;
}

//-----------------------------------------------------------------

bool  ConnectionToUser::LoginResult( const PacketDbQueryResult* dbResult )
{
   if( dbResult->successfulQuery == false || 
      dbResult->GetBucket().size() == 0 )// no records found
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

   CopyUserSettings( enigma, dbResult->serverLookup );
   m_connectionId =                   dbResult->id;
   if( m_loginKey.size() == 0 )
   {
      U32 hash = static_cast<U32>( GenerateUniqueHash( boost::lexical_cast< string >( dbResult->id ) + m_email ) );
      m_loginKey = GenerateUUID( GetCurrentMilliseconds() + hash );
   }

   return true;
}

//-----------------------------------------------------------------

bool  ConnectionToUser::StoreProductInfo( PacketDbQueryResult* dbResult )
{
   return true;
}

//-----------------------------------------------------------------

void  ConnectionToUser::CopyUserSettings( UserPlusProfileTable& enigma, U8 productId )
{
   UserPlusProfileTable::row row = *enigma.begin();

   m_id =                           row[ TableUserPlusProfile::Column_id ];
   m_userName =                     row[ TableUserPlusProfile::Column_name ];
   m_userUuid =                     row[ TableUserPlusProfile::Column_uuid ];
   m_email =                        row[ TableUserPlusProfile::Column_email ];
   m_passwordHash =                 row[ TableUserPlusProfile::Column_password_hash ];
   
   //m_lastLoginTime =                  row[ TableUserPlusProfile::Column_last_login_time ];
   m_loggedOutTime = 0;
   //m_loggedOutTime = GetDateFromString( m_lastLoginTime.c_str() );
   m_lastLoginTime =                 row[ TableUserPlusProfile::Column_last_logout_time ];// last time logged in

   m_isActive =                       boost::lexical_cast<bool>( row[ TableUserPlusProfile::Column_active] );

   string language = row[ TableUserPlusProfile::Column_language_id];
   if( language.size() > 0 && language != "NULL" )
   {
      m_languageId =                     boost::lexical_cast<int>( row[ TableUserPlusProfile::Column_language_id] );
   }

   string avatar = row[ TableUserPlusProfile::Column_mber_avatar];
   if( avatar.size() != 0 )
   m_avatarIcon =                      boost::lexical_cast< int > ( avatar );
   m_adminLevel =                      boost::lexical_cast< int > ( row[ TableUserPlusProfile::Column_admin_level] );
   m_marketingOptOut =                 boost::lexical_cast< bool >( row[ TableUserPlusProfile::Column_marketing_opt_out] );
   m_showWinLossRecord =               boost::lexical_cast< bool >( row[ TableUserPlusProfile::Column_show_win_loss_record] );
   m_showGenderProfile =               boost::lexical_cast< bool >( row[ TableUserPlusProfile::Column_show_gender_profile] );
   m_userMotto =                       boost::lexical_cast< string >( row[ TableUserPlusProfile::Column_user_motto] );
   m_displayOnlineStatusToOtherUsers = boost::lexical_cast< bool >( row[ TableUserPlusProfile::Column_display_online_status_to_other_users] );
   m_blockContactInvitations =         boost::lexical_cast< bool >( row[ TableUserPlusProfile::Column_block_contact_invitations] );
   m_blockGroupInvitations =           boost::lexical_cast< bool >( row[ TableUserPlusProfile::Column_block_group_invitations] );


   string tz = row[ TableUserPlusProfile::Column_time_zone];
   if( tz.size() != 0 )
   {
      m_timeZone =                       boost::lexical_cast< int >( tz );
   }

   m_gameProductId =                  productId;
}

//-----------------------------------------------------------------

void  ConnectionToUser::SaveUpdatedProfile( const PacketUpdateUserProfile* profileUpdate, int adminLevelOfCaller, bool writeToDB )
{
   if( m_isSavingUserProfile )
   {
      m_userManager->SendErrorToClient( m_connectionId, PacketErrorReport::ErrorType_Login, PacketErrorReport::ErrorType_Login_ProfileIsAlreadyBeingUpdated );
      return;
   }

   m_isSavingUserProfile = true;

   if( adminLevelOfCaller > 0 ) // only admins can change a user's name etc.
   {
      // some fields may be blank
      if( profileUpdate->userName.size() > 0 )
      {
         m_userName =                     profileUpdate->userName.c_str();
      }
      //userUuid =                       profileUpdate->userUuid;
      if( profileUpdate->email.size() > 0 )
      {
         m_email =                        profileUpdate->email.c_str();
      }

      if( profileUpdate->passwordHash.size() > 0 )
      {
         m_passwordHash =                 profileUpdate->passwordHash.c_str();
      }
   }
   
   //m_lastLoginTime =                  profileUpdate->m_lastLoginTime;
   //m_m_loggedOutTime =                  GetDateFromString( profileUpdate->m_loggedOutTime.c_str() );

   m_isActive =                           profileUpdate->isActive;
   m_languageId =                         profileUpdate->languageId;
   if( m_languageId < 1 || m_languageId > 12 )// limits on languages
      m_languageId = 1;

   m_adminLevel =                           profileUpdate->adminLevel;
   if( m_adminLevel > 2 )
      m_adminLevel = 2;

   //m_marketingOptOut =                    profileUpdate->marketingOptOut ? true:false;// accounting for non-boolean values
   //m_showWinLossRecord =                  profileUpdate->showWinLossRecord ? true:false;// accounting for non-boolean values
   //m_showGenderProfile =                  profileUpdate->showGenderProfile ? true:false;// accounting for non-boolean values

   ConnectionToUser* loadedConnection = m_userManager->GetLoadedUserConnectionByUuid( m_userUuid );
   if( loadedConnection != NULL && loadedConnection != this )
   {
      loadedConnection->SaveUpdatedProfile( profileUpdate, adminLevelOfCaller, false );// we will write if needed... no pass thru required
   }

   if( writeToDB )
   {
      
      WriteUserBasicsToAccount();

      //---------------------------------------

      WriteUserProfile();
      TellContactServerToReloadUserProfile();
      
   }
   PackUserProfileRequestAndSendToClient( m_connectionId );

   m_isSavingUserProfile = false;
}

//-----------------------------------------------------------------

void  ConnectionToUser::WriteUserBasicsToAccount()
{
   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =           m_connectionId;
   dbQuery->lookup =       DiplodocusLogin::QueryType_UpdateUsers; 
   dbQuery->meta =         "";
   dbQuery->serverLookup = m_gameProductId;
   dbQuery->isFireAndForget = true;

   string query = "UPDATE playdek.users SET user_name='%s',user_email='%s',user_pw_hash='%s',active=";
   query += boost::lexical_cast<string>( m_isActive?1:0 );
   query += ",language_id=";
   query += boost::lexical_cast<string>( m_languageId );
   query += "  WHERE user_id=";
   query += boost::lexical_cast<string>( m_id );
   dbQuery->query = query;
   dbQuery->escapedStrings.insert( m_userName );
   dbQuery->escapedStrings.insert( m_email );
   dbQuery->escapedStrings.insert( m_passwordHash );

   m_userManager->AddQueryToOutput( dbQuery );
}

//-----------------------------------------------------------------

void  ConnectionToUser::WriteUserProfile()
{
   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =           m_connectionId;
   dbQuery->lookup =       DiplodocusLogin::QueryType_UpdateUserProfile;
   dbQuery->meta =         "";
   dbQuery->serverLookup = m_gameProductId;
   dbQuery->isFireAndForget = true;

   string query = "UPDATE playdek.user_profile SET admin_level=";
   query += boost::lexical_cast<string>( m_adminLevel );
   query += ",marketing_opt_out=";
   query += boost::lexical_cast<string>( m_marketingOptOut?1:0 );
   query += ",show_win_loss_record=";
   query += boost::lexical_cast<string>( m_showWinLossRecord?1:0 );
   query += ",show_profile_gender=";
   query += boost::lexical_cast<string>( m_showGenderProfile?1:0 );
   query += ",mber_avatar=";
   query += boost::lexical_cast<string>( m_avatarIcon );
   if( m_userMotto.size() == NULL )
   {
      query += ",motto=NULL,";
   }
   else
   {
      query += ",motto='%s',";
   }
   query += "display_online_status_to_others=";
   query += boost::lexical_cast<string>( m_displayOnlineStatusToOtherUsers?1:0 );
   query += ",block_contact_invitations=";
   query += boost::lexical_cast<string>( m_blockContactInvitations?1:0 );
   query += ",block_group_invitations=";
   query += boost::lexical_cast<string>( m_blockGroupInvitations?1:0 );
   query += " WHERE user_id=";
   query += boost::lexical_cast<string>( m_id );
   
   dbQuery->query = query;
   dbQuery->escapedStrings.insert( m_userMotto );
   m_userManager->AddQueryToOutput( dbQuery );
}

//-----------------------------------------------------------------

void     ConnectionToUser::UpdateConnectionId( U32 connectId )
{
   m_connectionId = connectId;
}

//-----------------------------------------------------------------

bool  ConnectionToUser::BeginLogout( bool wasDisconnectedByError )
{
   if( m_loggedOutTime != 0 ) /// we are already logging out. The gateway may send us multiple logouts so we simply have to ignore further attemps
      return false;

   m_isLoggingOut = true;

   UpdateLastLoggedOutTime();

   status = LoginStatus_Invalid;

   time( &m_loggedOutTime ); // time stamp this guy
  /* if( wasDisconnectedByError )
   {
      return ;
   }*/
   if( wasDisconnectedByError == false )
   {
      PacketLogoutToClient* logout = new PacketLogoutToClient();
      logout->userName =            m_userName;// just for loggin purposes
      logout->uuid =                m_userUuid;
      m_userManager->SendPacketToGateway( logout, m_connectionId );
   }

   return FinalizeLogout();

   //return false;
}

//-----------------------------------------------------------------

bool  ConnectionToUser::FinalizeLogout()
{
   if( m_userUuid.size() == 0 )// this should never happen, but being careful never hurts.
      return false;

   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =              m_connectionId;
   dbQuery->lookup =          DiplodocusLogin::QueryType_UserLogout;
   dbQuery->isFireAndForget = true;// no result is needed
   
   string queryString = "UPDATE users AS user SET user.last_logout_timestamp=UTC_TIMESTAMP() WHERE user.uuid = '";
   queryString +=             m_userUuid;
   queryString += "'";
   dbQuery->query =           queryString;
   m_isLoggingOut = false;
   m_isReadyToBeCleanedUp = true;

   cout << "User "<< m_userName << ":" << m_userUuid << " logout at " << GetDateInUTC() << endl;

   //m_userManager->SendPacketToGateway( logout, m_connectionId );
   return m_userManager->AddQueryToOutput( dbQuery );
}

//-----------------------------------------------------------------

void  ConnectionToUser::AddProductVendorUuid( string text )
{
   std::transform( text.begin(), text.end(), text.begin(), ::tolower );

   bool found = false;
   vector< string >::iterator searchIt = productVendorUuids.begin();
   while( searchIt != productVendorUuids.end() )
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
      productVendorUuids.push_back( text );
   }
}

//-----------------------------------------------------------------

int   ConnectionToUser::FindProductVendorUuid( string text )
{
   std::transform( text.begin(), text.end(), text.begin(), ::tolower );

   vector< string >::iterator searchIt = productVendorUuids.begin();
   while( searchIt != productVendorUuids.end() )
   {
      if( *searchIt == text )
      {
         return ( searchIt - productVendorUuids.begin() );
      }
      searchIt++;
   }
   return DiplodocusLogin::ProductNotFound;
}

//-----------------------------------------------------------------

bool  ConnectionToUser::UpdateLastLoggedInTime()
{
   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =              m_connectionId ;
   dbQuery->lookup =          DiplodocusLogin::QueryType_UpdateLastLoggedInTime;
   dbQuery->isFireAndForget = true;// no result is needed

   string queryString = "UPDATE users AS user SET user.last_login_timestamp=UTC_TIMESTAMP() WHERE uuid = '";
   queryString +=             m_userUuid;
   queryString += "'";
   dbQuery->query =           queryString;

   return m_userManager->AddQueryToOutput( dbQuery );
}

//-----------------------------------------------------------------

bool  ConnectionToUser::UpdateLastLoggedOutTime()
{
   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =              m_connectionId ;
   dbQuery->lookup =          DiplodocusLogin::QueryType_UpdateLastLoggedOutTime;
   dbQuery->isFireAndForget = true;// no result is needed

   string queryString = "UPDATE users AS user SET user.last_logout_timestamp=UTC_TIMESTAMP() WHERE uuid = '";
   queryString += m_userUuid;
   queryString += "'";
   dbQuery->query =           queryString;

   return m_userManager->AddQueryToOutput( dbQuery );
}
                   
//-----------------------------------------------------------------

bool    ConnectionToUser:: SuccessfulLoginFinished( U32 connectId, bool isReloggedIn )
{
   m_isReadyToBeCleanedUp = false;
   m_isLoggingOut = false;// for relogin, we need this to be cleared.
   UpdateConnectionId( connectId );

   productVendorUuids.clear();
   productsWaitingForInsertionToDb.clear();

   bool success = true;

   if( m_isActive == false )
   {
      success = false; // we only have this one condition right now.
      cout << "User is inactive and will not be able to login" << endl;
   }

   PacketLoginToGateway* loginStatus = new PacketLoginToGateway();
   if( m_userName.size() )
   {
      loginStatus->userName = m_userName;
      loginStatus->uuid = m_userUuid;
      loginStatus->lastLogoutTime = m_lastLoginTime;
      loginStatus->loginKey = m_loginKey;
   }
   loginStatus->wasLoginSuccessful = success;
   loginStatus->adminLevel = m_adminLevel;

   time( &m_loginTime );
  /* if( isReloggedIn == false )
   {
      LoadUserProfile();
   }*/

   status = LoginStatus_LoggedIn;

   m_userManager->SendPacketToGateway( loginStatus, m_connectionId, 0.6f );// two second delay

   if( loginStatus->adminLevel > 0 )
   {
      m_userManager->SendErrorToClient( m_connectionId, PacketErrorReport::ErrorType_Login, PacketErrorReport::StatusSubtype_UserIsAdminAccount );
   }

  // RequestListOfGames( userUuid );

   RequestListOfPurchases( m_userUuid );

   //This is where we inform all of the games that the user is logged in.

   if( success == true )
   {
      cout << "User successfull login at " << GetDateInUTC() << endl;
   }
   else
   {
      cout << "User unsuccessfull login at " << GetDateInUTC() << endl;
   }
   return success;//SendLoginStatusToOtherServers( userName, userUuid, m_connectionId, m_gameProductId, m_lastLoginTime, active, email, passwordHash, userId, loginKey, true, false );
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

   if( m_userUuid.size() && m_connectionId != 0 )
   {
      PacketDbQuery* dbQuery = new PacketDbQuery;
      dbQuery->id =     m_connectionId ;
      dbQuery->lookup = DiplodocusLogin::QueryType_UserListOfUserProducts;
      dbQuery->meta = user_uuid;

      string queryString = "SELECT product.product_id, name_string, product.uuid, num_purchased, filter_name FROM product INNER JOIN user_join_product AS user_join_product ON product.uuid=user_join_product.product_id WHERE user_join_product.user_uuid='%s'";
      dbQuery->query =  queryString;
      dbQuery->escapedStrings.insert( user_uuid );

      return m_userManager->AddQueryToOutput( dbQuery );
   }
   return false;
}

//---------------------------------------------------------------

bool     ConnectionToUser:: HandleRequestForListOfPurchases( const PacketListOfUserPurchasesRequest* purchase )
{
   if( purchase->userUuid == m_userUuid )
   {
      cout << "HandleRequestForListOfPurchases: SendListOfProductsToClientAndAsset" << endl;
      SendListOfProductsToClientAndAsset( m_connectionId );
   }

   return true;
}

//---------------------------------------------------------------

void     ConnectionToUser:: SendListOfProductsToClientAndAsset( U32 m_connectionId )
{
   // Also, we must send before because the client is likely to begin requesting assets immediately and we 
   // will hand back the wrong assts in that case.
   m_userManager->SendListOfUserProductsToAssetServer( m_connectionId );

   SendListOfOwnedProductsToClient( m_connectionId );
}

//---------------------------------------------------------------

void     ConnectionToUser:: SendListOfOwnedProductsToClient( U32 m_connectionId )
{
   cout << "Sending user his/her list of products" << endl;

   PacketListOfUserAggregatePurchases* purchases = new PacketListOfUserAggregatePurchases;

   purchases->userUuid = m_userUuid;
   map< U32, ProductBrief >::iterator it = productsOwned.begin();
   while( it != productsOwned.end() )
   {
      const ProductBrief& pb = it->second;
      it++;

      if( pb.quantity == 0 )
         continue;

      PurchaseEntryExtended pe;
      pe.quantity = pb.quantity;
      if( pb.productDbId )
      {
         ProductInfo pi;
         m_userManager->GetProductByProductId( pb.productDbId, pi );
         if( pe.name.size() == 0 )
         {
            pe.name = m_userManager->GetStringLookup()->GetString( pi.lookupName, m_languageId );
         }
      }
      pe.productUuid = pb.uuid;

      purchases->purchases.push_back( pe );
   }

   int num = purchases->purchases.size();
   cout << "SendListOfOwnedProductsToClient:: list of products: num=" << num << endl;
  /* for( int i=0; i< num; i++ )
   {
      const PurchaseEntry& pe = purchases->purchases[i];
      
      cout << i << ") Uuid:    " << pe.productUuid << endl;
      cout << "   name:        " << pe.name << endl;
   }*/
   cout << "endlist" << endl << endl;

   m_userManager->SendPacketToGateway( purchases, m_connectionId );
}


//---------------------------------------------------------------

void     ConnectionToUser:: TellContactServerToReloadUserProfile()
{
   PacketUserUpdateProfile* profile = new PacketUserUpdateProfile;
   PackUserSettings( profile );
   profile->connectionId = m_connectionId;
   if( m_userManager->SendPacketToOtherServer( profile, m_connectionId ) == false )
   {
      PacketFactory factory;
      BasePacket* packet = static_cast< BasePacket* >( profile );
      factory.CleanupPacket( packet ); 
   }
}

//---------------------------------------------------------------

bool     ConnectionToUser:: AddPurchase( const PacketAddPurchaseEntry* purchase )
{
   if( m_adminLevel > 0 )
   {
      ProductInfo productInfo;
      string productUuid = purchase->productUuid.c_str();
      bool success = m_userManager->FindProductByUuid( productUuid, productInfo );
      if( success == false )
      {
         m_userManager->SendErrorToClient( m_connectionId, PacketErrorReport::ErrorType_Cheat_ProductUnknown );
         return false;
      }

      
      if( purchase->userUuid == this->m_userUuid ||
          purchase->userEmail == this->m_email ||
          purchase->userName == this->m_userName
         )// giving to self for admins
      {
         double price = 0.0;
         float numToGive = static_cast<float>( purchase->quantity );
         WriteProductToUserRecord( m_userUuid, productUuid, price, numToGive, m_userUuid, "add purchase entry to self by admin" );
         AddToProductsOwned( productInfo.productId, productInfo.lookupName, productUuid, numToGive, productInfo.vendorUuid );
        
         cout << "AddPurchase: SendListOfProductsToClientAndAsset" << endl;
         SendListOfProductsToClientAndAsset( m_connectionId );
         return true;
      }

      string userUuid =    purchase->userUuid.c_str();
      string userEmail =   purchase->userEmail;
      string userName =    purchase->userName;

      UserConnectionMapIterator  it = FindUser( userEmail, userUuid, userName );
      if( it == adminUserData.end() )
      {
         m_userManager->SendErrorToClient( m_connectionId, PacketErrorReport::ErrorType_Cheat_BadUserLookup );
         return true;
      }
      else
      {
         double price = 0.0;
         float numToGive = static_cast<float>( purchase->quantity );
         WriteProductToUserRecord( it->second.m_userUuid, productUuid, price, numToGive, m_userUuid, "add purchase entry by admin" );

         it->second.AddToProductsOwned( productInfo.productId, productInfo.lookupName, productUuid, numToGive, productInfo.vendorUuid );
         it->second.SendListOfOwnedProductsToClient( m_connectionId );

         ConnectionToUser* loadedConnection = m_userManager->GetLoadedUserConnectionByUuid( m_userUuid );
         if( loadedConnection )
         {
            loadedConnection->AddToProductsOwned( productInfo.productId, productInfo.lookupName, productInfo.uuid, 1, productInfo.vendorUuid );
            loadedConnection->SendListOfOwnedProductsToClient( m_connectionId );
         }
         return true;
      }
   }
   else
   {
      m_userManager->SendErrorToClient( m_connectionId, PacketErrorReport::ErrorType_Cheat_BadPermissions );
   }
   return false;
}


//---------------------------------------------------------------

bool     ConnectionToUser:: StoreUserPurchases( const PacketListOfUserAggregatePurchases* deviceReportedPurchases )// only works for self
{
   int numItems = deviceReportedPurchases->purchases.size();
   cout << " ************************ " << endl;
   cout << " user purchases reported for user: " << m_userName << " : " << m_userUuid << endl;
   if( numItems == 0 )
   {
      cout << "   None" << endl;
   }

   bool resendUserPurchaseList = false;

   for( int i=0; i< numItems; i++ )
   {
      const PurchaseEntryExtended& purchaseEntry = deviceReportedPurchases->purchases[i];
      int  originalProductNameIndex = m_userManager->FindProductByVendorUuid( purchaseEntry.productUuid );
      if( originalProductNameIndex == DiplodocusLogin::ProductNotFound )
         originalProductNameIndex = m_userManager->FindProductByName( purchaseEntry.name );

      //----------------
      if( purchaseEntry.name.size() == 0 )// we can't do anything with this.
      {
         cout << "   ***Invalid product id...title: " << purchaseEntry.productUuid << "   name: " << purchaseEntry.name << endl;
      }
      else
      {
         if( purchaseEntry.productUuid.size() == 0 )
         {
            cout << "   ***Invalid product id...title: " << purchaseEntry.productUuid << "   name: " << purchaseEntry.name << endl;
            m_userManager->SendErrorToClient( m_connectionId, PacketErrorReport::ErrorType_Purchase_ProductUnknown ); 
            continue;
         }

         if( originalProductNameIndex == DiplodocusLogin::ProductNotFound )
         {
            AddItemToProductTable( purchaseEntry );
         }

         if( StoreOffProductInUserRecord ( originalProductNameIndex, purchaseEntry.productUuid, purchaseEntry.quantity ) )
         {
            resendUserPurchaseList = true;
         }
      }
      cout << "  " << i << ":   title: " << purchaseEntry.name << endl;
      
   }

   cout << " ************************ " << endl;


   if( resendUserPurchaseList )
   {
      cout << "StoreUserPurchases: SendListOfProductsToClientAndAsset" << endl;
      SendListOfProductsToClientAndAsset( m_connectionId );
   }

   return true;
}

//---------------------------------------------------------------

void  ConnectionToUser:: AddItemToProductTable( const PurchaseEntryExtended& purchaseEntry )
{
   ProductInfo pi;
   pi.name = purchaseEntry.name.c_str();
   pi.vendorUuid = purchaseEntry.productUuid.c_str();
   std::transform( pi.vendorUuid.begin(), pi.vendorUuid.end(), pi.vendorUuid.begin(), ::tolower );
   // productUuid .. I don't know what to do with this.
   //pi.price = purchaseEntry.number_price;
   //pi.quantity = purchaseEntry.quantity;

   productsWaitingForInsertionToDb.push_back( pi );
   //productsOwned.insert( pi );
   m_userManager->AddNewProductToDb( purchaseEntry );
}

//---------------------------------------------------------------

bool     ConnectionToUser:: CanProductBePurchasedMultipleTimes( const ProductInfo& productInfo )
{
   // todo: verify that the product cannot be bought multiple times. 
   // Some IAP/DLC can be purchased multiple times. e.g. Tournament tickets, consumables
   return false;
}

//---------------------------------------------------------------

bool     ConnectionToUser:: StoreOffProductInUserRecord ( int userManagerIndex, 
                                                         const string& productUuid, 
                                                         float numPurchased)
{
   if( numPurchased <= 0 )
   {
      cout << "Quantity listed as too low... must be at least 1. Rejected entry: " << productUuid << endl;
      return false;
   }

   int userProductIndex = DiplodocusLogin::ProductNotFound;
   if( productUuid.size() )
   {
      // the order of the next two lines matters a lot.
      userProductIndex = FindProductVendorUuid( productUuid ); 
    /*  if( userProductIndex == DiplodocusLogin::ProductNotFound )
      {
         AddProductVendorUuid( productUuid );// we're gonna save the name, regardless.
      }*/
   }   

   //**  find the item in the user record and add it to the db if not **
   if( userManagerIndex != DiplodocusLogin::ProductNotFound )
   {
      ProductInfo productInfo;
      bool result = m_userManager->GetProductByIndex( userManagerIndex, productInfo );
      if( result == true )
      {
         // this logic is tricky. If I don't already have it.. or I can purchase it again...
         if( userProductIndex == DiplodocusLogin::ProductNotFound ||
                CanProductBePurchasedMultipleTimes( productInfo ) == true )
         {
            const string& productUuid = productInfo.uuid;
            WriteProductToUserRecord( m_userUuid, productUuid, 1.0, numPurchased, "", "" );
            AddToProductsOwned( productInfo.productId, productInfo.lookupName, productUuid, numPurchased, productInfo.vendorUuid );

            AddConversionProductsToUserPurchases( productInfo );
            return true;
         }
      }
   }
   return false;
}


void     ConnectionToUser:: AddConversionProductsToUserPurchases( const ProductInfo& productInfo )
{
   if( productInfo.convertsToProductId == 0 || productInfo.convertsToQuantity == 0 )
      return;

   ProductInfo productInfoLookup;
   if( m_userManager->GetProductByProductId( productInfo.convertsToProductId, productInfoLookup ) == false )
      return;
   //originalProductNameIndex = userManager->FindProductByName( purchaseEntry.name );
   StoreOffProductInUserRecord ( productInfo.convertsToProductId, productInfoLookup.uuid, static_cast< float >( productInfo.convertsToQuantity ) );
}


//---------------------------------------------------------------

void     ConnectionToUser:: AddCurrentlyLoggedInProductToUserPurchases()// only works for self
{
   const char* loggedInGameProductName = FindProductName( m_gameProductId );
   if( loggedInGameProductName == NULL )
   {
      cout << "Major error: user logging in with a product not identified" << endl;
      m_userManager->SendErrorToClient( m_connectionId, PacketErrorReport::ErrorType_Login_CannotAddCurrentProductToUser ); 
      
      return;
   }

   ProductInfo productInfo;
   bool found = m_userManager->GetProductByProductId( m_gameProductId, productInfo );
   if( found == false )
   {
      cout << "Major error: user logging in with a product not in our list of loaded products" << endl;
      return;
   }

   WriteProductToUserRecord( m_userUuid, productInfo.uuid, 0.0, 1, "user", "default by login" );
   float numToGive = 1;
   AddToProductsOwned( productInfo.productId, productInfo.lookupName, productInfo.uuid, numToGive, productInfo.vendorUuid );
}

//---------------------------------------------------------------

void     ConnectionToUser:: WriteProductToUserRecord( const string& productVendorUuid, double pricePaid )
{
   int userProductIndex = FindProductVendorUuid( productVendorUuid );
   //**  find the item in the user record and add it to the db if not **
   if( userProductIndex == DiplodocusLogin::ProductNotFound )// the user doesn't have the record, but the rest of the DB does.
   {
      m_userManager->SendErrorToClient( m_connectionId, PacketErrorReport::ErrorType_Cheat_ProductUnknown ); 
      return;
   }

   ProductInfo productInfo;
   bool result = m_userManager->GetProductByIndex( userProductIndex, productInfo );
   if( result == true )
   {
      WriteProductToUserRecord( m_userUuid, productInfo.uuid, pricePaid, 1, m_userUuid, "new product reported by user login" );
      AddToProductsOwned( productInfo.productId, productInfo.lookupName, productInfo.uuid, 1, productInfo.vendorUuid );
   }
   else
   {
      m_userManager->SendErrorToClient( m_connectionId, PacketErrorReport::ErrorType_Cheat_ProductUnknown ); 
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

   /*dbQuery->query = "INSERT INTO user_join_product VALUES( DEFAULT, '%s', '%s',DEFAULT,";
   dbQuery->query += boost::lexical_cast< string >( pricePaid );
   dbQuery->query += ",1,";
   dbQuery->query += boost::lexical_cast< string >( numPurchased );
   dbQuery->query += ",'%s','%s',NULL, 0)";*/
   dbQuery->query = "INSERT INTO playdek.user_join_product (user_uuid, product_id, price_paid, num_purchased, admin_provided, admin_notes ) VALUES ( ";
   dbQuery->query += "'%s', '%s', ";
   dbQuery->query += boost::lexical_cast< string >( pricePaid );
   dbQuery->query += ",";
   dbQuery->query += boost::lexical_cast< string >( numPurchased );
   dbQuery->query += ",'%s', '%s' );";


   dbQuery->escapedStrings.insert( userUuid );
   dbQuery->escapedStrings.insert( productUuid );
   dbQuery->escapedStrings.insert( adminId );
   dbQuery->escapedStrings.insert( adminNotes );

   m_userManager->AddQueryToOutput( dbQuery );

   m_userManager->SendErrorToClient( m_connectionId, PacketErrorReport::ErrorType_Status, PacketErrorReport::StatusSubtype_ProductAdded );
}


//---------------------------------------------------------------

void     ConnectionToUser:: StoreListOfUsersProductsFromDB( PacketDbQueryResult* dbResult, bool shouldAddLoggedInProduct )
{
   if( m_userManager->IsPrintingFunctionNames() )
   {
      cout << "StoreListOfUsersProductsFromDB" << endl;
   }
   bool  didFindGameProduct = false;

   ConnectionToUser* userWhoGetsProducts = this;
   ConnectionToUser* loadedConnection = NULL;
   bool loadedForSelf = true;
   if( dbResult->meta.size() && dbResult->meta != m_userUuid )// admin lookup
   {
      UserConnectionMapIterator  it = FindUser( "", dbResult->meta, "" );
      if( it != adminUserData.end() )
      {
         userWhoGetsProducts = &it->second;
      }
      loadedConnection = m_userManager->GetLoadedUserConnectionByUuid( m_userUuid );
      loadedForSelf = false;
   }

   // verify that this product is owned by the player and if not, then add an entry
   userWhoGetsProducts->ClearAllProductsOwned();
   UserOwnedProductSimpleTable  enigma( dbResult->bucket );
   UserOwnedProductSimpleTable::iterator      it = enigma.begin();
   int   numProducts = dbResult->GetBucket().size();
   numProducts = numProducts;

   int index = 0;

   if( m_userManager->IsPrintingVerbose() )
   {
      cout << "** products from DB ** " << endl;
   }
   while( it != enigma.end() )
   {
      UserOwnedProductSimpleTable::row       row = *it++;

      int productId =   boost::lexical_cast< int>   ( row[ TableUserOwnedProductSimple::Column_product_id ] );
      string stringLookupName =                       row[ TableUserOwnedProductSimple::Column_product_name_string ];
      string productUuid =                            row[ TableUserOwnedProductSimple::Column_product_uuid ];
      float  quantity = boost::lexical_cast< float> ( row[ TableUserOwnedProductSimple::Column_quantity ] );
      string productVendorUuid =                      row[ TableUserOwnedProductSimple::Column_filter_name ];

      if( m_userManager->IsPrintingVerbose() )
      {
         cout << index << ") vendorUuid:    " << productVendorUuid << endl;
         cout << "   productUuid:        " << productUuid << endl;
         cout << "   string lookup name: " << stringLookupName << endl;
      }
      if( m_gameProductId == productId )
      {
         didFindGameProduct = true;
      }
      userWhoGetsProducts->AddToProductsOwned( productId, stringLookupName, productUuid, quantity, productVendorUuid );
         
      if( loadedConnection != NULL )
      {
         loadedConnection->AddToProductsOwned( productId, stringLookupName, productUuid, quantity, productVendorUuid );
      }
      index++;
   }
   if( m_userManager->IsPrintingVerbose() )
   {
      cout << "** end products from DB ** " << endl;
   }

   if( didFindGameProduct == false && shouldAddLoggedInProduct == true && loadedForSelf == true )
   {
      AddCurrentlyLoggedInProductToUserPurchases();
   }

   if( m_userManager->IsPrintingFunctionNames() )
   {
      cout << "StoreListOfUsersProductsFromDB: SendListOfProductsToClientAndAsset" << endl;
   }
   userWhoGetsProducts->SendListOfProductsToClientAndAsset( m_connectionId );

   if( loadedForSelf == false )
   {
      userWhoGetsProducts->PackOtherUserProfileRequestAndSendToClient( m_connectionId );
   }

   if( m_hasRequestedPurchasesFromClient == false )
   {
      m_hasRequestedPurchasesFromClient = true;
      RequestListOfProductsFromClient();
   }
}

//---------------------------------------------------------------

void     ConnectionToUser:: RequestListOfProductsFromClient()
{
   PacketListOfUserPurchasesRequest* purchaseRequest = new PacketListOfUserPurchasesRequest;
   m_userManager->SendPacketToGateway( purchaseRequest, m_connectionId );
}


//---------------------------------------------------------------

bool     ConnectionToUser:: HandleCheats( const PacketCheat* cheat )
{
   cout << endl << "---- Cheat received ----" << endl;
   if( m_adminLevel < 1 )
   {
      cout << "user admin level is too low" << endl;
      return false;
   }

   //QueryType_GetProductListForUser
   const char* cheat_text = cheat->cheat.c_str();
   cheat_text = cheat_text;

   std::vector<std::string> strings;
   split( cheat->cheat, strings );

   
   //int count = 0;
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
      return m_userManager->SendErrorToClient( m_connectionId, PacketErrorReport::ErrorType_Cheat_UnrecognizedCommand );
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
   dbQuery->escapedStrings.insert( m_userUuid );

   m_userManager->AddQueryToOutput( dbQuery );

   m_userManager->SendErrorToClient( m_connectionId, PacketErrorReport::ErrorType_Status, PacketErrorReport::StatusSubtype_AllProductsRemoved );
   return true;
}

//---------------------------------------------------------------

bool     ConnectionToUser:: HandleCheat_AddProduct( const string& productName )
{
   int productIndex = m_userManager->FindProductByName( productName );
   if( productIndex == DiplodocusLogin::ProductNotFound )
   {
      m_userManager->SendErrorToClient( m_connectionId, PacketErrorReport::ErrorType_Cheat_ProductUnknown );
      return false;
   }

   ProductInfo productInfo;
   bool result = m_userManager->GetProductByIndex( productIndex, productInfo );
   if( result == true )
   {
      //WriteProductToUserRecord( userUuid, product.uuid, product.price );
      WriteProductToUserRecord( m_userUuid, productInfo.uuid, 0.0, 1, m_userUuid, "added by cheat" );
      AddToProductsOwned( productInfo.productId, productInfo.lookupName, productInfo.uuid, 1, productInfo.vendorUuid );
      ConnectionToUser* loadedConnection = m_userManager->GetLoadedUserConnectionByUuid( m_userUuid );
      if( loadedConnection )
      {
         loadedConnection->AddToProductsOwned( productInfo.productId, productInfo.lookupName, productInfo.uuid, 1, productInfo.vendorUuid );
         cout << "HandleCheat_AddProduct: SendListOfProductsToClientAndAsset" << endl;
         loadedConnection->SendListOfProductsToClientAndAsset( m_connectionId );
      }
   }

   return true;
}

//-----------------------------------------------------------------

template < typename type >
void  ConnectionToUser:: PackUserSettings( type* response )
{
   response->userName =          m_userName;
   response->userUuid =          m_userUuid;
   response->email =             m_email;
   response->lastLoginTime =     m_lastLoginTime;
   response->loggedOutTime =     m_lastLoginTime;

   response->adminLevel =        m_adminLevel;
   response->iconId =            m_avatarIcon;
   response->isActive =          m_isActive;
   response->showWinLossRecord = m_showWinLossRecord;
   response->marketingOptOut =   m_marketingOptOut;
   response->showGenderProfile = m_showGenderProfile;
   response->motto =             m_userMotto;

   response->displayOnlineStatusToOtherUsers = m_displayOnlineStatusToOtherUsers;
   response->blockContactInvitations =   m_blockContactInvitations;
   response->blockGroupInvitations = m_blockGroupInvitations;
}


void     ConnectionToUser:: PackUserProfileRequestAndSendToClient( U32 m_connectionId )
{
   PacketRequestUserProfileResponse* response = new PacketRequestUserProfileResponse;
   // string userName, string email, string userUuid, string m_lastLoginTime, string m_loggedOutTime, int m_adminLevel, bool m_isActive, bool showWinLossRecord, bool marketingOptOut, bool showGenderProfile
   PackUserSettings( response );

   /*response->profileKeyValues.insert( "name", m_userName );
   response->profileKeyValues.insert( "uuid", m_userUuid );
   response->profileKeyValues.insert( "email", m_email );
   response->profileKeyValues.insert( "last_login_time", m_lastLoginTime );
   response->profileKeyValues.insert( "last_logget_out", m_lastLoginTime );
   response->profileKeyValues.insert( "admin_level", boost::lexical_cast< string >( m_adminLevel ) );
   response->profileKeyValues.insert( "is_active", boost::lexical_cast< string >( m_isActive ? 1:0 ) );
   response->profileKeyValues.insert( "show_win_loss_record", boost::lexical_cast< string >( showWinLossRecord  ? 1:0 ) );
   response->profileKeyValues.insert( "marketing_opt_out", boost::lexical_cast< string >( marketingOptOut ? 1:0 ) );
   response->profileKeyValues.insert( "showGender_profile", boost::lexical_cast< string >( showGenderProfile ? 1:0 ) );
   response->profileKeyValues.insert( "mber_avatar", boost::lexical_cast< string >( avatarIcon ) );*/

   m_userManager->SendPacketToGateway( response, m_connectionId );
}

//-----------------------------------------------------------------

void     ConnectionToUser:: PackOtherUserProfileRequestAndSendToClient( U32 m_connectionId )
{
   PacketRequestOtherUserProfileResponse* response = new PacketRequestOtherUserProfileResponse;

   response->basicProfile.insert( "name", m_userName );
   response->basicProfile.insert( "uuid", m_userUuid );
   response->basicProfile.insert( "motto", m_userMotto );
   ///m_userManager->GetStringLookup()->GetString( lookupName, m_languageId );
   //response->basicProfile.insert( "language",  );
   response->basicProfile.insert( "show_win_loss_record", boost::lexical_cast< string >( m_showWinLossRecord  ? 1:0 ) );
   response->basicProfile.insert( "time_zone", boost::lexical_cast< string >( m_timeZone ) );

   response->basicProfile.insert( "avatar_icon", boost::lexical_cast< string >( m_avatarIcon ) );

   map< U32, ProductBrief >::iterator it = productsOwned.begin();
   while( it != productsOwned.end() )
   {
      const string& uuid = it->second.uuid;
      //int type = m_userManager->GetSalesOrganizer()->GetProductType( uuid );

      ProductInfo returnPi;
      if( m_userManager->FindProductByUuid( uuid, returnPi ) )
      {
         int type = returnPi.productType;
         if( type == GameProductType_Game || type == GameProductType_Dlc )
         {
            if( it->second.quantity > 0 )
            {
               response->productsOwned.insert( it->second.uuid, (int)(it->second.quantity ) );
            }
         }
      }
      it++;
   }

   m_userManager->SendPacketToGateway( response, m_connectionId );
}

//-----------------------------------------------------------------
//-----------------------------------------------------------------

void     ConnectionToUser:: ClearAllProductsOwned()
{
   cout << "ClearAllProductsOwned for user " << m_userName << ":" << m_userUuid << endl;
   productsOwned.clear();
   productVendorUuids.clear();
}

//-----------------------------------------------------------------

bool     ConnectionToUser:: AddToProductsOwned( int productDbId, const string& lookupName, const string& productUuid, float quantity, const string& vendorUuid  )
{
   map< U32, ProductBrief >::iterator it = productsOwned.find( productDbId );
   if( it != productsOwned.end() )
   {
      it->second.quantity += quantity;
      return false;
   }
   else
   {
      ProductInfo pi;
      m_userManager->GetProductByProductId( productDbId, pi );
      if( pi.isHidden == true )
         return false;

      ProductBrief brief;
      brief.productDbId = productDbId;
      
      brief.localizedName = m_userManager->GetStringLookup()->GetString( lookupName, m_languageId );

      brief.vendorUuid = vendorUuid;
      brief.uuid = productUuid;
      brief.quantity = quantity;
      productsOwned.insert( pair< U32, ProductBrief > ( productDbId, brief ) );

      AddProductVendorUuid( vendorUuid );
      return true;
   }
}

//-----------------------------------------------------------------

bool     ConnectionToUser:: RequestProfile( const PacketRequestUserProfile* profileRequest )
{
   if( profileRequest->uuid == m_userUuid || profileRequest->userName == m_userName || profileRequest->userEmail == m_email )
   {
      PackUserProfileRequestAndSendToClient( m_connectionId );
      return true;
   }
   if( m_adminLevel < 1 )
   {
      m_userManager->SendErrorToClient( m_connectionId, PacketErrorReport::ErrorType_Cheat_BadPermissions );
      return false;
   }

   UserConnectionMapIterator  it = FindUser( profileRequest->userEmail, profileRequest->uuid.c_str(), profileRequest->userName );
   if( it != adminUserData.end() )
   {
      it->second.PackUserProfileRequestAndSendToClient( m_connectionId );
      return true;
   }

   RequestProfile( profileRequest->userEmail, profileRequest->uuid.c_str(), profileRequest->userName, true, true );

   return false;
}

//-----------------------------------------------------------------

bool     ConnectionToUser:: RequestOthersProfile( const PacketRequestOtherUserProfile* profileRequest )
{
   RequestProfile( profileRequest->userName, profileRequest->userName, profileRequest->userName, false, profileRequest->fullProfile );

   return true;
}

//-----------------------------------------------------------------

void     ConnectionToUser:: RequestProfile( const string& email, const string& uuid, const string& name, bool asAdmin, bool fullProfile )
{
   string requestId = CreateLookupKey( email, uuid, name  );
  // submit request for user profile with this connection id
   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =           m_connectionId;
   dbQuery->lookup =       DiplodocusLogin::QueryType_AdminRequestUserProfile;
   dbQuery->meta =         boost::lexical_cast<string>( GenerateUniqueHash( requestId ) );
   dbQuery->serverLookup = asAdmin;
   dbQuery->customData  = new bool(fullProfile);

   // possible bug here... change this to only user username
   string temp = "SELECT * FROM users JOIN user_profile ON users.user_id=user_profile.user_id WHERE users.user_email='%s' OR users.uuid='%s' OR users.user_name='%s' LIMIT 1";
   dbQuery->escapedStrings.insert( email );
   dbQuery->escapedStrings.insert( uuid );
   dbQuery->escapedStrings.insert( name );
   dbQuery->query = temp;

   adminUserData.insert( UserConnectionPair( dbQuery->meta, ConnectionToUser() ) );
   m_userManager->AddQueryToOutput( dbQuery );
}

//-----------------------------------------------------------------

bool     ConnectionToUser:: UpdateProfile( const PacketUpdateUserProfile* updateProfileRequest )
{
   if( updateProfileRequest->userUuid == m_userUuid )
   {
      SaveUpdatedProfile( updateProfileRequest, m_adminLevel, true );
      return true;
   }

   if( updateProfileRequest->adminLevel >= m_adminLevel )// you must have admin privilidges to change other user's profiles.
   {
      m_userManager->SendErrorToClient( m_connectionId, PacketErrorReport::ErrorType_Cheat_BadPermissions );
      return false;
   }
    
  /* string requestId = CreateLookupKey( updateProfileRequest->email, updateProfileRequest->userUuid, updateProfileRequest->userName );
   if( requestId.size() != 0 )
   {
      map< string, ConnectionToUser> ::iterator it = adminUserData.find( requestId );// just use the existing data.
      if( it != adminUserData.end() )
      {
         it->second.SaveUpdatedProfile( updateProfileRequest, m_adminLevel,true );
         return true;
      }
   }
   
   map< string, ConnectionToUser> ::iterator it = adminUserData.begin();
   while( it != adminUserData.end() )
   {
      if( updateProfileRequest->userName.size() != 0 && it->second.userName == updateProfileRequest->userName )
      {
         it->second.SaveUpdatedProfile( updateProfileRequest, m_adminLevel,true );
         return true;
      }
      if( updateProfileRequest->userUuid.size() != 0 && it->second.userUuid == updateProfileRequest->userUuid )
      {
         it->second.SaveUpdatedProfile( updateProfileRequest, m_adminLevel,true );
         return true;
      }
      if( updateProfileRequest->email.size() != 0 && it->second.email == updateProfileRequest->email )
      {
         it->second.SaveUpdatedProfile( updateProfileRequest, m_adminLevel,true );
         return true;
      }
      it++;
   }*/

   UserConnectionMapIterator  it = FindUser( updateProfileRequest->email, updateProfileRequest->userUuid.c_str(), updateProfileRequest->userName );
   if( it != adminUserData.end() )
   {
      it->second.SaveUpdatedProfile( updateProfileRequest, m_adminLevel, true );
      return true;
   }

   m_userManager->SendErrorToClient( m_connectionId, PacketErrorReport::ErrorType_Cheat_BadUserLookup_TryLoadingUserFirst );

   return false;
}

//-----------------------------------------------------------------

bool     ConnectionToUser:: UpdateProfile( const PacketUpdateSelfProfile* updateProfileRequest )
{
   PacketUpdateSelfProfileResponse* response = new PacketUpdateSelfProfileResponse;
   response->avatarIconId = 0;

   // we can choose to add more over time
   bool isTheSame = (
                     updateProfileRequest->userName == m_userName &&
                     updateProfileRequest->email == m_email &&
                     updateProfileRequest->motto == m_userMotto && 
                     updateProfileRequest->avatarIconId == m_avatarIcon &&
                     updateProfileRequest->languageId == m_languageId &&
                     updateProfileRequest->showWinLossRecord == m_showWinLossRecord &&
                     updateProfileRequest->marketingOptOut == m_marketingOptOut &&
                     updateProfileRequest->showGenderProfile == m_showGenderProfile &&
                     updateProfileRequest->displayOnlineStatusToOtherUsers == m_displayOnlineStatusToOtherUsers &&
                     updateProfileRequest->blockContactInvitations == m_blockContactInvitations &&
                     updateProfileRequest->blockGroupInvitations == m_blockGroupInvitations
                     );

  /* bool isValid = ( updateProfileRequest->avatarIconId > 0 ) && 
      ( updateProfileRequest->languageId >= LanguageList_english && updateProfileRequest->languageId < LanguageList_count );*/

   if( isTheSame )//|| isValid == false )   
   {
      response->success = false;
   }
   else
   {
      if( updateProfileRequest->avatarIconId > 0 )
         m_avatarIcon = updateProfileRequest->avatarIconId;

      if ( updateProfileRequest->languageId >= LanguageList_english && updateProfileRequest->languageId < LanguageList_count )
         m_languageId = updateProfileRequest->languageId;

      if( updateProfileRequest->motto.size() && updateProfileRequest->motto.c_str() != m_userMotto )
         m_userMotto = updateProfileRequest->motto.c_str();
      if( updateProfileRequest->userName.size() && updateProfileRequest->userName.c_str() != m_userName )
         m_userName = updateProfileRequest->userName.c_str();

      if( updateProfileRequest->email.size() && updateProfileRequest->email.c_str() != m_email )
         m_email = updateProfileRequest->email.c_str();

      if( updateProfileRequest->showWinLossRecord == false ||
            updateProfileRequest->showWinLossRecord == true )
         m_showWinLossRecord = updateProfileRequest->showWinLossRecord;

      if( updateProfileRequest->marketingOptOut == false  ||
            updateProfileRequest->marketingOptOut == true )
         m_marketingOptOut = updateProfileRequest->marketingOptOut;

      if( updateProfileRequest->showGenderProfile == false  ||
            updateProfileRequest->showGenderProfile == true )
         m_showGenderProfile = updateProfileRequest->showGenderProfile;

      if( updateProfileRequest->displayOnlineStatusToOtherUsers == false  ||
            updateProfileRequest->displayOnlineStatusToOtherUsers == true )
         m_displayOnlineStatusToOtherUsers = updateProfileRequest->displayOnlineStatusToOtherUsers;

      if( updateProfileRequest->blockContactInvitations == false  ||
            updateProfileRequest->blockContactInvitations == true )
         m_blockContactInvitations = updateProfileRequest->blockContactInvitations;

      if( updateProfileRequest->blockGroupInvitations == false  ||
            updateProfileRequest->blockGroupInvitations == true )
         m_blockGroupInvitations = updateProfileRequest->blockGroupInvitations;

      WriteUserBasicsToAccount();
      WriteUserProfile();

      response->success = true;
      response->avatarIconId = m_avatarIcon;
   }

   
   PackUserProfileRequestAndSendToClient( m_connectionId );

   m_userManager->SendPacketToGateway( response, m_connectionId );

   if( response->success )
   {
      TellContactServerToReloadUserProfile();
   }

   return true;
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
      if( userName.size() != 0 && it->second.m_userName == userName )
      {
         return it;
      }
      if( userUuid.size() != 0 && it->second.m_userUuid == userUuid )
      {
         return it;
      }
      if( email.size() != 0 && it->second.m_email == email )
      {
         return it;
      }
      it++;
   }

   return adminUserData.end();
}

//////////////////////////////////////////////////////////////////////////
