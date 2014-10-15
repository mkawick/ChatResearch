// NetworkUtils.cpp

#include <iostream>

#include <memory.h>
#include <assert.h>
#include <stdio.h>

#include "Platform.h"

#if PLATFORM == PLATFORM_WINDOWS
   #if _MSC_VER < 1500
   //#include <windows.h>
   #include <winsock2.h>
   #endif

   #include <ws2tcpip.h>

   #pragma comment( lib, "ws2_32.lib" )

   #if (_MSC_VER == 1400) // 2005
   #pragma comment( lib, "libevent2005.lib")
   #elif  !defined ( CUSTOM_LIBEVENT ) // (_MSC_VER == 1500) // 2008
   #pragma comment( lib, "libevent.lib")
   #endif

   #pragma warning( disable : 4996 )
#else
   #include <arpa/inet.h>
   #include <sys/socket.h>
   #include <sys/types.h>
   #include <netdb.h>
   #include <ifaddrs.h>
   #include <netinet/in.h>
   #include <unistd.h>

   #include <errno.h>
#endif

#if PLATFORM == PLATFORM_LINUX
#include <linux/if_link.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include "DataTypes.h"
#include "Packets/Serialize.h"
#include "Utils/Utils.h"
using namespace std;

#include "./Logging/server_log.h"

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
      LogMessage( LOG_PRIO_ERR, "failed to set non-blocking socket\n" );
      return false;
   }

#endif

   return true;
}

void	   SetupListenAddress( struct sockaddr_in& ListenAddress, U16 ServerPort )
{
   memset( &ListenAddress, 0, sizeof(ListenAddress) );
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

void GetLocalIpAddress( char* buffer, size_t buflen ) 
{
   assert( buflen >= 16 );
   struct hostent *hostLocal;

   
#if PLATFORM == PLATFORM_WINDOWS
   const int  MAXHOSTNAMELEN = 256;
   char localHostname[ MAXHOSTNAMELEN ];
   gethostname( localHostname, MAXHOSTNAMELEN );
   if ( ( hostLocal = gethostbyname( localHostname ) ) == NULL ) 
   {  // get the host info
      LogMessage( LOG_PRIO_ERR, "gethostbyname error" );
   }
   struct in_addr **localAddrList = (struct in_addr **)hostLocal->h_addr_list;   
   strncpy( buffer, (char *)inet_ntoa(*localAddrList[0]), buflen );
#else
   struct ifaddrs *addrs, *tmp;
   getifaddrs(&addrs);
   tmp = addrs;

   U32 maxBuffLen = 256;
   char name[maxBuffLen];

   while (tmp) 
   {
       if (tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_INET)
       {
           struct sockaddr_in *pAddr = (struct sockaddr_in *)tmp->ifa_addr;
           strncpy( name, tmp->ifa_name, maxBuffLen );
           strncpy( buffer, inet_ntoa(pAddr->sin_addr), buflen );
       }

       tmp = tmp->ifa_next;
   }

   LogMessage( LOG_PRIO_INFO, "local addr name:%s; addr:%s", name, buffer );

   freeifaddrs(addrs);
#endif
   
}

bool IsPortBusy( int port )
{
   LogMessage( LOG_PRIO_INFO, "IsPortBusy (port : %d)", port );
   char portBuffer [256];
   sprintf( portBuffer, "%d", port );

   struct addrinfo *result = NULL;
   struct addrinfo hints;

   memset(&hints, 0, sizeof(hints));
   hints.ai_family = AF_INET;
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_protocol = IPPROTO_TCP;
   hints.ai_flags = AI_PASSIVE;

   // Resolve the server address and port
   int iResult = getaddrinfo( NULL, portBuffer, &hints, &result );
   if( iResult != 0 ) 
   {
      LogMessage( LOG_PRIO_ERR, "getaddrinfo failed with error: %d", iResult );
      LogMessage( LOG_PRIO_ERR, "Errno: %d", errno );
      return true;
   }

   // Create a SOCKET for connecting to server
   int ListenSocket = socket( result->ai_family, result->ai_socktype, result->ai_protocol );
   if( ListenSocket == SOCKET_ERROR) 
   {
      LogMessage( LOG_PRIO_ERR, "Failed to create socket" );
      LogMessage( LOG_PRIO_ERR, "Errno: %d", errno );
      freeaddrinfo(result);
      return 1;
   }

   // make is sharable so that you can quickly rebind
   int optval = 1;
   iResult = setsockopt( ListenSocket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast< const char*> ( &optval ), sizeof( optval ) );
   if( iResult == SOCKET_ERROR ) 
   {
      LogMessage( LOG_PRIO_ERR, "Failed to setsockopt" );
      LogMessage( LOG_PRIO_ERR, "Errno: %d", errno );
      freeaddrinfo(result);
      closesocket( ListenSocket );
      return true;
   }

   // Setup the TCP listening socket
   iResult = bind( ListenSocket, result->ai_addr, (int)result->ai_addrlen);
   if( iResult == SOCKET_ERROR ) 
   {
      LogMessage( LOG_PRIO_ERR, "Failed to bind socket" );
      LogMessage( LOG_PRIO_ERR, "Errno: %d", errno );
      freeaddrinfo(result);
      closesocket( ListenSocket );
      return true;
   }

   freeaddrinfo(result);

   int NumConnections = 5;
   iResult = listen( ListenSocket, NumConnections );
   if( iResult == SOCKET_ERROR ) 
   {
      LogMessage( LOG_PRIO_ERR, "Failed to listen to socket" );
      LogMessage( LOG_PRIO_ERR, "Errno: %d", errno );
      closesocket( ListenSocket );
      return true;
   }

   closesocket( ListenSocket );

   LogMessage( LOG_PRIO_INFO, "Port is free (port : %d)", port );

   return false;
}