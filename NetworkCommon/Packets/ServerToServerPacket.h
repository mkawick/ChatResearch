// ServerToServerPacket.h
#pragma once

#include "BasePacket.h"

///////////////////////////////////////////////////////////////

class PacketServerConnectionInfo : public BasePacket
{
public:
   enum 
   {
      PacketServerIdentifier_TypicalInfo,
      PacketServerIdentifier_ConnectionInfo,
      PacketServerIdentifier_Disconnect
   };

public:
   PacketServerConnectionInfo( int packet_type = PacketType_ServerInformation, int packet_sub_type = PacketServerIdentifier_ConnectionInfo  ): 
                           BasePacket( packet_type, packet_sub_type ), 
                           currentLoad( 0 ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string      serverAddress;
   U32         serverId;
   int         currentLoad;
   //string   timeLastConnection;
};

///////////////////////////////////////////////////////////////

class PacketServerIdentifier : public BasePacket
{
   
public:
   PacketServerIdentifier( int packet_type = PacketType_ServerInformation, int packet_sub_type = PacketServerConnectionInfo::PacketServerIdentifier_TypicalInfo ): 
                           BasePacket( packet_type, packet_sub_type ), 
                           serverId( 0 ), 
                           isGameServer( true ), 
                           isController( false ), 
                           isGateway( false ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string      serverName;
   string      serverAddress;
   U32         serverId;
   U16         serverPort;
   U32         gameInstanceId;// used to filter game packets
   bool        isGameServer;
   bool        isController;
   bool        isGateway;
};

///////////////////////////////////////////////////////////////

class PacketServerDisconnect : public BasePacket
{

public:
   PacketServerDisconnect( int packet_type = PacketType_ServerInformation, int packet_sub_type = PacketServerConnectionInfo::PacketServerIdentifier_Disconnect  ): 
                           BasePacket( packet_type, packet_sub_type ), 
                           serverId( 0 ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string      serverAddress;
   U32         serverId;
};

///////////////////////////////////////////////////////////////

class PacketServerToServerWrapper : public BasePacket
{
public:
   PacketServerToServerWrapper( int packet_type = PacketType_ServerToServerWrapper, int packet_sub_type = 0  ): BasePacket( packet_type, packet_sub_type ), serverId( 0 ), pPacket( NULL ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   U32         serverId;
   BasePacket* pPacket;
};

///////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////

class PacketServerJobWrapper : public BasePacket
{
public:
   PacketServerJobWrapper( int packet_type = PacketType_ServerJobWrapper, int packet_sub_type = 0  ): BasePacket( packet_type, packet_sub_type ), serverId( 0 ), pPacket( NULL ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   U32         serverId;
   U32         jobId;
   BasePacket* pPacket;
};


///////////////////////////////////////////////////////////////
