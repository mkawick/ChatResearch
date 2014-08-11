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

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   string      serverAddress;
   U32         serverId;
   int         currentLoad;
   //string   timeLastConnection;
};

///////////////////////////////////////////////////////////////

class PacketServerIdentifier : public BasePacket
{
public:
   enum GatewayType
   {
      GatewayType_None,
      GatewayType_Normal,
      GatewayType_Asset
   };
   
public:
   PacketServerIdentifier( int packet_type = PacketType_ServerInformation, int packet_sub_type = PacketServerConnectionInfo::PacketServerIdentifier_TypicalInfo ): 
                           BasePacket( packet_type, packet_sub_type ), 
                           serverId( 0 ), 
                           isGameServer( true ), 
                           isController( false ), 
                           gatewayType( GatewayType_None ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   string      serverName;
   string      serverAddress;
   string      externalIpAddress;
   U32         serverId;
   U16         serverPort;
   U32         gameInstanceId;// used to filter game packets
   U8          serverType;
   bool        isGameServer;
   bool        isController;
   U8          gatewayType;
};

///////////////////////////////////////////////////////////////

class PacketServerDisconnect : public BasePacket
{

public:
   PacketServerDisconnect( int packet_type = PacketType_ServerInformation, int packet_sub_type = PacketServerConnectionInfo::PacketServerIdentifier_Disconnect  ): 
                           BasePacket( packet_type, packet_sub_type ), 
                           serverId( 0 ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   string      serverAddress;
   U32         serverId;
};

///////////////////////////////////////////////////////////////

class PacketServerToServerWrapper : public BasePacket
{
public:
   PacketServerToServerWrapper( int packet_type = PacketType_ServerToServerWrapper, int packet_sub_type = 0  ): BasePacket( packet_type, packet_sub_type ), serverId( 0 ), pPacket( NULL ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   U32         serverId;
   BasePacket* pPacket;
};

///////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////

class PacketServerJobWrapper : public BasePacket
{
public:
   PacketServerJobWrapper( int packet_type = PacketType_ServerJobWrapper, int packet_sub_type = 0  ): BasePacket( packet_type, packet_sub_type ), serverId( 0 ), pPacket( NULL ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   U32         serverId;
   U32         jobId;
   BasePacket* pPacket;
};


///////////////////////////////////////////////////////////////
