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

//-----------------------------------------------------------------------------------------

FruitadensServerToServer::FruitadensServerToServer( const char* name ) : FruitadensServer( name ), m_serverId( 0 ), m_gameProductId( 0 )
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
      packet->packetType == PacketType_Gameplay )
   {
      return true;
   }
   return false;
}

//-----------------------------------------------------------------------------------------

int  FruitadensServerToServer::MainLoop_OutputProcessing()
{
   if( m_isConnected == false )
   {
      return 0;
   }

   if( m_packetsReadyToSend.size() > 0 )
   {
      m_mutex.lock();
      while( m_packetsReadyToSend.size() )
      {
         BasePacket* packet = m_packetsReadyToSend.front();

         PacketServerToServerWrapper wrapper;
         wrapper.gameProductId = m_gameProductId;
         wrapper.serverId = m_serverId;
         wrapper.pPacket = packet;

         SerializePacketOut( &wrapper );
         m_packetsReadyToSend.pop_front();

         delete packet;
      }
      m_mutex.unlock();
   }

   return 0;
}

//-----------------------------------------------------------------------------------------
