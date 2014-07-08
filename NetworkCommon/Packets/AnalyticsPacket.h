// AnalyticsPacket.h
#pragma once

#include "BasePacket.h"


///////////////////////////////////////////////////////////////////

class PacketAnalytics : public BasePacket 
{
public:
   enum StatType
   {
      StatType_Accumulator,
      StatType_SimpleValue
   };
public:
   PacketAnalytics( int packet_type = PacketType_Analytics, int packet_sub_type = 0 ) : 
               BasePacket( packet_type, packet_sub_type ),
               category( 0 ),
               subCategory( 0 ),
               statType( StatType_SimpleValue ),
               value( 0 )
               {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   U16         category;
   U16         subCategory;
   U8          statType;
   U8          pad;

   string      statName;
   string      serverReporting;
   string      timestamp;

   float       value;
};

///////////////////////////////////////////////////////////////