
#include <memory.h>

#include "FruitadensLogin.h"
#include "../NetworkCommon/Packets/PacketFactory.h"
#include "../NetworkCommon/Packets/BasePacket.h"
#include "../NetworkCommon/Packets/ServerToServerPacket.h"
#include "../NetworkCommon/Packets/GamePacket.h"
#include "../NetworkCommon/Packets/LoginPacket.h"

#include <assert.h>

//-----------------------------------------------------------------------------------------

FruitadensLogin::FruitadensLogin( const char* name ) : FruitadensServer( name )
{
   SetSleepTime( 30 );// Sleeping frees up CPU
}

//-----------------------------------------------------------------------------------------

/*void     FruitadensLogin::InitalConnectionCallback()
{
   //NotifyEndpointOfIdentification( serverName, serverAddress, serverId, serverPort, 0, false, false, true, false  );
}*/

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------

bool FruitadensLogin::FilterOutwardPacket( BasePacket* packet ) const
{
   // we should only pass login successful packets.. not even failure packets

   assert( m_serverId != 0 );

   //PacketType type = static_cast< PacketType > ( wrapper->pPacket->packetType );
   U32 packetType = static_cast< U32 >( packet->packetType );
   U32 packetSubType = static_cast< U32 >( packet->packetSubType );

   if( packetType == PacketType_ServerToServerWrapper )
   {
      return true;
   }

   if( packetType == PacketType_Login ) 
   {
      if( packetSubType == PacketLogin::LoginType_PrepareForUserLogin ||
         packetSubType == PacketLogin::LoginType_PrepareForUserLogout)
      {
         return true;
      }
      
      if( packetSubType == PacketLogin::LoginType_ListOfProductsS2S )
      {
         if( m_serverType == ServerType_Asset )
            return true;
         return false;
      }
   }

   if( packetType == PacketType_Gameplay ) 
   {
      if( packetSubType == PacketGameToServer::GamePacketType_ListOfGames )
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
