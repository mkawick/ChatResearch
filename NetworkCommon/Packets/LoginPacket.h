// LoginPacket.h

#pragma once

#include "BasePacket.h"

#pragma pack( push, 4 )

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
      LoginType_UserUpdateProfile,
      LoginType_UserListOfPurchasesWasUpdated,
      LoginType_LoginFromGateway
   };
public:
   PacketLogin( int packet_type = PacketType_Login, int packet_sub_type = LoginType_Login ): BasePacket( packet_type, packet_sub_type ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   UuidString        uuid;
   BoundedString80   userName;
   
   BoundedString80   loginKey;
   BoundedString32   password;
   FixedStringTiny   languageCode;
};

///////////////////////////////////////////////////////////////

class PacketLoginFromGateway : public PacketLogin
{
public:

public:
   PacketLoginFromGateway( int packet_type = PacketType_Login, int packet_sub_type = LoginType_LoginFromGateway ): PacketLogin( packet_type, packet_sub_type ), gatewayId( 0 ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   void  copy( const PacketLogin& login );

   U32               gatewayId;
};

///////////////////////////////////////////////////////////////

class PacketLogout : public BasePacket
{
public:
   PacketLogout(): BasePacket( PacketType_Login, PacketLogin::LoginType_Logout ), wasDisconnectedByError( false ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;
   
   bool  wasDisconnectedByError;
};

///////////////////////////////////////////////////////////////

class PacketLogoutToClient : public PacketLogin
{
public:
   PacketLogoutToClient(): PacketLogin( PacketType_Login, PacketLogin::LoginType_PacketLogoutToClient ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion ) { return PacketLogin::SerializeIn( data, bufferOffset, minorVersion ); }
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const { return PacketLogin::SerializeOut( data, bufferOffset, minorVersion ); }
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

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   TimeString  lastLogoutTime;
   bool        wasLoginSuccessful;
   U32         connectionId;  // used for a user ID.


   //U32      junk1;
   //string   junk2;
};


///////////////////////////////////////////////////////////////

class PacketLoginToGateway : public PacketLogin
{
public:
   PacketLoginToGateway(): PacketLogin( PacketType_Login, PacketLogin::LoginType_InformGatewayOfLoginStatus ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   TimeString  lastLogoutTime;
   bool        wasLoginSuccessful;
   U8          adminLevel;
   U8          languageId;
};

///////////////////////////////////////////////////////////////

class PacketCreateAccount : public BasePacket
{
public:
   PacketCreateAccount(): BasePacket( PacketType_Login, PacketLogin::LoginType_CreateAccount ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;
   
   BoundedString80   userName;
   BoundedString80   userEmail;
   BoundedString32   password;
   BoundedString80   deviceId;
   BoundedString80   deviceAccountId;
   U8                languageId;
};

///////////////////////////////////////////////////////////////

class PacketCreateAccountResponse : public BasePacket
{
public:
   PacketCreateAccountResponse(): BasePacket( PacketType_Login, PacketLogin::LoginType_CreateAccountResponse ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;
   
   BoundedString80   userName;
   BoundedString80   userEmail;
   bool              wasSuccessful;
};

///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////

struct PurchaseEntry
{
   PurchaseEntry() : quantity( 0 ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   //string   name;
   float       quantity;
   UuidString  productUuid;
};


struct PurchaseEntryExtended : public PurchaseEntry
{
   PurchaseEntryExtended() : PurchaseEntry() {}

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   BoundedString80   name;

};

struct ProductBriefPacketed
{
   ProductBriefPacketed() : productType( GameProductType_Game )  {}

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   BoundedString80      localizedName;
   UuidString           uuid;
   FixedString80        vendorUuid; // e.g. summonerwars.mv.bund.pt2
   UuidString           parentUuid;
   int                  productType;
   BoundedString32      iconName;
   //U32      junk1;
   //string   junk2;
};

//--------------------------------

class PacketListOfUserPurchasesRequest : public BasePacket
{
public:
   PacketListOfUserPurchasesRequest(): BasePacket( PacketType_Login, PacketLogin::LoginType_RequestListOfPurchases ) {}
   
   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   UuidString  userUuid;
   int         platformId;
};


//--------------------------------

class PacketAddPurchaseEntry : public BasePacket
{
public:
   PacketAddPurchaseEntry(): BasePacket( PacketType_Login, PacketLogin::LoginType_AddPurchaseEntry ), quantity( 1 ) {}
   
   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   // the beneficiary's data
   UuidString        userUuid; 
   BoundedString80   userEmail;
   BoundedString80   userName;

   
   UuidString        productUuid;
   BoundedString140  adminNotes;
   int               platformId; // for which platforms is this item. 0=all. -1=none
   int               quantity;
};

//--------------------------------

class PacketListOfUserAggregatePurchases : public BasePacket
{
public:
   PacketListOfUserAggregatePurchases(): BasePacket( PacketType_Login, PacketLogin::LoginType_ListOfAggregatePurchases ), platformId( 0 ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;
   
   int               platformId;
   UuidString        userUuid;
   SerializedVector< PurchaseEntryExtended > purchases;
};

//--------------------------------

class PacketListOfUserProductsS2S : public BasePacket
{
public:
   PacketListOfUserProductsS2S(): BasePacket( PacketType_Login, PacketLogin::LoginType_ListOfProductsS2S ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;
   
   UuidString     uuid;
   U32            connectionId;
   StringBucket   products;
};


///////////////////////////////////////////////////////////////

class PacketPrepareForUserLogin : public PacketLogin
{
public:
   PacketPrepareForUserLogin() : PacketLogin( PacketType_Login, PacketLogin::LoginType_PrepareForUserLogin ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   TimeString        lastLoginTime;
   U32               connectionId;
   BoundedString80   email;
   bool              active;
   U32               userId;
   U32               languageId;
   U32               gatewayId;
};

///////////////////////////////////////////////////////////////

class PacketPrepareForUserLogout : public BasePacket
{
public:
   PacketPrepareForUserLogout() : BasePacket( PacketType_Login, PacketLogin::LoginType_PrepareForUserLogout ), connectionId( 0 ), wasDisconnectedByError( false ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   UuidString        uuid;
   U32               connectionId;
   bool              wasDisconnectedByError;
};

///////////////////////////////////////////////////////////////

class PacketLoginThrottlePackets : public PacketLogin
{
public:
   PacketLoginThrottlePackets() : PacketLogin( PacketType_Login, PacketLogin::LoginType_ThrottleUsersConnection ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   U32      connectionId;
   U32      delayBetweenPacketsMs;
};

///////////////////////////////////////////////////////////////

class PacketLoginDebugThrottleUserPackets : public PacketLogin
{
public:
   PacketLoginDebugThrottleUserPackets() : PacketLogin( PacketType_Login, PacketLogin::LoginType_DebugThrottleUsersConnection ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   U8       level;
};

///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////

class PacketRequestUserProfile : public BasePacket
{
public:
   PacketRequestUserProfile() : BasePacket( PacketType_Login, PacketLogin::LoginType_RequestUserProfile ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   UuidString        uuid;// you can use any of these fields or all.
   BoundedString80   userEmail;
   BoundedString80   userName;
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

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   BoundedString80   userName;
   BoundedString32   passwordHash;
   BoundedString80   email;
   UuidString        userUuid;
   TimeString        lastLoginTime;
   TimeString        loggedOutTime;
   BoundedString140  motto;
   int               adminLevel;
   int               iconId;
   int               languageId;

   bool              isActive;
   bool              showWinLossRecord;
   bool              marketingOptOut;
   bool              showGenderProfile;

   bool              displayOnlineStatusToOtherUsers;
   bool              blockContactInvitations;
   bool              blockGroupInvitations;

   int               wins;
   int               losses;
   int               ties;

   //SerializedKeyValueVector< string > profileKeyValues;
};

///////////////////////////////////////////////////////////////

class PacketUpdateUserProfile : public BasePacket // limited to admin priveledges only
{
public:
   PacketUpdateUserProfile() : BasePacket( PacketType_Login, PacketLogin::LoginType_UpdateUserProfile ), 
      isActive( false ){}

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   BoundedString80   userName;
   BoundedString32   passwordHash;
   BoundedString80   email;
   UuidString        userUuid;
   int               adminLevel;
   int               languageId;

   bool              isActive;
   
};

///////////////////////////////////////////////////////////////

class PacketUpdateUserProfileResponse : public BasePacket // limited to admin priveledges only
{
public:
   PacketUpdateUserProfileResponse() : BasePacket( PacketType_Login, PacketLogin::LoginType_UpdateUserProfileResponse ), success( false ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   UuidString        uuid;// you can use any of these fields or all.
   bool              success;
};

///////////////////////////////////////////////////////////////

class PacketRequestListOfProducts : public BasePacket
{
public:
   PacketRequestListOfProducts(): BasePacket( PacketType_Login, PacketLogin::LoginType_RequestListOfProducts ), platformId( 0 ) {}
   
   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   int               platformId;
};

///////////////////////////////////////////////////////////////

class PacketRequestListOfProductsResponse : public BasePacket
{
public:
   PacketRequestListOfProductsResponse(): BasePacket( PacketType_Login, PacketLogin::LoginType_RequestListOfProductsResponse ), platformId( 0 ) {}
   
   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   int   platformId;
   SerializedVector< ProductBriefPacketed > products;
};

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

class PacketRequestOtherUserProfile : public BasePacket
{
public:
   PacketRequestOtherUserProfile() : BasePacket( PacketType_Login, PacketLogin::LoginType_RequestOtherUserProfile ), fullProfile( false ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   BoundedString80   userName;
   bool              fullProfile;
};

///////////////////////////////////////////////////////////////

class PacketRequestOtherUserProfileResponse : public BasePacket // error packet for no admin priv
{
public:
   PacketRequestOtherUserProfileResponse() : BasePacket( PacketType_Login, PacketLogin::LoginType_RequestOtherUserProfileResponse ){}

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   SerializedKeyValueVector< BoundedString32 >  basicProfile;
   SerializedKeyValueVector< int >              productsOwned; // only games and expansions
   SerializedKeyValueVector< BoundedString32 >  awards;
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

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   BoundedString80   userName;
   BoundedString32   passwordHash;
   BoundedString80   email;
   BoundedString140  motto;
   int               languageId;
   int               avatarIconId;

   bool              showWinLossRecord;
   bool              marketingOptOut;
   bool              showGenderProfile;
   bool              displayOnlineStatusToOtherUsers;
   bool              blockContactInvitations;
   bool              blockGroupInvitations;
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

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;
   U32      connectionId;
};

///////////////////////////////////////////////////////////////

class PacketUpdateSelfProfileResponse : public BasePacket // limited to admin priveledges only
{
public:
   PacketUpdateSelfProfileResponse() : BasePacket( PacketType_Login, PacketLogin::LoginType_UpdateSelfProfileResponse ), 
                              avatarIconId( 1 ),
                              success( false ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   int      avatarIconId;
   bool     success;
};

///////////////////////////////////////////////////////////////

class PacketListOfUserPurchasesUpdated : public BasePacket
{
public:
   PacketListOfUserPurchasesUpdated(): BasePacket( PacketType_Login, PacketLogin::LoginType_UserListOfPurchasesWasUpdated ) {}
   
   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   UuidString  userUuid;
   U32         userConnectionId;
};

///////////////////////////////////////////////////////////////

#pragma pack( pop )
