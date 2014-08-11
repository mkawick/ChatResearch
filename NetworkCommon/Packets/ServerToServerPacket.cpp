// ServerToServerPacket.cpp

#include "../ServerConstants.h"
#include "ServerToServerPacket.h"
#include "Serialize.h"
#include "PacketFactory.h"
#include <assert.h>


///////////////////////////////////////////////////////////////

bool  PacketServerConnectionInfo::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, serverAddress, minorVersion );
   Serialize::In( data, bufferOffset, serverId, minorVersion );
   Serialize::In( data, bufferOffset, currentLoad, minorVersion );

   return true;
}

bool  PacketServerConnectionInfo::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, serverAddress, minorVersion );
   Serialize::Out( data, bufferOffset, serverId, minorVersion );
   Serialize::Out( data, bufferOffset, currentLoad, minorVersion );
  
   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketServerIdentifier::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, serverName, minorVersion );
   Serialize::In( data, bufferOffset, serverAddress, minorVersion );
   Serialize::In( data, bufferOffset, externalIpAddress, minorVersion );
   Serialize::In( data, bufferOffset, serverId, minorVersion );
   Serialize::In( data, bufferOffset, serverType, minorVersion );
   Serialize::In( data, bufferOffset, serverPort, minorVersion );
   Serialize::In( data, bufferOffset, gameInstanceId, minorVersion );   
   Serialize::In( data, bufferOffset, isGameServer, minorVersion );
   Serialize::In( data, bufferOffset, isController, minorVersion );
   Serialize::In( data, bufferOffset, gatewayType, minorVersion );

   return true;
}

bool  PacketServerIdentifier::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, serverName, minorVersion );
   Serialize::Out( data, bufferOffset, serverAddress, minorVersion );
   Serialize::Out( data, bufferOffset, externalIpAddress, minorVersion );
   Serialize::Out( data, bufferOffset, serverId, minorVersion );
   Serialize::Out( data, bufferOffset, serverType, minorVersion );
   Serialize::Out( data, bufferOffset, serverPort, minorVersion );
   Serialize::Out( data, bufferOffset, gameInstanceId, minorVersion );
   Serialize::Out( data, bufferOffset, isGameServer, minorVersion );
   Serialize::Out( data, bufferOffset, isController, minorVersion );
   Serialize::Out( data, bufferOffset, gatewayType, minorVersion );
  
   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketServerDisconnect::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, serverAddress, minorVersion );
   Serialize::In( data, bufferOffset, serverId, minorVersion );

   return true;
}

bool  PacketServerDisconnect::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, serverAddress, minorVersion );
   Serialize::Out( data, bufferOffset, serverId, minorVersion );
  
   return true;
}
///////////////////////////////////////////////////////////////

bool  PacketServerToServerWrapper::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, serverId, minorVersion );

   delete pPacket; pPacket = NULL;
   PacketFactory packetFactory;

   return packetFactory.Parse( data, bufferOffset, &pPacket, minorVersion );
}

///////////////////////////////////////////////////////////////

bool  PacketServerToServerWrapper::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );   
   Serialize::Out( data, bufferOffset, serverId, minorVersion );

   if( pPacket == NULL )
   {
      assert( 0 );
   }
   pPacket->SerializeOut( data, bufferOffset, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketServerJobWrapper::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, serverId, minorVersion );
   Serialize::In( data, bufferOffset, jobId, minorVersion );

   delete pPacket; pPacket = NULL;
   PacketFactory packetFactory;

   return packetFactory.Parse( data, bufferOffset, &pPacket, minorVersion );
}

///////////////////////////////////////////////////////////////

bool  PacketServerJobWrapper::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );   
   Serialize::Out( data, bufferOffset, serverId, minorVersion );
   Serialize::Out( data, bufferOffset, jobId, minorVersion );

   if( pPacket == NULL )
   {
      assert( 0 );
   }
   pPacket->SerializeOut( data, bufferOffset, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PackageForServerIdentification( const string& serverName, const string& ipAddress, const string& externalIpAddress, U32 serverId, U8 serverType, U16 serverPort, U8 gameProductId, bool isGameServer, bool isController, bool requiresWrapper, U8 gatewayType, BasePacket** packet )
{
   PacketServerIdentifier* serverIdPacket = new PacketServerIdentifier;
   serverIdPacket->serverName = serverName;
   serverIdPacket->serverAddress = ipAddress;
   serverIdPacket->serverId = serverId;
   serverIdPacket->serverPort = serverPort;
   serverIdPacket->serverType = serverType;
   serverIdPacket->isGameServer = isGameServer;
   serverIdPacket->isController = isController;
   serverIdPacket->gatewayType = gatewayType;
   serverIdPacket->gameProductId = gameProductId;
   serverIdPacket->externalIpAddress = externalIpAddress;

   if( requiresWrapper )
   {
      PacketServerToServerWrapper* wrapper = new PacketServerToServerWrapper;
      wrapper->serverId = serverId;
      wrapper->pPacket = serverIdPacket;
      *packet = wrapper;
   }
   else
   {
      *packet = serverIdPacket;
   }

   return true;
}

///////////////////////////////////////////////////////////////

