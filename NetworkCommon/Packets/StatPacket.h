// StatPacket.h
#pragma once

#include "BasePacket.h"


///////////////////////////////////////////////////////////////////

class PacketStat : public BasePacket 
{
public:
   PacketStat( int packet_type = PacketType_Stat, int packet_sub_type = 0 ) : BasePacket( packet_type, packet_sub_type ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   statName;
   string   serverReporting;
   U16      category;
   U16      subCategory;

   float    value;
   float    subValue;
   float    minValue;
   float    maxValue;
   int      numValues;
   float    stdDev;
   
   string   beginTime;
   string   endTime;
};

///////////////////////////////////////////////////////////////