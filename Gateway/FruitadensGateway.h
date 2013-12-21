#pragma once

#include "../NetworkCommon/NetworkOut/Fruitadens.h"

#include "GatewayCommon.h"

//-----------------------------------------------------------------------------

class FruitadensGateway : public Fruitadens
{
public:
   FruitadensGateway( const char* name );
   ~FruitadensGateway();

   void     InitalConnectionCallback();

private:

   bool     FilterOutwardPacket( BasePacket* packet ) const;
   int      ProcessOutputFunction();
   void     PostProcessInputPackets( int bytesRead );

   int      ProcessInputFunction();
};

//-----------------------------------------------------------------------------