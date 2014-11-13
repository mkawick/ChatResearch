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
#include "../Utils/StringUtils.h"

#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
using boost::format;

#include "Khaan.h"
#include "Diplodocus.h"

/////////////////////////////////////////////////////////////////

Khaan ::Khaan( int socketId, bufferevent* be, int connectionId ) : ChainedInterface< BasePacket* >(), 
                                                                  m_socketId( socketId ), 
                                                                  m_bufferEvent( be ), 
                                                                  m_listeningPort( 0 ),
                                                                  m_timeOfConnection( 0 ),
                                                                  m_timeOfDisconnection( 0 ),
                                                                  m_maxBytesToSend( 2 * 1024 ),
                                                                  m_useLibeventToSend( true ),
                                                                  m_criticalFailure( false ),
                                                                  m_denyAllFutureData( false ),
                                                                  m_isDisconnected( false ),
                                                                  m_isInTelnetMode( false ),
                                                                  m_isExpectingMoreDataInPreviousPacket( false ),
                                                                  m_hasPacketsReceived( false ),
                                                                  m_hasPacketsToSend( false ),
                                                                  m_expectedBytesReceivedSoFar( 0 ),
                                                                  m_expectedBytes( 0 ),
                                                                  m_versionNumberMinor( NetworkVersionMinor ),
                                                                  m_outboundBuffer( NULL )
{
   //LogMessage( LOG_PRIO_INFO, "Khaan 1" );
   m_chainedType = ChainedType_InboundSocketConnector;
   m_tempBuffer[0] = 0;
   m_tempBuffer[1] = 0;

   SetConnectionId( connectionId );
   SetOutboudBufferSize( MaxBufferSize + 1024 );
}

/////////////////////////////////////////////////////////////////

Khaan ::~Khaan()
{
   LogMessage( LOG_PRIO_INFO, "Khaan::d'tor" );
   delete [] m_outboundBuffer;

    CloseConnection();

#if PLATFORM == PLATFORM_WINDOWS
   closesocket( GetSocketId() );
#elif PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX
   close( GetConnectionId() );
#endif
}

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

void     Khaan::DenyAllFutureData() 
{ 
   //LogMessage( LOG_PRIO_INFO, "Khaan 3" );
   m_denyAllFutureData = true; 
}

void   Khaan ::PreCleanup()
{
   //LogMessage( LOG_PRIO_INFO, "Khaan 4" );
   m_isDisconnected = true;
   DenyAllFutureData();
   if( HasDisconnected() == false )
   {
      time_t currentTime;
      time( &currentTime );
      SetTimeForDeletion( currentTime );
   }
}

/////////////////////////////////////////////////////////////////

void	Khaan :: SetIPAddress( const sockaddr_in& addr )
{
   ////LogMessage( LOG_PRIO_INFO, "Khaan 5" );
	m_ipAddress = addr;
}

/////////////////////////////////////////////////////////////////

bool     Khaan :: HasDeleteTimeElapsed( time_t& currentTime, int testTime ) const
{
   if( m_timeOfDisconnection == 0 )
      return false;

   //LogMessage( LOG_PRIO_INFO, "Khaan 6 %ul ,  %ul : %d  ", currentTime, m_timeOfDisconnection, testTime );

   if( difftime( currentTime, m_timeOfDisconnection ) >= testTime ) // once per second
   {
      return true;
   }
   return false;
}

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

bool	Khaan :: OnDataReceived( const U8* data, int length )
{
   if( m_criticalFailure || m_isDisconnected )
      return false;

   //LogMessage( LOG_PRIO_INFO, "Khaan 7" );

   BasePacket* packetIn = NULL;
   int offset = 0;
   PacketFactory parser;
   if( parser.Parse( data, offset, &packetIn, m_versionNumberMinor ) == true )
   {
      Threading::MutexLock  locker( m_inputChainListMutex );
         m_packetsIn.push_back( packetIn );
         m_hasPacketsReceived = true;
 
     /* ChainLinkIteratorType itOutputs = m_listOfOutputs.begin();
      while( itOutputs != m_listOfOutputs.end() )
      {
         ChainLink& chain = *itOutputs++;
         IChainedInterface* interfacePtr = chain.m_interface;

         ThreadEvent te;
         te.type = ThreadEvent_NeedsService;
         te.identifier = m_chainId;
         static_cast< ChainType*> ( interfacePtr )->PushInputEvent( &te );
      }*/
   }

 /*  if( packetIn )
   {
      PacketFactory factory;
      factory.CleanupPacket( packetIn );
   }*/

   return true;
}

/////////////////////////////////////////////////////////////////

bool	Khaan :: Update()
{
   if( m_isDisconnected )
   {
      if( m_listOfOutputs.size() == 0 )
      {
         return false;
      }

      ChainLinkIteratorType output = m_listOfOutputs.begin();
      IChainedInterface* chain = (*output).m_interface;
      if( chain )
      {
         //LogMessage( LOG_PRIO_INFO, "Khaan 4.. server->InputRemovalInProgress " );
         Diplodocus <Khaan> * server = static_cast< Diplodocus <Khaan> * >( chain );
         server->InputRemovalInProgress( this );
         m_listOfOutputs.clear();
      }
      return false;
   }

   if( m_hasPacketsReceived == true )
   {
      UpdateInwardPacketList();
   }
   if( m_hasPacketsToSend == true )
   {
      UpdateOutwardPacketList();
   }

   // I think that this makes sense
   CleanupAllEvents();

   if( m_hasPacketsToSend || m_hasPacketsReceived )// we didn't finish
   {
      LogMessage( LOG_PRIO_INFO, "Remaining packet out count: %d", m_packetsOut.size() );
      return false;
   }
   if( m_denyAllFutureData && m_hasPacketsToSend == false ) // shut it down
   {
      CloseConnection();
      return false;
   }

   //LogMessage( LOG_PRIO_INFO, "Khaan 8" );

   return true;
}

void  Khaan :: SetOutboudBufferSize( U32 size )
{
   //LogMessage( LOG_PRIO_INFO, "Khaan 8.1" );
   if( size < 1024 )
      size = 1024;
   if( size > MaxBufferSize )
      size = MaxBufferSize;

   m_maxBytesToSend = size;
   delete [] m_outboundBuffer;
   m_outboundBuffer = new U8[ m_maxBytesToSend ];
}

/////////////////////////////////////////////////////////////////

void	Khaan :: UpdateInwardPacketList()
{
   //LogMessage( LOG_PRIO_INFO, "Khaan 9" );
   if( m_hasPacketsReceived == false || m_isDisconnected )
      return;

   int numOutputs = static_cast<int>( m_listOfOutputs.size() );
   if( numOutputs > 1 )
   {
      assert( 0 );// need support for multiple outputs, each packet should be copied because of the memory ownership, or use shared pointers
   }

   {
      Threading::MutexLock  locker( m_inputChainListMutex );
      ChainLinkIteratorType output = m_listOfOutputs.begin();
      if( output != m_listOfOutputs.end() )
      {
         IChainedInterface* chain = (*output).m_interface;
         if( chain )
         {
            m_hasPacketsReceived = false;

            while( m_packetsIn.size() > 0 )
            {
               BasePacket* packet = m_packetsIn.front();
            
               TrackInwardPacketType( packet );
               static_cast< ChainType*> ( chain )->AddInputChainData( packet, m_socketId );

               m_packetsIn.pop_front();
            }
         }
      }
   }
}

/////////////////////////////////////////////////////////////////

int	Khaan :: SendData( const U8* buffer, int length )
{
   if( m_isDisconnected )
      return -1;
   //LogMessage( LOG_PRIO_INFO, "Khaan 10" );


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

/////////////////////////////////////////////////////////////////

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

/////////////////////////////////////////////////////////////////

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

/////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////

int	Khaan :: UpdateOutwardPacketList()
{
   if( m_hasPacketsToSend == false || m_isDisconnected )
      return 0;

   //cout << "Khaan :: UpdateOutwardPacketList:: lock 1" << endl;
   m_outputChainListMutex.lock();
      deque< BasePacket* > localQueue = m_packetsOut;
      m_packetsOut.clear();
   m_outputChainListMutex.unlock();
   m_hasPacketsToSend = false;
   //cout << "Khaan :: UpdateOutwardPacketList:: unlock 1" << endl;

   if( localQueue.size() == 0 )
      return 0;

   //LogMessage( LOG_PRIO_INFO, "Khaan 11" );

   int length;
   int offset = 0;
   PacketFactory factory;

   int totalBytesLeftToWrite = m_maxBytesToSend;
   U16 sizeOfLastWrite;
   int sizeOfHeader = sizeof( sizeOfLastWrite );
   int sizeSent = 0;

   while( localQueue.size() ) 
   {
      offset = 0;
      
      length = sizeOfHeader;// reserve space

      BasePacket* packet = localQueue.front();
      packet->SerializeOut( m_outboundBuffer, length, m_versionNumberMinor );
      
      totalBytesLeftToWrite -= length;
      if( totalBytesLeftToWrite < 0 )
         break;
      sizeSent += length + sizeOfHeader;

      sizeOfLastWrite = length - sizeOfHeader;
      Serialize::Out( m_outboundBuffer, offset, sizeOfLastWrite, m_versionNumberMinor );// write in the size
      
      SendData( m_outboundBuffer, length );

      DumpBuffer( m_outboundBuffer, offset, length );

      localQueue.pop_front();
      TrackOutwardPacketType( packet ); 
      factory.CleanupPacket( packet );
   }

   if( localQueue.size() ) // remainder
   {
      //cout << "Khaan :: UpdateOutwardPacketList:: lock 2" << endl;
      m_outputChainListMutex.lock();
         deque< BasePacket* >::reverse_iterator it = localQueue.rbegin();// reverse order
         while( it != localQueue.rend() )
         {
            BasePacket* packet = *it++;
            m_packetsOut.push_front( packet );
         }
         m_hasPacketsToSend = true;
      m_outputChainListMutex.unlock();
      //cout << "Khaan :: UpdateOutwardPacketList:: unlock 2" << endl;
   }

   return sizeSent;
}

/////////////////////////////////////////////////////////////////

bool  Khaan :: AddInputChainData( BasePacket*, U32 filingData ) 
{ 
   return false; 
}

/////////////////////////////////////////////////////////////////

bool Khaan :: AddOutputChainData( BasePacket* packet, U32 filingData ) 
{ 
   if( m_criticalFailure )
      return false;

   //LogMessage( LOG_PRIO_INFO, "Khaan 12" );

   //cout << "Khaan :: AddOutputChainData:: lock" << endl;
   m_outputChainListMutex.lock();
      AddOutputChainDataNoLock( packet );
   m_outputChainListMutex.unlock();
   //cout << "Khaan :: AddOutputChainData:: unlock" << endl;
   return true; 
}

/////////////////////////////////////////////////////////////////

bool Khaan :: AddOutputChainDataNoLock( BasePacket* packet ) 
{ 
   if( m_criticalFailure )
      return false;
   m_packetsOut.push_back( packet );
   m_hasPacketsToSend = true;
   return true; 
}

/////////////////////////////////////////////////////////////////

void  Khaan :: OnDataWritten( struct bufferevent *bev, void *user_data ) 
{
   //struct evbuffer *output = bufferevent_get_output(bev);
   //output->open();
    /* if (evbuffer_get_length(output) == 0) {
         printf("flushed answer\n");
         //bufferevent_free(bev);
     }*/
}
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

void     Khaan :: RegisterToReceiveNetworkTraffic()
{
   if( GetBufferEvent() == NULL )
      return;
   //LogMessage( LOG_PRIO_INFO, "Khaan 13" );


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

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

// Called by libevent when there is data to read.
void     Khaan :: OnDataAvailable( struct bufferevent* bufferEventObj, void* arg )
{
   Khaan*      This = (Khaan*) arg;
   if( This->m_isDisconnected )
      return;

   //LogMessage( LOG_PRIO_INFO, "Khaan 14" );

   //LogMessage( LOG_PRIO_INFO, "Data avail:" );
   struct evbuffer *input = bufferevent_get_input( bufferEventObj );
   size_t readLength = evbuffer_get_length( input );
   const U32   MaxBufferSize = 12*1024;// allowing for massive, reads that should never happen
   
   if( This == NULL )
      return;
   
   if( readLength > MaxBufferSize ) // this is highly unlikely
   {
      U8* tempBuffer = new U8 [ readLength ];

      size_t      numBytesReceived = bufferevent_read( bufferEventObj, tempBuffer, readLength );
      numBytesReceived = numBytesReceived;
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
      
      
      if( totalBytes > 0 && This->m_isDisconnected == false )
      {
       /*  LogMessage( LOG_PRIO_INFO, "data=[" );
         for( U32 i=0; i<totalBytes; i++ )
         {
            LogMessage( LOG_PRIO_INFO, (int)( dataBuffer[i] ) << " ";
         }
         LogMessage( LOG_PRIO_INFO, "]" );*/

         This->OnDataReceived( dataBuffer, static_cast< int>( totalBytes ) );
      }
   }
}

/////////////////////////////////////////////////////////////////

void     Khaan :: FlushReadBuffer()
{
   if( GetBufferEvent() && m_isDisconnected == false )
   bufferevent_flush( GetBufferEvent(), EV_READ, BEV_FINISHED );
}

/////////////////////////////////////////////////////////////////

void     Khaan :: CloseConnection()
{
   //LogMessage( LOG_PRIO_INFO, "Khaan::CloseConnection" );
   m_isDisconnected = true;
   if( GetBufferEvent() ) // this is set to NULL in Cleanup
   {
      bufferevent_free( GetBufferEvent() );
      Cleanup();
   }
}

/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////

void    Khaan :: Cleanup()
{
   // flush all pending packets
   ClearAllPacketsIn();
   ClearAllPacketsOut();

   if( m_bufferEvent == NULL )
      return;

   //LogMessage( LOG_PRIO_INFO, "Khaan::Cleanup" );
   PreCleanup();

   CleanupAllChainDependencies();

   m_bufferEvent = NULL;
   //bufferevent_free( GetBufferEvent() );
   //m_bufferEvent = NULL;
}

/////////////////////////////////////////////////////////////////

void     Khaan :: ClearAllPacketsIn()
{
   if( m_packetsIn.size() == 0 )// slight threading risk here... needs flag optimization
      return;

   LogMessage( LOG_PRIO_INFO, "Khaan::ClearAllPacketsIn" );
   m_inputChainListMutex.lock();
   deque< BasePacket* > localQueue = m_packetsIn;
   m_packetsIn.clear();
   m_hasPacketsReceived = false;
   m_inputChainListMutex.unlock();

   PacketFactory factory;
   while( localQueue.size() )
   {
      BasePacket* packet = localQueue.front();
      localQueue.pop_front();
      factory.CleanupPacket( packet );
   }
}

/////////////////////////////////////////////////////////////////

void     Khaan :: ClearAllPacketsOut()
{
   if( m_hasPacketsToSend == false )// slight threading risk here... needs flag optimization
      return;

   LogMessage( LOG_PRIO_INFO, "Khaan::ClearAllPacketsOut" );
   //cout << "Khaan :: ClearAllPacketsOut:: lock" << endl;
   m_outputChainListMutex.lock();
      deque< BasePacket* > localQueue = m_packetsOut;
      m_packetsOut.clear();
      m_hasPacketsToSend = false;
   m_outputChainListMutex.unlock();
   //cout << "Khaan :: ClearAllPacketsOut:: unlock" << endl;

   PacketFactory factory;
   while( localQueue.size() )
   {
      BasePacket* packet = localQueue.front();
      localQueue.pop_front();
      factory.CleanupPacket( packet );
   }
}

/////////////////////////////////////////////////////////////////

// Called by libevent when there is an error on the underlying socket descriptor.
void	   Khaan::OnSocketError( struct bufferevent* bufferEventObj, short events, void* context )
{
   LogMessage( LOG_PRIO_INFO, "Khaan::OnSocketError" );
   Khaan* khaan = (Khaan*) context;

   if (events & ( BEV_EVENT_EOF | BEV_EVENT_ERROR) )
   {
      /* Client disconnected, remove the read event and the
      * free the Khaan structure. */
      //LogMessage( LOG_PRIO_INFO, "Client disconnected." );
   }
   else 
   {
      LogMessage( LOG_PRIO_ERR, "Client socket error, disconnecting." );
   }

   //khaan->m_isDisconnected = true; // prevents handling of future data... just in case
   khaan->PreCleanup();
}

/////////////////////////////////////////////////////////////////
