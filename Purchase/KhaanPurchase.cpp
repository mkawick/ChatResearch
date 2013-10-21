#include "KhaanPurchase.h"

#include <iostream>
#include <memory.h>

#include "../NetworkCommon/Packets/PacketFactory.h"
#include "../NetworkCommon/Packets/BasePacket.h"
#include "../NetworkCommon/Packets/GamePacket.h"

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

// all code has been moved into the S2S base class.

///////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------

void	KhaanPurchase :: UpdateInwardPacketList()
{
   if( m_packetsIn.size() == 0 )
      return;

   int numOutputs = m_listOfOutputs.size();
   if( numOutputs > 1 )
   {
      assert( 0 );// need support for multiple outputs, each packet should be copied because of the memory ownership, or use shared pointers
   }

   ChainLinkIteratorType output = m_listOfOutputs.begin();

   ChainType* chain = static_cast< ChainType*> ( (*output).m_interface );

   if( chain )
   {
      while( m_packetsIn.size() > 0 )
      {
         BasePacket* packet = m_packetsIn.front();
      
         Threading::MutexLock  locker( m_inputChainListMutex );
         chain->AddInputChainData( packet, m_socketId );

         m_packetsIn.pop_front();
      }
   }
}

//------------------------------------------------------------------------------

void	KhaanPurchase :: UpdateOutwardPacketList()
{
   if( m_packetsOut.size() == 0 )
      return;

   int length = 0;
   int bufferOffset = 0;

   U8 buffer[ MaxBufferSize + 1024 ];
   PacketFactory factory;

   int num = m_packetsOut.size();
   // todo, plan for the degenerate case where a single packet is over 2k
   for( int i=0; i< num; i++ )
   {
      BasePacket* packet = m_packetsOut.front();

      int temp = bufferOffset;
      U16 sizeOfLastWrite = 0;
      bufferOffset += sizeof( sizeOfLastWrite );// reserve space
      packet->SerializeOut( buffer, bufferOffset ); 

      sizeOfLastWrite = bufferOffset - temp - sizeof( sizeOfLastWrite );// set aside two bytes
      Serialize::Out( buffer, temp, sizeOfLastWrite );// write in the size

      if( bufferOffset < (int)( MaxBufferSize - sizeof( BasePacket ) ) )// do not write past the end
      {
         factory.CleanupPacket( packet );
         m_packetsOut.pop_front();
         length = bufferOffset;
      }
      else
      {
         break;
      }
   }

   if( length > 0 )
   {
      static int numWrites = 0;
      numWrites ++;
      static int numBytes = 0;
      numBytes += length;

      int result = SendData( buffer, length );
   }
}
//------------------------------------------------------------------------------
