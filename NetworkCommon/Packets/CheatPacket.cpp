// CheatPacket.cpp

#include "../ServerConstants.h"

#if PLATFORM == PLATFORM_WINDOWS
#pragma warning( disable:4996 )
#endif

#include "CheatPacket.h"



///////////////////////////////////////////////////////////////

bool  PacketCheat::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, whichServer, minorVersion );
   Serialize::In( data, bufferOffset, cheat, minorVersion );

   return true;
}

bool  PacketCheat::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, whichServer, minorVersion );
   Serialize::Out( data, bufferOffset, cheat, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////