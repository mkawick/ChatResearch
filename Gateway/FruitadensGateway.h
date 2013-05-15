#pragma once

#include "../NetworkCommon/NetworkOut/fruitadens.h"

#include "GatewayCommon.h"

//-----------------------------------------------------------------------------

class FruitadensGateway : public Fruitadens
{
public:
   FruitadensGateway( const char* name );
   ~FruitadensGateway();

private:

   bool     FilterOutwardPacket( BasePacket* packet ) const;
   int      ProcessOutputFunction();
};

//-----------------------------------------------------------------------------