// NetworkLayer_SupportClasses.cpp

#include <queue>
#include <map>
#include <iostream>
using namespace std;

#include "NetworkWrapper_SupportClasses.h"


namespace Mber
{
///////////////////////////////////////////////////////////////////////////////////

void  ChatChannel::Print()
{
   U8 productId = gameProductId;
   int numUsers = userList.size();
   cout << "** channel name= " << channelName << endl;
   cout << "   num users " << numUsers << endl;
   SerializedKeyValueVector< string >::const_KVIterator it = userList.begin();
   while( it != userList.end() )
   {
      cout << "    User: " << it->value << endl;
      it++;
   }
}

bool  ChatChannel::HasUserInchannel( const string& userUuid )
{
   SerializedKeyValueVector< string >::const_KVIterator it = userList.begin();
   while( it != userList.end() )
   {
      if( userUuid == it->key )
         return true;
      it++;
   }
   return false;
}

///////////////////////////////////////////////////////////////////////////////////

AssetInfoExtended:: AssetInfoExtended() : data(NULL), size( NULL ), AssetInfo() 
{
}

AssetInfoExtended:: AssetInfoExtended( const AssetInfo& asset ) : data(NULL), size( NULL ), AssetInfo( asset ) 
{
}

AssetInfoExtended:: AssetInfoExtended( const AssetInfoExtended& asset ) : data(NULL), size( NULL ), AssetInfo( asset ) 
{
}

AssetInfoExtended:: ~AssetInfoExtended()
{
   ClearData();
}

void  AssetInfoExtended:: SetData( const U8* _data, int _size )
{
   ClearData();

   if( _size == 0 || _data == NULL )
      return;

   size = _size;
   data = new U8[ size ];
   memcpy( data, _data, size );
}

void  AssetInfoExtended:: ClearData()
{
   if( data )
      delete [] data;
   data = NULL;
   size = 0;
}

const AssetInfo&  AssetInfoExtended:: operator = ( const AssetInfo& asset )
{
   productId = asset.productId;
   assetHash = asset.assetHash;
   assetName = asset.assetName;
   version = asset.version;
   beginDate = asset.beginDate;
   endDate = asset.endDate;
   isOptional = asset.isOptional;
   category = asset.category;

   return *this;
}

const AssetInfoExtended&  AssetInfoExtended:: operator = ( const AssetInfoExtended& asset )
{
   productId = asset.productId;
   assetHash = asset.assetHash;
   assetName = asset.assetName;
   version = asset.version;
   beginDate = asset.beginDate;
   endDate = asset.endDate;
   isOptional = asset.isOptional;
   category = asset.category;

   SetData( asset.data, asset.size );
   return *this;
}

void  AssetInfoExtended:: MoveData( AssetInfoExtended& source )
{
   data = source.data; source.data = NULL;
   size = source.size; source.size = 0;
}


void  AssetInfoExtended::AddAssetData( PacketGameplayRawData* data )
{
   rawDataBuffer.AddPacket( data );
   if( rawDataBuffer.IsDone() )
   {
      rawDataBuffer.PrepPackage( *this );
   }
}

bool  AssetInfoExtended::IsAssetFullyLoaded() const
{
   if( size > 0 && data != NULL )
      return true;
   return false;
}

int   AssetInfoExtended::GetRemainingSize() const
{
   return rawDataBuffer.GetRemainingSize();
}

bool   AssetInfoExtended::NeedsClearing() const
{
   return rawDataBuffer.NeedsClearing();
}

void   AssetInfoExtended::ClearTempData()
{
   rawDataBuffer.ClearData();
}

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////

}