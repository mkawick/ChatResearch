// AssetPacket.h
#pragma once

#include "BasePacket.h"


struct AssetInfo
{
   U8       productId;
   string   assetHash;
   string   version;
   string   date;

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

      AssetType_GetListOfStaticAssets,
      AssetType_GetListOfStaticAssetsResponse,
      AssetType_GetListOfDynamicAssets, 
      AssetType_GetListOfDynamicAssetsResponse,
      AssetType_RequestAsset  // mostly for testing

   };

public:
   PacketAsset( int packet_type = PacketType_Asset, int packet_sub_type = AssetType_Base ) : BasePacket( packet_type, packet_sub_type ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;
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
   SerializedKeyValueVector< AssetInfo >   currentAssets;
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
   SerializedKeyValueVector< AssetInfo >   currentAssets;
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

///////////////////////////////////////////////////////////////////

class PacketAsset_RequestAsset : public PacketAsset
{
public:
   PacketAsset_RequestAsset() : PacketAsset( PacketType_Asset, AssetType_RequestAsset ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   AssetInfo   asset;
};

///////////////////////////////////////////////////////////////////