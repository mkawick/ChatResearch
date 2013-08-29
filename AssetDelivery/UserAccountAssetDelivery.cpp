#include "UserAccountAssetDelivery.h"
#include "DiplodocusAsset.h"
#include "AssetOrganizer.h"

#include "../NetworkCommon/Packets/AssetPacket.h"
#include "../NetworkCommon/Packets/GamePacket.h"
#include "../NetworkCommon/Packets/PacketFactory.h"

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

UserAccountAssetDelivery::UserAccountAssetDelivery( const UserTicket& ticket ) : m_userTicket( ticket ), m_status( Status_initial_login ), m_readyForCleanup( false ), m_assetManager( NULL )
{
}

UserAccountAssetDelivery::~UserAccountAssetDelivery()
{
}

void  UserAccountAssetDelivery::Update()
{
   if( m_assetManager == NULL )
      return;
}

void     UserAccountAssetDelivery::UserLoggedOut()
{
   m_status = Status_awaiting_cleanup;
   m_readyForCleanup = true;
   time( &m_logoutTime );
}

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
   return false;
}

//------------------------------------------------------------------------------------------------

bool     UserAccountAssetDelivery::GetListOfDynamicAssets( const PacketAsset_GetListOfDynamicAssets* packet )
{
   return false;
}

//------------------------------------------------------------------------------------------------

bool     UserAccountAssetDelivery::GetAsset( const PacketAsset_RequestAsset* packet )
{
   if( m_userTicket.connectionId != 0 )
   {
      U32 connectionId = m_userTicket.connectionId;
      const AssetDefinition* asset;
      bool found = m_assetManager->GetAssetOrganizer()->FindByHash( packet->asset.assetHash, asset );
      if( found )
      {
         U8* data = asset->fileData;
         int size = asset->fileSize;

         const int MaxSize = PacketGameplayRawData::MaxBufferSize  - sizeof( PacketGatewayWrapper );

         return SendRawData< PacketGameplayRawData, DiplodocusAsset > 
            ( data, size, PacketGameplayRawData::Asset, MaxSize, m_assetManager->GetServerId(), asset->productId, connectionId, m_assetManager );
      }
   }

   return false;
}

//------------------------------------------------------------------------------------------------
