// FruitadensGateway.cpp

#include "../NetworkCommon/ServerConstants.h"
#include "FruitadensGateway.h"
#include "../NetworkCommon/Packets/PacketFactory.h"
#include "../NetworkCommon/Packets/BasePacket.h"
#include "../NetworkCommon/Packets/ServerToServerPacket.h"

#include <iostream>
#include <iomanip>
#include <assert.h>

//-----------------------------------------------------------------------------------------

FruitadensGateway::FruitadensGateway( const char* name ) : Fruitadens( name )
{
   SetSleepTime( 30 );// Sleeping frees up CPU
}

//-----------------------------------------------------------------------------------------

FruitadensGateway::~FruitadensGateway()
{
}

//-----------------------------------------------------------------------------------------

bool FruitadensGateway::FilterOutwardPacket( BasePacket* packet ) const
{
   if( packet->packetType == PacketType_GatewayWrapper )
   {
      PacketGatewayWrapper* wrapper = static_cast< PacketGatewayWrapper* >( packet );

      PacketType type = static_cast< PacketType > ( wrapper->pPacket->packetType );
      if( m_serverType == ServerType_GameInstance )
      {
         if( type == PacketType_Gameplay )
         {
            // sending packets to the correct server.
            if( m_connectedServerId == wrapper->pPacket->gameInstanceId && 
                  wrapper->pPacket->gameProductId == m_connectedGameProductId )
            {
               return true;
            }
         }
      }
      else if( m_serverType == ServerType_Chat )
      {
         if( type == PacketType_Chat )
            return true;
         if( type == PacketType_UserInfo )// todo, remove me once we have other things in place
            return true;
         if( type == PacketType_GatewayInformation )
            return true;
      }
      else if( m_serverType == ServerType_Login )
      {
         if( type == PacketType_Login )// login is always acceptable... for now. Once we have a login server, we need to remove this exception.
         {
            return true;
         }
      }
      else if( m_serverType == ServerType_Contact )
      {
         if( type == PacketType_UserInfo )
         {
            return true;
         }
         if( type == PacketType_Contact )
         {
            return true;
         }
      }
      else if( m_serverType == ServerType_Asset )
      {
         if( type == PacketType_Asset )
         {
            return true;
         }
      }
   }
   else if( packet->packetType == PacketType_ServerToServerWrapper )
   {
      return true;
   }

   return false;
}

//-----------------------------------------------------------------------------------------

int   FruitadensGateway::ProcessInputFunction()
{
   // take any overflow data and save it.
   //memcpy( m_receiveBuffer, m_overflowBuffer, m_bytesInOverflow );
   //m_receiveBufferOffset = m_bytesInOverflow;// save the offset

   int returnVal = Fruitadens :: ProcessInputFunction();

   //m_bytesInOverflow = 0;
   return returnVal;
}

//-----------------------------------------------------------------------------------------

int  FruitadensGateway::ProcessOutputFunction()
{
   if( m_isConnected == false )
   {
      return 0;
   }

   if( m_packetsReadyToSend.size() > 0 )
   {
      U8 buffer[ MaxBufferSize ];
      int dangerZone = MaxBufferSize * 3/4;// 25%
      int offset = 0;
      PacketFactory factory;
      
      m_mutex.lock();
      while( m_packetsReadyToSend.size() && offset < dangerZone )
      {
         BasePacket* packet = m_packetsReadyToSend.front();
         m_packetsReadyToSend.pop_front();
         
         packet->SerializeOut( buffer, offset );

         factory.CleanupPacket( packet );
      }
      m_mutex.unlock();

      SendPacket( buffer, offset );
   }

   return 0;
}


//-----------------------------------------------------------------------------------------

void  FruitadensGateway::PostProcessInputPackets( int bytesRead )
{
   if( bytesRead < 1 )
   {
      return;
   }

   // we should always be receiving Gateway Wrappers.
   PacketGatewayWrapper wrapper;

   PacketFactory factory;
   int offset = 0;
   U16 size;
   while( offset < bytesRead )
   { 
      int preOffset = offset;
      Serialize::In( m_receiveBuffer, offset, size );

      wrapper.HeaderSerializeIn( m_receiveBuffer, offset );
//      if( offset + wrapper.size > bytesRead )// we must have cut off a packet in the middle.
      if( offset + size > bytesRead )
      {
         // copy remainder into temp buffer.
         m_bytesInOverflow = bytesRead - preOffset;
         memcpy( m_overflowBuffer, m_receiveBuffer + preOffset, m_bytesInOverflow );
         cout << "--- Overflow packets: " << m_bytesInOverflow << endl;
         return;
      }

      int previousOffset = offset;
      BasePacket* packetIn = NULL;
      if( factory.Parse( m_receiveBuffer, offset, &packetIn ) == true )
      {
         m_numPacketsReceived ++;
         HandlePacketReceived( packetIn );
      }
      else 
      {
         assert( 0 );
         offset = m_numPacketsReceived;// major failure here
      }
   }

}

//-----------------------------------------------------------------------------------------

void FruitadensGateway::InitalConnectionCallback()
{
   ChainLinkIteratorType   itInputs = m_listOfInputs.begin();
   while( itInputs != m_listOfInputs.end() )
   {
      ChainedInterface<BasePacket*>* inputPtr = itInputs->m_interface;
      inputPtr->OutputConnected( this );
      itInputs++;
   }
}

//-----------------------------------------------------------------------------------------
