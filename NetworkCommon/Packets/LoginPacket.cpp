// LoginPacket.cpp

#include "LoginPacket.h"

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////


bool  PacketLogin::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, uuid, minorVersion );
   Serialize::In( data, bufferOffset, userName, minorVersion );
   
   Serialize::In( data, bufferOffset, password, minorVersion );
   Serialize::In( data, bufferOffset, loginKey, minorVersion );
   Serialize::In( data, bufferOffset, languageCode, minorVersion );

   return true;
}

bool  PacketLogin::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, uuid, minorVersion );
   Serialize::Out( data, bufferOffset, userName, minorVersion );

   Serialize::Out( data, bufferOffset, password, minorVersion );
   Serialize::Out( data, bufferOffset, loginKey, minorVersion );
   Serialize::Out( data, bufferOffset, languageCode, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketLoginFromGateway::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketLogin::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, gatewayId, minorVersion );

   return true;
}

bool  PacketLoginFromGateway::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketLogin::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, gatewayId, minorVersion );

   return true;
}

void  PacketLoginFromGateway::copy( const PacketLogin& login )
{
   gameInstanceId = login.gameInstanceId;
   gameProductId = login.gameProductId;

   uuid = login.uuid;
   userName = login.userName;

   loginKey = login.loginKey;
   password = login.password;
   languageCode = login.languageCode;
   // gatewayId
}

///////////////////////////////////////////////////////////////

bool  PacketLogout::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, wasDisconnectedByError, minorVersion );
   return true;
}

bool  PacketLogout::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, wasDisconnectedByError, minorVersion );
   

   return true;
}


///////////////////////////////////////////////////////////////

bool  PacketLoginToClient::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketLogin::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, lastLogoutTime, minorVersion );
   Serialize::In( data, bufferOffset, wasLoginSuccessful, minorVersion );
   Serialize::In( data, bufferOffset, connectionId, minorVersion );

  /* if( minorVersion > 1 )
   {
      Serialize::In( data, bufferOffset, junk1, minorVersion );
      Serialize::In( data, bufferOffset, junk2, minorVersion );
   }*/

   return true;
}

bool  PacketLoginToClient::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketLogin::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, lastLogoutTime, minorVersion );
   Serialize::Out( data, bufferOffset, wasLoginSuccessful, minorVersion );
   Serialize::Out( data, bufferOffset, connectionId, minorVersion );

 /*  if( minorVersion > 1 )
   {
      Serialize::Out( data, bufferOffset, junk2, minorVersion );
      Serialize::Out( data, bufferOffset, junk2, minorVersion );
   }*/

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketLoginToGateway::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketLogin::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, lastLogoutTime, minorVersion );
   Serialize::In( data, bufferOffset, wasLoginSuccessful, minorVersion ); 
   Serialize::In( data, bufferOffset, adminLevel, minorVersion );
   Serialize::In( data, bufferOffset, languageId, minorVersion );

   return true;
}

bool  PacketLoginToGateway::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketLogin::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, lastLogoutTime, minorVersion );
   Serialize::Out( data, bufferOffset, wasLoginSuccessful, minorVersion );
   Serialize::Out( data, bufferOffset, adminLevel, minorVersion );
   Serialize::Out( data, bufferOffset, languageId, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketCreateAccount::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, userName, minorVersion );
   Serialize::In( data, bufferOffset, userEmail, minorVersion );
   Serialize::In( data, bufferOffset, password, minorVersion );
   Serialize::In( data, bufferOffset, deviceId, minorVersion );
   Serialize::In( data, bufferOffset, deviceAccountId, minorVersion );
   Serialize::In( data, bufferOffset, languageId, minorVersion );

   return true;
}

bool  PacketCreateAccount::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, userName, minorVersion );
   Serialize::Out( data, bufferOffset, userEmail, minorVersion );
   Serialize::Out( data, bufferOffset, password, minorVersion );
   Serialize::Out( data, bufferOffset, deviceId, minorVersion );
   Serialize::Out( data, bufferOffset, deviceAccountId, minorVersion );
   Serialize::Out( data, bufferOffset, languageId, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketCreateAccountResponse::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, userName, minorVersion );
   Serialize::In( data, bufferOffset, userEmail, minorVersion );
   Serialize::In( data, bufferOffset, wasSuccessful, minorVersion );

   return true;
}

bool  PacketCreateAccountResponse::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, userName, minorVersion );
   Serialize::Out( data, bufferOffset, userEmail, minorVersion );
   Serialize::Out( data, bufferOffset, wasSuccessful, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

bool  PurchaseEntry::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   //Serialize::In( data, bufferOffset, name );
   Serialize::In( data, bufferOffset, quantity, minorVersion );
   Serialize::In( data, bufferOffset, productUuid, minorVersion );

   return true;
}

bool  PurchaseEntry::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   //Serialize::Out( data, bufferOffset, name, minorVersion );
   Serialize::Out( data, bufferOffset, quantity, minorVersion );
   Serialize::Out( data, bufferOffset, productUuid, minorVersion );

   return true;
}
///////////////////////////////////////////////////////////////

bool  PurchaseEntryExtended::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PurchaseEntry::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, name, minorVersion );

   return true;
}

bool  PurchaseEntryExtended::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PurchaseEntry::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, name, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  ProductBriefPacketed::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   Serialize::In( data, bufferOffset, localizedName, minorVersion );
   Serialize::In( data, bufferOffset, uuid, minorVersion );
   Serialize::In( data, bufferOffset, vendorUuid, minorVersion );
   Serialize::In( data, bufferOffset, productType, minorVersion );
   Serialize::In( data, bufferOffset, parentUuid, minorVersion );
   Serialize::In( data, bufferOffset, iconName, minorVersion );

/*   if( minorVersion > 1 )
   {
      Serialize::In( data, bufferOffset, junk1, minorVersion );
      Serialize::In( data, bufferOffset, junk2, minorVersion );
   }
   else
   {
      junk1 = 0;
      junk2.clear();
   }*/

   return true;
}

bool  ProductBriefPacketed::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   Serialize::Out( data, bufferOffset, localizedName, minorVersion );
   Serialize::Out( data, bufferOffset, uuid, minorVersion );
   Serialize::Out( data, bufferOffset, vendorUuid, minorVersion );
   Serialize::Out( data, bufferOffset, productType, minorVersion );
   Serialize::Out( data, bufferOffset, parentUuid, minorVersion );
   Serialize::Out( data, bufferOffset, iconName, minorVersion );

  /* if( minorVersion > 1 )
   {
      Serialize::Out( data, bufferOffset, junk1, minorVersion );
      Serialize::Out( data, bufferOffset, junk2, minorVersion );
   }*/

   return true;
}


///////////////////////////////////////////////////////////////

bool  PacketListOfUserPurchasesRequest::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, platformId, minorVersion );
   Serialize::In( data, bufferOffset, userUuid, minorVersion );

   return true;
}

bool  PacketListOfUserPurchasesRequest::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, platformId, minorVersion );
   Serialize::Out( data, bufferOffset, userUuid, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketAddPurchaseEntry::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, userUuid, minorVersion );
   Serialize::In( data, bufferOffset, userEmail, minorVersion );
   Serialize::In( data, bufferOffset, userName, minorVersion );
   Serialize::In( data, bufferOffset, productUuid, minorVersion );
   Serialize::In( data, bufferOffset, adminNotes, minorVersion );
   Serialize::In( data, bufferOffset, platformId, minorVersion );
   Serialize::In( data, bufferOffset, quantity, minorVersion );

   return true;
}

bool  PacketAddPurchaseEntry::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, userUuid, minorVersion );
   Serialize::Out( data, bufferOffset, userEmail, minorVersion );
   Serialize::Out( data, bufferOffset, userName, minorVersion );
   Serialize::Out( data, bufferOffset, productUuid, minorVersion );
   Serialize::Out( data, bufferOffset, adminNotes, minorVersion );
   Serialize::Out( data, bufferOffset, platformId, minorVersion );
   Serialize::Out( data, bufferOffset, quantity, minorVersion );

   return true;
}


///////////////////////////////////////////////////////////////

bool  PacketListOfUserAggregatePurchases::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, platformId, minorVersion );
   Serialize::In( data, bufferOffset, userUuid, minorVersion );
   
   Serialize::In( data, bufferOffset, purchases, minorVersion );
   //purchases.SerializeIn( data, bufferOffset, minorVersion );

   return true;
}

bool  PacketListOfUserAggregatePurchases::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, platformId, minorVersion );
   Serialize::Out( data, bufferOffset, userUuid, minorVersion );

   Serialize::Out( data, bufferOffset, purchases, minorVersion );
   //purchases.SerializeOut( data, bufferOffset, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketListOfUserProductsS2S::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, uuid, minorVersion );
   Serialize::In( data, bufferOffset, connectionId, minorVersion );
   Serialize::In( data, bufferOffset, products, minorVersion );

   return true;
}

bool  PacketListOfUserProductsS2S::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, uuid, minorVersion );
   Serialize::Out( data, bufferOffset, connectionId, minorVersion );
   Serialize::Out( data, bufferOffset, products, minorVersion );

   return true;
}


///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////

bool  PacketPrepareForUserLogin::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketLogin::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, lastLoginTime, minorVersion );
   Serialize::In( data, bufferOffset, connectionId, minorVersion );
   Serialize::In( data, bufferOffset, email, minorVersion );
   Serialize::In( data, bufferOffset, active, minorVersion );
   Serialize::In( data, bufferOffset, userId, minorVersion );
   Serialize::In( data, bufferOffset, languageId, minorVersion );
   Serialize::In( data, bufferOffset, gatewayId, minorVersion );

   return true;
}

bool  PacketPrepareForUserLogin::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketLogin::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, lastLoginTime, minorVersion );
   Serialize::Out( data, bufferOffset, connectionId, minorVersion );
   Serialize::Out( data, bufferOffset, email, minorVersion );
   Serialize::Out( data, bufferOffset, active, minorVersion );
   Serialize::Out( data, bufferOffset, userId, minorVersion );
   Serialize::Out( data, bufferOffset, languageId, minorVersion );
   Serialize::Out( data, bufferOffset, gatewayId, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketPrepareForUserLogout::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, uuid, minorVersion );
   Serialize::In( data, bufferOffset, connectionId, minorVersion );
   Serialize::In( data, bufferOffset, wasDisconnectedByError, minorVersion );

   return true;
}

bool  PacketPrepareForUserLogout::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, uuid, minorVersion );
   Serialize::Out( data, bufferOffset, connectionId, minorVersion );
   Serialize::Out( data, bufferOffset, wasDisconnectedByError, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketLoginThrottlePackets::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, delayBetweenPacketsMs, minorVersion );

   return true;
}

bool  PacketLoginThrottlePackets::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, delayBetweenPacketsMs, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketLoginDebugThrottleUserPackets::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, level, minorVersion );

   return true;
}

bool  PacketLoginDebugThrottleUserPackets::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, level, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

bool  PacketRequestUserProfile::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, uuid, minorVersion );
   Serialize::In( data, bufferOffset, userEmail, minorVersion );
   Serialize::In( data, bufferOffset, userName, minorVersion );

   return true;
}

bool  PacketRequestUserProfile::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, uuid, minorVersion );
   Serialize::Out( data, bufferOffset, userEmail, minorVersion );
   Serialize::Out( data, bufferOffset, userName, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketRequestUserProfileResponse::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, userName, minorVersion );
   Serialize::In( data, bufferOffset, passwordHash, minorVersion );
   Serialize::In( data, bufferOffset, email, minorVersion );
   Serialize::In( data, bufferOffset, userUuid, minorVersion );
   Serialize::In( data, bufferOffset, lastLoginTime, minorVersion );
   Serialize::In( data, bufferOffset, loggedOutTime, minorVersion );   
   Serialize::In( data, bufferOffset, motto, minorVersion ); 
   Serialize::In( data, bufferOffset, adminLevel, minorVersion );
   Serialize::In( data, bufferOffset, iconId, minorVersion );  
   Serialize::In( data, bufferOffset, languageId, minorVersion ); 
   Serialize::In( data, bufferOffset, isActive, minorVersion );
   Serialize::In( data, bufferOffset, showWinLossRecord, minorVersion );
   Serialize::In( data, bufferOffset, marketingOptOut, minorVersion );
   Serialize::In( data, bufferOffset, showGenderProfile, minorVersion );
   Serialize::In( data, bufferOffset, displayOnlineStatusToOtherUsers, minorVersion );
   Serialize::In( data, bufferOffset, blockContactInvitations, minorVersion );
   Serialize::In( data, bufferOffset, blockGroupInvitations, minorVersion );
   Serialize::In( data, bufferOffset, wins, minorVersion );
   Serialize::In( data, bufferOffset, losses, minorVersion ); 
   Serialize::In( data, bufferOffset, ties, minorVersion ); 

   return true;
}

bool  PacketRequestUserProfileResponse::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, userName, minorVersion );
   Serialize::Out( data, bufferOffset, passwordHash, minorVersion );
   Serialize::Out( data, bufferOffset, email, minorVersion );
   Serialize::Out( data, bufferOffset, userUuid, minorVersion );
   Serialize::Out( data, bufferOffset, lastLoginTime, minorVersion );
   Serialize::Out( data, bufferOffset, loggedOutTime, minorVersion );
   Serialize::Out( data, bufferOffset, motto, minorVersion );
   Serialize::Out( data, bufferOffset, adminLevel, minorVersion );   
   Serialize::Out( data, bufferOffset, iconId, minorVersion );  
   Serialize::Out( data, bufferOffset, languageId, minorVersion );  
   Serialize::Out( data, bufferOffset, isActive, minorVersion );
   Serialize::Out( data, bufferOffset, showWinLossRecord, minorVersion );
   Serialize::Out( data, bufferOffset, marketingOptOut, minorVersion );
   Serialize::Out( data, bufferOffset, showGenderProfile, minorVersion );
   Serialize::Out( data, bufferOffset, displayOnlineStatusToOtherUsers, minorVersion );
   Serialize::Out( data, bufferOffset, blockContactInvitations, minorVersion );
   Serialize::Out( data, bufferOffset, blockGroupInvitations, minorVersion );
   Serialize::Out( data, bufferOffset, wins, minorVersion );
   Serialize::Out( data, bufferOffset, losses, minorVersion ); 
   Serialize::Out( data, bufferOffset, ties, minorVersion ); 

   return true;
}

///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////

bool  PacketUpdateUserProfile::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, userName, minorVersion );
   Serialize::In( data, bufferOffset, passwordHash, minorVersion );
   Serialize::In( data, bufferOffset, email, minorVersion );
   Serialize::In( data, bufferOffset, userUuid, minorVersion ); 
   Serialize::In( data, bufferOffset, adminLevel, minorVersion );  
   Serialize::In( data, bufferOffset, languageId, minorVersion ); 
   Serialize::In( data, bufferOffset, isActive, minorVersion );
 /*  Serialize::In( data, bufferOffset, showWinLossRecord );
   Serialize::In( data, bufferOffset, marketingOptOut );
   Serialize::In( data, bufferOffset, showGenderProfile );
   Serialize::In( data, bufferOffset, displayOnlineStatusToOtherUsers );
   Serialize::In( data, bufferOffset, blockContactInvitations );
   Serialize::In( data, bufferOffset, blockGroupInvitations );*/

   return true;
}

bool  PacketUpdateUserProfile::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, userName, minorVersion );
   Serialize::Out( data, bufferOffset, passwordHash, minorVersion );
   Serialize::Out( data, bufferOffset, email, minorVersion );
   Serialize::Out( data, bufferOffset, userUuid, minorVersion );  
   Serialize::Out( data, bufferOffset, adminLevel, minorVersion );  
   Serialize::Out( data, bufferOffset, languageId, minorVersion ); 
   Serialize::Out( data, bufferOffset, isActive, minorVersion );
 /*  Serialize::Out( data, bufferOffset, showWinLossRecord );
   Serialize::Out( data, bufferOffset, marketingOptOut );
   Serialize::Out( data, bufferOffset, showGenderProfile );
   Serialize::Out( data, bufferOffset, displayOnlineStatusToOtherUsers );
   Serialize::Out( data, bufferOffset, blockContactInvitations );
   Serialize::Out( data, bufferOffset, blockGroupInvitations );*/

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketUpdateUserProfileResponse::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, uuid, minorVersion );
   Serialize::In( data, bufferOffset, success, minorVersion );

   return true;
}

bool  PacketUpdateUserProfileResponse::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, uuid, minorVersion );
   Serialize::Out( data, bufferOffset, success, minorVersion );

   return true;
}
///////////////////////////////////////////////////////////////

bool  PacketRequestListOfProducts::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, platformId, minorVersion );

   return true;
}

bool  PacketRequestListOfProducts::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, platformId, minorVersion );

   return true;
}
///////////////////////////////////////////////////////////////


bool  PacketRequestListOfProductsResponse::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, platformId, minorVersion );
   Serialize::In( data, bufferOffset, products, minorVersion );
   //products.SerializeIn( data, bufferOffset, minorVersion );

   return true;
}

bool  PacketRequestListOfProductsResponse::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, platformId, minorVersion );
   Serialize::Out( data, bufferOffset, products, minorVersion );
   //products.SerializeOut( data, bufferOffset, minorVersion );

   return true;
}
///////////////////////////////////////////////////////////////


bool  PacketRequestOtherUserProfile::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{ 
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, userName, minorVersion );
   Serialize::In( data, bufferOffset, fullProfile, minorVersion );


   return true; 
}

bool  PacketRequestOtherUserProfile::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{ 
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, userName, minorVersion );
   Serialize::Out( data, bufferOffset, fullProfile, minorVersion );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketRequestOtherUserProfileResponse::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{ 
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );

   Serialize::In( data, bufferOffset, basicProfile, minorVersion );
   Serialize::In( data, bufferOffset, productsOwned, minorVersion );
   Serialize::In( data, bufferOffset, awards, minorVersion );
   //basicProfile.SerializeIn( data, bufferOffset, minorVersion );
   //productsOwned.SerializeIn( data, bufferOffset, minorVersion );
   //awards.SerializeIn( data, bufferOffset, minorVersion );

   return true; 
}

bool  PacketRequestOtherUserProfileResponse::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{ 
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );

   Serialize::Out( data, bufferOffset, basicProfile, minorVersion );
   Serialize::Out( data, bufferOffset, productsOwned, minorVersion );
   Serialize::Out( data, bufferOffset, awards, minorVersion );
   //basicProfile.SerializeOut( data, bufferOffset, minorVersion );
   //productsOwned.SerializeOut( data, bufferOffset, minorVersion );
   //awards.SerializeOut( data, bufferOffset, minorVersion );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketUpdateSelfProfile::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, userName, minorVersion );
   Serialize::In( data, bufferOffset, passwordHash, minorVersion );
   Serialize::In( data, bufferOffset, email, minorVersion ); 
   Serialize::In( data, bufferOffset, motto, minorVersion ); 
     
   Serialize::In( data, bufferOffset, languageId, minorVersion ); 
   Serialize::In( data, bufferOffset, avatarIconId, minorVersion );
   
   Serialize::In( data, bufferOffset, showWinLossRecord, minorVersion );
   Serialize::In( data, bufferOffset, marketingOptOut, minorVersion );
   Serialize::In( data, bufferOffset, showGenderProfile, minorVersion );
   Serialize::In( data, bufferOffset, displayOnlineStatusToOtherUsers, minorVersion );
   Serialize::In( data, bufferOffset, blockContactInvitations, minorVersion );
   Serialize::In( data, bufferOffset, blockGroupInvitations, minorVersion );

  /* Serialize::In( data, bufferOffset, wins );
   Serialize::In( data, bufferOffset, losses ); 
   Serialize::In( data, bufferOffset, ties ); */

   return true;
}

bool  PacketUpdateSelfProfile::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, userName, minorVersion );
   Serialize::Out( data, bufferOffset, passwordHash, minorVersion );
   Serialize::Out( data, bufferOffset, email, minorVersion ); 
   Serialize::Out( data, bufferOffset, motto, minorVersion ); 

   Serialize::Out( data, bufferOffset, languageId, minorVersion );    
   Serialize::Out( data, bufferOffset, avatarIconId, minorVersion ); 
   
   Serialize::Out( data, bufferOffset, showWinLossRecord, minorVersion );
   Serialize::Out( data, bufferOffset, marketingOptOut, minorVersion );
   Serialize::Out( data, bufferOffset, showGenderProfile, minorVersion );
   Serialize::Out( data, bufferOffset, displayOnlineStatusToOtherUsers, minorVersion );
   Serialize::Out( data, bufferOffset, blockContactInvitations, minorVersion );
   Serialize::Out( data, bufferOffset, blockGroupInvitations, minorVersion );
   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketUserUpdateProfile::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{ 
   PacketRequestUserProfileResponse::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, connectionId, minorVersion );

   return true; 
}

bool  PacketUserUpdateProfile::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{ 
   PacketRequestUserProfileResponse::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, connectionId, minorVersion );

   return true; 
}

///////////////////////////////////////////////////////////////
bool  PacketUpdateSelfProfileResponse::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{ 
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, avatarIconId, minorVersion );
   Serialize::In( data, bufferOffset, success, minorVersion );

   return true; 
}

bool  PacketUpdateSelfProfileResponse::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{ 
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, avatarIconId, minorVersion );
   Serialize::Out( data, bufferOffset, success, minorVersion );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketListOfUserPurchasesUpdated::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{ 
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, userUuid, minorVersion );
   Serialize::In( data, bufferOffset, userConnectionId, minorVersion );

   return true; 
}

bool  PacketListOfUserPurchasesUpdated::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{ 
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, userUuid, minorVersion );
   Serialize::Out( data, bufferOffset, userConnectionId, minorVersion );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketLogin_LogoutAllUsers::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{ 
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, connectionIds, minorVersion );
   return true; 
}

bool  PacketLogin_LogoutAllUsers::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{ 
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, connectionIds, minorVersion );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketLogin_ListOfMissingFeatures::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{ 
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, services, minorVersion );
   return true; 
}

bool  PacketLogin_ListOfMissingFeatures::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{ 
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, services, minorVersion );

   return true; 
}

///////////////////////////////////////////////////////////////
