#include "../Platform.h"
#include <iostream>
using namespace std;

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

FruitadensServerToServer::FruitadensServerToServer( const char* name ) : FruitadensServer( name )
{
}

//-----------------------------------------------------------------------------------------

FruitadensServerToServer::~FruitadensServerToServer(void)
{
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
            wrapper.serverId = m_localServerId;
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

FruitadensServerToServer*     PrepS2SOutwardConnection( const string& ipaddress, U16 port, U32 serverId, const string& serverName, ServerType serverType, 
                                                       LinkedInterface* diplodocus, const string& localAddress, U16 localPort, U32 gameProductId )
{
   string connectionText = "fruity to ";
   if( serverName.size() )
   {
      connectionText += serverName;
   }
   else
   {
      connectionText += "stats";
   }
   FruitadensServerToServer* remoteServer = new FruitadensServerToServer( connectionText.c_str() );
   remoteServer->SetConnectedServerType( serverType );
   remoteServer->SetServerUniqueId( serverId );

   remoteServer->AddInputChain( diplodocus );

   remoteServer->NotifyEndpointOfIdentification( serverName, localAddress, serverId, localPort, gameProductId, false, false, true, true );
   cout << "Remote server: " << ipaddress << ":" << port << endl;
   remoteServer->Connect( ipaddress.c_str(), port );
   remoteServer->Resume();

   return remoteServer;
}

//-----------------------------------------------------------------------------------------
