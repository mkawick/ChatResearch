// Khaan.cpp

#include <event2/event.h>
#include <event2/buffer.h>

#include <iostream>

#include "../ServerConstants.h"
#include "../Packets/BasePacket.h"
#include "../Packets/ServerToServerPacket.h"
#include "../Packets/PacketFactory.h"
#include "../Utils/CommandLineParser.h"
#include "../Utils/Utils.h"

#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
using boost::format;

#include "Khaan.h"
#include "Diplodocus.h"

//-----------------------------------------------------------------------

Khaan :: Khaan() : ChainedInterface< BasePacket* >(), 
                  m_socketId (0), 
                  m_bufferEvent(NULL), 
                  m_timeOfConnection( 0 ),
                  m_useLibeventToSend( true ),
                  m_criticalFailure( false )
{
}

Khaan ::Khaan( int socketId, bufferevent* be, int connectionId ) : ChainedInterface< BasePacket* >(), 
                                                                  m_socketId( socketId ), 
                                                                  m_bufferEvent( be ), 
                                                                  m_timeOfConnection( 0 ),
                                                                  m_useLibeventToSend( true ),
                                                                  m_criticalFailure( false )
{
   SetConnectionId( connectionId );
}

//-----------------------------------------------------------------------

Khaan ::~Khaan()
{
   bufferevent_free( GetBufferEvent() );
#if PLATFORM == PLATFORM_WINDOWS
   closesocket( GetSocketId() );
#elif PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX
   close( GetConnectionId() );
#endif
}

//-----------------------------------------------------------------------
//-----------------------------------------------------------------------------

void   Khaan ::PreCleanup()
{
   if( m_listOfOutputs.size() == 0 )
      return;

   ChainLinkIteratorType output = m_listOfOutputs.begin();

   IChainedInterface* chain = (*output).m_interface;
   if( chain )
   {
      Diplodocus <Khaan> * server = static_cast< Diplodocus <Khaan> * >( chain );
      server->InputRemovalInProgress( this );
   }
}

//-----------------------------------------------------------------------------

void	Khaan :: SetIPAddress( const sockaddr_in& addr )
{
	m_ipAddress = addr;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

bool	Khaan :: OnDataReceived( unsigned char* data, int length )
{
   if( m_criticalFailure )
      return false;

   BasePacket* packetIn = NULL;
   int offset = 0;
   PacketFactory parser;
   if( parser.Parse( data, offset, &packetIn ) == true )
   {
      m_packetsIn.push_back( packetIn );
 
      ChainLinkIteratorType itOutputs = m_listOfOutputs.begin();
      while( itOutputs != m_listOfOutputs.end() )
      {
         ChainLink& chain = *itOutputs++;
         IChainedInterface* interfacePtr = chain.m_interface;

         ThreadEvent te;
         te.type = ThreadEvent_NeedsService;
         te.identifier = m_chainId;
         static_cast< ChainType*> ( interfacePtr )->PushInputEvent( &te );
      }
   }

   if( packetIn )
      delete packetIn;

   return true;
}

//-----------------------------------------------------------------------------

bool	Khaan :: Update()
{
   UpdateInwardPacketList();
   UpdateOutwardPacketList();

   // I think that this makes sense
   CleanupAllEvents();

   if( m_packetsOut.size() || m_packetsIn.size() )// we didn't finish
   {
      cout << "Remaining packet out count: " << m_packetsOut.size() << endl;
      return false;
   }

   return true;
}

//------------------------------------------------------------------------------

void	Khaan :: UpdateInwardPacketList()
{
   if( m_packetsIn.size() == 0 )
      return;

   int numOutputs = static_cast<int>( m_listOfOutputs.size() );
   if( numOutputs > 1 )
   {
      assert( 0 );// need support for multiple outputs, each packet should be copied because of the memory ownership, or use shared pointers
   }

   ChainLinkIteratorType output = m_listOfOutputs.begin();
   IChainedInterface* chain = (*output).m_interface;

   if( chain )
   {
      while( m_packetsIn.size() > 0 )
      {
         BasePacket* packet = m_packetsIn.front();
      
         TrackInwardPacketType( packet );

         Threading::MutexLock  locker( m_inputChainListMutex );
         static_cast< ChainType*> ( chain )->AddInputChainData( packet, m_socketId );

         m_packetsIn.pop_front();
      }
   }
}

//------------------------------------------------------------------------------

int	Khaan :: SendData( const U8* buffer, int length )
{
   if( m_useLibeventToSend )
   {
      bufferevent*	bev = GetBufferEvent();
      struct evbuffer* outputBuffer = bufferevent_get_output( bev );
      evbuffer_lock( outputBuffer );

      if( outputBuffer == NULL )
      {
         m_criticalFailure = true;
         return false;
      }

      int result = evbuffer_add( outputBuffer, buffer, length );

      evbuffer_unlock( outputBuffer );
      return result;
   }


   return send( m_socketId, (const char* )buffer, length, 0 );

}

//------------------------------------------------------------------------------

void	Khaan :: UpdateOutwardPacketList()
{
   if( m_packetsOut.size() == 0 )
      return;

   int length;
   int numPacketsPackaged = 0;
   int offset = 0;

   U8 buffer[ MaxBufferSize + 1024 ];
   PacketFactory factory;

   int totalBytesLeftToWrite = MaxBufferSize + 1024;

   // todo, plan for the degenerate case where a single packet is over 2k
   m_outputChainListMutex.lock();
   while( m_packetsOut.size() ) 
   {
      offset = 0;
      U16 sizeOfLastWrite;
      int sizeOfHeader = sizeof( sizeOfLastWrite );
      length = sizeOfHeader;// reserve space

      BasePacket* packet = m_packetsOut.front();      
      packet->SerializeOut( buffer, length );
      
      totalBytesLeftToWrite -= length;
      if( totalBytesLeftToWrite < 0 )
         break;

      sizeOfLastWrite = length - sizeOfHeader;
      Serialize::Out( buffer, offset, sizeOfLastWrite );// write in the size

      SendData( buffer, length );
      m_packetsOut.pop_front();
      TrackOutwardPacketType( packet );
      factory.CleanupPacket( packet );
   }
   m_outputChainListMutex.unlock();   
}

//---------------------------------------------------------------

bool  Khaan :: AddInputChainData( BasePacket*, U32 filingData ) 
{ 
   return false; 
}

//---------------------------------------------------------------

bool Khaan :: AddOutputChainData( BasePacket* packet, U32 filingData ) 
{ 
   if( m_criticalFailure )
      return false;

   m_outputChainListMutex.lock();
   m_packetsOut.push_back( packet );
   m_outputChainListMutex.unlock();
   return true; 
}

//-----------------------------------------------------------------------------

void  Khaan :: OnDataWritten( struct bufferevent *bev, void *user_data ) 
{
   //struct evbuffer *output = bufferevent_get_output(bev);
   //output->open();
    /* if (evbuffer_get_length(output) == 0) {
         printf("flushed answer\n");
         //bufferevent_free(bev);
     }*/
}
//---------------------------------------------------------------
//---------------------------------------------------------------

void     Khaan :: RegisterToReceiveNetworkTraffic()
{
   bufferevent_setcb( GetBufferEvent(), 
                      OnDataAvailable, 
                      OnDataWritten, 
                      OnSocketError, 
                      this );

   // We have to enable it before our callbacks will be called. 
   bufferevent_enable( GetBufferEvent(), EV_READ | EV_WRITE );

   struct evbuffer* outputBuffer = bufferevent_get_output( GetBufferEvent() );
   evbuffer_enable_locking( outputBuffer, NULL );

   DisableNagle( m_socketId );

   PreStart();

   time( &m_timeOfConnection );
}

//---------------------------------------------------------------
//---------------------------------------------------------------

// Called by libevent when there is data to read.
void     Khaan :: OnDataAvailable( struct bufferevent* bufferEventObj, void* arg )
{
   const U32   MaxBufferSize = 12*1024;// allowing for massive, reads that should never happen
   
   Khaan*      This = (Khaan*) arg;
   U8          data[ MaxBufferSize ];
   size_t      numBytesReceived;

   /* Read 12k at a time and send it to all connected clients. */
   while( 1 )
   {
      numBytesReceived = bufferevent_read( bufferEventObj, data, sizeof( data ) );
      if( numBytesReceived <= 0 ) // nothing received
      {
         break;
      }

      if( This )
      {
         This->OnDataReceived( data, static_cast< int>( numBytesReceived ) );
      }
   }
}

//---------------------------------------------------------------

void     Khaan :: FlushReadBuffer()
{
   bufferevent_flush( GetBufferEvent(), EV_READ, BEV_FINISHED );
}

//---------------------------------------------------------------

void     Khaan :: CloseConnection()
{
   if( GetBufferEvent() )
   {
      bufferevent_free( GetBufferEvent() );
      Cleanup();
   }
}

//---------------------------------------------------------------

// Called by libevent when there is an error on the underlying socket descriptor.
void	   Khaan::OnSocketError( struct bufferevent* bufferEventObj, short events, void* context )
{
   Khaan* khaan = (Khaan*) context;

   if (events & ( BEV_EVENT_EOF | BEV_EVENT_ERROR) )
   {
      /* Client disconnected, remove the read event and the
      * free the Khaan structure. */
      cout << "Client disconnected." << endl;
   }
   else 
   {
      cout << "Client socket error, disconnecting." << endl;
   }

   khaan->Cleanup();
   delete khaan;
}

//-----------------------------------------------------------------------------

void    Khaan :: Cleanup()
{
   PreCleanup();

   CleanupAllChainDependencies();

   // flush all pending packets
   ClearAllPacketsIn();
   ClearAllPacketsOut();
   //bufferevent_free( GetBufferEvent() );
   //m_bufferEvent = NULL;
}

//---------------------------------------------------------------

void     Khaan :: ClearAllPacketsIn()
{
   while( m_packetsIn.size() )
   {
      BasePacket* packet = m_packetsIn.front();
      m_packetsIn.pop_front();
      delete packet;
   }
}

//---------------------------------------------------------------

void     Khaan :: ClearAllPacketsOut()
{
   while( m_packetsOut.size() )
   {
      BasePacket* packet = m_packetsOut.front();
      m_packetsOut.pop_front();
      delete packet;
   }
}

//---------------------------------------------------------------


//---------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
