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

   Serialize::In( data, bufferOffset, gameNetworkVersion, minorVersion );

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

   Serialize::Out( data, bufferOffset, gameNetworkVersion, minorVersion );
  
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

   if( packetFactory.Parse( data, bufferOffset, &pPacket, minorVersion ) == false )
   {
      return false;
   }

   return true;
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

   if( packetFactory.Parse( data, bufferOffset, &pPacket, minorVersion ) == false )
   {
      return false;
   }

   return true;
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

bool  PackageForS2S( U32 serverId, U8 gameProductId, BasePacket* packetToBeWrapped, BasePacket** packet )
{
   PacketServerToServerWrapper* wrapper = new PacketServerToServerWrapper;
   wrapper->serverId = serverId;
   wrapper->gameProductId = gameProductId;
   wrapper->pPacket = packetToBeWrapped;
   *packet = wrapper;

   return true;
}

///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////

bool  PacketServerToServer_GatewayRequestLB_ConnectionIds::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, serverAddress, minorVersion );   
   Serialize::In( data, bufferOffset, serverId, minorVersion );

   return true;
}

bool  PacketServerToServer_GatewayRequestLB_ConnectionIds::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, serverAddress, minorVersion );   
   Serialize::Out( data, bufferOffset, serverId, minorVersion );
  
   return true;
}

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

bool  PacketServerToServer_GatewayRequestLB_ConnectionIdsResponse::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, beginningId, minorVersion );
   Serialize::In( data, bufferOffset, countId, minorVersion );

   return true;
}

bool  PacketServerToServer_GatewayRequestLB_ConnectionIdsResponse::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, beginningId, minorVersion );
   Serialize::Out( data, bufferOffset, countId, minorVersion );
  
   return true;
}

///////////////////////////////////////////////////////////////

bool  operator == ( const ScheduledOutage& lhs, const ScheduledOutage& rhs )
{
   if( lhs.gatewayId ==           rhs.gatewayId &&
       lhs.type ==                rhs.type &&
       lhs.gameId ==              rhs.gameId &&
       lhs.beginTime ==           rhs.beginTime &&
       lhs.downTimeInSeconds ==   rhs.downTimeInSeconds &&
       lhs.cancelled ==           rhs.cancelled )
         return true;
   return false;
}


bool  ScheduledOutage::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   Serialize::In( data, bufferOffset, gatewayId, minorVersion );
   U8 t;
   Serialize::In( data, bufferOffset, t, minorVersion );
   type = static_cast< ServerType > ( t );
   Serialize::In( data, bufferOffset, gameId, minorVersion );
   Serialize::In( data, bufferOffset, beginTime, minorVersion );
   Serialize::In( data, bufferOffset, downTimeInSeconds, minorVersion );
   Serialize::In( data, bufferOffset, cancelled, minorVersion );

   return true;
}

bool  ScheduledOutage::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   Serialize::Out( data, bufferOffset, gatewayId, minorVersion );

   U8 t = type;
   Serialize::Out( data, bufferOffset, t, minorVersion );
   Serialize::Out( data, bufferOffset, gameId, minorVersion );
   Serialize::Out( data, bufferOffset, beginTime, minorVersion );
   Serialize::Out( data, bufferOffset, downTimeInSeconds, minorVersion );
   Serialize::Out( data, bufferOffset, cancelled, minorVersion );
  
   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketServerConnectionInfo_ServerOutageSchedule::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, scheduledOutages, minorVersion );

   return true;
}

bool  PacketServerConnectionInfo_ServerOutageSchedule::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, scheduledOutages, minorVersion );
  
   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketServerConnectionInfo_KeepAlive::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, packetNo, minorVersion );

   return true;
}

bool  PacketServerConnectionInfo_KeepAlive::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, packetNo, minorVersion );
  
   return true;
}

///////////////////////////////////////////////////////////////