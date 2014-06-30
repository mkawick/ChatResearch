// NetworkUtils.h

#pragma once

bool     InitializeSockets();
void     ShutdownSockets();
int		SetSocketToNonblock( int ListenSocket );
void     DisableNagle( int socketId );
void     GetLocalIpAddress( char* buffer, size_t buflen );
bool     IsPortBusy( int port );