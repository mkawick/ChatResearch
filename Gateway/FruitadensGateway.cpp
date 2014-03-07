// FruitadensGateway.cpp

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
using boost::format;

#include "../NetworkCommon/ServerConstants.h"
#include "../NetworkCommon/Packets/PacketFactory.h"
#include "../NetworkCommon/Packets/BasePacket.h"
#include "../NetworkCommon/Packets/CheatPacket.h"
#include "../NetworkCommon/Packets/ServerToServerPacket.h"
#include "../NetworkCommon/Utils/CommandLineParser.h"

#include <iostream>
#include <iomanip>
#include <assert.h>

#include "FruitadensGateway.h"
#include "DiplodocusGateway.h"

//-----------------------------------------------------------------------------------------

FruitadensGateway::FruitadensGateway( const char* name ) : FruitadensServer( name ), m_gateway( NULL )
{
   SetSleepTime( 16 );// Sleeping frees up CPU
}

//-----------------------------------------------------------------------------------------

FruitadensGateway::~FruitadensGateway()
{
}

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------

void  FruitadensGateway::FindGateway()
{
   if( m_gateway == NULL )
   {
      Threading::MutexLock locker( m_inputChainListMutex );
      // we don't do much interpretation here, we simply pass output data onto our output, which should be the DB or other servers.
      ChainLinkIteratorType itInput = m_listOfInputs.begin();
      if( itInput != m_listOfInputs.end() )// only one input currently supported.
      {
         m_gateway = static_cast< DiplodocusGateway*> ( (*itInput).m_interface );
      }
   }
}

//-----------------------------------------------------------------------------------------

// just a quick and dirty test, not thorough
bool     FruitadensGateway::AcceptsPacketType( U32 packetType ) const
{
   if( packetType == PacketType_Cheat )// these are filtered differently
   {
      return true;
   }
   switch(  m_serverType )
   {
   case ServerType_GameInstance:
      {
         if( packetType == PacketType_Gameplay )
         {
            return true;
         }
         if( packetType == PacketType_Tournament )
         {
            return true;
         }
      }
      break;

   case ServerType_Chat:
      {
         if( packetType == PacketType_Chat )
            return true;
         if( packetType == PacketType_UserInfo )// todo, remove me once we have other things in place
            return true;
         if( packetType == PacketType_GatewayInformation )
            return true;
      }
      break;

   case ServerType_Login:
      {
         if( packetType == PacketType_Login )// login is always acceptable... for now. Once we have a login server, we need to remove this exception.
         {
            return true;
         }
      }
      break;
   case ServerType_Contact:
      {
         if( packetType == PacketType_UserInfo )
         {
            return true;
         }
         if( packetType == PacketType_Contact )
         {
            return true;
         }
      }
      break;
   case ServerType_Asset:
      {
         if( packetType == PacketType_Asset )
         {
            return true;
         }
      }
      break;
   case ServerType_Purchase:
      {
         if( packetType == PacketType_Purchase )
         {
            return true;
         }
      }
      break;
   case ServerType_Stat:
      {
         if( packetType == PacketType_Stat )
         {
            return true;
         }
      }
      break;
   }

   return false;
}

//-----------------------------------------------------------------------------------------
//************************************************
// NOTE: Any change below must be reflected above
//************************************************
bool FruitadensGateway::FilterOutwardPacket( BasePacket* packet ) const
{
   if( packet->packetType == PacketType_GatewayWrapper )
   {
      PacketGatewayWrapper* wrapper = static_cast< PacketGatewayWrapper* >( packet );

      PacketType packetType = static_cast< PacketType > ( wrapper->pPacket->packetType );

      if( packetType == PacketType_Cheat )// these are filtered differently
      {
         PacketCheat* cheatPacket = static_cast <PacketCheat*> ( wrapper->pPacket );
         int serverType = cheatPacket->whichServer;

         if( m_serverType == serverType )
            return true;
         return false;
      }
      
      switch(  m_serverType )
      {
      case ServerType_GameInstance:
         {
            if( packetType == PacketType_Gameplay )
            {
               // sending packets to the correct server.
               if( m_connectedServerId == wrapper->pPacket->gameInstanceId && 
                     wrapper->pPacket->gameProductId == m_connectedGameProductId )
               {
                  m_gateway->TrackCountStats( StatTrackingConnections::StatTracking_GamePacketsSentToGame, 1, m_connectedGameProductId );
                  return true;
               }
            }
            if( packetType == PacketType_Tournament )
            {
               if( m_connectedServerId == wrapper->pPacket->gameInstanceId && 
                     wrapper->pPacket->gameProductId == m_connectedGameProductId )
               {
                  return true;
               }
            }
         }
         break;

      case ServerType_Chat:
         {
            if( packetType == PacketType_Chat )
               return true;
            if( packetType == PacketType_UserInfo )// todo, remove me once we have other things in place
               return true;
            if( packetType == PacketType_GatewayInformation )
               return true;
         }
         break;

      case ServerType_Login:
         {
            if( packetType == PacketType_Login )// login is always acceptable... for now. Once we have a login server, we need to remove this exception.
            {
               return true;
            }
         }
         break;
      case ServerType_Contact:
         {
            if( packetType == PacketType_UserInfo )
            {
               return true;
            }
            if( packetType == PacketType_Contact )
            {
               return true;
            }
         }
         break;
      case ServerType_Asset:
         {
            if( packetType == PacketType_Asset )
            {
               return true;
            }
         }
         break;
      case ServerType_Purchase:
         {
            if( packetType == PacketType_Purchase )
            {
               return true;
            }
         }
         break;
      case ServerType_Stat:
         {
            if( packetType == PacketType_Stat )
            {
               return true;
            }
         }
         break;
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

   FindGateway();

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

      //int previousOffset = offset;
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

