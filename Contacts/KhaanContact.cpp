#include <iostream>
#include <memory.h>

#include "../NetworkCommon/ServerConstants.h"

#if PLATFORM == PLATFORM_WINDOWS
#pragma warning( disable:4996 )
#endif

#include "../NetworkCommon/Packets/PacketFactory.h"
#include "../NetworkCommon/Packets/BasePacket.h"
#include "../NetworkCommon/Packets/GamePacket.h"

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

// all code has been moved into the S2S base class.

///////////////////////////////////////////////////////////////////
