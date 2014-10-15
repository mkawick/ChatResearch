// DbPackets.h

#pragma once

#include "BasePacket.h"

#pragma pack( push, 4 )

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
   BasePacketDbQuery( int packet_type = PacketType_DbQuery, int packet_sub_type = QueryType_Query ): 
               BasePacket( packet_type, packet_sub_type ), 
               lookup( 0 ), 
               id( 0 ),                
               serverLookup( 0 ), 
               dbConnectionType( 0 ), 
               customData( NULL ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   int      lookup;
   U32      id;
   U32      serverLookup;
   U8       dbConnectionType;
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

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

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

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   bool                 successfulQuery;
   int                  insertId; // rarely correct.. only for inserts
   DynamicDataBucket    bucket;// this could be slow with large datasets.. look into optimizations here

   const DataSet&       GetBucket() const { return bucket.bucket; }
private: 
   const BasePacketDbQuery& operator = (const BasePacketDbQuery& );
};

///////////////////////////////////////////////////////////////

#pragma pack( pop )