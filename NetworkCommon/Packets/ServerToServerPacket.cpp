// ServerToServerPacket.cpp

#include "../ServerConstants.h"
#include "ServerToServerPacket.h"
#include "../Serialize.h"
#include "PacketFactory.h"
#include <assert.h>


///////////////////////////////////////////////////////////////

bool  PacketServerIdentifier::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, serverName );
   Serialize::In( data, bufferOffset, serverId );
   Serialize::In( data, bufferOffset, gameInstanceId );   
   Serialize::In( data, bufferOffset, isGameServer );
   Serialize::In( data, bufferOffset, isController );

   return true;
}

bool  PacketServerIdentifier::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, serverName );
   Serialize::Out( data, bufferOffset, serverId );
   Serialize::Out( data, bufferOffset, gameInstanceId );
   Serialize::Out( data, bufferOffset, isGameServer );
   Serialize::Out( data, bufferOffset, isController );
  
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

bool  PackageForServerIdentification( const string& serverName, U32 serverId, bool isGameServer, bool isController, bool requiresWrapper, BasePacket** packet )
{
   PacketServerIdentifier* serverIdPacket = new PacketServerIdentifier;
   serverIdPacket->serverName = serverName;
   serverIdPacket->serverId = serverId;
   serverIdPacket->isGameServer = isGameServer;
   serverIdPacket->isController = isController;

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
