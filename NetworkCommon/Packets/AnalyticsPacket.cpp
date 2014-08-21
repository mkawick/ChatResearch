#include "AnalyticsPacket.h"

///////////////////////////////////////////////////////////////

bool  PacketAnalytics::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   
   Serialize::In( data, bufferOffset, category, minorVersion );
   Serialize::In( data, bufferOffset, subCategory, minorVersion );
   Serialize::In( data, bufferOffset, statType, minorVersion );
   Serialize::In( data, bufferOffset, statName, minorVersion );

   Serialize::In( data, bufferOffset, serverReporting, minorVersion );
   Serialize::In( data, bufferOffset, timestamp, minorVersion );
   Serialize::In( data, bufferOffset, value, minorVersion );

   return true;
}

bool  PacketAnalytics::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   
   Serialize::Out( data, bufferOffset, category, minorVersion );
   Serialize::Out( data, bufferOffset, subCategory, minorVersion );
   Serialize::Out( data, bufferOffset, statType, minorVersion );
   Serialize::Out( data, bufferOffset, statName, minorVersion );

   Serialize::Out( data, bufferOffset, serverReporting, minorVersion );
   Serialize::Out( data, bufferOffset, timestamp, minorVersion );
   Serialize::Out( data, bufferOffset, value, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////
