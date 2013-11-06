#include "../NetworkCommon/Platform.h"

#if PLATFORM == PLATFORM_WINDOWS
#include <winsock2.h>

#elif PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX

#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>

#endif

#include "FruitadensServerToServer.h"
#include "../NetworkCommon/Packets/BasePacket.h"
#include "../NetworkCommon/Packets/ChatPacket.h"
#include "../NetworkCommon/Packets/GamePacket.h"
#include "../NetworkCommon/Packets/ServerToServerPacket.h"
#include "../NetworkCommon/Packets/PacketFactory.h"

//-----------------------------------------------------------------------------------------

FruitadensServerToServer::FruitadensServerToServer( const char* name ) : Fruitadens( name ), m_serverId( 0 )
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

bool  FruitadensServerToServer::FilterOutwardPacket( BasePacket* packet ) const
{
   // packets going out should not be PacketType_ServerToServerWrapper, they will be wrapped when outbound
   if( packet->packetType == PacketType_ServerInformation ||
      packet->packetType == PacketType_Gameplay || 
      packet->packetType == PacketType_Tournament ||// mostly for purchasing
      packet->packetType == PacketType_Chat )
   {
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
