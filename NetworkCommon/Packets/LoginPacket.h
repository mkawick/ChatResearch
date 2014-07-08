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
      LoginType_EchoToServer,
      LoginType_EchoToClient,
      LoginType_InformClientOfLoginStatus,
      LoginType_InformGatewayOfLoginStatus,
      LoginType_PrepareForUserLogin,
      LoginType_PrepareForUserLogout,
      LoginType_CreateAccount,
      LoginType_CreateAccountResponse,
      LoginType_RequestListOfPurchases,
      LoginType_AddPurchaseEntry,
      LoginType_ListOfAggregatePurchases,
      LoginType_ListOfProductsS2S,
      LoginType_RequestUserProfile,
      LoginType_RequestUserProfileResponse,
      LoginType_UpdateUserProfile,
      LoginType_UpdateUserProfileResponse,
      LoginType_RequestListOfProducts,
      LoginType_RequestListOfProductsResponse,
      LoginType_RequestOtherUserProfile,
      LoginType_RequestOtherUserProfileResponse,
      LoginType_UpdateSelfProfile,
      LoginType_UpdateSelfProfileResponse,
      LoginType_ThrottleUsersConnection,
      LoginType_DebugThrottleUsersConnection,
      LoginType_UserUpdateProfile
   };
public:
   PacketLogin( int packet_type = PacketType_Login, int packet_sub_type = LoginType_Login ): BasePacket( packet_type, packet_sub_type ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   UuidString uuid;
   string   userName;
   
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

class PacketLogin_EchoToServer : public BasePacket
{
public:
   PacketLogin_EchoToServer(): BasePacket( PacketType_Login, PacketLogin::LoginType_EchoToServer ) {}
};

///////////////////////////////////////////////////////////////

class PacketLogin_EchoToClient : public BasePacket
{
public:
   PacketLogin_EchoToClient(): BasePacket( PacketType_Login, PacketLogin::LoginType_EchoToClient ) {}
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
   U8       languageId;
};

///////////////////////////////////////////////////////////////

class PacketCreateAccount : public BasePacket
{
public:
   PacketCreateAccount(): BasePacket( PacketType_Login, PacketLogin::LoginType_CreateAccount ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;
   
   string    userName;
   string    userEmail;
   string    password;
   string    deviceId;
   string    deviceAccountId;
   U8        languageId;
};

///////////////////////////////////////////////////////////////

class PacketCreateAccountResponse : public BasePacket
{
public:
   PacketCreateAccountResponse(): BasePacket( PacketType_Login, PacketLogin::LoginType_CreateAccountResponse ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;
   
   string   userName;
   string   userEmail;
   bool     wasSuccessful;
};

///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////

struct PurchaseEntry
{
   PurchaseEntry() : quantity( 0 ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   name;
   float    quantity;
   string   productUuid;
};

struct ProductBriefPacketed
{
   ProductBriefPacketed() : productType( GameProductType_Game )  {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   UuidString     uuid;
   FixedString80  vendorUuid; // e.g. summonerwars.mv.bund.pt2
   UuidString     parentUuid;
   int            productType;
   string         iconName;
};

//--------------------------------

class PacketListOfUserPurchasesRequest : public BasePacket
{
public:
   PacketListOfUserPurchasesRequest(): BasePacket( PacketType_Login, PacketLogin::LoginType_RequestListOfPurchases ) {}
   
   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   UuidString  userUuid;
   int      platformId;
};


//--------------------------------

class PacketAddPurchaseEntry : public BasePacket
{
public:
   PacketAddPurchaseEntry(): BasePacket( PacketType_Login, PacketLogin::LoginType_AddPurchaseEntry ), quantity( 1 ) {}
   
   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   // the beneficiary's data
   UuidString     userUuid; 
   string         userEmail;
   string         userName;

   
   UuidString     productUuid;
   string         adminNotes;
   int            platformId; // for which platforms is this item. 0=all. -1=none
   int            quantity;
};

//--------------------------------

class PacketListOfUserAggregatePurchases : public BasePacket
{
public:
   PacketListOfUserAggregatePurchases(): BasePacket( PacketType_Login, PacketLogin::LoginType_ListOfAggregatePurchases ), platformId( 0 ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;
   
   int   platformId;
   SerializedVector< PurchaseEntry > purchases;
};

//--------------------------------

class PacketListOfUserProductsS2S : public BasePacket
{
public:
   PacketListOfUserProductsS2S(): BasePacket( PacketType_Login, PacketLogin::LoginType_ListOfProductsS2S ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;
   
   UuidString     uuid;
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
   U32      languageId;
};

///////////////////////////////////////////////////////////////

class PacketPrepareForUserLogout : public BasePacket
{
public:
   PacketPrepareForUserLogout() : BasePacket( PacketType_Login, PacketLogin::LoginType_PrepareForUserLogout ), connectionId( 0 ), wasDisconnectedByError( false ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   UuidString  uuid;
   U32   connectionId;
   bool  wasDisconnectedByError;
};

///////////////////////////////////////////////////////////////

class PacketLoginThrottlePackets : public PacketLogin
{
public:
   PacketLoginThrottlePackets() : PacketLogin( PacketType_Login, PacketLogin::LoginType_ThrottleUsersConnection ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   U32      connectionId;
   U32      delayBetweenPacketsMs;
};

///////////////////////////////////////////////////////////////

class PacketLoginDebugThrottleUserPackets : public PacketLogin
{
public:
   PacketLoginDebugThrottleUserPackets() : PacketLogin( PacketType_Login, PacketLogin::LoginType_DebugThrottleUsersConnection ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   U8       level;
};

///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////

class PacketRequestUserProfile : public BasePacket
{
public:
   PacketRequestUserProfile() : BasePacket( PacketType_Login, PacketLogin::LoginType_RequestUserProfile ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   UuidString  uuid;// you can use any of these fields or all.
   string   userEmail;
   string   userName;
};

///////////////////////////////////////////////////////////////

class PacketRequestUserProfileResponse : public BasePacket // error packet for no admin priv
{
public:
   PacketRequestUserProfileResponse() : BasePacket( PacketType_Login, PacketLogin::LoginType_RequestUserProfileResponse ), 
                              adminLevel( 0 ), 
                              iconId( 1 ), 
                              languageId( 1 ),
                              isActive( true ),  
                              showWinLossRecord( false ),
                              marketingOptOut( false ), 
                              showGenderProfile( false ),
                              displayOnlineStatusToOtherUsers( false ),
                              blockContactInvitations( false ),
                              blockGroupInvitations( false )

   {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   userName;
   string   passwordHash;
   string   email;
   UuidString  userUuid;
   string   lastLoginTime;
   string   loggedOutTime;
   string   motto;
   int      adminLevel;
   int      iconId;
   int      languageId;

   bool     isActive;
   bool     showWinLossRecord;
   bool     marketingOptOut;
   bool     showGenderProfile;

   bool     displayOnlineStatusToOtherUsers;
   bool     blockContactInvitations;
   bool     blockGroupInvitations;

   int      wins;
   int      losses;
   int      ties;

   //SerializedKeyValueVector< string > profileKeyValues;
};

///////////////////////////////////////////////////////////////

class PacketUpdateUserProfile : public BasePacket // limited to admin priveledges only
{
public:
   PacketUpdateUserProfile() : BasePacket( PacketType_Login, PacketLogin::LoginType_UpdateUserProfile ), 
      isActive( false ){}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   userName;
   string   passwordHash;
   string   email;
   UuidString  userUuid;
   int      adminLevel;
   int      languageId;

   bool     isActive;
   
};

///////////////////////////////////////////////////////////////

class PacketUpdateUserProfileResponse : public BasePacket // limited to admin priveledges only
{
public:
   PacketUpdateUserProfileResponse() : BasePacket( PacketType_Login, PacketLogin::LoginType_UpdateUserProfileResponse ), success( false ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   UuidString  uuid;// you can use any of these fields or all.
   bool     success;
};

///////////////////////////////////////////////////////////////

class PacketRequestListOfProducts : public BasePacket
{
public:
   PacketRequestListOfProducts(): BasePacket( PacketType_Login, PacketLogin::LoginType_RequestListOfProducts ), platformId( 0 ) {}
   
   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   int   platformId;
};

///////////////////////////////////////////////////////////////

class PacketRequestListOfProductsResponse : public BasePacket
{
public:
   PacketRequestListOfProductsResponse(): BasePacket( PacketType_Login, PacketLogin::LoginType_RequestListOfProductsResponse ), platformId( 0 ) {}
   
   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   int   platformId;
   SerializedVector< ProductBriefPacketed > products;
};

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

class PacketRequestOtherUserProfile : public BasePacket
{
public:
   PacketRequestOtherUserProfile() : BasePacket( PacketType_Login, PacketLogin::LoginType_RequestOtherUserProfile ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   userName;
};

///////////////////////////////////////////////////////////////

class PacketRequestOtherUserProfileResponse : public BasePacket // error packet for no admin priv
{
public:
   PacketRequestOtherUserProfileResponse() : BasePacket( PacketType_Login, PacketLogin::LoginType_RequestOtherUserProfileResponse ){}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   SerializedKeyValueVector< string > basicProfile;
   SerializedKeyValueVector< int > productsOwned; // only games and expansions
   SerializedKeyValueVector< string > awards;
};

///////////////////////////////////////////////////////////////

class    PacketUpdateSelfProfile : public BasePacket 
{
public:
   PacketUpdateSelfProfile() : BasePacket( PacketType_Login, PacketLogin::LoginType_UpdateSelfProfile ), 
                              languageId( 1 ),
                              avatarIconId( false ), 
                              showWinLossRecord( false ),
                              marketingOptOut( false ), 
                              showGenderProfile( false ),
                              displayOnlineStatusToOtherUsers( false ),
                              blockContactInvitations( false ),
                              blockGroupInvitations( false )
   {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   userName;
   string   passwordHash;
   string   email;
   string   motto;
   int      languageId;
   int      avatarIconId;

   bool     showWinLossRecord;
   bool     marketingOptOut;
   bool     showGenderProfile;
   bool     displayOnlineStatusToOtherUsers;
   bool     blockContactInvitations;
   bool     blockGroupInvitations;
};

///////////////////////////////////////////////////////////////

class    PacketUserUpdateProfile : public PacketRequestUserProfileResponse 
{
public:
   PacketUserUpdateProfile() : PacketRequestUserProfileResponse(),
                              connectionId( 0 )
   {
      packetSubType = PacketLogin::LoginType_UserUpdateProfile;
   }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;
   U32      connectionId;
};

///////////////////////////////////////////////////////////////

class PacketUpdateSelfProfileResponse : public BasePacket // limited to admin priveledges only
{
public:
   PacketUpdateSelfProfileResponse() : BasePacket( PacketType_Login, PacketLogin::LoginType_UpdateSelfProfileResponse ), 
                              avatarIconId( 1 ),
                              success( false ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   int      avatarIconId;
   bool     success;
};

///////////////////////////////////////////////////////////////

