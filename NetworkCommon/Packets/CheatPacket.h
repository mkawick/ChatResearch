// CheatPacket.h
#pragma once

#include "BasePacket.h"

///////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////

class PacketCheat : public BasePacket
{
   enum
   {
      Cheat_Basic = 1
   };
public:
   PacketCheat(): BasePacket( PacketType_Cheat, PacketCheat::Cheat_Basic ) {}
   
   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;
   
   int      whichServer; // the client should tell us which server gets this... use PacketType to designate which server
   string   cheat;
};


///////////////////////////////////////////////////////////////////
