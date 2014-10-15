// CheatPacket.h
#pragma once

#include "BasePacket.h"

///////////////////////////////////////////////////////////////////

#pragma pack( push, 4 )

///////////////////////////////////////////////////////////////

class PacketCheat : public BasePacket
{
public:
   enum
   {
      Cheat_Basic = 1,
      Cheat_Admin_RequestUserProfile
   };
public:
   PacketCheat(): BasePacket( PacketType_Cheat, PacketCheat::Cheat_Basic ) {}
   
   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;
   
   int               whichServer; // the client should tell us which server gets this... use PacketType to designate which server
   BoundedString32   cheat;
};


///////////////////////////////////////////////////////////////////
#pragma pack( pop )