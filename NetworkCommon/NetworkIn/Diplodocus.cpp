// Diplodocus.cpp


#include <iostream>
using namespace std;

#include <memory.h>

#include "../Platform.h"
#include "../DataTypes.h"
#include "../Serialize.h"
#include "Diplodocus.h"


#if PLATFORM == PLATFORM_WINDOWS
#pragma comment( lib, "ws2_32.lib" )
#endif
#pragma comment(lib, "libevent.lib")


event_base* BasePacketChainHandler::m_LibEventInstance = NULL;
