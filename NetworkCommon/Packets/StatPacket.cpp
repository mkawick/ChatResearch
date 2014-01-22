#include "StatPacket.h"

///////////////////////////////////////////////////////////////

bool  PacketStat::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   
   Serialize::In( data, bufferOffset, category );
   Serialize::In( data, bufferOffset, subCategory );
   Serialize::In( data, bufferOffset, statType );
   Serialize::In( data, bufferOffset, statName );

   Serialize::In( data, bufferOffset, serverReporting );
   Serialize::In( data, bufferOffset, timestamp );
   Serialize::In( data, bufferOffset, value );

   return true;
}

bool  PacketStat::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   
   Serialize::Out( data, bufferOffset, category );
   Serialize::Out( data, bufferOffset, subCategory );
   Serialize::Out( data, bufferOffset, statType );
   Serialize::Out( data, bufferOffset, statName );

   Serialize::Out( data, bufferOffset, serverReporting );
   Serialize::Out( data, bufferOffset, timestamp );
   Serialize::Out( data, bufferOffset, value );

   return true;
}

///////////////////////////////////////////////////////////////