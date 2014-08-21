// DbPackets.cpp

#include "BasePacket.h"
#include "DbPacket.h"
///////////////////////////////////////////////////////////////

bool  BasePacketDbQuery::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, id, minorVersion );
   Serialize::In( data, bufferOffset, lookup, minorVersion );
   Serialize::In( data, bufferOffset, serverLookup, minorVersion );
   Serialize::In( data, bufferOffset, dbConnectionType, minorVersion );
   
   Serialize::In( data, bufferOffset, meta, minorVersion );

   return true;
}

bool  BasePacketDbQuery::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, id, minorVersion );
   Serialize::Out( data, bufferOffset, lookup, minorVersion );
   Serialize::Out( data, bufferOffset, serverLookup, minorVersion );
   Serialize::Out( data, bufferOffset, dbConnectionType, minorVersion );

   Serialize::Out( data, bufferOffset, meta, minorVersion );

   return true;
}
/*
BasePacketDbQuery::~BasePacketDbQuery()
{
}*/

///////////////////////////////////////////////////////////////

bool  PacketDbQuery::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacketDbQuery::SerializeIn( data, bufferOffset, minorVersion );

   Serialize::In( data, bufferOffset, isFireAndForget, minorVersion );
   Serialize::In( data, bufferOffset, query, minorVersion );
   Serialize::In( data, bufferOffset, escapedStrings, minorVersion );

   return true;
}

bool  PacketDbQuery::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacketDbQuery::SerializeOut( data, bufferOffset, minorVersion );
   
   Serialize::Out( data, bufferOffset, isFireAndForget, minorVersion );
   Serialize::Out( data, bufferOffset, query, minorVersion );
   Serialize::Out( data, bufferOffset, escapedStrings, minorVersion );

   return true;
}

/*
PacketDbQuery::~PacketDbQuery()
{
}
*/
///////////////////////////////////////////////////////////////

bool  PacketDbQueryResult::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacketDbQuery::SerializeIn( data, bufferOffset, minorVersion );

   Serialize::In( data, bufferOffset, successfulQuery, minorVersion );
   Serialize::In( data, bufferOffset, insertId, minorVersion );
   Serialize::In( data, bufferOffset, bucket, minorVersion );
   //bucket.SerializeIn( data, bufferOffset, minorVersion );

   return true;
}

bool  PacketDbQueryResult::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacketDbQuery::SerializeOut( data, bufferOffset, minorVersion );
   
   Serialize::Out( data, bufferOffset, successfulQuery, minorVersion );
   Serialize::Out( data, bufferOffset, insertId, minorVersion );
   Serialize::Out( data, bufferOffset, bucket, minorVersion );
   //bucket.SerializeOut( data, bufferOffset, minorVersion );

   return true;
}
/*
PacketDbQueryResult::~PacketDbQueryResult()
{
}
*/
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
