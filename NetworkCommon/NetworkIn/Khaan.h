// Khaan.h

#pragma once

#if PLATFORM == PLATFORM_WINDOWS

#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <ws2tcpip.h>

#elif PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX

#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>

#endif

//#include <list>
#include <deque>
#include <algorithm>
//#include <iostream>
//#include <map>
using namespace std;
#pragma warning (disable:4996)

#include <assert.h>

#include <event2/listener.h>
#include <event2/bufferevent.h>
#include "../DataTypes.h"

#include "../ChainedArchitecture/Thread.h"
#include "../ChainedArchitecture/ChainedInterface.h"

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
	
	//-----------------------------------------------
	
	bool	operator == ( const Khaan& RHS );
	bool	operator == ( const Khaan* RHS );
	bool	operator != ( const Khaan& RHS );
	bool	operator != ( const Khaan* RHS );

	//-----------------------------------------------

   bool           Update();

public:

   //void           SetupLibeventCallbacks( event_base* libEvent, int socket );
   void           RegisterToReceiveNetworkTraffic();
   virtual bool	OnDataReceived( unsigned char* data, int length );

   virtual void   UpdateInwardPacketList();// this class doesn't do much with the data. It's up to the derived classes to decide what to do with it
   virtual void   UpdateOutwardPacketList();

   bool           AddInputChainData( BasePacket*, U32 filingData = -1 );
   bool           AddOutputChainData( BasePacket*, U32 filingData = -1 );

   static void    SignalledUpdate( evutil_socket_t fd, short what, void *arg );
protected:
   
   static void    DataAvailable( struct bufferevent* bev, void* arg );
   static void	   HandleSocketError( struct bufferevent* bev, short what, void* arg );
   void           FlushReadBuffer();
   void           CloseConnection();

protected:
	U32				m_socketId;
	bufferevent*	m_bufferEvent;
	sockaddr_in		m_ipAddress;

   deque< BasePacket* > m_packetsOut;
   deque< BasePacket* > m_packetsIn;//ToBeProcessed;
};

///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////
