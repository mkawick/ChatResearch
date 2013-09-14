// CheatPacket.cpp

#include "CheatPacket.h"



///////////////////////////////////////////////////////////////

bool  PacketCheat::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, whichServer );
   Serialize::In( data, bufferOffset, cheat );

   return true;
}

bool  PacketCheat::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, whichServer );
   Serialize::Out( data, bufferOffset, cheat );

   return true;
}

///////////////////////////////////////////////////////////////