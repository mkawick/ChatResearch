#include "UserAccountAssetDelivery.h"
#include "DiplodocusAsset.h"
#include "AssetOrganizer.h"

#include "../NetworkCommon/Packets/AssetPacket.h"
#include "../NetworkCommon/Packets/GamePacket.h"
#include "../NetworkCommon/Packets/PacketFactory.h"

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

void  Copy( const AssetDefinition* asset, AssetInfo& assetInfo )
{
   //assetInfo.productId  = gameProductId;
   assetInfo.assetHash  = asset->hash;
   if( asset->name.size() > 0 )
   {
      assetInfo.assetName  = asset->name;
   }
   else
   {
      assetInfo.assetName  = asset->path;
   }
   assetInfo.version    = asset->version;
   assetInfo.beginDate  = asset->beginTime;
   assetInfo.endDate    = asset->endTime;
   assetInfo.isOptional = asset->isOptional;
}

UserAccountAssetDelivery::UserAccountAssetDelivery( const UserTicket& ticket ) : m_userTicket( ticket ), m_status( Status_initial_login ), m_readyForCleanup( false ), m_assetManager( NULL )
{
}

   //---------------------------------------

UserAccountAssetDelivery::~UserAccountAssetDelivery()
{
}

   //---------------------------------------

void  UserAccountAssetDelivery::Update()
{
   if( m_assetManager == NULL )
      return;
}

   //---------------------------------------

void     UserAccountAssetDelivery::UserLoggedOut()
{
   m_status = Status_awaiting_cleanup;
   m_readyForCleanup = true;
   time( &m_logoutTime );
}

   //---------------------------------------

bool     UserAccountAssetDelivery::LogoutExpired()
{
   if( m_readyForCleanup == false )
      return false;

   time_t currentTime;
   time( &currentTime );
   if( difftime( currentTime, m_logoutTime ) >= 3 ) // 3 seconds
   {
      return true;
   }
   return false;
}

   //---------------------------------------

void     UserAccountAssetDelivery::SetupProductFilterNames( const StringBucket& bucket )
{
   m_productFilterNames.clear();

   list< string >::const_iterator it = bucket.bucket.begin();
   while( it != bucket.bucket.end() )
   {
      m_productFilterNames.insert( *it++ );
   }
}

   //---------------------------------------

bool     UserAccountAssetDelivery::LoginKeyMatches( const string& loginKey ) const
{
   if( loginKey == m_userTicket.userTicket )
      return true;

   return false;
}

//------------------------------------------------------------------------------------------------

bool     UserAccountAssetDelivery::HandleRequestFromClient( const PacketAsset* packet )
{
   switch( packet->packetSubType )
   {
   case PacketAsset::AssetType_GetListOfStaticAssets:
      return GetListOfStaticAssets( static_cast< const PacketAsset_GetListOfStaticAssets* >( packet ) );

   case PacketAsset::AssetType_GetListOfDynamicAssets:
      return GetListOfDynamicAssets( static_cast< const PacketAsset_GetListOfDynamicAssets* >( packet ) );

   case PacketAsset::AssetType_RequestAsset:
      return GetAsset( static_cast< const PacketAsset_RequestAsset* >( packet ) );
   }

   return false;
}


//------------------------------------------------------------------------------------------------

bool     UserAccountAssetDelivery::GetListOfStaticAssets( const PacketAsset_GetListOfStaticAssets* packet )
{
   if( m_assetManager == NULL )
      assert( 0 );

   U32 connectionId = m_userTicket.connectionId;

   //PacketAsset_GetListOfStaticAssets
   U8 gameProductId = packet->gameProductId;
   vector< string > assetIds;
   const AssetOrganizer* assetOrganizer = m_assetManager->GetStaticAssetOrganizer();
   assetOrganizer->GetListOfAssets( gameProductId, packet->platformId, assetIds );

   //-----------------------------

   PacketAsset_GetListOfStaticAssetsResponse*    response = new PacketAsset_GetListOfStaticAssetsResponse;
   vector< string >::iterator it = assetIds.begin();
   while( it != assetIds.end() )
   {
      const AssetDefinition * asset;
      bool  found = assetOrganizer->FindByHash( *it++ , asset );
      if( found )
      {
         AssetInfo assetInfo;
         Copy( asset, assetInfo ); // operator = is not an option in this version of VS

         assetInfo.productId  = gameProductId;

         response->updatedAssets.insert( assetInfo.assetHash, assetInfo );
      }
   }

   PacketGatewayWrapper* wrapper = new PacketGatewayWrapper;
   wrapper->SetupPacket( response, connectionId );
   
   m_assetManager->AddOutputChainData( wrapper, connectionId );
   return true;
}

//------------------------------------------------------------------------------------------------

bool     UserAccountAssetDelivery::GetListOfDynamicAssets( const PacketAsset_GetListOfDynamicAssets* packet )
{
   U32 connectionId = m_userTicket.connectionId;
   //m_userTicket.

   U8 gameProductId = packet->gameProductId;
   vector< string > assetIds;
   const AssetOrganizer* assetOrganizer = m_assetManager->GetDynamicAssetOrganizer();
   assetOrganizer->GetListOfAssets( packet->platformId, m_productFilterNames, assetIds );
   // 1) reduce the set of products by device type.

   //-----------------------------
   PacketAsset_GetListOfDynamicAssetsResponse*    response = new PacketAsset_GetListOfDynamicAssetsResponse;
   vector< string >::iterator it = assetIds.begin();
   while( it != assetIds.end() )
   {
      const AssetDefinition * asset;
      bool  found = assetOrganizer->FindByHash( *it++ , asset );
      if( found )
      {
         AssetInfo assetInfo;
         Copy( asset, assetInfo ); // operator = is not an option in this version of VS

         assetInfo.productId  = gameProductId;

         response->updatedAssets.insert( assetInfo.assetHash, assetInfo );
      }
   }

   PacketGatewayWrapper* wrapper = new PacketGatewayWrapper;
   wrapper->SetupPacket( response, connectionId );
   
   m_assetManager->AddOutputChainData( wrapper, connectionId );

   /// product filter
   return true;
}

//------------------------------------------------------------------------------------------------

bool     UserAccountAssetDelivery::GetAsset( const PacketAsset_RequestAsset* packet )
{
   if( m_assetManager == NULL )
      assert( 0 );

   if( m_userTicket.connectionId != 0 )
   {
      U8* data = NULL;
      int size = 0;

      U32 connectionId = m_userTicket.connectionId;
      const AssetDefinition* asset;
      bool found = m_assetManager->GetStaticAssetOrganizer()->FindByHash( packet->assetHash, asset );
      if( found == false )
      {
         found = m_assetManager->GetDynamicAssetOrganizer()->FindByHash( packet->assetHash, asset );
      }
      if( found )
      {
         data = asset->fileData;
         size = asset->fileSize;

         const int MaxSize = PacketGameplayRawData::MaxBufferSize  - sizeof( PacketGatewayWrapper );
         cout << "File being sent = " << asset->name << endl;
         cout << "   path = " << asset->path << endl;
         cout << "   size = " << asset->fileSize << endl;

         return SendRawData< PacketGameplayRawData, DiplodocusAsset > 
            ( data, size, PacketGameplayRawData::Asset, MaxSize, m_assetManager->GetServerId(), asset->productId, asset->hash, connectionId, m_assetManager );
      }
      else
      {
         cout << " Attempt to load unknown file: " << packet->assetHash << endl;
         m_assetManager->SendErrorToClient( m_userTicket.connectionId, PacketErrorReport::ErrorType_Asset_UnknownAsset );
      }
   }
   else
   {
      cout << " Attempt to load file but user is not connected: " << packet->assetHash << endl;
      //m_assetManager->SendErrorToClient( m_userTicket.connectionId, ErrorType_Asset_UserDisconnected );
   }

   return false;
}

//------------------------------------------------------------------------------------------------
