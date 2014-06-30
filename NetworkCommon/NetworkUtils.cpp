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
   #include <sys/socket.h>
   #include <netinet/in.h>
   #include <arpa/inet.h>
#endif


#include "DataTypes.h"
#include "Packets/Serialize.h"
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

#define  MAXHOSTNAMELEN  256
   char localHostname[ MAXHOSTNAMELEN ];
   gethostname( localHostname, MAXHOSTNAMELEN );
   if ( ( hostLocal = gethostbyname( localHostname ) ) == NULL ) 
   {  // get the host info
      cout << "gethostbyname error" << endl;
   }
   struct in_addr **localAddrList = (struct in_addr **)hostLocal->h_addr_list;   
   strncpy( buffer, (char *)inet_ntoa(*localAddrList[0]), buflen );
}

bool IsPortBusy( int port )
{
   int ListenSocket = INVALID_SOCKET;

   char portBuffer [256];
   itoa( port, portBuffer, 10 );

   struct addrinfo *result = NULL;
   struct addrinfo hints;

   ZeroMemory(&hints, sizeof(hints));
   hints.ai_family = AF_INET;
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_protocol = IPPROTO_TCP;
   hints.ai_flags = AI_PASSIVE;

   // Resolve the server address and port
   int iResult = getaddrinfo(NULL, portBuffer, &hints, &result);
   if( iResult != 0 ) 
   {
     cout << "getaddrinfo failed with error: " << iResult << endl;
     return true;
   }

   // Create a SOCKET for connecting to server
   ListenSocket = socket( result->ai_family, result->ai_socktype, result->ai_protocol );
   if( ListenSocket == INVALID_SOCKET) 
   {
      cout << "Failed to create socket" << endl;
      freeaddrinfo(result);
      return 1;
   }

   // Setup the TCP listening socket
   iResult = bind( ListenSocket, result->ai_addr, (int)result->ai_addrlen);
   if( iResult == SOCKET_ERROR ) 
   {
      cout << "Failed to bind socket" << endl;
      freeaddrinfo(result);
      closesocket( ListenSocket );
      return true;
   }

   freeaddrinfo(result);

   int NumConnections = 5;
   iResult = listen( ListenSocket, NumConnections );
   if( iResult == SOCKET_ERROR ) 
   {
      cout << "Failed to listen to socket" << endl;
      closesocket( ListenSocket );
      return true;
   }

   closesocket( ListenSocket );

   return false;
}