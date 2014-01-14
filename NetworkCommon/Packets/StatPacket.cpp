#include "StatPacket.h"

///////////////////////////////////////////////////////////////

bool  PacketStat::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, statName );
   Serialize::In( data, bufferOffset, serverReporting );
   Serialize::In( data, bufferOffset, category );
   Serialize::In( data, bufferOffset, subCategory );

   Serialize::In( data, bufferOffset, value );
   Serialize::In( data, bufferOffset, subValue );
   Serialize::In( data, bufferOffset, minValue );

   Serialize::In( data, bufferOffset, maxValue );
   Serialize::In( data, bufferOffset, numValues );
   Serialize::In( data, bufferOffset, stdDev );

   Serialize::In( data, bufferOffset, beginTime );
   Serialize::In( data, bufferOffset, endTime );

   return true;
}

bool  PacketStat::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   
   Serialize::Out( data, bufferOffset, statName );
   Serialize::Out( data, bufferOffset, serverReporting );
   Serialize::Out( data, bufferOffset, category );
   Serialize::Out( data, bufferOffset, subCategory );

   Serialize::Out( data, bufferOffset, value );
   Serialize::Out( data, bufferOffset, subValue );
   Serialize::Out( data, bufferOffset, minValue );

   Serialize::Out( data, bufferOffset, maxValue );
   Serialize::Out( data, bufferOffset, numValues );
   Serialize::Out( data, bufferOffset, stdDev );

   Serialize::Out( data, bufferOffset, beginTime );
   Serialize::Out( data, bufferOffset, endTime );

   return true;
}

///////////////////////////////////////////////////////////////