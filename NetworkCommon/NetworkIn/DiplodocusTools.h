#pragma once

// DiplodocusTools.h

#include "../Packets/LoginPacket.h"

void  LogMessage_LoginPacket( const PacketPrepareForUserLogin* loginPacket );
void  LogMessage_LogoutPacket( const PacketPrepareForUserLogout* loginPacket );