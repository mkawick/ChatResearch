#pragma once

#include "../../NetworkCommon/DataTypes.h"
#include "../../NetworkCommon/NetworkOut/Fruitadens.h"

class BasePacket;

///////////////////////////////////////////////////////////////

class FruitadensServerToServer : public Fruitadens
{
public:
   FruitadensServerToServer( const char* name );
   ~FruitadensServerToServer();

   void  SetServerId( U32 serverId ); // when routing messages back to a server, it's good to know from where it came.

protected:

   bool FilterOutwardPacket( BasePacket* packet ) const;
   int  ProcessOutputFunction();

   U32      m_serverId;
};

///////////////////////////////////////////////////////////////