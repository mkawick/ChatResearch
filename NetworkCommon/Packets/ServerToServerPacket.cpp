// ServerToServerPacket.cpp

#include "../ServerConstants.h"
#include "ServerToServerPacket.h"
#include "Serialize.h"
#include "PacketFactory.h"
#include <assert.h>


///////////////////////////////////////////////////////////////

bool  PacketServerConnectionInfo::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, serverAddress );
   Serialize::In( data, bufferOffset, serverId );
   Serialize::In( data, bufferOffset, currentLoad );

   return true;
}

bool  PacketServerConnectionInfo::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, serverAddress );
   Serialize::Out( data, bufferOffset, serverId );
   Serialize::Out( data, bufferOffset, currentLoad );
  
   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketServerIdentifier::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, serverName );
   Serialize::In( data, bufferOffset, serverAddress );
   Serialize::In( data, bufferOffset, externalIpAddress );
   Serialize::In( data, bufferOffset, serverId );
   Serialize::In( data, bufferOffset, serverType );
   Serialize::In( data, bufferOffset, serverPort );
   Serialize::In( data, bufferOffset, gameInstanceId );   
   Serialize::In( data, bufferOffset, isGameServer );
   Serialize::In( data, bufferOffset, isController );
   Serialize::In( data, bufferOffset, gatewayType );

   return true;
}

bool  PacketServerIdentifier::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, serverName );
   Serialize::Out( data, bufferOffset, serverAddress );
   Serialize::Out( data, bufferOffset, externalIpAddress );
   Serialize::Out( data, bufferOffset, serverId );
   Serialize::Out( data, bufferOffset, serverType );
   Serialize::Out( data, bufferOffset, serverPort );
   Serialize::Out( data, bufferOffset, gameInstanceId );
   Serialize::Out( data, bufferOffset, isGameServer );
   Serialize::Out( data, bufferOffset, isController );
   Serialize::Out( data, bufferOffset, gatewayType );
  
   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketServerDisconnect::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, serverAddress );
   Serialize::In( data, bufferOffset, serverId );

   return true;
}

bool  PacketServerDisconnect::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, serverAddress );
   Serialize::Out( data, bufferOffset, serverId );
  
   return true;
}
///////////////////////////////////////////////////////////////

bool  PacketServerToServerWrapper::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, serverId );

   delete pPacket; pPacket = NULL;
   PacketFactory packetFactory;

   if( packetFactory.Parse( data, bufferOffset, &pPacket ) == false )
   {
      return false;
   }

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketServerToServerWrapper::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );   
   Serialize::Out( data, bufferOffset, serverId );

   if( pPacket == NULL )
   {
      assert( 0 );
   }
   pPacket->SerializeOut( data, bufferOffset );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketServerJobWrapper::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, serverId );
   Serialize::In( data, bufferOffset, jobId );

   delete pPacket; pPacket = NULL;
   PacketFactory packetFactory;

   if( packetFactory.Parse( data, bufferOffset, &pPacket ) == false )
   {
      return false;
   }

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketServerJobWrapper::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );   
   Serialize::Out( data, bufferOffset, serverId );
   Serialize::Out( data, bufferOffset, jobId );

   if( pPacket == NULL )
   {
      assert( 0 );
   }
   pPacket->SerializeOut( data, bufferOffset );

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

