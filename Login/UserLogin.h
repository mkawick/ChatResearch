// UserLogin.h
#pragma once

#include "LoginCommon.h"
#include "../NetworkCommon/ServerConstants.h"

#include <string>
#include <map>
#include <vector>
using namespace std;
#include "ProductInfo.h"
#include "../NetworkCommon/Packets/LoginPacket.h"

class LoginMainThread;

class BasePacket;
class PacketDbQuery;
class PacketDbQueryResult;
class PacketLoginFromGateway;
class PacketListOfUserAggregatePurchases;
class PacketRequestUserProfile;
class PacketListOfUserPurchasesRequest;
class PacketUpdateSelfProfile;

//////////////////////////////////////////////////////////////////////////

struct UserCustomData
{
   //U32         queryType;
   stringhash  hashLookup;
   U8          gameProductId;
};

//////////////////////////////////////////////////////////////////////////

class UserLogin
{
public:
   UserLogin( const string& name, const string& pword );

   void     SetManager( LoginMainThread* mainThread )  { m_loginMainThread = mainThread; }

   void           SetHashLookup( stringhash h )                   { m_hashLookup = h; }
   void           SetExternalLookups( U32 id, U32 requestorId )   { m_lookupId = id, m_requestorConnectionId = requestorId;}
   stringhash     GetHashLookup()      const { return m_hashLookup; }
   int            GetConnectionId( int gatewayId ) const;
   const string&  GetId()              const { return m_userId; }
   const string&  GetUsername()        const { return m_userName; }
   
   const string&  GetUuid()            const { return m_userUuid; }
   const string&  GetPassword()        const { return m_passwordHash; }
   const string&  GetEmail()           const { return m_email; }
   const string&  GetAssetServerKey()  const { return m_assetServerKey; }
   const string&  GetLastLoginTime()   const { return m_lastLoginTime; }
   const string&  GetLastLogoutTime()  const { return m_lastLogoutTime; }
   const string&  GetuserMotto()       const { return m_userMotto; }
   int            GetUserAvatar()      const { return m_avatarIcon; }

   U32            GetLanguageId()      const { return m_languageId; }
   bool           GetIsValid()         const { return m_isValid; }
   bool           GetIsActive()        const { return m_isActive; }

   U8             GetProductId( U32 userConnectionId ) const;
   U32            GetGatewayId( U32 userConnectionId ) const;
   U32            GetConnectionId( U32 gatewayId ) const;
   bool           GetListOfConnectedIds( vector< LoginConnectionDetails >& listOfActiveConnections ) const;
   bool           GetConnectionInfo( U8 gameProductId, U32& connectionId, U32& gatewayId );// returns false if not connected

   time_t         GetEarliestLoginTime()  const;
   time_t         GetLatestLogoutTime()  const;
   bool           IsLoggedIntoAProduct()  const;

   bool           RequiresStateSendToAllServers() const;
   bool           SendStatusToGateway( U32 userConnectionId, bool success );
   void           SendStateToAllServers( U32 userConnectionId, U32 gatewayId, U8 gameProductId, bool isLoggedIn, bool disconnectedByError );
   void           PackUserProfile_SendToClient( U32 connectionId, U32 gatewayId );
   void           LoadUserAccount_SendingProfileToOtherUser( const string& userName, U32 id );

   const vector< string >& GetProductVendorUuids() const { return productVendorUuids; }

   bool     HandlePacket( const BasePacket* packet, U32 connectionId, U32 gatewayId, U8 gameProductId );
   bool     HandleQueryResult( const PacketDbQueryResult* query );
   bool     Disconnect( U32 connectionId );
   bool     LoginFinishedSuccessfully( U32 userConnectionId );
   void     RequestListOfPurchases( U32 connectionId ) const;

private:
   enum QueryType
   {
      QueryType_Login,
      QueryType_Logout,
      QueryType_LoadUserAccount_SendingProfileToOtherUser,
      QueryType_RequestUserProductsOwned,
      QueryType_RequestOtherUserProfile,
      QueryType_UserListOfUserProducts,
      QueryType_GetProductListForUser,
      QueryType_AddProductInfoToUser,
      QueryType_UpdateUserProfile,
      QueryType_UserListOfGame,
      QueryType_UpdateUsers,
      QueryType_UpdateLastLoggedOutTime
   };
   enum 
   {
      ProductNotFound = -1
   };

private:
   void     LogUserOutOfExistingConnection( U8 gameProductId );
   void     AddConnectionId( U32 connectionId, U32 gatewayId, U8 gameProductId );

   bool     LogUserIn( const PacketLoginFromGateway* loginPacket, U32 connectionId );
   bool     Logout( U32 connectionId, bool wasDisconnectedByError );
   bool     StoreUserPurchases( const PacketListOfUserAggregatePurchases* deviceReportedPurchases, U32 connectionId );
   bool     UpdateLastLoggedOutTime();

   //bool     FinalizeLogout();
   bool     CanContinueLoggingIn() const { return m_isActive; }

   bool     SetupQueryForLogin( U32 connectionId );
   bool     HandleUserLoginResult( U32 connectionId, const PacketDbQueryResult* dbResult );
   bool     HandleLoadUserAccount_SendingProfileToOtherUser( const PacketDbQueryResult* dbResult );
   bool     SendLoginResultToGateway( U32 connectionId );
   bool     LoginResult( const PacketDbQueryResult* dbResult );
   void     WriteUserProfile();
   bool     RequestProfile( const PacketRequestUserProfile* profileRequest );
   void     PackUserProfileRequestAndSendToClient( U32 connectionId, U32 gatewayId );

   int      FindConnectionDetails( U32 connectionId ) const;
   
   bool     HandleRequestForListOfPurchases( const PacketListOfUserPurchasesRequest* purchase, U32 connectionId );
   bool     HandleRequestListOfProducts( const PacketRequestListOfProducts* purchaseRequest, U32 connectionId );
   void     SendPurchasesToClient( const string& userUuid, U32 connectionID );

   void     RequestListOfProductsFromClient( U32 connectionId );
   void     StoreListOfUsersProductsFromDB( U32 connectionId, const PacketDbQueryResult* dbResult );
   void     SendListOfProductsToClientAndAsset( U32 connectionId, U32 gatewayId );
   void     ClearAllProductsOwned();
   void     SendListOfOwnedProductsToClient( U32 connectionId, U32 gatewayId );
   bool     AddToProductsOwned( int productDbId, const string& lookupName, const string& productUuid, float quantity, const string& vendorUuid  );
   void     AddConversionProductsToUserPurchases( const ProductInfo& productInfo );
   void     AddCurrentlyLoggedInProductToUserPurchases( U8 gameProductId );
   int      FindProductVendorUuid( string text ) const;
   void     AddProductVendorUuid( string text );

   void     AddItemToProductTable( const PurchaseEntryExtended& purchaseEntry );
   bool     StoreOffProductInUserRecord ( int userManagerIndex, 
                                                         const string& productUuid, 
                                                         float numPurchased);
   //void     WriteProductToUserRecord( const string& userUuid, const string& productUuid, double pricePaid, float numPurchased, string adminId, string adminNotes );

   void     WriteProductToUserRecord( const string& productVendorUuid, double pricePaid, U32 userConnectionId );
   void     WriteProductToUserRecord( const string& userUuid, const string& productUuid, double pricePaid, float numPurchased,  U32 userConnectionId );

   bool     EchoHandler( U32 connectionId );
   bool     UpdateProfile( U32 connectionId, const PacketUpdateSelfProfile* updateProfileRequest );
   void     WriteUserBasicsToAccount( U32 connectionId );
   void     WriteUserProfile( U32 connectionId );
   void     TellContactServerToReloadUserProfile( U32 connectionId );

   template < typename type >
   void  PackUserSettings( type* response ) const;

private:
   stringhash              m_hashLookup;
   string                  m_userId;
   string                  m_userName;
   string                  m_userUuid;
   string                  m_passwordHash;
   string                  m_email;
   string                  m_assetServerKey;
   string                  m_lastLoginTime;
   string                  m_lastLogoutTime;
   string                  m_userMotto;
   int                     m_avatarIcon;
   int                     m_loginAttemptCount;

   int                     m_timeZone;
   int                     m_languageId;
   bool                    m_isValid;
   bool                    m_isActive;
   bool                    m_showWinLossRecord;
   bool                    m_marketingOptOut;
   bool                    m_showGenderProfile;
   bool                    m_displayOnlineStatusToOtherUsers;
   bool                    m_blockContactInvitations;
   bool                    m_blockGroupInvitations;

   bool                    m_hasRequestedPurchasesFromClient;
   bool                    m_requiresStateSendToAllServers;

   vector< string >        productVendorUuids;
   vector< ProductInfo >   productsWaitingForInsertionToDb;
   map< U32, ProductBrief >  m_productsOwned;

   int                     m_lookupId;
   int                     m_requestorConnectionId;

private:
   LoginConnectionDetails  m_connectionDetails[ GameProductId_NUM_GAMES ];
   LoginMainThread*        m_loginMainThread;
};

//////////////////////////////////////////////////////////////////////////