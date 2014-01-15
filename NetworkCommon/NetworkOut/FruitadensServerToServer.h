#pragma once

#include "../DataTypes.h"
#include "../NetworkOut/Fruitadens.h"

class BasePacket;

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