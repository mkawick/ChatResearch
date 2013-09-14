// Diplodocus.cpp


#include <iostream>
#include <memory.h>

#include "../Platform.h"
#include "../DataTypes.h"
#include "../Serialize.h"
#include "Diplodocus.h"

using namespace std;

#if PLATFORM == PLATFORM_WINDOWS
#pragma comment( lib, "ws2_32.lib" )
#pragma comment(lib, "libevent.lib")
#endif



event_base* BasePacketChainHandler::m_LibEventInstance = NULL;
