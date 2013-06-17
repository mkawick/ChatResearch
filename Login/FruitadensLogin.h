#pragma once
#include "../NetworkCommon/NetworkOut/Fruitadens.h"

///////////////////////////////////////////////////////////////////////////////

class FruitadensLogin : public Fruitadens
{
public:
   FruitadensLogin( const char* name );

private:

   bool     FilterOutwardPacket( BasePacket* packet ) const;
   int      ProcessOutputFunction();
};

///////////////////////////////////////////////////////////////////////////////
