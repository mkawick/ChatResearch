// UserAccountAssetDelivery.cpp

#include "UserAccountAssetDelivery.h"
#include "AssetMainThread.h"
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
   assetInfo.category   = asset->category;
}

UserAccountAssetDelivery::UserAccountAssetDelivery() : m_status( Status_initial_login ), m_readyForCleanup( false ), m_assetManager( NULL ), m_maxNumAssetReturnedByCategory( 12 )
{
   m_userTicket.userId = 0xFFFFFFFF;
   m_userTicket.uuid = "generic_user";
   m_userTicket.userName = "generic_user";
   m_userTicket.connectionId = 0xFFFFFFFF;/// only after this person has logged in
   m_userTicket.userTicket = "generic_user";
   m_userTicket.gameProductId = 1;
}

UserAccountAssetDelivery::UserAccountAssetDelivery( const UserTicket& ticket ) : m_userTicket( ticket ), m_status( Status_initial_login ), m_readyForCleanup( false ), m_assetManager( NULL ), m_maxNumAssetReturnedByCategory( 250 )
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
   case PacketAsset::AssetType_GetListOfAssetCategories:
      return GetListOfAssetCategories( static_cast< const PacketAsset_GetListOfAssetCategories* >( packet ) );

   case PacketAsset::AssetType_GetListOfAssets:
      return GetListOfAssets( static_cast< const PacketAsset_GetListOfAssets* >( packet ) );

   case PacketAsset::AssetType_RequestAsset:
      return GetAsset( static_cast< const PacketAsset_RequestAsset* >( packet ) );

   case PacketAsset::AssetType_EchoToServer:
      EchoHandler();
      return true;
   }

   return false;
}


//------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------

bool     UserAccountAssetDelivery::GetListOfAssetCategories( const PacketAsset_GetListOfAssetCategories* packet )
{
   vector<string> listOfStrings;
   m_assetManager->GetListOfAssetCategories( listOfStrings );
   if( listOfStrings.size() == 0 )
   {
      m_assetManager->SendErrorToClient( m_userTicket.connectionId, PacketErrorReport::ErrorType_Asset_NoCategoriesAvailable );
      return false;
   }

   PacketAsset_GetListOfAssetCategoriesResponse*    response = new PacketAsset_GetListOfAssetCategoriesResponse;
   vector< string >::iterator it = listOfStrings.begin();
   while( it != listOfStrings.end() )
   {
      response->assetcategory.push_back( *it++ );
   }

   U32 connectionId = m_userTicket.connectionId;
   PacketGatewayWrapper* wrapper = new PacketGatewayWrapper;
   wrapper->SetupPacket( response, connectionId );
   
   m_assetManager->AddOutputChainData( wrapper, connectionId );
   return true;
}

//------------------------------------------------------------------------------------------------

bool     UserAccountAssetDelivery::GetListOfAssets( const PacketAsset_GetListOfAssets* packet )
{
   U8 gameProductId = packet->gameProductId;
   vector< string > assetIds;
   const AssetOrganizer*   assetOrganizer = m_assetManager->GetAssetOrganizer( packet->assetCategory );
   if( assetOrganizer == NULL )
   {
      m_assetManager->SendErrorToClient( m_userTicket.connectionId, PacketErrorReport::ErrorType_Asset_UnknownAssetCategory );
      return false;
   }

   assetOrganizer->GetListOfAssets( packet->platformId, m_productFilterNames, packet->compressionType, assetIds, m_maxNumAssetReturnedByCategory );
   
   // here we further reduce the set by compression type in the rare case that it is specified
   // FixedStringTiny   compressionType;

   // 1) reduce the set of products by device type.

   //-----------------------------
   PacketAsset_GetListOfAssetsResponse*    response = new PacketAsset_GetListOfAssetsResponse;
   response->assetCategory = packet->assetCategory;
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

   U32 connectionId = m_userTicket.connectionId;
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
      const AssetDefinition* asset = m_assetManager->GetAsset( packet->assetHash );
      if( asset != NULL )
      {
         int version = packet->fileVersion;
         LoadedFile file;
         if( asset->FindFile( version, file ) == true )
         {
            data = file.fileData;
            size = file.fileSize;

            const int MaxSize = PacketGameplayRawData::MaxBufferSize  - sizeof( PacketGatewayWrapper );
            cout << "File being sent = " << asset->name << endl;
            cout << "   path = " << asset->path << endl;
            cout << "   size = " << size << endl;

            return SendRawData< PacketGameplayRawData, AssetMainThread > 
               ( data, size, PacketGameplayRawData::Asset, MaxSize, m_assetManager->GetServerId(), asset->productId, asset->hash, connectionId, m_assetManager );
         }
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

bool     UserAccountAssetDelivery::EchoHandler()
{
   cout << " Echo " << endl;
   PacketAsset_EchoToClient* echo = new PacketAsset_EchoToClient;
   U32 connectionId = m_userTicket.connectionId;
   PacketGatewayWrapper* wrapper = new PacketGatewayWrapper;
   wrapper->SetupPacket( echo, connectionId );
   
   m_assetManager->AddOutputChainData( wrapper, connectionId );
   return true;
}

//------------------------------------------------------------------------------------------------
