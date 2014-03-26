#pragma once
#include "../NetworkCommon/NetworkOut/Fruitadens.h"

///////////////////////////////////////////////////////////////////////////////

class FruitadensLogin : public FruitadensServer
{
public:
   FruitadensLogin( const char* name );

private:

   //void     InitalConnectionCallback();
   bool     FilterOutwardPacket( BasePacket* packet ) const;
   int      MainLoop_OutputProcessing();
};

///////////////////////////////////////////////////////////////////////////////
