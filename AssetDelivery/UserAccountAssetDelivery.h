#pragma once

#include "AssetCommon.h"
#include <time.h>

class DiplodocusAsset;
class PacketAsset;
class PacketAsset_GetListOfStaticAssets;
class PacketAsset_GetListOfDynamicAssets;
class PacketAsset_RequestAsset;

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
   UserAccountAssetDelivery( const UserTicket& ticket );
   ~UserAccountAssetDelivery();

   const UserTicket& GetUserTicket() const { return m_userTicket; }
   void              SetConnectionId( U32 connId ) { m_userTicket.connectionId = connId; }

   void              SetServer( DiplodocusAsset* assetManager ) { m_assetManager = assetManager; }
   bool              HandleRequestFromClient( const PacketAsset* packet );

   void              UserLoggedOut();
   void              Update();

private:

   bool              GetListOfStaticAssets( const PacketAsset_GetListOfStaticAssets* packet );
   bool              GetListOfDynamicAssets( const PacketAsset_GetListOfDynamicAssets* packet );
   bool              GetAsset( const PacketAsset_RequestAsset* packet );

   UserTicket        m_userTicket;
   Status            m_status;
   bool              m_readyForCleanup;
   DiplodocusAsset*  m_assetManager;
   time_t            m_logoutTime;
};

///////////////////////////////////////////////////////////////////
