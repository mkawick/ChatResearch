// Diplodocus.cpp

#include <iostream>

#include <memory.h>
#include <stdio.h>

#if PLATFORM == PLATFORM_WINDOWS
#pragma comment( lib, "ws2_32.lib" )
#endif
#pragma comment(lib, "libevent.lib")


#include "Platform.h"
#include "DataTypes.h"
#include "Serialize.h"
using namespace std;

// prototypes


//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------

//                                      utility functions 

//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------

//event_base* BasePacketChainHandler::m_LibEventInstance = NULL;

// Set a socket to non-blocking mode.
int		SetSocketToNonblock( int ListenSocket )
{
#if PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX

   int nonBlocking = 1;
   if ( fcntl( ListenSocket, F_SETFL, O_NONBLOCK, nonBlocking ) == -1 )
   {
      printf( "failed to set non-blocking socket\n" );
      return false;
   }

#elif PLATFORM == PLATFORM_WINDOWS

   DWORD nonBlocking = 1;
   if ( ioctlsocket( ListenSocket, FIONBIO, &nonBlocking ) != 0 )
   {
      printf( "failed to set non-blocking socket\n" );
      return false;
   }

#endif

   return true;
}

void	   SetupListenAddress( struct sockaddr_in& ListenAddress, U16 ServerPort )
{
   memset(&ListenAddress, 0, sizeof(ListenAddress));
   ListenAddress.sin_family = AF_INET;
   ListenAddress.sin_addr.s_addr = INADDR_ANY;
   ListenAddress.sin_port = htons( ServerPort );
}


bool InitializeSockets()
{
   #if PLATFORM == PLATFORM_WINDOWS
      WSADATA WsaData;
      return WSAStartup( MAKEWORD(2,2), &WsaData ) == NO_ERROR;

   #elif PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX
      return true;
   #endif
}

void ShutdownSockets()
{
   #if PLATFORM == PLATFORM_WINDOWS
      WSACleanup();
   #endif
}

void  DisableNagle( int socketId )
{
   const char nagleOff = 1;
	//int error =
   setsockopt( socketId, IPPROTO_TCP, TCP_NODELAY, &nagleOff, sizeof( nagleOff ) );
   //assert(error == 0);
   /*
#ifdef __APPLE__
    // We want failed writes to return EPIPE, not deliver SIGPIPE. See TCPConnection::write.
    error = setsockopt(socket, SOL_SOCKET, SO_NOSIGPIPE, &nagleOff, sizeof(nagleOff));
    ASSERT(error == 0);
#endif*/
}
