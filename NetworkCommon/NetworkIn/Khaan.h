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
#include "../Utils/KeepAliveSignaller.h"

using namespace std;

///////////////////////////////////////////////////////////////

class BasePacket;
typedef ChainedInterface < BasePacket* >  BasePacketChainedType;
//-----------------------------------------------------------------------------------

// todo, add timeout feature in case the connection starts but we never receive the login packet.
class Khaan : public BasePacketChainedType
{
public:
   
	Khaan( int socketId, bufferevent* be, int connectionId = 0 );
	virtual ~Khaan();

   const char* GetClassName() const { return "Khaan"; }

   virtual void   PreCleanup();
   virtual void   PreStart(){}
   void           Cleanup();
   void           ClearAllPacketsIn();
   void           ClearAllPacketsOut();

	//-----------------------------------------------

   bool           IsConnected() const { return m_isDisconnected == false; }
	U32				GetSocketId() const { return m_socketId; }
	bufferevent*	GetBufferEvent() const { return m_bufferEvent; }

	void				SetIPAddress( const sockaddr_in& addr );
	const sockaddr_in&	GetIPAddress() const { return m_ipAddress; }

   void           SetPort( U16 port ) { m_listeningPort = port; }
   U16            GetPort() const { return m_listeningPort; }

   void           RequireKeepAlive( bool isRequired ) { m_keepAlive.Enable( isRequired ); }
   void           SetKeepAliveTimeoutInSeconds( int seconds ) { m_keepAlive.SetTimeout( seconds ); }

   time_t         GetConnectionTime() const { return m_timeOfConnection; }

   void           SetTimeForDeletion( time_t& currentTime ) { m_timeOfDisconnection = currentTime; }
   time_t         GetDisconnectTime() const { return m_timeOfDisconnection; }
   bool           HasDisconnected() const { return m_timeOfDisconnection != 0; }
   bool           HasDeleteTimeElapsed( time_t& currentTime, int testTime = 5 ) const ;
   
   void           CloseConnection();
	
	//-----------------------------------------------

   bool           Update();
   bool           NeedsUpdate() const { return m_packetsOut.size() != 0; }

   void           SendThroughLibEvent( bool useLibeventToSend = true ) { m_useLibeventToSend = useLibeventToSend; }
   void           SetOutboudBufferSize( U32 size );
public:

   virtual U32    GetServerId() { return 0; } // only for inheritance purposes. KS2S implements this
   void           RegisterToReceiveNetworkTraffic();
   virtual bool	OnDataReceived( const U8* data, int length );
   //bool           HandleKeepAlivePackets( const BasePacket* packetIn );

   virtual void   UpdateInwardPacketList();// this class doesn't do much with the data. It's up to the derived classes to decide what to do with it
   virtual int    UpdateOutwardPacketList();

   virtual bool   TrackInwardPacketType( const BasePacket* packet ) { return false; }
   virtual bool   TrackOutwardPacketType( const BasePacket* packet ) { return false; }
   bool           AddInputChainData( BasePacket*, U32 filingData = -1 );
   bool           AddOutputChainData( BasePacket*, U32 filingData = -1 );
   bool           AddOutputChainDataNoLock( BasePacket* );

   int	         SendData( const U8* buffer, int length );
   static void    SignalledUpdate( evutil_socket_t fd, short what, void *arg );

   virtual void   DenyAllFutureData();
protected:
   
   static void    OnDataAvailable( struct bufferevent* bev, void* This );
   static void	   OnSocketError( struct bufferevent* bev, short what, void* This );
   static void    OnDataWritten( struct bufferevent *bev, void *This );
   void           FlushReadBuffer();

   bool           HandleTelnetModeData( const U8* data, int length );
   void           SendTelnetInstructions();

   //bool           HasKeepAliveExpired() const;
   //void           SendKeepAlive();

protected:
	U32				m_socketId;
	bufferevent*	m_bufferEvent;
	sockaddr_in		m_ipAddress;
   U16            m_listeningPort;

   time_t         m_timeOfConnection;
   time_t         m_timeOfDisconnection;
   /*time_t         m_keepAliveLastSentTime;
   int            m_secondsBeforeTimeout;
   int            m_keepAlivePacketCounter;
   bool           m_isAwaitingKeepAliveReturn;
   bool           m_requiresKeepAlive;*/
   KeepAliveSignaller   m_keepAlive;
   U32            m_maxBytesToSend;

   bool           m_useLibeventToSend;
   bool           m_criticalFailure;
   bool           m_denyAllFutureData;

   bool           m_isDisconnected;
   bool           m_isInTelnetMode;
   bool           m_isExpectingMoreDataInPreviousPacket;

   bool           m_hasPacketsReceived;
   bool           m_hasPacketsToSend;

   int            m_expectedBytesReceivedSoFar;
   int            m_expectedBytes;

   U8             m_versionNumberMinor;
   U8             m_tempBuffer[ MaxBufferSize ];
   U8*            m_outboundBuffer;


   deque< BasePacket* > m_packetsOut;
   deque< BasePacket* > m_packetsIn;//ToBeProcessed;

private:
   Khaan();
   Khaan( const Khaan& );
   const Khaan& operator = ( const Khaan& );
};

///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////
