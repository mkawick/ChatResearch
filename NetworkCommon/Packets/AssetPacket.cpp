// AssetPacket.cpp

#include <string>

#include "AssetPacket.h"
using namespace std;

///////////////////////////////////////////////////////////////

bool  PacketAsset::SerializeIn( const U8* data, int& bufferOffset )
{ 
   BasePacket::SerializeIn( data, bufferOffset );

   return true; 
}

bool  PacketAsset::SerializeOut( U8* data, int& bufferOffset ) const 
{ 
   BasePacket::SerializeOut( data, bufferOffset );

   return true; 
}



bool  AssetInfo::SerializeIn( const U8* data, int& bufferOffset )
{
   Serialize::In( data, bufferOffset, productId );
   Serialize::In( data, bufferOffset, isOptional );
   Serialize::In( data, bufferOffset, assetHash );
   Serialize::In( data, bufferOffset, assetName );
   Serialize::In( data, bufferOffset, version );
   Serialize::In( data, bufferOffset, beginDate );
   Serialize::In( data, bufferOffset, endDate );
   Serialize::In( data, bufferOffset, category );

   return true;
}

bool  AssetInfo::SerializeOut( U8* data, int& bufferOffset ) const
{
   Serialize::Out( data, bufferOffset, productId );
   Serialize::Out( data, bufferOffset, isOptional );
   Serialize::Out( data, bufferOffset, assetHash );
   Serialize::Out( data, bufferOffset, assetName );
   Serialize::Out( data, bufferOffset, version );
   Serialize::Out( data, bufferOffset, beginDate );
   Serialize::Out( data, bufferOffset, endDate );
   Serialize::Out( data, bufferOffset, category );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketAsset_EchoToServer::SerializeIn( const U8* data, int& bufferOffset )
{ 
   PacketAsset::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, uuid );
   Serialize::In( data, bufferOffset, loginKey );

   return true; 
}

bool  PacketAsset_EchoToServer::SerializeOut( U8* data, int& bufferOffset ) const 
{ 
   PacketAsset::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, uuid );
   Serialize::Out( data, bufferOffset, loginKey );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketAsset_TestNotification::SerializeIn( const U8* data, int& bufferOffset )
{ 
   PacketAsset::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, message );
   Serialize::In( data, bufferOffset, senderName );
   Serialize::In( data, bufferOffset, senderUuid );
   Serialize::In( data, bufferOffset, type );

   return true; 
}

bool  PacketAsset_TestNotification::SerializeOut( U8* data, int& bufferOffset ) const 
{ 
   PacketAsset::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, message );
   Serialize::Out( data, bufferOffset, senderName );
   Serialize::Out( data, bufferOffset, senderUuid );
   Serialize::Out( data, bufferOffset, type );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketAsset_GetListOfStaticAssets::SerializeIn( const U8* data, int& bufferOffset )
{ 
   PacketAsset::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, uuid );
   Serialize::In( data, bufferOffset, loginKey );
   Serialize::In( data, bufferOffset, platformId );

   return true; 
}

bool  PacketAsset_GetListOfStaticAssets::SerializeOut( U8* data, int& bufferOffset ) const 
{ 
   PacketAsset::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, uuid );
   Serialize::Out( data, bufferOffset, loginKey );
   Serialize::Out( data, bufferOffset, platformId );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketAsset_GetListOfStaticAssetsResponse::SerializeIn( const U8* data, int& bufferOffset )
{ 
   PacketAsset::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, updatedAssets );

   return true; 
}

bool  PacketAsset_GetListOfStaticAssetsResponse::SerializeOut( U8* data, int& bufferOffset ) const 
{ 
   PacketAsset::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, updatedAssets );

   return true; 
}

///////////////////////////////////////////////////////////////


bool  PacketAsset_GetListOfDynamicAssets::SerializeIn( const U8* data, int& bufferOffset )
{ 
   PacketAsset::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, uuid );
   Serialize::In( data, bufferOffset, loginKey );
   Serialize::In( data, bufferOffset, platformId );

   return true; 
}

bool  PacketAsset_GetListOfDynamicAssets::SerializeOut( U8* data, int& bufferOffset ) const 
{ 
   PacketAsset::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, uuid );
   Serialize::Out( data, bufferOffset, loginKey );
   Serialize::Out( data, bufferOffset, platformId );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketAsset_GetListOfDynamicAssetsResponse::SerializeIn( const U8* data, int& bufferOffset )
{ 
   PacketAsset::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, updatedAssets );

   return true; 
}

bool  PacketAsset_GetListOfDynamicAssetsResponse::SerializeOut( U8* data, int& bufferOffset ) const 
{ 
   PacketAsset::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, updatedAssets );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketAsset_GetListOfAssetCategories::SerializeIn( const U8* data, int& bufferOffset )
{ 
   PacketAsset::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, uuid );
   Serialize::In( data, bufferOffset, loginKey );

   return true; 
}

bool  PacketAsset_GetListOfAssetCategories::SerializeOut( U8* data, int& bufferOffset ) const 
{ 
   PacketAsset::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, uuid );
   Serialize::Out( data, bufferOffset, loginKey );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketAsset_GetListOfAssetCategoriesResponse::SerializeIn( const U8* data, int& bufferOffset )
{ 
   PacketAsset::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, assetcategory );

   return true; 
}

bool  PacketAsset_GetListOfAssetCategoriesResponse::SerializeOut( U8* data, int& bufferOffset ) const 
{ 
   PacketAsset::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, assetcategory );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketAsset_GetListOfAssets::SerializeIn( const U8* data, int& bufferOffset )
{ 
   PacketAsset::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, uuid );
   Serialize::In( data, bufferOffset, loginKey );
   Serialize::In( data, bufferOffset, assetCategory );
   Serialize::In( data, bufferOffset, platformId );

   return true; 
}

bool  PacketAsset_GetListOfAssets::SerializeOut( U8* data, int& bufferOffset ) const 
{ 
   PacketAsset::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, uuid );
   Serialize::Out( data, bufferOffset, loginKey );
   Serialize::Out( data, bufferOffset, assetCategory );
   Serialize::Out( data, bufferOffset, platformId );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketAsset_GetListOfAssetsResponse::SerializeIn( const U8* data, int& bufferOffset )
{ 
   PacketAsset::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, assetCategory );
   Serialize::In( data, bufferOffset, updatedAssets );

   return true; 
}

bool  PacketAsset_GetListOfAssetsResponse::SerializeOut( U8* data, int& bufferOffset ) const 
{ 
   PacketAsset::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, assetCategory );
   Serialize::Out( data, bufferOffset, updatedAssets );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketAsset_RequestAsset::SerializeIn( const U8* data, int& bufferOffset )
{ 
   PacketAsset::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, uuid );
   Serialize::In( data, bufferOffset, loginKey );
   Serialize::In( data, bufferOffset, assetHash );

   return true; 
}

bool  PacketAsset_RequestAsset::SerializeOut( U8* data, int& bufferOffset ) const 
{ 
   PacketAsset::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, uuid );
   Serialize::Out( data, bufferOffset, loginKey );
   Serialize::Out( data, bufferOffset, assetHash );

   return true; 
}

///////////////////////////////////////////////////////////////