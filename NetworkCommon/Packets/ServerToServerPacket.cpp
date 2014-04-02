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
   Serialize::In( data, bufferOffset, serverId );
   Serialize::In( data, bufferOffset, serverPort );
   Serialize::In( data, bufferOffset, gameInstanceId );   
   Serialize::In( data, bufferOffset, isGameServer );
   Serialize::In( data, bufferOffset, isController );
   Serialize::In( data, bufferOffset, isGateway );

   return true;
}

bool  PacketServerIdentifier::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, serverName );
   Serialize::Out( data, bufferOffset, serverAddress );
   Serialize::Out( data, bufferOffset, serverId );
   Serialize::Out( data, bufferOffset, serverPort );
   Serialize::Out( data, bufferOffset, gameInstanceId );
   Serialize::Out( data, bufferOffset, isGameServer );
   Serialize::Out( data, bufferOffset, isController );
   Serialize::Out( data, bufferOffset, isGateway );
  
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

bool  PackageForServerIdentification( const string& serverName, const string& ipAddress, U32 serverId, U16 serverPort, U8 gameProductId, bool isGameServer, bool isController, bool requiresWrapper, bool isGateway, BasePacket** packet )
{
   PacketServerIdentifier* serverIdPacket = new PacketServerIdentifier;
   serverIdPacket->serverName = serverName;
   serverIdPacket->serverAddress = ipAddress;
   serverIdPacket->serverId = serverId;
   serverIdPacket->serverPort = serverPort;
   serverIdPacket->isGameServer = isGameServer;
   serverIdPacket->isController = isController;
   serverIdPacket->isGateway = isGateway;
   serverIdPacket->gameProductId = gameProductId;

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

