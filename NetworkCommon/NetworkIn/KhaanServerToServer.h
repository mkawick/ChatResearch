
#pragma once
#include "Khaan.h"

#include "../Packets/ServerToServerPacket.h"

//////////////////////////////////////////////////////////////////////////////////

class KhaanServerToServer : public Khaan
{
public:
   KhaanServerToServer();
   KhaanServerToServer( int id, bufferevent* be );

   const char* GetClassName() const { return "KhaanServerToServer"; }

   bool	   OnDataReceived( const U8* data, int length );

   
   void     SetServerName( const string& name ) { m_serverName = name; }
   void     SetServerAddr( const string& addr ) { m_serverAddress = addr; }
   void     SetServerId( U32 id) { m_serverId = id; }
   void     SetServerName( U16 port) { m_serverPort = port; }
   void     SetIsGameServer( bool isServer) { m_isGameServer = isServer; }
   void     SetIsController( bool isController) { m_isController = isController; }
   //void     SetIsGateway( bool isGateWay ) { m_isGateway = isGateWay; }
   void     SetGatewayType( PacketServerIdentifier::GatewayType type ) { m_gatewayType = type; }

   string   GetServerName() const { return m_serverName; }
   string   GetServerAddress() const { return m_serverAddress; }
   const string& GetExternalIpAddress() const { return m_externalIpAddress; }
   U32      GetServerId() const { return m_serverId; }
   U16      GetServerPort() const { return m_serverPort; }
   bool     IsGameServer() const { return m_isGameServer; }
   bool     IsController() const { return m_isController; }
   U8       GetGatewayType() const { return m_gatewayType; }
   //bool     IsGateway() const { if( m_gatewayType == PacketServerIdentifier::GatewayType_None ) return false; return true; }

protected:
   void  PreCleanup();

   bool           HandleInwardSerializedPacket( const U8* data, int& offset );
   void	         UpdateInwardPacketList();
   void           RequestUpdate();
   virtual bool   PassPacketOn( BasePacket* serverId, U32 connectionId );// this is good to override
   bool           HandleCommandFromGateway( BasePacket* packet, U32 connectionId );
   void           SaveOffServerIdentification( const PacketServerIdentifier* serverId );

   string      m_serverName;
   string      m_serverAddress;
   string      m_externalIpAddress;
   U32         m_serverId;
   U16         m_serverPort;
   bool        m_isGameServer;
   bool        m_isController;
   U8          m_gatewayType;

  /* bool                 m_isExpectingMoreDataInPreviousPacket;
   int                  m_expectedBytesReceivedSoFar;
   int                  m_expectedBytes;*/
};

//////////////////////////////////////////////////////////////////////////////////