// LoginPacket.h

#pragma once

#include "BasePacket.h"

///////////////////////////////////////////////////////////////

class PacketLogin : public BasePacket
{
public:
   enum LoginType
   {
      LoginType_Login,
      LoginType_Logout,
      LoginType_PacketLogoutToClient,
      LoginType_InformClientOfLoginStatus,
      LoginType_InformGatewayOfLoginStatus,
      LoginType_PrepareForUserLogin,
      LoginType_PrepareForUserLogout,
      LoginType_CreateAccount,
      LoginType_CreateAccountResponse,
      LoginType_RequestListOfPurchases,
      LoginType_ListOfPurchases,
      LoginType_ListOfProductsS2S,
      LoginType_RequestUserProfile,
      LoginType_RequestUserProfileResponse,
      LoginType_UpdateUserProfile,
      LoginType_UpdateUserProfileResponse,
   };
public:
   PacketLogin( int packet_type = PacketType_Login, int packet_sub_type = LoginType_Login ): BasePacket( packet_type, packet_sub_type ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   uuid;
   string   username;
   
   string   loginKey;
   string   password;
   string   languageCode;
};

///////////////////////////////////////////////////////////////

class PacketLogout : public BasePacket
{
public:
   PacketLogout(): BasePacket( PacketType_Login, PacketLogin::LoginType_Logout ), wasDisconnectedByError( false ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;
   
   bool  wasDisconnectedByError;
};

///////////////////////////////////////////////////////////////

class PacketLogoutToClient : public PacketLogin
{
public:
   PacketLogoutToClient(): PacketLogin( PacketType_Login, PacketLogin::LoginType_PacketLogoutToClient ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset ) { return PacketLogin::SerializeIn( data, bufferOffset ); }
   bool  SerializeOut( U8* data, int& bufferOffset ) const { return PacketLogin::SerializeOut( data, bufferOffset ); }
};


///////////////////////////////////////////////////////////////

class PacketLoginToClient : public PacketLogin
{
public:
   PacketLoginToClient(): PacketLogin( PacketType_Login, PacketLogin::LoginType_InformClientOfLoginStatus ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   lastLogoutTime;
   bool     wasLoginSuccessful;
   U32      connectionId;  // used for a user ID.
};


///////////////////////////////////////////////////////////////

class PacketLoginToGateway : public PacketLogin
{
public:
   PacketLoginToGateway(): PacketLogin( PacketType_Login, PacketLogin::LoginType_InformGatewayOfLoginStatus ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   lastLogoutTime;
   bool     wasLoginSuccessful;
   U8       adminLevel;
};

///////////////////////////////////////////////////////////////

class PacketCreateAccount : public BasePacket
{
public:
   PacketCreateAccount(): BasePacket( PacketType_Login, PacketLogin::LoginType_CreateAccount ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;
   
   string username;
   string useremail;
   string password;
   string deviceId;
   string deviceAccountId;
   U8     languageId;
};

///////////////////////////////////////////////////////////////

class PacketCreateAccountResponse : public BasePacket
{
public:
   PacketCreateAccountResponse(): BasePacket( PacketType_Login, PacketLogin::LoginType_CreateAccountResponse ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;
   
   string   username;
   string   useremail;
   bool     wasSuccessful;
};

///////////////////////////////////////////////////////////////

struct PurchaseEntry
{
   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   productStoreId; // apple's name for it.
   string   name;
   string   price;
   double   number_price;
   string   date; // not currently available
};

//--------------------------------

class PacketRequestListOfUserPurchases : public BasePacket
{
public:
   PacketRequestListOfUserPurchases(): BasePacket( PacketType_Login, PacketLogin::LoginType_RequestListOfPurchases ) {}
   
   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   int   platformId;
   bool  requestUserOnly;
};

//--------------------------------

class PacketListOfUserPurchases : public BasePacket
{
public:
   PacketListOfUserPurchases(): BasePacket( PacketType_Login, PacketLogin::LoginType_ListOfPurchases ), isAllProducts( false ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;
   
   int   platformId;
   bool  isAllProducts;
   SerializedVector< PurchaseEntry > purchases;
};

//--------------------------------

class PacketListOfUserProductsS2S : public BasePacket
{
public:
   PacketListOfUserProductsS2S(): BasePacket( PacketType_Login, PacketLogin::LoginType_ListOfProductsS2S ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;
   
   string         uuid;
   U32            connectionId;
   StringBucket   products;
};


///////////////////////////////////////////////////////////////

class PacketPrepareForUserLogin : public PacketLogin
{
public:
   PacketPrepareForUserLogin() : PacketLogin( PacketType_Login, PacketLogin::LoginType_PrepareForUserLogin ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   lastLoginTime;
   U32      connectionId;
   string   email;
   bool     active;
   U32      userId;
};

///////////////////////////////////////////////////////////////

class PacketPrepareForUserLogout : public BasePacket
{
public:
   PacketPrepareForUserLogout() : BasePacket( PacketType_Login, PacketLogin::LoginType_PrepareForUserLogout ), connectionId( 0 ), wasDisconnectedByError( false ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string uuid;
   U32   connectionId;
   bool  wasDisconnectedByError;
};

///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////

class PacketRequestUserProfile : public BasePacket
{
public:
   PacketRequestUserProfile() : BasePacket( PacketType_Login, PacketLogin::LoginType_RequestUserProfile ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   uuid;// you can use any of these fields or all.
   string   userEmail;
   string   userName;
};

///////////////////////////////////////////////////////////////

class PacketRequestUserProfileResponse : public BasePacket // error packet for no admin priv
{
public:
   PacketRequestUserProfileResponse() : BasePacket( PacketType_Login, PacketLogin::LoginType_RequestUserProfileResponse ), 
                              isActive( false ), 
                              showWinLossRecord( false ),
                              marketingOptOut( false ), 
                              showGenderProfile( false ){}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   username;
   string   passwordHash;
   string   email;
   string   userUuid;
   string   lastLoginTime;
   string   loggedOutTime;
   int      adminLevel;

   bool     isActive;
   bool     showWinLossRecord;
   bool     marketingOptOut;
   bool     showGenderProfile;
};

///////////////////////////////////////////////////////////////

class PacketUpdateUserProfile : public BasePacket // limited to admin priveledges only
{
public:
   PacketUpdateUserProfile() : BasePacket( PacketType_Login, PacketLogin::LoginType_UpdateUserProfile ), 
                              isActive( false ), 
                              showWinLossRecord( false ),
                              marketingOptOut( false ), 
                              showGenderProfile( false ){}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   username;
   string   passwordHash;
   string   email;
   string   userUuid;
   string   lastLoginTime;
   string   loggedOutTime;
   int      adminLevel;

   bool     isActive;
   bool     showWinLossRecord;
   bool     marketingOptOut;
   bool     showGenderProfile;
};

///////////////////////////////////////////////////////////////

class PacketUpdateUserProfileResponse : public BasePacket // limited to admin priveledges only
{
public:
   PacketUpdateUserProfileResponse() : BasePacket( PacketType_Login, PacketLogin::LoginType_UpdateUserProfileResponse ), success( false ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   uuid;// you can use any of these fields or all.
   bool     success;
};

///////////////////////////////////////////////////////////////