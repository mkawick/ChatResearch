#include "UserAccountAssetDelivery.h"

#include "../NetworkCommon/Packets/AssetPacket.h"

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
   return false;
}

//------------------------------------------------------------------------------------------------
