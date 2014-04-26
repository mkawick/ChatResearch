// DbPackets.h

#pragma once

#include "BasePacket.h"

///////////////////////////////////////////////////////////////


class BasePacketDbQuery : public BasePacket
{
public:
   enum QueryType
   {
      QueryType_Query,
      QueryType_Result
   };
   typedef list< DataRow >  DataSet;
public:
   BasePacketDbQuery( int packet_type = PacketType_DbQuery, int packet_sub_type = QueryType_Query ): BasePacket( packet_type, packet_sub_type ), id( 0 ), lookup( 0 ), serverLookup( 0 ), hitsTempDb( false ), customData( NULL ) {}
   //BasePacketDbQuery( const BasePacketDbQuery& query ) : BasePacket( query.packetType, query.packetSubType), id( query.id ), lookup( query.lookup ), serverLookup( query.serverLookup ), hitsTempDb( query.hitsTempDb ), customData( query.customData )  {}
   //~BasePacketDbQuery();

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   U32      id;
   int      lookup;
   U32      serverLookup;
   bool     hitsTempDb;
   string   meta;

   void*          customData;

private: 
   const BasePacketDbQuery& operator = (const BasePacketDbQuery& );
};

///////////////////////////////////////////////////////////////

class PacketDbQuery : public BasePacketDbQuery
{
public:
   PacketDbQuery( int packet_type = PacketType_DbQuery, int packet_sub_type = QueryType_Query ): BasePacketDbQuery( packet_type, packet_sub_type ), isFireAndForget( false ) {}

   //PacketDbQuery( const PacketDbQuery& query ) : BasePacketDbQuery( query ), isFireAndForget( query.isFireAndForget ){}
   //~PacketDbQuery();

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   bool           isFireAndForget;
   string         query;
   StringBucket   escapedStrings;

private: 
   const BasePacketDbQuery& operator = (const BasePacketDbQuery& );
};


///////////////////////////////////////////////////////////////

class PacketDbQueryResult : public BasePacketDbQuery
{
public:
   PacketDbQueryResult( int packet_type = PacketType_DbQuery, int packet_sub_type = QueryType_Result  ): BasePacketDbQuery( packet_type, packet_sub_type ), successfulQuery( false ), insertId( 0 ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   bool                 successfulQuery;
   int                  insertId; // rarely correct.. only for inserts
   DynamicDataBucket    bucket;// this could be slow with large datasets.. look into optimizations here

   DataSet&  Bucket() { return bucket.bucket; }
private: 
   const BasePacketDbQuery& operator = (const BasePacketDbQuery& );
};

///////////////////////////////////////////////////////////////