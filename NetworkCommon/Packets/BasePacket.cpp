// BasePacket.cpp

#include "../ServerConstants.h"
#include "BasePacket.h"
#include "../Serialize.h"
#include "PacketFactory.h"

#include <assert.h>

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

bool  StringBucket::SerializeIn( const U8* data, int& bufferOffset )
{ 
   int numStrings = 0;
   Serialize::In( data, bufferOffset, numStrings );

   for( int i=0; i<numStrings; i++ )
   {
      string temp;
      Serialize::In( data, bufferOffset, temp );
      bucket.push_back( temp );
   }

   return true; 
}

bool  StringBucket::SerializeOut( U8* data, int& bufferOffset ) const 
{ 
   int numStrings = static_cast< int >( bucket.size() );
   Serialize::Out( data, bufferOffset, numStrings );

   
   DataSet::const_iterator it = bucket.begin();
   while( it != bucket.end() )
   {
      const string& value = *it++;

      Serialize::Out( data, bufferOffset, value );
   }

   return true; 
}

///////////////////////////////////////////////////////////////


#ifdef _MEMLEAK_TESTING_
DynamicDataBucket::~DynamicDataBucket()
{
   bucket.clear();
}
#endif

bool  DynamicDataBucket::SerializeIn( const U8* data, int& bufferOffset )
{ 
   int numRows = 0, numColumns = 0;
   Serialize::In( data, bufferOffset, numRows );
   Serialize::In( data, bufferOffset, numColumns );
   bucket.clear();

   for( int i=0; i<numRows; i++ )
   {
      bucket.push_back( DataRow() );
      DataRow& newRow = *bucket.rbegin();
      for (int j=0; j<numColumns; j++ )
      {
         string temp;
         Serialize::In( data, bufferOffset, temp );
         newRow.push_back( temp );
      }
   }

   return true; 
}

bool  DynamicDataBucket::SerializeOut( U8* data, int& bufferOffset ) const 
{ 
   int numRows = static_cast< int >( bucket.size() );
   Serialize::Out( data, bufferOffset, numRows );

   DataSet::const_iterator it = bucket.begin();
   int numColumns = static_cast< int >( (*it).size() );
   Serialize::Out( data, bufferOffset, numColumns );

   while( it != bucket.end() )
   {
      const DataRow& newRow = *it++;

      DataRow::const_iterator    rowIt = newRow.begin();
      while( rowIt != newRow.end() )
      {
         const string& value = *rowIt;

         Serialize::Out( data, bufferOffset, value );

         ++ rowIt;
      }
   }

   return true; 
}


void  DynamicDataBucket::operator = ( const list< DataRow >& copydata )
{
   bucket.clear(); 
   list< DataRow >::const_iterator    it = copydata.begin();
   while( it != copydata.end() )
   {
      const DataRow& listref = *it++;
      DataRow::const_iterator rowit = listref.begin();

      bucket.push_back( DataRow() );
      DataRow& newrow = *bucket.rbegin();
      while( rowit != listref.end() )
      {
         newrow.push_back( *rowit++ );
      }
   }
}

void  DynamicDataBucket::operator = ( const list< list< string > >& copyData )
{
   bucket.clear(); 
   list< list< string > >::const_iterator    it = copyData.begin();
   while( it != copyData.end() )
   {
      const list< string >& listRef = *it++;
      list< string >::const_iterator rowIt = listRef.begin();

      bucket.push_back( DataRow() );
      DataRow& newRow = *bucket.rbegin();
      while( rowIt != listRef.end() )
      {
         newRow.push_back( *rowIt++ );
      }
   }
}


#ifdef _MEMORY_TEST_
int BasePacket::m_counter = 0;
#endif

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

bool  BasePacket::SerializeIn( const U8* data, int& bufferOffset )
{ 
   Serialize::In( data, bufferOffset, packetType );
   Serialize::In( data, bufferOffset, packetSubType );
   Serialize::In( data, bufferOffset, versionNumber );
   Serialize::In( data, bufferOffset, gameProductId );   
   Serialize::In( data, bufferOffset, gameInstanceId );

   return true; 
}

bool  BasePacket::SerializeOut( U8* data, int& bufferOffset ) const 
{ 
   Serialize::Out( data, bufferOffset, packetType );
   Serialize::Out( data, bufferOffset, packetSubType );
   Serialize::Out( data, bufferOffset, versionNumber );
   Serialize::Out( data, bufferOffset, gameProductId );
   Serialize::Out( data, bufferOffset, gameInstanceId );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketCommsHandshake::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, serverHashedKey );

   return true;
}

bool  PacketCommsHandshake::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, serverHashedKey );

   return true;
}

///////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////


bool  ChannelInfo::SerializeIn( const U8* data, int& bufferOffset )
{
   Serialize::In( data, bufferOffset, channelName );
   Serialize::In( data, bufferOffset, channelUuid );
   Serialize::In( data, bufferOffset, gameProduct );
   Serialize::In( data, bufferOffset, gameId );
   Serialize::In( data, bufferOffset, numNewChats );
   Serialize::In( data, bufferOffset, isActive );

   return true;
}

bool  ChannelInfo::SerializeOut( U8* data, int& bufferOffset ) const
{
   Serialize::Out( data, bufferOffset, channelName );
   Serialize::Out( data, bufferOffset, channelUuid );
   Serialize::Out( data, bufferOffset, gameProduct );
   Serialize::Out( data, bufferOffset, gameId );
   Serialize::Out( data, bufferOffset, numNewChats );
   Serialize::Out( data, bufferOffset, isActive );

   return true;
}


///////////////////////////////////////////////////////////////


bool  PacketFriendsList::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   friendList.SerializeIn( data, bufferOffset );

   return true;
}

bool  PacketFriendsList::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   friendList.SerializeOut( data, bufferOffset );

   return true;
}

///////////////////////////////////////////////////////////////


bool  PacketGroupsList::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   groupList.SerializeIn( data, bufferOffset );

   return true;
}

bool  PacketGroupsList::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   groupList.SerializeOut( data, bufferOffset );

   return true;
}

///////////////////////////////////////////////////////////////

PacketChatChannelList::~PacketChatChannelList()
{
   channelList.clear();
}

bool  PacketChatChannelList::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   channelList.SerializeIn( data, bufferOffset );

   return true;
}

bool  PacketChatChannelList::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   channelList.SerializeOut( data, bufferOffset );

   return true;
}

///////////////////////////////////////////////////////////////


bool  PacketUserStateChange::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, uuid );
   Serialize::In( data, bufferOffset, username );

   return true;
}

bool  PacketUserStateChange::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, uuid );
   Serialize::Out( data, bufferOffset, username );

   return true;
}

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

U8 PacketGatewayWrapper::SerializeBuffer[ PacketGatewayWrapper::BufferSize ];
void  PacketGatewayWrapper::SetupPacket( BasePacket* packet, U32 connId )
{
   size = 0;
   int tempSize = 0;
   packet->SerializeOut( SerializeBuffer, tempSize ); // get the size info
   size = tempSize;
   pPacket = packet;
   connectionId = connId;

   gameInstanceId = packet->gameInstanceId;
   gameProductId = packet->gameProductId;
   //packet->
}

///////////////////////////////////////////////////////////////

bool  PacketGatewayWrapper::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, connectionId );
   Serialize::In( data, bufferOffset, size );

   delete pPacket; pPacket = NULL;
   PacketFactory packetFactory;

   if( packetFactory.Parse( data, bufferOffset, &pPacket ) == false )
   {
      return false;
   }

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketGatewayWrapper::HeaderSerializeIn( const U8* data, int bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, connectionId );
   Serialize::In( data, bufferOffset, size );

   delete pPacket; pPacket = NULL;
   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketGatewayWrapper::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );   
   Serialize::Out( data, bufferOffset, connectionId );
   Serialize::Out( data, bufferOffset, size );

   if( pPacket == NULL )
   {
      assert( 0 );
   }
   pPacket->SerializeOut( data, bufferOffset );

   return true;
}

///////////////////////////////////////////////////////////////


bool  PacketErrorReport::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, statusInfo );

   return true;
}

bool  PacketErrorReport::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, statusInfo );

   return true;
}

///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////