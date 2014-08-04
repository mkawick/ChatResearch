// Khaan.cpp

#include <event2/event.h>
#include <event2/buffer.h>

#include <iostream>

#include "../ServerConstants.h"
#include "../NetworkUtils.h"

#if PLATFORM == PLATFORM_WINDOWS
#pragma warning( disable:4996 )
#endif

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
                  m_listeningPort( 0 ),
                  m_timeOfConnection( 0 ),
                  m_useLibeventToSend( true ),
                  m_criticalFailure( false ),
                  m_isInTelnetMode( false ),
                  m_isDisconnected( false ),
                  m_outboundBuffer( NULL ),
                  m_isExpectingMoreDataInPreviousPacket( false ),
                  m_expectedBytesReceivedSoFar( 0 ),
                  m_expectedBytes( 0 )
                  
{
   m_tempBuffer[0] = 0;
   m_tempBuffer[1] = 0;
   SetOutboudBufferSize( MaxBufferSize + 1024 );
}

Khaan ::Khaan( int socketId, bufferevent* be, int connectionId ) : ChainedInterface< BasePacket* >(), 
                                                                  m_socketId( socketId ), 
                                                                  m_bufferEvent( be ), 
                                                                  m_listeningPort( 0 ),
                                                                  m_timeOfConnection( 0 ),
                                                                  m_useLibeventToSend( true ),
                                                                  m_criticalFailure( false ),
                                                                  m_isInTelnetMode( false ),
                                                                  m_isDisconnected( false ),
                                                                  m_outboundBuffer( NULL ),
                                                                  m_isExpectingMoreDataInPreviousPacket( false ),
                                                                  m_expectedBytesReceivedSoFar( 0 ),
                                                                  m_expectedBytes( 0 )
{
   m_tempBuffer[0] = 0;
   m_tempBuffer[1] = 0;

   SetConnectionId( connectionId );
   SetOutboudBufferSize( MaxBufferSize + 1024 );
}

//-----------------------------------------------------------------------

Khaan ::~Khaan()
{
   delete [] m_outboundBuffer;
   if( GetBufferEvent() )
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
   m_isDisconnected = true;
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

bool	Khaan :: OnDataReceived( const U8* data, int length )
{
   if( m_criticalFailure || m_isDisconnected )
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
   if( m_isDisconnected )
      return false;

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

void  Khaan :: SetOutboudBufferSize( U32 size )
{
   if( size < 1024 )
      size = 1024;
   if( size > MaxBufferSize )
      size = MaxBufferSize;

   m_maxBytesToSend = size;
   delete [] m_outboundBuffer;
   m_outboundBuffer = new U8[ m_maxBytesToSend ];
}

//------------------------------------------------------------------------------

void	Khaan :: UpdateInwardPacketList()
{
   if( m_packetsIn.size() == 0 || m_isDisconnected )
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
   if( m_isDisconnected )
      return -1;

   if( m_useLibeventToSend )
   {
      bufferevent*	bev = GetBufferEvent();
      int result = -1;
      if( bev )
      {
         struct evbuffer* outputBuffer = bufferevent_get_output( bev );

         if( outputBuffer == NULL )
         {
            m_criticalFailure = true;
            return false;
         }

         // buffer locks happen internally
         result = evbuffer_add( outputBuffer, buffer, length );
      
      }
      return result;
   }


   return send( m_socketId, (const char* )buffer, length, 0 );
}

//------------------------------------------------------------------------------

bool  Khaan :: HandleTelnetModeData( const U8* data, int length )
{
   // eventually, we will add specialized commands and handling. 
   // now, we just echo.
   SendData( data, length );

   if( *data == 3 || // ctrl-c
      *data == 27 ) // escape
   {
      string notice( "\r\nTELNET->exit()\r\n");
      SendData( reinterpret_cast< const U8* >( notice.c_str() ), notice.size() );
      CloseConnection();
   }
   return true;
}

//------------------------------------------------------------------------------

void  Khaan::SendTelnetInstructions()
{
   string notice( "\r\n->TELNET\r\n");
   SendData( reinterpret_cast< const U8* >( notice.c_str() ), notice.size() );
   notice = "  *** you have entered telnet mode.            *** \r\n";
   SendData( reinterpret_cast< const U8* >( notice.c_str() ), notice.size() );
   notice = "  *** all things that you type will be echoed. *** \r\n";
   SendData( reinterpret_cast< const U8* >( notice.c_str() ), notice.size() );
   notice = "  *** crl-c and esc both exit.                 ***\r\n";
   SendData( reinterpret_cast< const U8* >( notice.c_str() ), notice.size() );
}

//------------------------------------------------------------------------------

int	Khaan :: UpdateOutwardPacketList()
{
   if( m_packetsOut.size() == 0 || m_isDisconnected )
      return 0;

   int length;
   int numPacketsPackaged = 0;
   int offset = 0;

   
   PacketFactory factory;

   int totalBytesLeftToWrite = m_maxBytesToSend;
   U16 sizeOfLastWrite;
   int sizeOfHeader = sizeof( sizeOfLastWrite );
   int sizeSent = 0;

   while( m_packetsOut.size() ) 
   {
      offset = 0;
      
      length = sizeOfHeader;// reserve space

      BasePacket* packet = m_packetsOut.front();      
      packet->SerializeOut( m_outboundBuffer, length );
    /*  if( packet->packetType == 9 )
      {
         sizeOfHeader = sizeOfHeader;
      }*/
      
      totalBytesLeftToWrite -= length;
      if( totalBytesLeftToWrite < 0 )
         break;
      sizeSent += length + sizeOfHeader;

      sizeOfLastWrite = length - sizeOfHeader;
      Serialize::Out( m_outboundBuffer, offset, sizeOfLastWrite );// write in the size

      SendData( m_outboundBuffer, length );
      m_packetsOut.pop_front();
      TrackOutwardPacketType( packet ); 
      factory.CleanupPacket( packet );
   }

   return sizeSent;
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

  // m_outputChainListMutex.lock();
   m_packetsOut.push_back( packet );
  // m_outputChainListMutex.unlock();
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
   if( GetBufferEvent() == NULL )
      return;

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
   Khaan*      This = (Khaan*) arg;
   if( This->m_isDisconnected )
      return;

   //cout << "Data avail:" << endl;
   struct evbuffer *input = bufferevent_get_input( bufferEventObj );
   size_t readLength = evbuffer_get_length( input );
   const U32   MaxBufferSize = 12*1024;// allowing for massive, reads that should never happen
   
   if( This == NULL )
      return;
   
   if( readLength > MaxBufferSize ) // this is highly unlikely
   {
      U8* tempBuffer = new U8 [ readLength ];

      size_t      numBytesReceived = bufferevent_read( bufferEventObj, tempBuffer, readLength );
      This->OnDataReceived( tempBuffer, static_cast< int>( readLength ) );

      delete [] tempBuffer;
      
   }
   else
   {
      U8          dataBuffer[ MaxBufferSize ];
      size_t      numBytesReceived, totalBytes = 0;

      do
      {
         numBytesReceived = bufferevent_read( bufferEventObj, dataBuffer+totalBytes, sizeof( dataBuffer ) );
         totalBytes += numBytesReceived;
      } while( numBytesReceived > 0 );
      
      
      if( totalBytes > 0 && m_isDisconnected == false )
      {
       /*  cout << "data=[" << endl;
         for( U32 i=0; i<totalBytes; i++ )
         {
            cout << (int)( dataBuffer[i] ) << " ";
         }
         cout << "]" << endl;*/

         This->OnDataReceived( dataBuffer, static_cast< int>( totalBytes ) );
      }
   }
}

//---------------------------------------------------------------

void     Khaan :: FlushReadBuffer()
{
   if( GetBufferEvent() && m_isDisconnected == false )
   bufferevent_flush( GetBufferEvent(), EV_READ, BEV_FINISHED );
}

//---------------------------------------------------------------

void     Khaan :: CloseConnection()
{
   m_isDisconnected = true;
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

   m_bufferEvent = NULL;
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
