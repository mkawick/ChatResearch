#pragma once

// NetworkLayer_SupportClasses.h

#include "../NetworkCommon/ServerConstants.h" // defines game product ids

#include "../NetworkCommon/Packets/BasePacket.h"
#include "../NetworkCommon/Packets/GamePacket.h"
#include "../NetworkCommon/Packets/AssetPacket.h"

#include <list>
#include <string>
using namespace std;

namespace Mber
{

///////////////////////////////////////////////////////

struct Demographics
{
   bool     isPrivate;// if true, most of the rest of this info is invalid.
   string   username;
   string   email;
   time_t   lastLoginTime;
   // ...
   list< string > listOfGamesOwned;
   int      gender;
   int      avatar;
   string   location;
   int      timeZoneGmt;

};

//-------------------------------------------

struct RegisteredProduct
{
   string   id; 
   string   title; 
   string   description; 
   string   price; 
   string   localized_price; 
   double   number_price;
   float    quantity;
};

//-------------------------------------------
//-------------------------------------------

class BasicUser
{
public:
   string   userName;
   string   UUID;
   U32      avatarId; 
   bool     isOnline;
};

//-------------------------------------------
//-------------------------------------------

class ChatChannel
{
public:
   ChatChannel(): gameInstanceId( 0 ) {}
   string   channelName;
   string   channelDetails;
   string   uuid;
   U8       gameProductId;
   U32      gameInstanceId;

   SerializedKeyValueVector< string >   userList; // not necessarily friends

   void  Clear()
   {
      channelName.clear();
      channelDetails.clear();
      uuid.clear();
      gameProductId = 0;
      gameInstanceId = 0;
      userList.clear();
   }
   void  Print();

   bool  HasUserInchannel( const string& userUuid );
};

//-------------------------------------------

typedef vector< ChatChannel >    ChatChannelVector;


//-------------------------------------------
//-------------------------------------------

class Group
{
   string groupName;
   string groupMotto;
   int    avatarId;
   string chatChannel; // the venn diagrap for groups and channels may not be 1-1
   vector< BasicUser > usersInGroup;
};

//-------------------------------------------

///////////////////////////////////////////////////////
struct AssetInfoExtended;

class RawDataAccumulator
{
public:
   RawDataAccumulator() : numBytes( 0 ), isReadyToBeCleared( false ) {}

   void  AddPacket( PacketGameplayRawData * );
   bool  IsDone() const;
   void  PrepPackage( U8* & data, int& size );
   void  PrepPackage( AssetInfoExtended& asset );

   void  ClearData();
   bool  NeedsClearing() const;

   int   GetRemainingSize() const
   { 
      if( packetsOfData.size() > 1 )
      {
         return (*packetsOfData.begin())->index * PacketGameplayRawData::MaxBufferSize;// only an estimate
      }
      return 0;
   }

   int   numBytes;
   bool  isReadyToBeCleared;
   deque< PacketGameplayRawData* > packetsOfData;
};

///////////////////////////////////////////////////////

struct AssetInfoExtended : public AssetInfo
{
   U8*                     data;
   int                     size;
   RawDataAccumulator      rawDataBuffer;

   AssetInfoExtended();
   AssetInfoExtended( const AssetInfo& asset );
   AssetInfoExtended( const AssetInfoExtended& asset );
   ~AssetInfoExtended();

   const AssetInfo&  operator = ( const AssetInfo& asset );
   const AssetInfoExtended&  operator = ( const AssetInfoExtended& asset );

   void  SetData( const U8* data, int size );
   void  ClearData();
   bool  IsDataValid() const { if( data && size ) return true; return false; }
   void  MoveData( AssetInfoExtended& source );

   bool  SerializeIn( const U8* data, int& bufferOffset ) { return false; } // do not serialize
   bool  SerializeOut( U8* data, int& bufferOffset ) const { return false; }

   // loading from the network
   void  AddAssetData( PacketGameplayRawData* data );
   bool  IsAssetFullyLoaded() const;
   int   GetRemainingSize() const;
   bool  NeedsClearing() const;
   void  ClearTempData();
};

///////////////////////////////////////////////////////

typedef vector< AssetInfoExtended > AssetCollection;
typedef map< string, AssetCollection > AssetMap;
typedef pair< string, AssetCollection > AssetMapPair;
typedef AssetMap::iterator AssetMapIter;
typedef AssetMap::const_iterator AssetMapConstIter;

///////////////////////////////////////////////////////

}