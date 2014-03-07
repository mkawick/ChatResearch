// AssetPacket.h
#pragma once

#include "BasePacket.h"


struct AssetInfo
{
   U8       productId;
   bool     isOptional;
   string   assetHash;
   string   assetName;
   string   version;
   string   beginDate, endDate;
   string   category;

   AssetInfo()
   {
      Clear();
   }
   void  Clear()
   {
      isOptional = false;
      productId = 0;
      assetHash.clear();
      assetName.clear();
      version.clear();
      beginDate.clear();
      endDate.clear();
      category.clear();
   }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;
};

///////////////////////////////////////////////////////////////////

class PacketAsset : public BasePacket 
{
public:
   enum AssetType
   {
      AssetType_Base,
      AssetType_TestNotification,
      AssetType_EchoToServer,
      AssetType_EchoToClient,

      AssetType_GetListOfStaticAssets,
      AssetType_GetListOfStaticAssetsResponse,
      AssetType_GetListOfDynamicAssets, 
      AssetType_GetListOfDynamicAssetsResponse,

      AssetType_GetListOfAssetCategories,
      AssetType_GetListOfAssetCategoriesResponse,
      AssetType_GetListOfAssets,
      AssetType_GetListOfAssetsResponse,

      AssetType_RequestAsset  // mostly for testing

   };

public:
   PacketAsset( int packet_type = PacketType_Asset, int packet_sub_type = AssetType_Base ) : BasePacket( packet_type, packet_sub_type ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;
};

///////////////////////////////////////////////////////////////

class PacketAsset_EchoToServer : public PacketAsset
{
public:
   PacketAsset_EchoToServer(): PacketAsset( PacketType_Asset, PacketAsset::AssetType_EchoToServer ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   uuid;
   string   loginKey;
};

///////////////////////////////////////////////////////////////

class PacketAsset_EchoToClient : public BasePacket
{
public:
   PacketAsset_EchoToClient(): BasePacket( PacketType_Asset, PacketAsset::AssetType_EchoToClient ) {}
};

///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////

class PacketAsset_TestNotification : public PacketAsset
{
public:
   PacketAsset_TestNotification() : PacketAsset( PacketType_Asset, AssetType_TestNotification ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   message;
   string   senderName;
   string   senderUuid;
   int      type;
};


///////////////////////////////////////////////////////////////

class PacketAsset_GetListOfStaticAssets : public PacketAsset
{
public:
   PacketAsset_GetListOfStaticAssets() : PacketAsset( PacketType_Asset, AssetType_GetListOfStaticAssets ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   uuid;
   string   loginKey;
   int      platformId;
   //SerializedKeyValueVector< AssetInfo >   currentAssets;
};

///////////////////////////////////////////////////////////////////

class PacketAsset_GetListOfStaticAssetsResponse : public PacketAsset
{
public:
   PacketAsset_GetListOfStaticAssetsResponse() : PacketAsset( PacketType_Asset, AssetType_GetListOfStaticAssetsResponse ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   SerializedKeyValueVector< AssetInfo >   updatedAssets;
};

///////////////////////////////////////////////////////////////

class PacketAsset_GetListOfDynamicAssets : public PacketAsset
{
public:
   PacketAsset_GetListOfDynamicAssets() : PacketAsset( PacketType_Asset, AssetType_GetListOfDynamicAssets ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   uuid;
   string   loginKey;
   int      platformId;
};

///////////////////////////////////////////////////////////////////

class PacketAsset_GetListOfDynamicAssetsResponse : public PacketAsset
{
public:
   PacketAsset_GetListOfDynamicAssetsResponse() : PacketAsset( PacketType_Asset, AssetType_GetListOfDynamicAssetsResponse ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   SerializedKeyValueVector< AssetInfo >   updatedAssets;
};


///////////////////////////////////////////////////////////////

class PacketAsset_GetListOfAssetCategories : public PacketAsset
{
public:
   PacketAsset_GetListOfAssetCategories() : PacketAsset( PacketType_Asset, AssetType_GetListOfAssetCategories ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   uuid;
   string   loginKey;
};

///////////////////////////////////////////////////////////////////

class PacketAsset_GetListOfAssetCategoriesResponse : public PacketAsset
{
public:
   PacketAsset_GetListOfAssetCategoriesResponse() : PacketAsset( PacketType_Asset, AssetType_GetListOfAssetCategoriesResponse ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   SerializedVector< string >   assetcategory;
};

///////////////////////////////////////////////////////////////

class PacketAsset_GetListOfAssets : public PacketAsset
{
public:
   PacketAsset_GetListOfAssets() : PacketAsset( PacketType_Asset, AssetType_GetListOfAssets ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   uuid;
   string   loginKey;
   string   assetCategory;
   int      platformId;
};

///////////////////////////////////////////////////////////////////

class PacketAsset_GetListOfAssetsResponse : public PacketAsset
{
public:
   PacketAsset_GetListOfAssetsResponse() : PacketAsset( PacketType_Asset, AssetType_GetListOfAssetsResponse ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   assetCategory;
   SerializedKeyValueVector< AssetInfo >   updatedAssets;
};

///////////////////////////////////////////////////////////////////

class PacketAsset_RequestAsset : public PacketAsset
{
public:
   PacketAsset_RequestAsset() : PacketAsset( PacketType_Asset, AssetType_RequestAsset ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string      uuid;
   string      loginKey;
   string      assetHash;
};

///////////////////////////////////////////////////////////////////