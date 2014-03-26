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
   int      MainLoop_OutputProcessing();
   void     PostProcessInputPackets( int bytesRead );

   int      MainLoop_InputProcessing();
   void     FindGateway();

   DiplodocusGateway* m_gateway;
};

//-----------------------------------------------------------------------------