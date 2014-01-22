// Khaan.h

#pragma once

#ifndef PLATFORM
#include "../Platform.h"
#endif

#if PLATFORM == PLATFORM_WINDOWS

#undef UNICODE
#define WIN32_LEAN_AND_MEAN

#if _MSC_VER < 1500
//#include <windows.h>
#include <winsock2.h>
#endif

#include <ws2tcpip.h>
#pragma warning (disable:4996)

#elif PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX

#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>

#endif


#include <deque>
#include <algorithm>
#include <assert.h>

#include <event2/listener.h>
#include <event2/bufferevent.h>
#include "../DataTypes.h"

#include "../ChainedArchitecture/Thread.h"
#include "../ChainedArchitecture/ChainedInterface.h"

using namespace std;

///////////////////////////////////////////////////////////////

class BasePacket;
//-----------------------------------------------------------------------------------

// todo, add timeout feature in case the connection starts but we never receive the login packet.
class Khaan : public ChainedInterface < BasePacket* >
{
public:
   Khaan();
	Khaan( int socketId, bufferevent* be, int connectionId = 0 );
	virtual ~Khaan();

   virtual void   PreCleanup();
   virtual void   PreStart(){}
   void           Cleanup();
   void           ClearAllPacketsIn();
   void           ClearAllPacketsOut();

	//-----------------------------------------------

	U32				GetSocketId() const { return m_socketId; }
	bufferevent*	GetBufferEvent() const { return m_bufferEvent; }

	void				SetIPAddress( const sockaddr_in& addr );
	const sockaddr_in&	GetIPAddress() const { return m_ipAddress; }

   void           SetPort( U16 port ) { m_listeningPort = port; }
   U16            GetPort() const { return m_listeningPort; }
	
	//-----------------------------------------------

   bool           Update();

   void           SendThroughLibEvent( bool useLibeventToSend = true ) { m_useLibeventToSend = useLibeventToSend; }

public:

   //void           SetupLibeventCallbacks( event_base* libEvent, int socket );
   void           RegisterToReceiveNetworkTraffic();
   virtual bool	OnDataReceived( unsigned char* data, int length );

   virtual void   UpdateInwardPacketList();// this class doesn't do much with the data. It's up to the derived classes to decide what to do with it
   virtual void   UpdateOutwardPacketList();

   virtual bool   TrackInwardPacketType( const BasePacket* packet ) { return false; }
   virtual bool   TrackOutwardPacketType( const BasePacket* packet ) { return false; }
   bool           AddInputChainData( BasePacket*, U32 filingData = -1 );
   bool           AddOutputChainData( BasePacket*, U32 filingData = -1 );

   int	         SendData( const U8* buffer, int length );
   static void    SignalledUpdate( evutil_socket_t fd, short what, void *arg );
protected:
   
   static void    OnDataAvailable( struct bufferevent* bev, void* This );
   static void	   OnSocketError( struct bufferevent* bev, short what, void* This );
   static void    OnDataWritten( struct bufferevent *bev, void *This );
   void           FlushReadBuffer();
   void           CloseConnection();

protected:
	U32				m_socketId;
	bufferevent*	m_bufferEvent;
	sockaddr_in		m_ipAddress;
   U16            m_listeningPort;

   bool           m_useLibeventToSend;

   deque< BasePacket* > m_packetsOut;
   deque< BasePacket* > m_packetsIn;//ToBeProcessed;
};

///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////
