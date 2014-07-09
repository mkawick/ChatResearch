#include "../Platform.h"
#include <iostream>
using namespace std;

#if PLATFORM == PLATFORM_WINDOWS
#include <winsock2.h>
#pragma warning( disable:4996 )

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
   SetSleepTime( 16 );// Sleeping frees up CPU
}

//-----------------------------------------------------------------------------------------

FruitadensServerToServer::~FruitadensServerToServer()
{
}

//-----------------------------------------------------------------------------------------

void  FruitadensServerToServer::AddToOutwardFilters( U16 packetType )
{
   Threading::MutexLock    locker( m_mutex );
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

   Threading::MutexLock    locker( m_mutex );
   vector< U16 >::const_iterator it = outwardPacketFilters.begin();
   while( it != outwardPacketFilters.end() )
   {
      if( *it++ == packet->packetType ) 
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
      PacketFactory factory;
      m_mutex.lock();
      PacketQueue packetQueue = m_packetsReadyToSend;
      m_packetsReadyToSend.clear();
      m_mutex.unlock();

      while( packetQueue.size() )
      {
         BasePacket* packet = packetQueue.front();

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
         packetQueue.pop_front();

         factory.CleanupPacket( packet );
      }
      
   }

   return 0;
}

//-----------------------------------------------------------------------------------------

FruitadensServerToServer*     PrepS2SOutwardConnection( const string& ipaddress, U16 port, U32 serverId, const string& serverName, ServerType serverType, 
                                                       LinkedInterface* diplodocus, const string& localAddress, U16 localPort, U32 gameProductId )
{
   assert(0);;// verify that this is no longer used
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

   remoteServer->NotifyEndpointOfIdentification( serverName, localAddress, serverId, serverType, localPort, gameProductId, false, false, true, (U8)34, "" );
   cout << "Remote server: " << ipaddress << ":" << port << endl;
   remoteServer->Connect( ipaddress.c_str(), port );
   remoteServer->Resume();

   return remoteServer;
}

//-----------------------------------------------------------------------------------------
