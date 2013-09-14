
#include <memory.h>

#include "FruitadensLogin.h"
#include "../NetworkCommon/Packets/PacketFactory.h"
#include "../NetworkCommon/Packets/BasePacket.h"
#include "../NetworkCommon/Packets/ServerToServerPacket.h"
#include "../NetworkCommon/Packets/GamePacket.h"

#include <assert.h>

//-----------------------------------------------------------------------------------------

FruitadensLogin::FruitadensLogin( const char* name ) : Fruitadens( name )
{
   SetSleepTime( 30 );// Sleeping frees up CPU
}

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------

bool FruitadensLogin::FilterOutwardPacket( BasePacket* packet ) const
{
   // we should only pass login successful packets.. not even failure packets

   assert( m_serverId != 0 );

   //PacketType type = static_cast< PacketType > ( wrapper->pPacket->packetType );

   if( packet->packetType == PacketType_ServerToServerWrapper )
   {
      return true;
   }

   if( packet->packetType == PacketType_Login ) 
   {
      PacketType subtype = static_cast< PacketType > ( packet->packetSubType );

      if( subtype == PacketLogin::LoginType_PrepareForUserLogin ||
         subtype == PacketLogin::LoginType_PrepareForUserLogout)
      {
         return true;
      }
      
      if( subtype == PacketLogin::LoginType_ListOfProductsS2S )
      {
         if( m_serverType == ServerType_Asset )
            return true;
         return false;
      }
   }

   if( packet->packetType == PacketType_Gameplay ) 
   {
      if( packet->packetSubType == PacketGameToServer::GamePacketType_ListOfGames )
      {
         return true;
      }
   }

   return false;
}

//-----------------------------------------------------------------------------------------

int  FruitadensLogin::ProcessOutputFunction()
{
   if( m_isConnected == false )
   {
      return 0;
   }

   if( m_packetsReadyToSend.size() > 0 )
   {
      PacketFactory factory;
      m_mutex.lock();
      while( m_packetsReadyToSend.size() )
      {
         BasePacket* packet = m_packetsReadyToSend.front();
         m_packetsReadyToSend.pop_front();

         if( packet->packetType == PacketType_ServerToServerWrapper )         
         {
            SerializePacketOut( packet );
         }
         else
         {
            PacketServerToServerWrapper wrapper;
            wrapper.serverId = m_serverId;
            wrapper.pPacket = packet;
            SerializePacketOut( &wrapper );
         }
         
         factory.CleanupPacket( packet );
      }
      m_mutex.unlock();
   }

   return 0;
}

//-----------------------------------------------------------------------------------------
