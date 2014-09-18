#pragma once
#include "../NetworkCommon/NetworkOut/Fruitadens.h"

///////////////////////////////////////////////////////////////////////////////

class FruitadensLogin : public FruitadensServer
{
public:
   FruitadensLogin( const char* name );

   const char*  GetClassName() const { return "FruitadensLogin"; }

   //int       CallbackFunction();

private:
   bool     FilterOutwardPacket( BasePacket* packet ) const;
   int      MainLoop_OutputProcessing();
};

///////////////////////////////////////////////////////////////////////////////
