// ServerToServerPacket.h
#pragma once

#include "BasePacket.h"

///////////////////////////////////////////////////////////////

class PacketServerIdentifier : public BasePacket
{
public:
   PacketServerIdentifier( int packet_type = PacketType_ServerInformation, int packet_sub_type = 0  ): BasePacket( packet_type, packet_sub_type ), serverId( 0 ), isGameServer( true ), isController( false ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string      serverName;
   U32         serverId;
   bool        isGameServer;
   bool        isController;
};

///////////////////////////////////////////////////////////////

class PacketServerToServerWrapper : public BasePacket
{
public:
   PacketServerToServerWrapper( int packet_type = PacketType_ServerToServerWrapper, int packet_sub_type = 0  ): BasePacket( packet_type, packet_sub_type ), pPacket( NULL ), serverId( 0 ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   U32         serverId;
   BasePacket* pPacket;
};

///////////////////////////////////////////////////////////////

class PacketServerJobWrapper : public BasePacket
{
public:
   PacketServerJobWrapper( int packet_type = PacketType_ServerJobWrapper, int packet_sub_type = 0  ): BasePacket( packet_type, packet_sub_type ), pPacket( NULL ), serverId( 0 ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   U32         serverId;
   U32         jobId;
   BasePacket* pPacket;
};

///////////////////////////////////////////////////////////////
