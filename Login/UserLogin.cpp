// UserLogin.cpp

#include "UserLogin.h"
#include "../NetworkCommon/Utils/Utils.h"
#include "../NetworkCommon/Utils/StringUtils.h"

#include "../NetworkCommon/Packets/DBPacket.h"
#include "../NetworkCommon/Packets/LoginPacket.h"

#include "../NetworkCommon/Utils/TableWrapper.h"

#include "../NetworkCommon/Database/StringLookup.h"

#include "LoginMainThread.h"
#include "ProductManager.h"

#include <boost/lexical_cast.hpp>


const int normalLogoutExpireTime = 18; // seconds

//////////////////////////////////////////////////////////////////////////

UserLogin::UserLogin( const string& name, const string& pword ) :
            m_hashLookup( 0 ),
            m_userId( "" ),
            m_userName( name ), 
            m_userUuid(""),
            m_passwordHash( pword ),
            m_email( name ),
            m_assetServerKey(""),
            m_lastLoginTime(""),
            m_lastLogoutTime(""),
            m_userMotto(),
            m_avatarIcon( 0 ),
            m_loginAttemptCount( 0 ),
            m_timeZone( 0 ),
            m_languageId( LanguageList_english ),
            m_isValid( false ),
            m_isActive( true ),
            m_showWinLossRecord( true ),
            m_marketingOptOut( false ),
            m_showGenderProfile( false ),
            m_displayOnlineStatusToOtherUsers( false ),
            m_blockContactInvitations( false ),
            m_blockGroupInvitations( false ),
            //m_isSavingUserProfile( false ),
            m_hasRequestedPurchasesFromClient( false ),
            m_requiresStateSendToAllServers( false ),
            m_lookupId( 0 ),
            m_requestorConnectionId( 0 ),
            m_loginMainThread( NULL )
{
   for( int i=0; i< GameProductId_NUM_GAMES; i++ )
   {
      m_connectionDetails[ i ].gameProductId = i;
   }
}

//////////////////////////////////////////////////////////////////////////

void     UserLogin::LogUserOutOfExistingConnection( U8 gameProductId )
{
   UserLogin_ConnectionProductDetails& details = m_connectionDetails[ gameProductId ];

   if( details.loginState == LoginConnectionDetails::LoginState_IsLoggedIn )
   {
      LogMessage( LOG_PRIO_INFO, "--------------------------------------------" );
      LogMessage( LOG_PRIO_INFO, "  UserLogin::LogUserOutOfExistingConnection for product: %d", (U32) gameProductId );

      m_loginMainThread->BroadcastLoginStatusSimple( details.connectionId, 
                               gameProductId, 
                                false, 
                                true,
                                details.gatewayId );
      details.loginState = LoginConnectionDetails::Loginstate_IsLoggedOut;
      details.connectionId = 0;
      details.gatewayId = 0;
   }
}

//////////////////////////////////////////////////////////////////////////

void     UserLogin::AddConnectionId( U32 connectionId, U32 gatewayId, U8 gameProductId )
{
   UserLogin_ConnectionProductDetails& details = m_connectionDetails[ gameProductId ];   

   LogMessage( LOG_PRIO_INFO, "--------------------------------------------" );
   LogMessage( LOG_PRIO_INFO, "  UserLogin::AddConnectionId for product: %d", (U32) gameProductId );
   LogMessage( LOG_PRIO_INFO, "  new connId: %u, gateway id %u: ", connectionId, gatewayId );
   LogMessage( LOG_PRIO_INFO, "--------------------------------------------" );

   details.loginState = LoginConnectionDetails::LoginState_BeginningLogin;
   details.connectionId = connectionId;
   details.gatewayId = gatewayId;
}

//////////////////////////////////////////////////////////////////////////

bool     UserLogin::HandlePacket( const BasePacket* packet, U32 connectionId, U32 gatewayId, U8 gameProductId )
{
   U8 packetType = packet->packetType;
   U8 packetSubType = packet->packetSubType;
   switch( packetType )
   {
   case PacketType_Login:
      {
         switch( packetSubType )
         {
            case PacketLogin::LoginType_LoginFromGateway:
            {
               const PacketLoginFromGateway* login = static_cast< const PacketLoginFromGateway* >( packet );
               LogUserIn( login, connectionId );//, connectionId, gameProductId );
               break;
            }
            case PacketLogin::LoginType_Logout:
            {
               const PacketLogout* logout = static_cast< const PacketLogout*>( packet );
               Logout( connectionId, logout->wasDisconnectedByError );
               break;
            }
            
            case PacketLogin::LoginType_ListOfAggregatePurchases:
            {
               const PacketListOfUserAggregatePurchases* purchases = static_cast< const PacketListOfUserAggregatePurchases*>( packet );
               StoreUserPurchases( purchases, connectionId );
               break;
            }
            case PacketLogin::LoginType_RequestListOfPurchases:
            {
               const PacketListOfUserPurchasesRequest* purchases = static_cast< const PacketListOfUserPurchasesRequest*>( packet );
               HandleRequestForListOfPurchases( purchases, connectionId );
               break;
            }
            case PacketLogin::LoginType_RequestListOfProducts:
            {
               const PacketRequestListOfProducts* product = static_cast< const PacketRequestListOfProducts* >( packet );
               HandleRequestListOfProducts( product, connectionId );
               break;
            }
            case PacketLogin::LoginType_RequestUserProfile:
            {
               const PacketRequestUserProfile* profileRequest = static_cast< const PacketRequestUserProfile* >( packet );
               RequestProfile( profileRequest );
               break;
            }
            case PacketLogin::LoginType_UpdateSelfProfile:
            {
               const PacketUpdateSelfProfile* updateProfileRequest = static_cast< const PacketUpdateSelfProfile* >( packet );
               UpdateProfile( connectionId, updateProfileRequest );
               break;
            }
            
            case PacketLogin::LoginType_AddPurchaseEntry:
            {
               const PacketAddPurchaseEntry* addPurchase = static_cast< const PacketAddPurchaseEntry*>( packet );
               //AddPurchase( userConnectionId, addPurchase );
               // not supported
               break;
            }
            case PacketLogin::LoginType_EchoToServer:
            {
               EchoHandler( connectionId );
               break;
            }
            case PacketLogin::LoginType_UserListOfPurchasesWasUpdated:
            {
               const PacketListOfUserPurchasesUpdated* userInventory = static_cast<const PacketListOfUserPurchasesUpdated*>( packet );
               SendPurchasesToClient( userInventory->userUuid, userInventory->userConnectionId );
               break;
            }
         }
         break;
      }
      
  /* case PacketType_Purchase:
      {
         switch( packetSubType )
         {
         case PacketPurchase:: PurchaseType_ValidatePurchaseReceipt:
            {
            const PacketPurchase_ValidatePurchaseReceipt* receipt = static_cast< const PacketPurchase_ValidatePurchaseReceipt* >( packet );
               //LogUserIn( login, connectionId );//, connectionId, gameProductId );
               StoreNewUserProducts( receipt );
               break;
            
            }
         }
         break;
      }*/
      
   }
   return true;
}

//////////////////////////////////////////////////////////////////////////

U8       UserLogin::GetProductId( U32 userConnectionId ) const
{
   return FindConnectionDetails( userConnectionId );
}

U32      UserLogin::GetGatewayId( U32 userConnectionId ) const
{
   int whichConnection = FindConnectionDetails( userConnectionId );
   if( whichConnection == GameProductId_NUM_GAMES )
   {
      return 0;
   }
   return  m_connectionDetails[ whichConnection ].gatewayId;
}

U32      UserLogin::GetConnectionId( U32 gatewayId ) const
{
   for( U32 i=0; i< GameProductId_NUM_GAMES; i++ )
   {
      const UserLogin_ConnectionProductDetails& details = m_connectionDetails[ i ];
      if( details.loginState == LoginConnectionDetails::LoginState_IsLoggedIn )
      {
         if( details.gatewayId == gatewayId )
            return i;
      }
   }
   return GameProductId_NUM_GAMES;
}

bool     UserLogin::GetListOfConnectedIds( vector< LoginConnectionDetails >& listOfActiveConnections ) const
{
   listOfActiveConnections.clear();
   for( U32 i=0; i< GameProductId_NUM_GAMES; i++ )
   {
      const UserLogin_ConnectionProductDetails& details = m_connectionDetails[ i ];
      if( details.loginState == LoginConnectionDetails::LoginState_IsLoggedIn )
      {
         listOfActiveConnections.push_back( details );
      }
   }
   if( listOfActiveConnections.size() )
      return true;
   return false;
}

bool     UserLogin::GetConnectionInfo( U8 gameProductId, U32& connectionId, U32& gatewayId )// returns false if not connected
{
   assert( gameProductId < GameProductId_NUM_GAMES );
   connectionId = 0;
   gatewayId = 0;

   const UserLogin_ConnectionProductDetails& details = m_connectionDetails[ gameProductId ];
   if( details.loginState != LoginConnectionDetails::LoginState_IsLoggedIn )
   {
      return false;
   }
   connectionId = details.connectionId;
   gatewayId = details.gatewayId;
   return true;
}

time_t   UserLogin::GetEarliestLoginTime() const 
{ 
   time_t earliestLoginTime;
   time( &earliestLoginTime );
   bool  wasSet = false;
   for( int i=0; i< GameProductId_NUM_GAMES; i++ )
   {
      const UserLogin_ConnectionProductDetails& details = m_connectionDetails[ i ];
      if( details.loginState == LoginConnectionDetails::LoginState_IsLoggedIn )
      {
         if( details.loginTime && details.loginTime < earliestLoginTime )
         {
            earliestLoginTime = details.loginTime;
         }
      }
   }
   if( wasSet )
      return earliestLoginTime; 
   else
      return 0;
}

time_t        UserLogin:: GetLatestLogoutTime()  const
{
   time_t latestLoginTime = 0;
   bool  wasSet = false;
   for( int i=0; i< GameProductId_NUM_GAMES; i++ )
   {
      const UserLogin_ConnectionProductDetails& details = m_connectionDetails[ i ];
      if( details.loginState == LoginConnectionDetails::Loginstate_IsLoggedOut )
      {
         if( details.logoutTime && details.logoutTime > latestLoginTime )
         {
            latestLoginTime = details.loginTime;
         }
      }
   }
   return latestLoginTime; 
}


//////////////////////////////////////////////////////////////////////////

bool     UserLogin::IsLoggedIntoAProduct()  const
{
   for( int i=0; i< GameProductId_NUM_GAMES; i++ )
   {
      const UserLogin_ConnectionProductDetails& details = m_connectionDetails[ i ];
      if( details.loginState == LoginConnectionDetails::LoginState_IsLoggedIn )
      {
         return true;
      }
   }
   return false;
}

bool     UserLogin::RequiresStateSendToAllServers() const
{
   return m_requiresStateSendToAllServers;
}

void     UserLogin::SendStateToAllServers( U32 userConnectionId, U32 gatewayId, U8 gameProductId, bool isLoggedIn, bool disconnectedByError )
{
   m_loginMainThread->SendLoginStatusToOtherServers( m_userName, m_userUuid, userConnectionId, gameProductId, 
                                    GetDateInUTC(), m_isActive, 
                                    m_email, m_passwordHash, m_userId, m_assetServerKey, m_languageId, isLoggedIn, 
                                    disconnectedByError, gatewayId );
}

//////////////////////////////////////////////////////////////////////////

bool     UserLogin:: LogUserIn( const PacketLoginFromGateway* loginPacket, U32 connectionId )
{
   m_userName = Trim( loginPacket->userName ); 
   m_passwordHash = Trim( loginPacket->password );
   //const string& loginKey = Trim( loginPacket->loginKey );

   U8 gameProductId = loginPacket->gameProductId;
   U32 gatewayId = loginPacket->gatewayId;

   LogUserOutOfExistingConnection( gameProductId );

   AddConnectionId( connectionId, gatewayId, gameProductId );

   SetupQueryForLogin( connectionId );

   return true;
}

//////////////////////////////////////////////////////////////////////////

bool     UserLogin::Logout( U32 connectionId, bool wasDisconnectedByError )
{
   U32 gatewayId = GetGatewayId( connectionId );
   if( gatewayId == 0 )
      return false;

   if( wasDisconnectedByError == false )
   {
      PacketLogoutToClient* logout = new PacketLogoutToClient();
      logout->userName =            m_userName;// just for loggin purposes
      logout->uuid =                m_userUuid;
      m_loginMainThread->SendPacketToGateway( logout, connectionId, gatewayId );
   }

   UserLogin_ConnectionProductDetails& details = m_connectionDetails[ GetProductId( connectionId ) ];

   SendStateToAllServers( details.connectionId, details.gatewayId, details.gameProductId, false, wasDisconnectedByError );

   details.connectionId = 0;
   details.gatewayId = 0;
   details.loginState = LoginConnectionDetails::Loginstate_IsLoggedOut;
   time( &details.logoutTime );
   return UpdateLastLoggedOutTime();

   //return FinalizeLogout();
}

//-----------------------------------------------------------------

bool  UserLogin::UpdateLastLoggedOutTime()
{
   if( m_userUuid.size() == 0 )// this should never happen, but being careful never hurts.
      return false;

   for( int i=0; i< GameProductId_NUM_GAMES; i++ )
   {
      if( m_connectionDetails[ i ].loginState == LoginConnectionDetails::LoginState_IsLoggedIn )
         return false;
   }

   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =              0 ;
   dbQuery->lookup =          LoginMainThread::QueryType_UserQuery;
   dbQuery->isFireAndForget = true;// no result is needed
   dbQuery->serverLookup =    QueryType_UpdateLastLoggedOutTime;

   string queryString = "UPDATE users AS user SET user.last_logout_timestamp=UTC_TIMESTAMP() WHERE uuid = '";
   queryString += m_userUuid;
   queryString += "'";
   dbQuery->query =           queryString;

   return m_loginMainThread->AddQueryToOutput( dbQuery );
}

//////////////////////////////////////////////////////////////////////////

bool     UserLogin:: UpdateProfile( U32 connectionId, const PacketUpdateSelfProfile* updateProfileRequest )
{
   U32 gatewayId = GetGatewayId( connectionId );
   if( gatewayId == 0 )
      return false;

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

      WriteUserBasicsToAccount( connectionId );
      WriteUserProfile( connectionId );

      response->success = true;
      response->avatarIconId = m_avatarIcon;
   }

   
   PackUserProfileRequestAndSendToClient( connectionId, gatewayId );

   m_loginMainThread->SendPacketToGateway( response, connectionId, gatewayId );

   if( response->success )
   {
      TellContactServerToReloadUserProfile( connectionId );
   }

   return true;
}

//////////////////////////////////////////////////////////////////////////

void  UserLogin::WriteUserBasicsToAccount( U32 connectionId )
{
   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =           connectionId;
   dbQuery->lookup =       LoginMainThread::QueryType_UserQuery; 
   dbQuery->meta =         "";
   dbQuery->serverLookup = QueryType_UpdateUsers;
   dbQuery->isFireAndForget = true;

   string query = "UPDATE playdek.users SET user_name='%s',user_email='%s',user_pw_hash='%s',active=";
   query += boost::lexical_cast<string>( m_isActive?1:0 );
   query += ",language_id=";
   query += boost::lexical_cast<string>( m_languageId );
   query += "  WHERE user_id=";
   query += boost::lexical_cast<string>( m_userId );
   dbQuery->query = query;
   dbQuery->escapedStrings.insert( m_userName );
   dbQuery->escapedStrings.insert( m_email );
   dbQuery->escapedStrings.insert( m_passwordHash );

   m_loginMainThread->AddQueryToOutput( dbQuery );
}

//////////////////////////////////////////////////////////////////////////

void  UserLogin::WriteUserProfile( U32 connectionId )
{
   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =           connectionId ;
   dbQuery->lookup =       LoginMainThread::QueryType_UserQuery;
   dbQuery->meta =         "";
   dbQuery->serverLookup = QueryType_UpdateUserProfile;
   dbQuery->isFireAndForget = true;

   string query = "UPDATE playdek.user_profile SET admin_level=0,marketing_opt_out=";
   query += boost::lexical_cast<string>( m_marketingOptOut?1:0 );
   query += ",show_win_loss_record=";
   query += boost::lexical_cast<string>( m_showWinLossRecord?1:0 );
   query += ",show_profile_gender=";
   query += boost::lexical_cast<string>( m_showGenderProfile?1:0 );
   query += ",mber_avatar=";
   query += boost::lexical_cast<string>( m_avatarIcon );
   if( m_userMotto.size() == 0 )
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
   query += boost::lexical_cast<string>( m_userId );
   
   dbQuery->query = query;
   dbQuery->escapedStrings.insert( m_userMotto );
   m_loginMainThread->AddQueryToOutput( dbQuery );
}

//////////////////////////////////////////////////////////////////////////

bool     UserLogin::EchoHandler( U32 connectionId )
{
   U32 gatewayId = GetGatewayId( connectionId );
   if( gatewayId == 0 )
      return false;

   LogMessage( LOG_PRIO_INFO, " Echo " );
   PacketLogin_EchoToClient* echo = new PacketLogin_EchoToClient;
   m_loginMainThread->SendPacketToGateway( echo, connectionId, gatewayId );
   return true;
}

//////////////////////////////////////////////////////////////////////////

/*bool     UserLogin:: AddPurchase( const PacketAddPurchaseEntry* purchase )
{
   if( m_adminLevel > 0 )
   {
      ProductInfo productInfo;
      string productUuid = purchase->productUuid.c_str();
      bool success = m_loginMainThread->FindProductByUuid( productUuid, productInfo );
      if( success == false )
      {
         m_loginMainThread->SendErrorToClient( m_connectionDetails.connectionId, m_connectionDetails.gatewayId, PacketErrorReport::ErrorType_Cheat_ProductUnknown );
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
        
         LogMessage( LOG_PRIO_INFO, "AddPurchase: SendListOfProductsToClientAndAsset" );
         SendListOfProductsToClientAndAsset( m_connectionDetails.connectionId, m_connectionDetails.gatewayId );
         return true;
      }

      string userUuid =    purchase->userUuid.c_str();
      string userEmail =   purchase->userEmail;
      string userName =    purchase->userName;

      UserConnectionMapIterator  it = FindUser( userEmail, userUuid, userName );
      if( it == adminUserData.end() )
      {
         m_loginMainThread->SendErrorToClient( m_connectionDetails.connectionId, m_connectionDetails.gatewayId, PacketErrorReport::ErrorType_Cheat_BadUserLookup );
         return true;
      }
      else
      {
         double price = 0.0;
         float numToGive = static_cast<float>( purchase->quantity );
         WriteProductToUserRecord( it->second.m_userUuid, productUuid, price, numToGive, m_userUuid, "add purchase entry by admin" );

         it->second.AddToProductsOwned( productInfo.productId, productInfo.lookupName, productUuid, numToGive, productInfo.vendorUuid );
         it->second.SendListOfOwnedProductsToClient( m_connectionDetails.connectionId, m_connectionDetails.gatewayId );

         AddToProductsOwned( productInfo.productId, productInfo.lookupName, productInfo.uuid, 1, productInfo.vendorUuid );
         SendListOfOwnedProductsToClient( m_connectionDetails.connectionId, m_connectionDetails.gatewayId );
         return true;
      }
   }
   else
   {
      m_loginMainThread->SendErrorToClient( m_connectionDetails.connectionId, m_connectionDetails.gatewayId, PacketErrorReport::ErrorType_Cheat_BadPermissions );
   }
   return false;
}*/

//////////////////////////////////////////////////////////////////////////

bool     UserLogin:: Disconnect( U32 userConnectionId )
{
   int whichConnection = FindConnectionDetails( userConnectionId );
   if( whichConnection == GameProductId_NUM_GAMES )
   {
      LogMessage( LOG_PRIO_ERR, "Major bug:" );
      LogMessage( LOG_PRIO_ERR, "while handling HandleQueryResult, we could not find the connection details." );
      return false;
   }

   LoginConnectionDetails& connection = m_connectionDetails[ whichConnection ];
   

   if( connection.loginState == LoginConnectionDetails::LoginState_IsLoggedIn )
   {
      time_t currentTime;
      time( &currentTime );
      connection.logoutTime = currentTime;
      connection.loginState = LoginConnectionDetails::LoginState_IsLoggingOut;
      //FinalizeLogout();
      UpdateLastLoggedOutTime();
   }
   return true;
}


//---------------------------------------------------------------

void     UserLogin:: TellContactServerToReloadUserProfile( U32 connectionId )
{
   PacketUserUpdateProfile* profile = new PacketUserUpdateProfile;
   PackUserSettings( profile );
   profile->connectionId = connectionId;
   if( m_loginMainThread->SendPacketToOtherServer( profile, connectionId ) == false )
   {
      PacketFactory factory;
      BasePacket* packet = static_cast< BasePacket* >( profile );
      factory.CleanupPacket( packet ); 
   }
}

//////////////////////////////////////////////////////////////////////////
/*
bool     UserLogin::FinalizeLogout()
{
   if( m_userUuid.size() == 0 )// this should never happen, but being careful never hurts.
      return false;

   for( int i=0; i< GameProductId_NUM_GAMES; i++ )
   {
      if( m_connectionDetails[ i ].loginState == LoginConnectionDetails::LoginState_IsLoggedIn )
         return false;
   }

   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =              0;
   dbQuery->lookup =          LoginMainThread::QueryType_UserQuery;//QueryType_UserLogout;
   dbQuery->isFireAndForget = true;// no result is needed
   dbQuery->serverLookup = QueryType_Logout;
   
   string queryString = "UPDATE users AS user SET user.last_logout_timestamp=UTC_TIMESTAMP() WHERE user.uuid = '";
   queryString +=             m_userUuid;
   queryString += "'";
   dbQuery->query =           queryString;

   LogMessage( LOG_PRIO_INFO, "User %s:%s logout at %s", m_userName.c_str(), m_userUuid.c_str(), GetDateInUTC().c_str() );

   //m_loginMainThread->SendPacketToGateway( logout, m_connectionDetails.connectionId, m_connectionDetails.gatewayId );
   return m_loginMainThread->AddQueryToOutput( dbQuery );
}*/

//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////

bool  UserLogin:: SetupQueryForLogin( U32 connectionId )
{
   if( m_loginMainThread->IsPrintingFunctionNames() )
   {
      cout << "fn: " << __FUNCTION__ << endl;
   }

   //*********************************************************************************
   // perhaps some validation here is in order like is this user valid based on the key
   //*********************************************************************************

   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =           connectionId;
   dbQuery->lookup =       LoginMainThread::QueryType_UserQuery;
   //dbQuery->meta;
   dbQuery->serverLookup = QueryType_Login;

   UserCustomData* customData = new UserCustomData;
   customData->hashLookup = m_hashLookup;
   //customData->gameProductId = gameProductId;

   dbQuery->customData =   customData;

   dbQuery->query = "SELECT * FROM users JOIN user_profile ON users.user_id=user_profile.user_id WHERE users.user_email='%s' AND users.user_pw_hash='%s'";
   dbQuery->escapedStrings.insert( m_userName );
   dbQuery->escapedStrings.insert( m_passwordHash );

   return m_loginMainThread->AddQueryToOutput( dbQuery );
}

void  UserLogin::LoadUserAccount_SendingProfileToOtherUser( const string& userName, U32 id )
{
   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =           id;
   dbQuery->lookup =       LoginMainThread::QueryType_UserQuery;
   dbQuery->serverLookup = QueryType_LoadUserAccount_SendingProfileToOtherUser;
   dbQuery->meta         = userName;

  /* UserCustomData* customData = new UserCustomData;
   customData->hashLookup = m_hashLookup;
   //customData->queryType = QueryType_Login;

   dbQuery->customData =   customData;*/

   dbQuery->query = "SELECT * FROM users JOIN user_profile ON users.user_id=user_profile.user_id WHERE users.user_name='%s'";
   dbQuery->escapedStrings.insert( userName );

   m_loginMainThread->AddQueryToOutput( dbQuery );
}

//////////////////////////////////////////////////////////////////////////

bool  UserLogin::HandleQueryResult( const PacketDbQueryResult* dbResult )
{
   U32 userConnectionId = dbResult->id;
   int whichConnection = FindConnectionDetails( userConnectionId );
   LoginConnectionDetails connection;
   if( whichConnection == GameProductId_NUM_GAMES )
   {
      connection = m_connectionDetails[ whichConnection ];
      //LogMessage( LOG_PRIO_ERR, "Major bug:" );
      //LogMessage( LOG_PRIO_ERR, "while handling HandleQueryResult, we could not find the connection details." );
      //return false;
   }

   bool  wasHandled = false;
  // LoginConnectionDetails connection = m_connectionDetails[ whichConnection ];

   switch( dbResult->serverLookup )
   {
   case QueryType_Login:
      {
         if( HandleUserLoginResult( userConnectionId, dbResult ) == false )
         {/*
            m_loginMainThread->SendErrorToClient( userConnectionId, connection.gatewayId, PacketErrorReport::ErrorType_UserBadLogin );  
            string str = "User not valid and db query failed, userName: ";
            str += m_userName;
            str += ", password: ";
            str += m_passwordHash;
            str += ", connectionId: ";
            str += boost::lexical_cast< string> ( userConnectionId );
            str += ", gatewayId: ";
            str += boost::lexical_cast< string> ( connection.gatewayId );
            
            cout << "Error:" << str << endl;
            cout << m_passwordHash << endl;

            LogMessage( LOG_PRIO_ERR, str.c_str() );
            connection->ForceCleanupState();
            time_t currentTime;
            time( &currentTime );
            currentTime += normalLogoutExpireTime - 3;// give us 3 seconds to cleanup
            connection->SetLoggedOutTime( currentTime );
            m_loginMainThread->ForceUserLogoutAndBlock( userConnectionId, connection.gatewayId );
            wasHandled = false;
         }
         else
         {*/
            wasHandled = true;
         }
      }
      break;
   case QueryType_LoadUserAccount_SendingProfileToOtherUser:
      {
         if( HandleLoadUserAccount_SendingProfileToOtherUser( dbResult ) == false )
         {
            m_isValid = false;
         }
         wasHandled = true;
      }
      break;
   case QueryType_RequestOtherUserProfile:
      {
         //m_loginMainThread->HandleAdminRequestUserProfile( userConnectionId, dbResult );
         wasHandled = true;
      }
      break;
   case QueryType_RequestUserProductsOwned:
      {
         if( dbResult->successfulQuery == false )//|| dbResult->GetBucket().size() == 0 )
         {
            string str = "Query failed looking up a user products ";
            str += m_userName;
            str += ", uuid: ";
            str += m_userUuid;
            LogMessage( LOG_PRIO_ERR, str.c_str() );
            m_loginMainThread->ForceUserLogoutAndBlock( userConnectionId, connection.gatewayId );
            wasHandled = false;
         }
         else
         {
            StoreListOfUsersProductsFromDB( userConnectionId, dbResult );
            if( m_lookupId )
            {
            }
            wasHandled = true;
         }
      }
      break;
  /* case QueryType_GetProductListForUser:
      {
         SendListOfPurchasesToUser( userConnectionId, dbResult );
         wasHandled = true;
      }
      break;*/
   }
   return wasHandled;
}

//////////////////////////////////////////////////////////////////////////

bool     UserLogin::HandleUserLoginResult( U32 userConnectionId, const PacketDbQueryResult* dbResult )
{
   int whichConnection = FindConnectionDetails( userConnectionId );

   if( m_loginMainThread->IsPrintingFunctionNames() )
   {
      cout << "fn: " << __FUNCTION__ << endl;
   }
   if( whichConnection == GameProductId_NUM_GAMES )
   {
      LogMessage( LOG_PRIO_ERR, "Major bug:" );
      LogMessage( LOG_PRIO_ERR, "while handling HandleUserLoginResult, we could not find the connection details." );
      return false;
   }

   if( dbResult->customData )
   {
      UserCustomData* data = static_cast< UserCustomData* >( dbResult->customData );
      delete data;
   }

   LoginConnectionDetails& connectionDetails = m_connectionDetails[ whichConnection ];
   bool  isAccountValid = ( dbResult->successfulQuery == true && dbResult->GetBucket().size() > 0 );
   if( isAccountValid == false )
   {
      m_loginMainThread->ForceUserLogoutAndBlock( userConnectionId, connectionDetails.gatewayId );
      LogMessage( LOG_PRIO_ERR, "Connect user: Cannot continue logging in" );
      LogMessage( LOG_PRIO_ERR, "User record not found" );
      LogMessage( LOG_PRIO_ERR, "Name :%s ", m_userName.c_str() );
      LogMessage( LOG_PRIO_ERR, "Email :%s ", m_email.c_str() );
      LogMessage( LOG_PRIO_ERR, "Uuid :%s ", m_userUuid.c_str() );
      LogMessage( LOG_PRIO_ERR, "PwdHash :%s ", m_passwordHash.c_str() );
      LogMessage( LOG_PRIO_ERR, "Id :%s ", m_userId.c_str() );
      connectionDetails.IncreaseLoginAttemptCount();
      connectionDetails.ClearConnection();

      return false;
   }

   // ---------------------------------
   // lots happening here
   LoginResult( dbResult );
   // ---------------------------------

   if( CanContinueLoggingIn() == false )
   {
      m_loginMainThread->SendErrorToClient( userConnectionId, connectionDetails.gatewayId, PacketErrorReport::ErrorType_Login_CannotAddCurrentProductToUser ); 
      
      m_loginMainThread->ForceUserLogoutAndBlock( userConnectionId, connectionDetails.gatewayId );
      connectionDetails.ClearConnection();
      LogMessage( LOG_PRIO_ERR, "Connect user: Cannot continue logging in" );
      
      return false;
   }

   time_t currentTime;
   time( &currentTime );
   connectionDetails.loginTime = currentTime;

   const bool isLoggedIn = true; 
   const bool wasDisconnectedByError = false;
   m_hasRequestedPurchasesFromClient = false;// rerequest the product list

   m_requiresStateSendToAllServers = true;
   m_loginMainThread->BroadcastLoginStatus( userConnectionId, 
                                connectionDetails.gameProductId, 
                                isLoggedIn, 
                                wasDisconnectedByError,
                                connectionDetails.gatewayId );

   /*
   m_loginMainThread->BroadcastLoginStatusSimple( connectionId, 
                               gameProductId, 
                                true, 
                                false,
                                gatewayId ); 
   */
         
   connectionDetails.ClearLoginAttemptCount();

   RequestListOfPurchases( userConnectionId );

   return false;
}

//////////////////////////////////////////////////////////////////////////

bool  UserLogin::HandleLoadUserAccount_SendingProfileToOtherUser( const PacketDbQueryResult* dbResult )
{
   U32 userConnectionId = m_requestorConnectionId;
   U32 gatewayId = m_loginMainThread->GetGatewayId( m_requestorConnectionId );
   const string& username = dbResult->meta;
   if( gatewayId == 0 ) // the user making the request has logged out.
   {
      m_isValid = false;
      return false;
   }

   bool  isAccountValid = ( dbResult->successfulQuery == true && dbResult->GetBucket().size() > 0 );
   if( isAccountValid == false )
   {
      m_loginMainThread->SendErrorToClient( m_requestorConnectionId, gatewayId, PacketErrorReport::ErrorType_Cheat_BadUserLookup );
      LogMessage( LOG_PRIO_ERR, "HandleLoadUserAccount_SendingProfileToOtherUser error looking up user" );
      LogMessage( LOG_PRIO_ERR, "User record not found" );
      LogMessage( LOG_PRIO_ERR, "username :%s ", username.c_str() );
      return false;
   }

   // ---------------------------------
   // lots happening here
   LoginResult( dbResult );
   // ---------------------------------

   if( CanContinueLoggingIn() == false )
   {
      m_loginMainThread->SendErrorToClient( m_requestorConnectionId, gatewayId, PacketErrorReport::ErrorType_Cheat_BadUserLookup );
   }

/*   time_t currentTime;
   time( &currentTime );
   connectionDetails.loginTime = currentTime;

   const bool isLoggedIn = true; 
   const bool wasDisconnectedByError = false;*/
   m_hasRequestedPurchasesFromClient = false;// rerequest the product list

   m_requiresStateSendToAllServers = false;
  /* m_loginMainThread->BroadcastLoginStatus( userConnectionId, 
                                connectionDetails.gameProductId, 
                                isLoggedIn, 
                                wasDisconnectedByError,
                                connectionDetails.gatewayId );*/
         
   //connectionDetails.ClearLoginAttemptCount();

   m_loginMainThread->FixupLookupInfo( m_lookupId, m_userName, m_userUuid );
   RequestListOfPurchases( m_lookupId );
   return true;
}

//////////////////////////////////////////////////////////////////////////

bool  UserLogin::LoginResult( const PacketDbQueryResult* dbResult )
{
   if( dbResult->successfulQuery == false || 
      dbResult->GetBucket().size() == 0 )// no records found
   {
      return false;
   }

   int whichConnection = FindConnectionDetails( dbResult->id );
   if( whichConnection == GameProductId_NUM_GAMES )
   {
      //LogMessage( LOG_PRIO_ERR, "Major bug:" );
      //LogMessage( LOG_PRIO_ERR, "while handling LoginResult, we could not find the connection details." );
      //return false;
   }

   
   {
      UserPlusProfileTable             enigma( dbResult->bucket );
      UserPlusProfileTable::row        row = *enigma.begin();
      string lookup_email;
      if( dbResult->successfulQuery == true )
      {
         lookup_email = row[ TableUserPlusProfile::Column_email ];
      }

      //UserPlusProfileTable::row row = *enigma.begin();

      m_userId =                          row[ TableUserPlusProfile::Column_id ];
      m_userName =                        row[ TableUserPlusProfile::Column_name ];
      m_userUuid =                        row[ TableUserPlusProfile::Column_uuid ];
      m_email =                           row[ TableUserPlusProfile::Column_email ];
      m_passwordHash =                    row[ TableUserPlusProfile::Column_password_hash ];
      
      if( whichConnection != GameProductId_NUM_GAMES )
      {
         LoginConnectionDetails& connection = m_connectionDetails[ whichConnection ];
         connection.logoutTime = 0;
         connection.lastLoggedInTime =       GetDateFromString( row[ TableUserPlusProfile::Column_last_logout_time ].c_str() );// last time logged in
      }

      m_isActive =                        boost::lexical_cast<bool>( row[ TableUserPlusProfile::Column_active] );

      string language =                   row[ TableUserPlusProfile::Column_language_id ];
      if( language.size() > 0 && language != "NULL" )
      {
         m_languageId =                   boost::lexical_cast<int>( row[ TableUserPlusProfile::Column_language_id] );
      }

      string avatar = row[ TableUserPlusProfile::Column_mber_avatar];
      if( avatar.size() != 0 )
      {
         m_avatarIcon =                   boost::lexical_cast< int > ( avatar );
      }
      //m_adminLevel =                      boost::lexical_cast< int > ( row[ TableUserPlusProfile::Column_admin_level] );
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
         m_timeZone =                     boost::lexical_cast< int >( tz );
      }

      //m_connectionDetails.gameProductId = productId;
   }
   //m_connectionDetails.connectionId =                   dbResult->id;
   if( m_assetServerKey.size() == 0 )
   {
      string stringToHash( boost::lexical_cast< string >( dbResult->id ) + m_email );
      U32 hash = static_cast<U32>( GenerateUniqueHash( stringToHash ) );
      m_assetServerKey = GenerateUUID( GetCurrentMilliseconds() + hash );
   }

   m_isValid = true;

   return true;
}

//////////////////////////////////////////////////////////////////////////

bool    UserLogin:: SendStatusToGateway( U32 userConnectionId, bool success )
{
   int whichConnection = FindConnectionDetails( userConnectionId );
   if( whichConnection == GameProductId_NUM_GAMES )
   {
      LogMessage( LOG_PRIO_ERR, "Major bug:" );
      LogMessage( LOG_PRIO_ERR, "while handling HandleQueryResult, we could not find the connection details." );
      return false;
   }
   PacketLoginToGateway* loginStatus = new PacketLoginToGateway();
   if( m_userName.size() )
   {
      loginStatus->userName = m_userName;
      loginStatus->userEmail = m_email;
      loginStatus->uuid = m_userUuid;
      loginStatus->lastLogoutTime = m_lastLoginTime;
      loginStatus->loginKey = m_assetServerKey;
   }

   LoginConnectionDetails& connectionDetails = m_connectionDetails[ whichConnection ];
   loginStatus->wasLoginSuccessful = success;
   loginStatus->gameProductId = connectionDetails.gameProductId;

   float packetDelay = 0.4f; // release and other performant solutions
   /// we delay this becuase we want the other servers a chance to react to logout
   /// before the client can log in again.
#ifdef DEBUG
   packetDelay = 0.9f;// based on testing, the contact server and others can take a long time to process logout
#endif

   m_loginMainThread->SendPacketToGateway( loginStatus, userConnectionId, connectionDetails.gatewayId, packetDelay );// two second delay
   return true;
}

//////////////////////////////////////////////////////////////////////////

bool    UserLogin:: LoginFinishedSuccessfully( U32 userConnectionId )
{
   int whichConnection = FindConnectionDetails( userConnectionId );
   if( whichConnection == GameProductId_NUM_GAMES )
   {
      LogMessage( LOG_PRIO_ERR, "Major bug:" );
      LogMessage( LOG_PRIO_ERR, "while handling HandleQueryResult, we could not find the connection details." );
      return false;
   }

   LoginConnectionDetails& connectionDetails = m_connectionDetails[ whichConnection ];

   //m_isReadyToBeCleanedUp = false;
   //m_isLoggingOut = false;// for relogin, we need this to be cleared.
//   UpdateConnectionId( connectId );
   connectionDetails.loginState = LoginConnectionDetails::LoginState_IsLoggedIn;

   productVendorUuids.clear();
   productsWaitingForInsertionToDb.clear();

   bool success = true;

   if( m_isActive == false )
   {
      success = false; // we only have this one condition right now.
      LogMessage( LOG_PRIO_ERR, "User is inactive and will not be able to login" );
   }

   //This is where we inform all of the games that the user is logged in.
   if( success == true )
   {
      if( m_productsOwned.size() == 0 )
      {
         RequestListOfPurchases( userConnectionId );
      }
      LogMessage( LOG_PRIO_INFO, "User successful login at %s", GetDateInUTC().c_str() );
   }
   else
   {
      LogMessage( LOG_PRIO_INFO, "User unsuccessful login at %s", GetDateInUTC().c_str() );
   }
   return success;
}

//////////////////////////////////////////////////////////////////////////

void  UserLogin::WriteUserProfile()
{
   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =           0;
   dbQuery->lookup =       LoginMainThread::QueryType_UserQuery;
   dbQuery->meta =         "";
   dbQuery->serverLookup = QueryType_UpdateUserProfile;
   dbQuery->isFireAndForget = true;

   string query = "UPDATE playdek.user_profile SET marketing_opt_out=";
   query += boost::lexical_cast<string>( m_marketingOptOut?1:0 );
   query += ",show_win_loss_record=";
   query += boost::lexical_cast<string>( m_showWinLossRecord?1:0 );
   query += ",show_profile_gender=";
   query += boost::lexical_cast<string>( m_showGenderProfile?1:0 );
   query += ",mber_avatar=";
   query += boost::lexical_cast<string>( m_avatarIcon );
   if( m_userMotto.size() == 0 )
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
   query += boost::lexical_cast<string>( m_userId );
   
   dbQuery->query = query;
   dbQuery->escapedStrings.insert( m_userMotto );
   m_loginMainThread->AddQueryToOutput( dbQuery );
}
//-----------------------------------------------------------------

template < typename type >
void  UserLogin:: PackUserSettings( type* response ) const
{
   response->userName =          m_userName;
   response->userUuid =          m_userUuid;
   response->email =             m_email;
   response->lastLoginTime =     m_lastLoginTime;
   response->loggedOutTime =     m_lastLoginTime;

   //response->adminLevel =        m_adminLevel;
   response->iconId =            m_avatarIcon;
   //response->isActive =          m_isActive;
   response->showWinLossRecord = m_showWinLossRecord;
   response->marketingOptOut =   m_marketingOptOut;
   response->showGenderProfile = m_showGenderProfile;
   response->motto =             m_userMotto;

   response->displayOnlineStatusToOtherUsers = m_displayOnlineStatusToOtherUsers;
   response->blockContactInvitations =   m_blockContactInvitations;
   response->blockGroupInvitations = m_blockGroupInvitations;
}

//////////////////////////////////////////////////////////////////////////

int      UserLogin::FindConnectionDetails( U32 connectionId ) const 
{
   for( int i=0; i< GameProductId_NUM_GAMES; i++ )
   {
      const UserLogin_ConnectionProductDetails& details = m_connectionDetails[ i ];
      if( details.loginState != LoginConnectionDetails::LoginState_NeverLoggedIn )
      {
         if( details.connectionId == connectionId )
            return i;
      }
   }
   return GameProductId_NUM_GAMES;
}

//////////////////////////////////////////////////////////////////////////

void     UserLogin::RequestListOfPurchases( U32 userConnectionId ) const
{
 /*  int whichConnection = FindConnectionDetails( userConnectionId );
   if( whichConnection == GameProductId_NUM_GAMES )
   {
      LogMessage( LOG_PRIO_ERR, "Major bug:" );
      LogMessage( LOG_PRIO_ERR, "while handling RequestListOfPurchases, we could not find the connection details." );
      return;
   }

   const LoginConnectionDetails& connection = m_connectionDetails[ whichConnection ];*/

   if( m_userUuid.size() && userConnectionId )
   {
      PacketDbQuery* dbQuery = new PacketDbQuery;
      dbQuery->id =        userConnectionId;
      dbQuery->lookup =    LoginMainThread::QueryType_UserQuery;
      //dbQuery->meta = user_uuid;
      dbQuery->serverLookup = QueryType_RequestUserProductsOwned;

      string queryString = "SELECT product.product_id, name_string, product.uuid, num_purchased, filter_name FROM product INNER JOIN user_join_product AS user_join_product ON product.uuid=user_join_product.product_id WHERE user_join_product.user_uuid='%s'";
      dbQuery->query =  queryString;
      dbQuery->escapedStrings.insert( m_userUuid );

      m_loginMainThread->AddQueryToOutput( dbQuery );
   }
}

/*
//---------------------------------------------------------------

bool     UserLogin:: RequestListOfPurchases( const string& user_uuid )
{

   if( m_userUuid.size() && m_connectionDetails.connectionId != 0 )
   {
      PacketDbQuery* dbQuery = new PacketDbQuery;
      dbQuery->id =     m_connectionDetails.connectionId ;
      dbQuery->lookup = DiplodocusLogin::QueryType_UserListOfUserProducts;
      dbQuery->meta = user_uuid;

      string queryString = "SELECT product.product_id, name_string, product.uuid, num_purchased, filter_name FROM product INNER JOIN user_join_product AS user_join_product ON product.uuid=user_join_product.product_id WHERE user_join_product.user_uuid='%s'";
      dbQuery->query =  queryString;
      dbQuery->escapedStrings.insert( user_uuid );

      return m_loginMainThread->AddQueryToOutput( dbQuery );
   }
   return false;
}
*/
//---------------------------------------------------------------

bool     UserLogin:: HandleRequestForListOfPurchases( const PacketListOfUserPurchasesRequest* purchase, U32 connectionId )
{
   SendPurchasesToClient( purchase->userUuid, connectionId );

   return true;
}

void     UserLogin:: SendPurchasesToClient( const string& userUuid, U32 connectionId )
{
   if( userUuid == m_userUuid )
   {
      int whichConnection = FindConnectionDetails( connectionId );
      if( whichConnection == GameProductId_NUM_GAMES )
      {
         LogMessage( LOG_PRIO_ERR, "Major bug:" );
         LogMessage( LOG_PRIO_ERR, "while handling HandleRequestForListOfPurchases, we could not find the connection details." );
         return;
      }

      LoginConnectionDetails& connectionDetails = m_connectionDetails[ whichConnection ];

      LogMessage( LOG_PRIO_INFO, "HandleRequestForListOfPurchases: SendListOfProductsToClientAndAsset" );
      SendListOfOwnedProductsToClient( connectionId, connectionDetails.gatewayId );
   }
}

//////////////////////////////////////////////////////////////////////////

bool     UserLogin:: HandleRequestListOfProducts( const PacketRequestListOfProducts* productRequest, U32 connectionId )
{
   ProductManager* productManager = m_loginMainThread->GetProductManager();
   assert( productManager );

   int whichConnection = FindConnectionDetails( connectionId );
   if( whichConnection == GameProductId_NUM_GAMES )
   {
      LogMessage( LOG_PRIO_ERR, "Major bug:" );
      LogMessage( LOG_PRIO_ERR, "while handling HandleRequestListOfProducts, we could not find the connection details." );
      return false;
   }

   LoginConnectionDetails& connectionDetails = m_connectionDetails[ whichConnection ];

   return productManager->SendListOfAvailableProducts( connectionId, connectionDetails.gatewayId, connectionDetails.gameProductId, m_languageId );
}

//////////////////////////////////////////////////////////////////////////

void     UserLogin:: StoreListOfUsersProductsFromDB( U32 userConnectionId, const PacketDbQueryResult* dbResult )
{
   if( m_loginMainThread->IsPrintingFunctionNames() )
   {
      LogMessage( LOG_PRIO_ERR, "StoreListOfUsersProductsFromDB" );
   }
   int whichConnection = FindConnectionDetails( userConnectionId );
   U32 gatewayId = 0;
   U32 loggedInProductId = 0;

   if( whichConnection != GameProductId_NUM_GAMES )
   {
      //LogMessage( LOG_PRIO_ERR, "Major bug:" );
      //LogMessage( LOG_PRIO_ERR, "while handling StoreListOfUsersProductsFromDB, we could not find the connection details." );
      //return;
      LoginConnectionDetails& connectionDetails = m_connectionDetails[ whichConnection ];
      gatewayId = connectionDetails.gatewayId;
      loggedInProductId = connectionDetails.gameProductId;
   }

   

   bool  didFindGameProduct = false;

   // verify that this product is owned by the player and if not, then add an entry
   ClearAllProductsOwned();
   UserOwnedProductSimpleTable  enigma( dbResult->bucket );
   UserOwnedProductSimpleTable::iterator      it = enigma.begin();
   int   numProducts = dbResult->GetBucket().size();
   numProducts = numProducts;

   int index = 0;

   if( m_loginMainThread->IsPrintingVerbose() )
   {
      LogMessage( LOG_PRIO_ERR, "** products from DB ** " );
   }
   while( it != enigma.end() )
   {
      UserOwnedProductSimpleTable::row       row = *it++;

      int productId =   boost::lexical_cast< int>   ( row[ TableUserOwnedProductSimple::Column_product_id ] );
      string stringLookupName =                       row[ TableUserOwnedProductSimple::Column_product_name_string ];
      string productUuid =                            row[ TableUserOwnedProductSimple::Column_product_uuid ];
      float  quantity = boost::lexical_cast< float> ( row[ TableUserOwnedProductSimple::Column_quantity ] );
      string productVendorUuid =                      row[ TableUserOwnedProductSimple::Column_filter_name ];

      if( m_loginMainThread->IsPrintingVerbose() )
      {
         LogMessage( LOG_PRIO_INFO, "%d) vendorUuid:    %s", index, productVendorUuid.c_str() );
         LogMessage( LOG_PRIO_INFO, "   productUuid:        ", productUuid.c_str() );
         LogMessage( LOG_PRIO_INFO, "   string lookup name: ", stringLookupName.c_str() );
      }
      if( loggedInProductId == productId )
      {
         didFindGameProduct = true;
      }
      AddToProductsOwned( productId, stringLookupName, productUuid, quantity, productVendorUuid );
        
      index++;
   }
   if( m_loginMainThread->IsPrintingVerbose() )
   {
      LogMessage( LOG_PRIO_INFO, "** end products from DB ** " );
   }

   if( didFindGameProduct == false && loggedInProductId != 0 )
   {
      AddCurrentlyLoggedInProductToUserPurchases( loggedInProductId );
   }

   if( m_loginMainThread->IsPrintingFunctionNames() )
   {
      LogMessage( LOG_PRIO_INFO, "StoreListOfUsersProductsFromDB: SendListOfProductsToClientAndAsset" );
   }
   if( gatewayId != 0 )// only applies to a user who is logged in, not to others requesting profiles.
   {
      SendListOfProductsToClientAndAsset( userConnectionId, gatewayId );
   }

  /* if( loadedForSelf == false )
   {
      PackOtherUserProfileRequestAndSendToClient( connectionDetails.connectionId, connectionDetails.gatewayId );
   }*/

   if( m_hasRequestedPurchasesFromClient == false && 
      gatewayId != 0 )// don't make a request if the user is not connected.
   {
      m_hasRequestedPurchasesFromClient = true;
      RequestListOfProductsFromClient( userConnectionId );
   }
   if( m_requestorConnectionId != 0 )
   {
      gatewayId = m_loginMainThread->GetGatewayId( m_requestorConnectionId );
      //SendListOfOwnedProductsToClient( m_requestorConnectionId, gatewayId );
      PackUserProfile_SendToClient( m_requestorConnectionId, gatewayId );
      m_requestorConnectionId = 0;
   }
}

//////////////////////////////////////////////////////////////////////////

void     UserLogin:: RequestListOfProductsFromClient( U32 userConnectionId )
{
   int whichConnection = FindConnectionDetails( userConnectionId );
   if( whichConnection == GameProductId_NUM_GAMES )
   {
      LogMessage( LOG_PRIO_ERR, "Major bug:" );
      LogMessage( LOG_PRIO_ERR, "while handling RequestListOfPurchases, we could not find the connection details." );
      return;
   }

   LoginConnectionDetails& connectionDetails = m_connectionDetails[ whichConnection ];

   PacketListOfUserPurchasesRequest* purchaseRequest = new PacketListOfUserPurchasesRequest;
   m_loginMainThread->SendPacketToGateway( purchaseRequest, userConnectionId, connectionDetails.gatewayId );
}

//////////////////////////////////////////////////////////////////////////

void     UserLogin:: SendListOfProductsToClientAndAsset( U32 connectionId, U32 gatewayId )
{
   // Also, we must send before because the client is likely to begin requesting assets immediately and we 
   // will hand back the wrong assts in that case.
   m_loginMainThread->SendListOfUserProductsToAssetServer( connectionId, gatewayId );

   SendListOfOwnedProductsToClient( connectionId, gatewayId );
}

//////////////////////////////////////////////////////////////////////////

void     UserLogin:: AddCurrentlyLoggedInProductToUserPurchases( U8 gameProductId )// only works for self
{
   ProductManager* productManager = m_loginMainThread->GetProductManager();
   assert( productManager );

   const char* loggedInGameProductName = FindProductName( gameProductId );
   LoginConnectionDetails& connectionDetails = m_connectionDetails[ gameProductId ];
   if( loggedInGameProductName == NULL )
   {
      if( connectionDetails.connectionId != 0 )
      {
         m_loginMainThread->SendErrorToClient( connectionDetails.connectionId, connectionDetails.gatewayId, PacketErrorReport::ErrorType_Login_CannotAddCurrentProductToUser ); 
      }
      LogMessage( LOG_PRIO_ERR, "Major error: user logging in with a product not identified %d", connectionDetails.gameProductId );

      return;
   }

   ProductInfo productInfo;
   bool found = productManager->GetProductByProductId( gameProductId, productInfo );
   if( found == false )
   {
      LogMessage( LOG_PRIO_ERR, "Major error: user logging in with a product not in our list of loaded products %d", gameProductId );
      return;
   }

   WriteProductToUserRecord( m_userUuid, productInfo.uuid, 0.0, 1, connectionDetails.connectionId );
   float numToGive = 1;
   AddToProductsOwned( productInfo.productId, productInfo.lookupName, productInfo.uuid, numToGive, productInfo.vendorUuid );
}

//////////////////////////////////////////////////////////////////////////

void     UserLogin:: WriteProductToUserRecord( const string& productVendorUuid, double pricePaid, U32 userConnectionId )
{
   int whichConnection = FindConnectionDetails( userConnectionId );
   if( whichConnection == GameProductId_NUM_GAMES )
   {
      LogMessage( LOG_PRIO_ERR, "Major bug:" );
      LogMessage( LOG_PRIO_ERR, "while handling WriteProductToUserRecord, we could not find the connection details." );
      return;
   }

   LoginConnectionDetails& connectionDetails = m_connectionDetails[ whichConnection ];

   int userProductIndex = FindProductVendorUuid( productVendorUuid );
   //**  find the item in the user record and add it to the db if not **
   if( userProductIndex == ProductNotFound )// the user doesn't have the record, but the rest of the DB does.
   {
      m_loginMainThread->SendErrorToClient( connectionDetails.connectionId, connectionDetails.gatewayId, PacketErrorReport::ErrorType_Cheat_ProductUnknown ); 
      return;
   }

   ProductManager* productManager = m_loginMainThread->GetProductManager();
   assert( productManager );

   ProductInfo productInfo;
   bool result = productManager->GetProductByIndex( userProductIndex, productInfo );
   if( result == true )
   {
      WriteProductToUserRecord( m_userUuid, productInfo.uuid, pricePaid, 1, connectionDetails.connectionId );
      AddToProductsOwned( productInfo.productId, productInfo.lookupName, productInfo.uuid, 1, productInfo.vendorUuid );
   }
   else
   {
      m_loginMainThread->SendErrorToClient( connectionDetails.connectionId, connectionDetails.gatewayId, PacketErrorReport::ErrorType_Cheat_ProductUnknown ); 
   }
}

//////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------

void  UserLogin::AddProductVendorUuid( string text )
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

int   UserLogin::FindProductVendorUuid( string text ) const
{
   std::transform( text.begin(), text.end(), text.begin(), ::tolower );

   vector< string >::const_iterator searchIt = productVendorUuids.begin();
   while( searchIt != productVendorUuids.end() )
   {
      if( *searchIt == text )
      {
         return ( searchIt - productVendorUuids.begin() );
      }
      searchIt++;
   }
   return ProductNotFound;
}

//-----------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////

void     UserLogin:: WriteProductToUserRecord( const string& userUuid, const string& productUuid, double pricePaid, float numPurchased,  U32 userConnectionId )
{
   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =           0;
   dbQuery->lookup =       LoginMainThread::QueryType_UserQuery;
   dbQuery->serverLookup = QueryType_AddProductInfoToUser;
   dbQuery->meta =         productUuid;
   dbQuery->serverLookup = 0;
   dbQuery->isFireAndForget = true;

   dbQuery->query = "INSERT INTO playdek.user_join_product (user_uuid, product_id, price_paid, num_purchased ) VALUES ( ";
   dbQuery->query += "'%s', '%s', ";
   dbQuery->query += boost::lexical_cast< string >( pricePaid );
   dbQuery->query += ",";
   dbQuery->query += boost::lexical_cast< string >( numPurchased );
   dbQuery->query += " );";


   dbQuery->escapedStrings.insert( userUuid );
   dbQuery->escapedStrings.insert( productUuid );

   m_loginMainThread->AddQueryToOutput( dbQuery );

   if( userConnectionId )
   {
      int whichConnection = FindConnectionDetails( userConnectionId );
      if( whichConnection == GameProductId_NUM_GAMES )
      {
         LogMessage( LOG_PRIO_ERR, "Major bug:" );
         LogMessage( LOG_PRIO_ERR, "while handling WriteProductToUserRecord, we could not find the connection details." );
         return;
      }
      LoginConnectionDetails& connectionDetails = m_connectionDetails[ whichConnection ];
      m_loginMainThread->SendErrorToClient( connectionDetails.connectionId, connectionDetails.gatewayId, PacketErrorReport::ErrorType_Status, PacketErrorReport::StatusSubtype_ProductAdded );
   }
}


//////////////////////////////////////////////////////////////////////////

void     UserLogin:: SendListOfOwnedProductsToClient( U32 connectionId, U32 gatewayId )
{
   LogMessage( LOG_PRIO_INFO, "Sending user his/her list of products" );

   PacketListOfUserAggregatePurchases* purchases = new PacketListOfUserAggregatePurchases;

   ProductManager* productManager = m_loginMainThread->GetProductManager();
   assert( productManager );

   purchases->userUuid = m_userUuid;
   map< U32, ProductBrief >::iterator it = m_productsOwned.begin();
   while( it != m_productsOwned.end() )
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
         productManager->GetProductByProductId( pb.productDbId, pi );
         if( pe.name.size() == 0 )
         {
            pe.name = m_loginMainThread->GetStringLookup()->GetString( pi.lookupName, m_languageId );
         }
      }
      pe.productUuid = pb.uuid;

      purchases->purchases.push_back( pe );
   }

   int num = purchases->purchases.size();
   LogMessage( LOG_PRIO_INFO, "SendListOfOwnedProductsToClient:: list of products: num= %d", num );
  /* for( int i=0; i< num; i++ )
   {
      const PurchaseEntry& pe = purchases->purchases[i];
      
      cout << i << ") Uuid:    " << pe.productUuid << endl;
      cout << "   name:        " << pe.name << endl;
   }*/
   //cout << "endlist" << endl << endl;

   m_loginMainThread->SendPacketToGateway( purchases, connectionId, gatewayId );
}

//////////////////////////////////////////////////////////////////////////

bool     UserLogin:: StoreUserPurchases( const PacketListOfUserAggregatePurchases* deviceReportedPurchases, U32 connectionId )
{
   int numItems = deviceReportedPurchases->purchases.size();
   LogMessage( LOG_PRIO_INFO, "**************************" );
   LogMessage( LOG_PRIO_INFO, " user purchases reported for user: %s : %s", m_userName.c_str(), m_userUuid.c_str() );
   if( numItems == 0 )
   {
      LogMessage( LOG_PRIO_INFO, "   None" );
   }

   bool resendUserPurchaseList = false;
   ProductManager* productManager = m_loginMainThread->GetProductManager();
   assert( productManager );

   for( int i=0; i< numItems; i++ )
   {
      const PurchaseEntryExtended& purchaseEntry = deviceReportedPurchases->purchases[i];
      int  originalProductNameIndex = productManager->FindProductByVendorUuid( purchaseEntry.productUuid );
      if( originalProductNameIndex == ProductManager::ProductNotFound )
         originalProductNameIndex = productManager->FindProductByName( purchaseEntry.name );

      //----------------
      if( purchaseEntry.name.size() == 0 )// we can't do anything with this.
      {
         LogMessage( LOG_PRIO_ERR, "   ***Invalid product id...title: %s   name: %s", purchaseEntry.productUuid.c_str(), purchaseEntry.name.c_str() );
      }
      else
      {
         if( purchaseEntry.productUuid.size() == 0 )
         {
            LogMessage( LOG_PRIO_INFO, "   ***Invalid product id...title: %s   name: %s", purchaseEntry.productUuid.c_str(), purchaseEntry.name.c_str() );
            m_loginMainThread->SendErrorToClient( connectionId, GetGatewayId( connectionId ), PacketErrorReport::ErrorType_Purchase_ProductUnknown ); 
            continue;
         }

         if( originalProductNameIndex == ProductManager::ProductNotFound )
         {
            AddItemToProductTable( purchaseEntry );
         }

         if( StoreOffProductInUserRecord ( originalProductNameIndex, purchaseEntry.productUuid, purchaseEntry.quantity ) )
         {
            resendUserPurchaseList = true;
         }
      }
      LogMessage( LOG_PRIO_INFO, "  %d:   title: %s", i, purchaseEntry.name.c_str() );
      
   }

   LogMessage( LOG_PRIO_INFO, "************************* " );


   if( resendUserPurchaseList )
   {
      LogMessage( LOG_PRIO_INFO, "StoreUserPurchases: SendListOfProductsToClientAndAsset" );
      SendListOfProductsToClientAndAsset( connectionId, GetGatewayId( connectionId ) );
   }

   return true;
}


//////////////////////////////////////////////////////////////////////////

void  UserLogin:: AddItemToProductTable( const PurchaseEntryExtended& purchaseEntry )
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

   ProductManager* productManager = m_loginMainThread->GetProductManager();
   assert( productManager );
   productManager->AddProduct( purchaseEntry );
}

//---------------------------------------------------------------

bool  UserLogin:: StoreOffProductInUserRecord ( int userManagerIndex, 
                                                         const string& productUuid, 
                                                         float numPurchased)
{
   if( numPurchased <= 0 )
   {
      LogMessage( LOG_PRIO_ERR, "Quantity listed as too low... must be at least 1. Rejected entry: %d", productUuid.c_str() ) ;
      return false;
   }

   int userProductIndex = ProductManager::ProductNotFound;
   if( productUuid.size() )
   {
      // the order of the next two lines matters a lot.
      userProductIndex = FindProductVendorUuid( productUuid ); 
    /*  if( userProductIndex == ProductManager::ProductNotFound )
      {
         AddProductVendorUuid( productUuid );// we're gonna save the name, regardless.
      }*/
   }   

   //**  find the item in the user record and add it to the db if not **
   if( userManagerIndex != ProductManager::ProductNotFound )
   {
      ProductManager* productManager = m_loginMainThread->GetProductManager();
      assert( productManager );

      ProductInfo productInfo;
      bool result = productManager->GetProductByIndex( userManagerIndex, productInfo );
      if( result == true )
      {
         // this logic is tricky. If I don't already have it.. or I can purchase it again...
         if( ( userProductIndex == ProductManager::ProductNotFound ) ||
              ( productManager->CanProductBePurchasedMultipleTimes( productInfo ) == true ) )
         {
            const string& productUuid = productInfo.uuid;
            WriteProductToUserRecord( m_userUuid, productUuid, 1.0, numPurchased, 0 );// conn id not needed
            AddToProductsOwned( productInfo.productId, productInfo.lookupName, productUuid, numPurchased, productInfo.vendorUuid );

            AddConversionProductsToUserPurchases( productInfo );
            return true;
         }
      }
   }
   return false;
}

//---------------------------------------------------------------

void     UserLogin:: AddConversionProductsToUserPurchases( const ProductInfo& productInfo )
{
   if( productInfo.convertsToProductId == 0 || productInfo.convertsToQuantity == 0 )
      return;

   ProductInfo productInfoLookup;
   ProductManager* productManager = m_loginMainThread->GetProductManager();
      assert( productManager );

   if( productManager->GetProductByProductId( productInfo.convertsToProductId, productInfoLookup ) == false )
      return;
   //originalProductNameIndex = userManager->FindProductByName( purchaseEntry.name );
   StoreOffProductInUserRecord ( productInfo.convertsToProductId, productInfoLookup.uuid, static_cast< float >( productInfo.convertsToQuantity ) );
}

//////////////////////////////////////////////////////////////////////////--

void     UserLogin:: ClearAllProductsOwned()
{
   LogMessage( LOG_PRIO_INFO, "ClearAllProductsOwned for user %s : %s", m_userName.c_str(), m_userUuid.c_str() );
   m_productsOwned.clear();
   productVendorUuids.clear();
}

//////////////////////////////////////////////////////////////////////////--

bool     UserLogin:: AddToProductsOwned( int productDbId, const string& lookupName, const string& productUuid, float quantity, const string& vendorUuid  )
{
   map< U32, ProductBrief >::iterator it = m_productsOwned.find( productDbId );
   if( it != m_productsOwned.end() )
   {
      it->second.quantity += quantity;
      return false;
   }
   else
   {
      ProductManager* productManager = m_loginMainThread->GetProductManager();
      assert( productManager );

      ProductInfo pi;
      productManager->GetProductByProductId( productDbId, pi );
      if( pi.isHidden == true )
         return false;

      ProductBrief product;
      product.productDbId = productDbId;
      
      product.localizedName = m_loginMainThread->GetStringLookup()->GetString( lookupName, m_languageId );

      product.vendorUuid = vendorUuid;
      product.uuid = productUuid;
      product.quantity = quantity;
      m_productsOwned.insert( pair< U32, ProductBrief > ( productDbId, product ) );

      AddProductVendorUuid( vendorUuid );
      return true;
   }
}

//////////////////////////////////////////////////////////////////////////

bool     UserLogin:: RequestProfile( const PacketRequestUserProfile* profileRequest )
{
   if( profileRequest->uuid == m_userUuid || profileRequest->userName == m_userName || 
      profileRequest->userEmail == m_email )
   {
      U8 gameProductId = profileRequest->gameProductId;

      LoginConnectionDetails& connectionDetails = m_connectionDetails[ gameProductId ];
      if( connectionDetails.connectionId != 0 )
      {
         PackUserProfileRequestAndSendToClient( connectionDetails.connectionId, connectionDetails.gatewayId );
      }
      return true;
   }
 /*  if( m_adminLevel < 1 )
   {
      m_loginMainThread->SendErrorToClient( m_connectionDetails.connectionId, m_connectionDetails.gatewayId, PacketErrorReport::ErrorType_Cheat_BadPermissions );
      return false;
   }*/

 /*  UserConnectionMapIterator  it = FindUser( profileRequest->userEmail, profileRequest->uuid.c_str(), profileRequest->userName );
   if( it != adminUserData.end() )
   {
      it->second.PackUserProfileRequestAndSendToClient( m_connectionDetails.connectionId );
      return true;
   }

   RequestProfile( profileRequest->userEmail, profileRequest->uuid.c_str(), profileRequest->userName, true, true );
*/
   return false;
}

//////////////////////////////////////////////////////////////////////////

void     UserLogin:: PackUserProfileRequestAndSendToClient( U32 connectionId, U32 gatewayId )
{
   PacketRequestUserProfileResponse* response = new PacketRequestUserProfileResponse;
   PackUserSettings( response );

   m_loginMainThread->SendPacketToGateway( response, connectionId, gatewayId );
}

//////////////////////////////////////////////////////////////////////////

void     UserLogin:: PackUserProfile_SendToClient( U32 connectionId, U32 gatewayId )
{
   PacketRequestOtherUserProfileResponse* response = new PacketRequestOtherUserProfileResponse;

   response->basicProfile.insert( "name", m_userName );
   response->basicProfile.insert( "uuid", m_userUuid );
   response->basicProfile.insert( "motto", m_userMotto );
   response->basicProfile.insert( "show_win_loss_record", boost::lexical_cast< string >( m_showWinLossRecord  ? 1:0 ) );
   response->basicProfile.insert( "time_zone", boost::lexical_cast< string >( m_timeZone ) );

   response->basicProfile.insert( "avatar_icon", boost::lexical_cast< string >( m_avatarIcon ) );

   ProductManager* productManager = m_loginMainThread->GetProductManager();
   assert( productManager );
   map< U32, ProductBrief >::iterator it = m_productsOwned.begin();
   while( it != m_productsOwned.end() )
   {
      const string& uuid = it->second.uuid;

      ProductInfo returnPi;
      if( productManager->FindProductByUuid( uuid, returnPi ) )
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

   m_loginMainThread->SendPacketToGateway( response, connectionId, gatewayId );
}

//////////////////////////////////////////////////////////////////////////
