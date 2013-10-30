// LoginPacket.cpp

#include "LoginPacket.h"

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////


bool  PacketLogin::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, uuid );
   Serialize::In( data, bufferOffset, userName );
   
   Serialize::In( data, bufferOffset, password );
   Serialize::In( data, bufferOffset, loginKey );
   Serialize::In( data, bufferOffset, languageCode );
   
   return true;
}

bool  PacketLogin::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, uuid );
   Serialize::Out( data, bufferOffset, userName );

   Serialize::Out( data, bufferOffset, password );
   Serialize::Out( data, bufferOffset, loginKey );
   Serialize::Out( data, bufferOffset, languageCode );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketLogout::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, wasDisconnectedByError );
   return true;
}

bool  PacketLogout::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, wasDisconnectedByError );
   

   return true;
}


///////////////////////////////////////////////////////////////

bool  PacketLoginToClient::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketLogin::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, lastLogoutTime );
   Serialize::In( data, bufferOffset, wasLoginSuccessful );
   Serialize::In( data, bufferOffset, connectionId );

   return true;
}

bool  PacketLoginToClient::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketLogin::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, lastLogoutTime );
   Serialize::Out( data, bufferOffset, wasLoginSuccessful );
   Serialize::Out( data, bufferOffset, connectionId );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketLoginToGateway::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketLogin::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, lastLogoutTime );
   Serialize::In( data, bufferOffset, wasLoginSuccessful ); 
   Serialize::In( data, bufferOffset, adminLevel );

   return true;
}

bool  PacketLoginToGateway::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketLogin::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, lastLogoutTime );
   Serialize::Out( data, bufferOffset, wasLoginSuccessful );
   Serialize::Out( data, bufferOffset, adminLevel );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketCreateAccount::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, userName );
   Serialize::In( data, bufferOffset, useremail );
   Serialize::In( data, bufferOffset, password );
   Serialize::In( data, bufferOffset, deviceId );
   Serialize::In( data, bufferOffset, deviceAccountId );
   Serialize::In( data, bufferOffset, languageId );

   return true;
}

bool  PacketCreateAccount::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, userName );
   Serialize::Out( data, bufferOffset, useremail );
   Serialize::Out( data, bufferOffset, password );
   Serialize::Out( data, bufferOffset, deviceId );
   Serialize::Out( data, bufferOffset, deviceAccountId );
   Serialize::Out( data, bufferOffset, languageId );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketCreateAccountResponse::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, userName );
   Serialize::In( data, bufferOffset, useremail );
   Serialize::In( data, bufferOffset, wasSuccessful );

   return true;
}

bool  PacketCreateAccountResponse::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, userName );
   Serialize::Out( data, bufferOffset, useremail );
   Serialize::Out( data, bufferOffset, wasSuccessful );

   return true;
}

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

bool  PurchaseEntry::SerializeIn( const U8* data, int& bufferOffset )
{
   Serialize::In( data, bufferOffset, name );
   Serialize::In( data, bufferOffset, productStoreId );
   Serialize::In( data, bufferOffset, price );
   Serialize::In( data, bufferOffset, number_price );
   Serialize::In( data, bufferOffset, quantity );
   
   Serialize::In( data, bufferOffset, date );

   return true;
}

bool  PurchaseEntry::SerializeOut( U8* data, int& bufferOffset ) const
{
   Serialize::Out( data, bufferOffset, name );
   Serialize::Out( data, bufferOffset, productStoreId );
   Serialize::Out( data, bufferOffset, price );
   Serialize::Out( data, bufferOffset, number_price );
   Serialize::Out( data, bufferOffset, quantity );

   Serialize::Out( data, bufferOffset, date );

   return true;
}
///////////////////////////////////////////////////////////////

bool  ProductBriefPacketed::SerializeIn( const U8* data, int& bufferOffset )
{
   Serialize::In( data, bufferOffset, uuid );
   Serialize::In( data, bufferOffset, filterName );
   Serialize::In( data, bufferOffset, quantity );

   return true;
}

bool  ProductBriefPacketed::SerializeOut( U8* data, int& bufferOffset ) const
{
   Serialize::Out( data, bufferOffset, uuid );
   Serialize::Out( data, bufferOffset, filterName );
   Serialize::Out( data, bufferOffset, quantity );

   return true;
}


///////////////////////////////////////////////////////////////

bool  PacketListOfUserPurchasesRequest::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, platformId );
   Serialize::In( data, bufferOffset, userUuid );

   return true;
}

bool  PacketListOfUserPurchasesRequest::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, platformId );
   Serialize::Out( data, bufferOffset, userUuid );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketAddPurchaseEntry::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, userUuid );
   Serialize::In( data, bufferOffset, userEmail );
   Serialize::In( data, bufferOffset, userName );
   Serialize::In( data, bufferOffset, productUuid );
   Serialize::In( data, bufferOffset, adminNotes );
   Serialize::In( data, bufferOffset, platformId );
   Serialize::In( data, bufferOffset, quantity );

   return true;
}

bool  PacketAddPurchaseEntry::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, userUuid );
   Serialize::Out( data, bufferOffset, userEmail );
   Serialize::Out( data, bufferOffset, userName );
   Serialize::Out( data, bufferOffset, productUuid );
   Serialize::Out( data, bufferOffset, adminNotes );
   Serialize::Out( data, bufferOffset, platformId );
   Serialize::Out( data, bufferOffset, quantity );

   return true;
}


///////////////////////////////////////////////////////////////

bool  PacketListOfUserAggregatePurchases::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, platformId );
   Serialize::In( data, bufferOffset, purchases );

   return true;
}

bool  PacketListOfUserAggregatePurchases::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, platformId );
   Serialize::Out( data, bufferOffset, purchases );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketListOfUserProductsS2S::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, uuid );
   Serialize::In( data, bufferOffset, connectionId );
   Serialize::In( data, bufferOffset, products );

   return true;
}

bool  PacketListOfUserProductsS2S::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, uuid );
   Serialize::Out( data, bufferOffset, connectionId );
   Serialize::Out( data, bufferOffset, products );

   return true;
}


///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////

bool  PacketPrepareForUserLogin::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketLogin::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, lastLoginTime );
   Serialize::In( data, bufferOffset, connectionId );
   Serialize::In( data, bufferOffset, email );
   Serialize::In( data, bufferOffset, active );
   Serialize::In( data, bufferOffset, userId );

   return true;
}

bool  PacketPrepareForUserLogin::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketLogin::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, lastLoginTime );
   Serialize::Out( data, bufferOffset, connectionId );
   Serialize::Out( data, bufferOffset, email );
   Serialize::Out( data, bufferOffset, active );
   Serialize::Out( data, bufferOffset, userId );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketPrepareForUserLogout::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, uuid );
   Serialize::In( data, bufferOffset, connectionId );
   Serialize::In( data, bufferOffset, wasDisconnectedByError );

   return true;
}

bool  PacketPrepareForUserLogout::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, uuid );
   Serialize::Out( data, bufferOffset, connectionId );
   Serialize::Out( data, bufferOffset, wasDisconnectedByError );

   return true;
}

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

bool  PacketRequestUserProfile::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, uuid );
   Serialize::In( data, bufferOffset, userEmail );
   Serialize::In( data, bufferOffset, userName );

   return true;
}

bool  PacketRequestUserProfile::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, uuid );
   Serialize::Out( data, bufferOffset, userEmail );
   Serialize::Out( data, bufferOffset, userName );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketRequestUserProfileResponse::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, userName );
   Serialize::In( data, bufferOffset, passwordHash );
   Serialize::In( data, bufferOffset, email );
   Serialize::In( data, bufferOffset, userUuid );
   Serialize::In( data, bufferOffset, lastLoginTime );
   Serialize::In( data, bufferOffset, loggedOutTime );   
   Serialize::In( data, bufferOffset, adminLevel );     
   Serialize::In( data, bufferOffset, languageId ); 
   Serialize::In( data, bufferOffset, isActive );
   Serialize::In( data, bufferOffset, showWinLossRecord );
   Serialize::In( data, bufferOffset, marketingOptOut );
   Serialize::In( data, bufferOffset, showGenderProfile );
   Serialize::In( data, bufferOffset, profileKeyValues );

   return true;
}

bool  PacketRequestUserProfileResponse::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, userName );
   Serialize::Out( data, bufferOffset, passwordHash );
   Serialize::Out( data, bufferOffset, email );
   Serialize::Out( data, bufferOffset, userUuid );
   Serialize::Out( data, bufferOffset, lastLoginTime );
   Serialize::Out( data, bufferOffset, loggedOutTime );   
   Serialize::Out( data, bufferOffset, adminLevel );   
   Serialize::Out( data, bufferOffset, languageId );  
   Serialize::Out( data, bufferOffset, isActive );
   Serialize::Out( data, bufferOffset, showWinLossRecord );
   Serialize::Out( data, bufferOffset, marketingOptOut );
   Serialize::Out( data, bufferOffset, showGenderProfile );
   Serialize::Out( data, bufferOffset, profileKeyValues );

   return true;
}

///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////

bool  PacketUpdateUserProfile::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, userName );
   Serialize::In( data, bufferOffset, passwordHash );
   Serialize::In( data, bufferOffset, email );
   Serialize::In( data, bufferOffset, userUuid );
   //Serialize::In( data, bufferOffset, lastLoginTime );
   //Serialize::In( data, bufferOffset, loggedOutTime );   
   Serialize::In( data, bufferOffset, adminLevel );  
   Serialize::In( data, bufferOffset, languageId ); 
   Serialize::In( data, bufferOffset, isActive );
   Serialize::In( data, bufferOffset, showWinLossRecord );
   Serialize::In( data, bufferOffset, marketingOptOut );
   Serialize::In( data, bufferOffset, showGenderProfile );

   return true;
}

bool  PacketUpdateUserProfile::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, userName );
   Serialize::Out( data, bufferOffset, passwordHash );
   Serialize::Out( data, bufferOffset, email );
   Serialize::Out( data, bufferOffset, userUuid );
   //Serialize::Out( data, bufferOffset, lastLoginTime );
   //Serialize::Out( data, bufferOffset, loggedOutTime );   
   Serialize::Out( data, bufferOffset, adminLevel );  
   Serialize::Out( data, bufferOffset, languageId ); 
   Serialize::Out( data, bufferOffset, isActive );
   Serialize::Out( data, bufferOffset, showWinLossRecord );
   Serialize::Out( data, bufferOffset, marketingOptOut );
   Serialize::Out( data, bufferOffset, showGenderProfile );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketUpdateUserProfileResponse::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, uuid );
   Serialize::In( data, bufferOffset, success );

   return true;
}

bool  PacketUpdateUserProfileResponse::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, uuid );
   Serialize::Out( data, bufferOffset, success );

   return true;
}
///////////////////////////////////////////////////////////////

bool  PacketRequestListOfProducts::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, platformId );

   return true;
}

bool  PacketRequestListOfProducts::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, platformId );

   return true;
}
///////////////////////////////////////////////////////////////


bool  PacketRequestListOfProductsResponse::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, platformId );
   Serialize::In( data, bufferOffset, products );

   return true;
}

bool  PacketRequestListOfProductsResponse::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, platformId );
   Serialize::Out( data, bufferOffset, products );

   return true;
}
///////////////////////////////////////////////////////////////


bool  PacketRequestOtherUserProfile::SerializeIn( const U8* data, int& bufferOffset )
{ 
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, userName );

   return true; 
}

bool  PacketRequestOtherUserProfile::SerializeOut( U8* data, int& bufferOffset ) const 
{ 
   BasePacket::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, userName );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketRequestOtherUserProfileResponse::SerializeIn( const U8* data, int& bufferOffset )
{ 
   BasePacket::SerializeIn( data, bufferOffset );
  /* Serialize::In( data, bufferOffset, userName );
   Serialize::In( data, bufferOffset, userUuid );
   Serialize::In( data, bufferOffset, profileImage );
   Serialize::In( data, bufferOffset, profileIcon );
   Serialize::In( data, bufferOffset, gmtTimeSoneDifferential );
   Serialize::In( data, bufferOffset, showWinLossRecord );*/

   Serialize::In( data, bufferOffset, basicProfile );
   Serialize::In( data, bufferOffset, productsOwned );
   Serialize::In( data, bufferOffset, awards );

   return true; 
}

bool  PacketRequestOtherUserProfileResponse::SerializeOut( U8* data, int& bufferOffset ) const 
{ 
   BasePacket::SerializeOut( data, bufferOffset );
  /* Serialize::Out( data, bufferOffset, userName );
   Serialize::Out( data, bufferOffset, userUuid );
   Serialize::Out( data, bufferOffset, profileImage );
   Serialize::Out( data, bufferOffset, profileIcon );
   Serialize::Out( data, bufferOffset, gmtTimeSoneDifferential );
   Serialize::Out( data, bufferOffset, showWinLossRecord );*/

   Serialize::Out( data, bufferOffset, basicProfile );
   Serialize::Out( data, bufferOffset, productsOwned );
   Serialize::Out( data, bufferOffset, awards );

   return true; 
}

///////////////////////////////////////////////////////////////