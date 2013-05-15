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
   int numStrings = bucket.size();
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
   int numRows = bucket.size();
   Serialize::Out( data, bufferOffset, numRows );

   DataSet::const_iterator it = bucket.begin();
   int numColumns = (*it).size();
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

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

bool  BasePacket::SerializeIn( const U8* data, int& bufferOffset )
{ 
   Serialize::In( data, bufferOffset, serverType );
   Serialize::In( data, bufferOffset, packetType );
   Serialize::In( data, bufferOffset, packetSubType );
   Serialize::In( data, bufferOffset, versionNumber );
   Serialize::In( data, bufferOffset, gameInstanceId );
   return true; 
}

bool  BasePacket::SerializeOut( U8* data, int& bufferOffset ) const 
{ 
   Serialize::Out( data, bufferOffset, serverType );
   Serialize::Out( data, bufferOffset, packetType );
   Serialize::Out( data, bufferOffset, packetSubType );
   Serialize::Out( data, bufferOffset, versionNumber );
   Serialize::Out( data, bufferOffset, gameInstanceId );
   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketLogin::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, uuid );
   Serialize::In( data, bufferOffset, username );
   Serialize::In( data, bufferOffset, password );
   Serialize::In( data, bufferOffset, loginKey );

   return true;
}

bool  PacketLogin::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, uuid );
   Serialize::Out( data, bufferOffset, username );
   Serialize::Out( data, bufferOffset, password );
   Serialize::Out( data, bufferOffset, loginKey );

   return true;
}


///////////////////////////////////////////////////////////////

bool  PacketLoginToClient::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketLogin::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, wasLoginSuccessful );

   return true;
}

bool  PacketLoginToClient::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketLogin::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, wasLoginSuccessful );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketLoginToGateway::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketLogin::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, wasLoginSuccessful );

   return true;
}

bool  PacketLoginToGateway::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketLogin::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, wasLoginSuccessful );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketPrepareForUserLogin::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketLogin::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, lastLoginTime );
   Serialize::In( data, bufferOffset, connectionId );

   return true;
}

bool  PacketPrepareForUserLogin::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketLogin::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, lastLoginTime );
   Serialize::Out( data, bufferOffset, connectionId );

   return true;
}


///////////////////////////////////////////////////////////////

bool  PacketPrepareForUserLogout::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, connectionId );

   return true;
}

bool  PacketPrepareForUserLogout::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, connectionId );

   return true;
}


///////////////////////////////////////////////////////////////


bool  ChannelInfo::SerializeIn( const U8* data, int& bufferOffset )
{
   Serialize::In( data, bufferOffset, channelName );
   Serialize::In( data, bufferOffset, isActive );

   return true;
}

bool  ChannelInfo::SerializeOut( U8* data, int& bufferOffset ) const
{
   Serialize::Out( data, bufferOffset, channelName );
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

bool  BasePacketDbQuery::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, id );
   Serialize::In( data, bufferOffset, lookup );
   Serialize::In( data, bufferOffset, serverLookup );
   
   Serialize::In( data, bufferOffset, meta );

   return true;
}

bool  BasePacketDbQuery::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, id );
   Serialize::Out( data, bufferOffset, lookup );
   Serialize::Out( data, bufferOffset, serverLookup );

   Serialize::Out( data, bufferOffset, meta );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketDbQuery::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacketDbQuery::SerializeIn( data, bufferOffset );

   Serialize::In( data, bufferOffset, isFireAndForget );
   Serialize::In( data, bufferOffset, query );

   return true;
}

bool  PacketDbQuery::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacketDbQuery::SerializeOut( data, bufferOffset );
   
   Serialize::Out( data, bufferOffset, isFireAndForget );
   Serialize::Out( data, bufferOffset, query );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketDbQueryResult::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacketDbQuery::SerializeIn( data, bufferOffset );

   Serialize::In( data, bufferOffset, successfulQuery );
   bucket.SerializeIn( data, bufferOffset );

   return true;
}

bool  PacketDbQueryResult::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacketDbQuery::SerializeOut( data, bufferOffset );
   
   Serialize::Out( data, bufferOffset, successfulQuery );
   bucket.SerializeOut( data, bufferOffset );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketGatewayWrapper::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, connectionId );

   delete pPacket; pPacket = NULL;
   PacketFactory packetFactory;

   if( packetFactory.Parse( data, bufferOffset, &pPacket ) == false )
   {
      return false;
   }

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketGatewayWrapper::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );   
   Serialize::Out( data, bufferOffset, connectionId );

   if( pPacket == NULL )
   {
      assert( 0 );
   }
   pPacket->SerializeOut( data, bufferOffset );

   return true;
}

///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////