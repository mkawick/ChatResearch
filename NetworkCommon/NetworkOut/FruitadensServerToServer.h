#pragma once

#include "../DataTypes.h"
#include "../NetworkOut/Fruitadens.h"

class BasePacket;

typedef ChainedInterface < BasePacket* > LinkedInterface;
///////////////////////////////////////////////////////////////

class FruitadensServerToServer : public FruitadensServer
{
public:
   FruitadensServerToServer( const char* name );
   ~FruitadensServerToServer();

   void  SetServerId( U32 serverId ); // when routing messages back to a server, it's good to know from where it came.
   void  SetGameProductId( U8 gameProductId ){ m_gameProductId = gameProductId; }

   void  AddToOutwardFilters( U16 packetType );

protected:

   bool  FilterOutwardPacket( BasePacket* packet ) const;
   int   ProcessOutputFunction();

   U32      m_serverId;
   U8       m_gameProductId;

   vector< U16 > outwardPacketFilters;
};

///////////////////////////////////////////////////////////////

FruitadensServerToServer*     PrepS2SOutwardConnection( const string& ipaddress, U16 port, U32 serverId, const string& serverName, ServerType serverType, 
                                                       LinkedInterface* diplodocus, const string& localAddress, U16 localPort, U32 gameProductId = 0 );

///////////////////////////////////////////////////////////////