#pragma once

#include "../NetworkCommon/NetworkOut/Fruitadens.h"

#include "GatewayCommon.h"

class DiplodocusGateway;
//-----------------------------------------------------------------------------

class FruitadensGateway : public FruitadensServer
{
public:
   FruitadensGateway( const char* name );
   ~FruitadensGateway();

   bool     AcceptsPacketType( U32 type ) const;

private:

   bool     FilterOutwardPacket( BasePacket* packet ) const;
   int      ProcessOutputFunction();
   void     PostProcessInputPackets( int bytesRead );

   int      ProcessInputFunction();
   void     FindGateway();

   DiplodocusGateway* m_gateway;
};

//-----------------------------------------------------------------------------