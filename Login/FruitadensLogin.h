#pragma once
#include "../NetworkCommon/NetworkOut/fruitadens.h"

///////////////////////////////////////////////////////////////////////////////

class FruitadensLogin : public Fruitadens
{
public:
   FruitadensLogin( const char* name );
   void     SetServerUniqueId( U32 id ) { m_serverId = id; }

private:

   bool     FilterOutwardPacket( BasePacket* packet ) const;
   int      ProcessOutputFunction();

   U32                  m_serverId;
};

///////////////////////////////////////////////////////////////////////////////
