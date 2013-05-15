// PacketFactory.h

#pragma once

#include "../DataTypes.h"

class BasePacket;
///////////////////////////////////////////////////////////////

// The following class should be derived from and provide the uniqueness for each server product. The definition of which packets
// can be parsed, what they should have in them, etc. This should be a behavior-only class with minimal data needed for packing
// the various packets. This is a policy-class as recommended by Alexandrescu.

class PacketFactory
{
public:
	PacketFactory();
   bool     Parse( const U8* bufferIn, int& bufferOffset, BasePacket** packetOut ) const;// be sure to check the return value

private:

   bool     ParseLogin( const U8* bufferIn, int& bufferOffset, const BasePacket* firstPassParse, BasePacket** packetOut ) const;
   bool     ParseChat( const U8* bufferIn, int& bufferOffset, const BasePacket* firstPassParse, BasePacket** packetOut ) const;
   bool     ParseUserInfo( const U8* bufferIn, int& bufferOffset, const BasePacket* firstPassParse, BasePacket** packetOut ) const;
   bool     ParseDbQuery( const U8* bufferIn, int& bufferOffset, const BasePacket* firstPassParse, BasePacket** packetOut ) const;
   bool     ParseGame( const U8* bufferIn, int& bufferOffset, const BasePacket* firstPassParse, BasePacket** packetOut ) const;

   bool     ParseServerToServerWrapper( const U8* bufferIn, int& bufferOffset, const BasePacket* firstPassParse, BasePacket** packetOut ) const;
   bool     ParseServerInfo( const U8* bufferIn, int& bufferOffset, const BasePacket* firstPassParse, BasePacket** packetOut ) const;
   bool     ParseGatewayWrapper( const U8* bufferIn, int& bufferOffset, const BasePacket* firstPassParse, BasePacket** packetOut ) const;
};

///////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////