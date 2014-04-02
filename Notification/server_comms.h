
#ifndef _H_SERVER_COMMS_
#define _H_SERVER_COMMS_

#if defined(_WIN32)
// Define _WINSOCKAPI_ so windows.h will not include old WinSock header.
#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_
#endif

#include <windows.h>
#include <winsock2.h>

#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <unistd.h>

typedef int SOCKET;
#define SOCKET_ERROR -1

#define closesocket(s)  close(s)
#endif   //_WIN32

int NET_Initialize();
SOCKET InitSocketUDP( unsigned short port_number );
SOCKET InitSocketTCP( unsigned short port_number );

#endif