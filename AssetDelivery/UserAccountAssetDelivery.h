#pragma once

#include "AssetCommon.h"
#include <time.h>
#include <set>
using namespace std;

class AssetMainThread;
class PacketAsset;
/*class PacketAsset_GetListOfStaticAssets;
class PacketAsset_GetListOfDynamicAssets;*/
class PacketAsset_GetListOfAssets;
class PacketAsset_GetListOfAssetCategories;
class PacketAsset_RequestAsset;
class StringBucket;

///////////////////////////////////////////////////////////////////

class UserAccountAssetDelivery
{
   enum Status
   {
      Status_initial_login,
      Status_query_assets_for_product,
      Status_awating_user_login,
      Status_assemble_assets_for_delivery,
      Status_send_files_to_client,
      Status_awaiting_cleanup
   };

public:
   
   //---------------------------------------

   UserAccountAssetDelivery();// only for generic users
   UserAccountAssetDelivery( const UserTicket& ticket );
   ~UserAccountAssetDelivery();

   //---------------------------------------

   bool              LoginKeyMatches( const string& loginKey ) const;
   const UserTicket& GetUserTicket() const { return m_userTicket; }
   void              SetConnectionId( U32 connId ) { m_userTicket.connectionId = connId; }
   void              SetGatewayId( U32 gatewayId ) { m_userTicket.gatewayId = gatewayId; }

   void              SetServer( AssetMainThread* assetManager ) { m_assetManager = assetManager; }
   bool              HandleRequestFromClient( const PacketAsset* packet );

   void              UserLoggedOut();
   void              ClearLoggedOutStatus();
   bool              LogoutExpired();
   void              Update();

   void              SetupProductFilterNames( const StringBucket& bucket );

   void              SetMaxNumerOfAssetsReturnedPerCategory( int num ) { m_maxNumAssetReturnedByCategory = num; }

   //---------------------------------------

private:

   bool              GetListOfAssetCategories( const PacketAsset_GetListOfAssetCategories* packet );
   bool              GetListOfAssets( const PacketAsset_GetListOfAssets* packet );
   bool              EchoHandler();
   bool              GetAsset( const PacketAsset_RequestAsset* packet );

   UserTicket        m_userTicket;
   Status            m_status;
   bool              m_readyForCleanup;
   AssetMainThread*  m_assetManager;
   time_t            m_logoutTime;
   set< string >     m_productFilterNames;

   int               m_maxNumAssetReturnedByCategory;
};

///////////////////////////////////////////////////////////////////
