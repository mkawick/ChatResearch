#include "../Platform.h"

#if PLATFORM == PLATFORM_WINDOWS
#include <winsock2.h>

#elif PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX

#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>

#endif

#include "FruitadensServerToServer.h"
#include "../Packets/BasePacket.h"
#include "../Packets/ChatPacket.h"
#include "../Packets/GamePacket.h"
#include "../Packets/ServerToServerPacket.h"
#include "../Packets/PacketFactory.h"

//-----------------------------------------------------------------------------------------

FruitadensServerToServer::FruitadensServerToServer( const char* name ) : FruitadensServer( name ), m_serverId( 0 )
{
}

//-----------------------------------------------------------------------------------------

FruitadensServerToServer::~FruitadensServerToServer(void)
{
}

//-----------------------------------------------------------------------------------------

void  FruitadensServerToServer::SetServerId( U32 serverId ) // when routing messages back to a server, it's good to know from where it came.
{
   m_serverId = serverId;
}

//-----------------------------------------------------------------------------------------

void  FruitadensServerToServer::AddToOutwardFilters( U16 packetType )
{
   vector< U16 >::const_iterator it = outwardPacketFilters.begin();
   while( it != outwardPacketFilters.end() )
   {
      if( *it++ == packetType ) 
         return;
   }

   outwardPacketFilters.push_back( packetType );
}

//-----------------------------------------------------------------------------------------

bool  FruitadensServerToServer::FilterOutwardPacket( BasePacket* packet ) const
{
   // packets going out should not be PacketType_ServerToServerWrapper, they will be wrapped when outbound
   U8 packetType = packet->packetType;
   if(   packetType == PacketType_ServerInformation ||
         packetType == PacketType_Gameplay ||
         packetType == PacketType_ServerToServerWrapper )
   {
      return true;
   }

   vector< U16 >::const_iterator it = outwardPacketFilters.begin();
   while( it != outwardPacketFilters.end() )
   {
      if( *it++ == packet->packetType ) 
         return true;
   }
   return false;
}

//-----------------------------------------------------------------------------------------

int  FruitadensServerToServer::ProcessOutputFunction()
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
        /* PacketServerToServerWrapper wrapper;
         wrapper.serverId = m_serverId;
         wrapper.pPacket = packet;*/

         //SerializePacketOut( &wrapper );
         m_packetsReadyToSend.pop_front();

         factory.CleanupPacket( packet );
      }
      m_mutex.unlock();
   }

   return 0;
}

//-----------------------------------------------------------------------------------------
