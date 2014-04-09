// DbPackets.cpp

#include "BasePacket.h"
#include "DbPacket.h"
///////////////////////////////////////////////////////////////

bool  BasePacketDbQuery::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, id );
   Serialize::In( data, bufferOffset, lookup );
   Serialize::In( data, bufferOffset, serverLookup );
   Serialize::In( data, bufferOffset, hitsTempDb );
   
   Serialize::In( data, bufferOffset, meta );

   return true;
}

bool  BasePacketDbQuery::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, id );
   Serialize::Out( data, bufferOffset, lookup );
   Serialize::Out( data, bufferOffset, serverLookup );
   Serialize::Out( data, bufferOffset, hitsTempDb );

   Serialize::Out( data, bufferOffset, meta );

   return true;
}
/*
BasePacketDbQuery::~BasePacketDbQuery()
{
}*/

///////////////////////////////////////////////////////////////

bool  PacketDbQuery::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacketDbQuery::SerializeIn( data, bufferOffset );

   Serialize::In( data, bufferOffset, isFireAndForget );
   Serialize::In( data, bufferOffset, query );
   Serialize::In( data, bufferOffset, escapedStrings );

   return true;
}

bool  PacketDbQuery::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacketDbQuery::SerializeOut( data, bufferOffset );
   
   Serialize::Out( data, bufferOffset, isFireAndForget );
   Serialize::Out( data, bufferOffset, query );
   Serialize::Out( data, bufferOffset, escapedStrings );

   return true;
}

/*
PacketDbQuery::~PacketDbQuery()
{
}
*/
///////////////////////////////////////////////////////////////

bool  PacketDbQueryResult::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacketDbQuery::SerializeIn( data, bufferOffset );

   Serialize::In( data, bufferOffset, successfulQuery );
   Serialize::In( data, bufferOffset, insertId );
   bucket.SerializeIn( data, bufferOffset );

   return true;
}

bool  PacketDbQueryResult::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacketDbQuery::SerializeOut( data, bufferOffset );
   
   Serialize::Out( data, bufferOffset, successfulQuery );
   Serialize::Out( data, bufferOffset, insertId );
   bucket.SerializeOut( data, bufferOffset );

   return true;
}
/*
PacketDbQueryResult::~PacketDbQueryResult()
{
}
*/
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
