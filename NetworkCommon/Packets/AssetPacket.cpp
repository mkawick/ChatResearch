// AssetPacket.cpp

#include <string>

#include "AssetPacket.h"
using namespace std;

///////////////////////////////////////////////////////////////

bool  PacketAsset::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{ 
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );

   return true; 
}

bool  PacketAsset::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{ 
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );

   return true; 
}



bool  AssetInfo::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   Serialize::In( data, bufferOffset, productId, minorVersion );
   Serialize::In( data, bufferOffset, isOptional, minorVersion );
   Serialize::In( data, bufferOffset, assetHash, minorVersion );
   Serialize::In( data, bufferOffset, assetName, minorVersion );
   Serialize::In( data, bufferOffset, version, minorVersion );
   Serialize::In( data, bufferOffset, beginDate, minorVersion );
   Serialize::In( data, bufferOffset, endDate, minorVersion );
   Serialize::In( data, bufferOffset, category, minorVersion );

   return true;
}

bool  AssetInfo::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   Serialize::Out( data, bufferOffset, productId, minorVersion );
   Serialize::Out( data, bufferOffset, isOptional, minorVersion );
   Serialize::Out( data, bufferOffset, assetHash, minorVersion );
   Serialize::Out( data, bufferOffset, assetName, minorVersion );
   Serialize::Out( data, bufferOffset, version, minorVersion );
   Serialize::Out( data, bufferOffset, beginDate, minorVersion );
   Serialize::Out( data, bufferOffset, endDate, minorVersion );
   Serialize::Out( data, bufferOffset, category, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketAsset_EchoToServer::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{ 
   PacketAsset::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, uuid, minorVersion );
   Serialize::In( data, bufferOffset, loginKey, minorVersion );

   return true; 
}

bool  PacketAsset_EchoToServer::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{ 
   PacketAsset::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, uuid, minorVersion );
   Serialize::Out( data, bufferOffset, loginKey, minorVersion );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketAsset_TestNotification::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{ 
   PacketAsset::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, message, minorVersion );
   Serialize::In( data, bufferOffset, senderName, minorVersion );
   Serialize::In( data, bufferOffset, senderUuid, minorVersion );
   Serialize::In( data, bufferOffset, type, minorVersion );

   return true; 
}

bool  PacketAsset_TestNotification::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{ 
   PacketAsset::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, message, minorVersion );
   Serialize::Out( data, bufferOffset, senderName, minorVersion );
   Serialize::Out( data, bufferOffset, senderUuid, minorVersion );
   Serialize::Out( data, bufferOffset, type, minorVersion );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketAsset_GetListOfStaticAssets::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{ 
   PacketAsset::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, uuid, minorVersion );
   Serialize::In( data, bufferOffset, loginKey, minorVersion );
   Serialize::In( data, bufferOffset, platformId, minorVersion );

   return true; 
}

bool  PacketAsset_GetListOfStaticAssets::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{ 
   PacketAsset::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, uuid, minorVersion );
   Serialize::Out( data, bufferOffset, loginKey, minorVersion );
   Serialize::Out( data, bufferOffset, platformId, minorVersion );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketAsset_GetListOfStaticAssetsResponse::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{ 
   PacketAsset::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, updatedAssets, minorVersion );
   //updatedAssets.SerializeIn( data, bufferOffset, minorVersion );

   return true; 
}

bool  PacketAsset_GetListOfStaticAssetsResponse::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{ 
   PacketAsset::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, updatedAssets, minorVersion );
   //updatedAssets.SerializeOut( data, bufferOffset, minorVersion );

   return true; 
}

///////////////////////////////////////////////////////////////


bool  PacketAsset_GetListOfDynamicAssets::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{ 
   PacketAsset::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, uuid, minorVersion );
   Serialize::In( data, bufferOffset, loginKey, minorVersion );
   Serialize::In( data, bufferOffset, platformId, minorVersion );

   return true; 
}

bool  PacketAsset_GetListOfDynamicAssets::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{ 
   PacketAsset::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, uuid, minorVersion );
   Serialize::Out( data, bufferOffset, loginKey, minorVersion );
   Serialize::Out( data, bufferOffset, platformId, minorVersion );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketAsset_GetListOfDynamicAssetsResponse::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{ 
   PacketAsset::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, updatedAssets, minorVersion );
   //updatedAssets.SerializeIn( data, bufferOffset, minorVersion );

   return true; 
}

bool  PacketAsset_GetListOfDynamicAssetsResponse::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{ 
   PacketAsset::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, updatedAssets, minorVersion );
   //updatedAssets.SerializeOut( data, bufferOffset, minorVersion );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketAsset_GetListOfAssetCategories::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{ 
   PacketAsset::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, uuid, minorVersion );
   Serialize::In( data, bufferOffset, loginKey, minorVersion );

   return true; 
}

bool  PacketAsset_GetListOfAssetCategories::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{ 
   PacketAsset::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, uuid, minorVersion );
   Serialize::Out( data, bufferOffset, loginKey, minorVersion );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketAsset_GetListOfAssetCategoriesResponse::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{ 
   PacketAsset::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, assetcategory, minorVersion );
   //assetcategory.SerializeIn( data, bufferOffset, minorVersion );

   return true; 
}

bool  PacketAsset_GetListOfAssetCategoriesResponse::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{ 
   PacketAsset::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, assetcategory, minorVersion );
   //assetcategory.SerializeOut( data, bufferOffset, minorVersion );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketAsset_GetListOfAssets::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{ 
   PacketAsset::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, uuid, minorVersion );
   Serialize::In( data, bufferOffset, loginKey, minorVersion );
   Serialize::In( data, bufferOffset, assetCategory, minorVersion );
   Serialize::In( data, bufferOffset, platformId, minorVersion );

   return true; 
}

bool  PacketAsset_GetListOfAssets::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{ 
   PacketAsset::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, uuid, minorVersion );
   Serialize::Out( data, bufferOffset, loginKey, minorVersion );
   Serialize::Out( data, bufferOffset, assetCategory, minorVersion );
   Serialize::Out( data, bufferOffset, platformId, minorVersion );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketAsset_GetListOfAssetsResponse::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{ 
   PacketAsset::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, assetCategory, minorVersion );
   Serialize::In( data, bufferOffset, updatedAssets, minorVersion );
   //updatedAssets.SerializeIn( data, bufferOffset, minorVersion );

   return true; 
}

bool  PacketAsset_GetListOfAssetsResponse::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{ 
   PacketAsset::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, assetCategory, minorVersion );
   Serialize::Out( data, bufferOffset, updatedAssets, minorVersion );
   //updatedAssets.SerializeOut( data, bufferOffset, minorVersion );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketAsset_RequestAsset::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{ 
   PacketAsset::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, uuid, minorVersion );
   Serialize::In( data, bufferOffset, loginKey, minorVersion );
   Serialize::In( data, bufferOffset, assetHash, minorVersion );

   return true; 
}

bool  PacketAsset_RequestAsset::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{ 
   PacketAsset::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, uuid, minorVersion );
   Serialize::Out( data, bufferOffset, loginKey, minorVersion );
   Serialize::Out( data, bufferOffset, assetHash, minorVersion );

   return true; 
}

///////////////////////////////////////////////////////////////