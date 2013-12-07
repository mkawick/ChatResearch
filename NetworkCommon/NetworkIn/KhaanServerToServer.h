#pragma once
#include "Khaan.h"

class PacketServerIdentifier;

//////////////////////////////////////////////////////////////////////////////////

class KhaanServerToServer : public Khaan
{
public:
   KhaanServerToServer() : Khaan( 0, NULL ), m_serverId( 0 ), m_isGameServer( false ), m_isController( false ), m_isGateway( false ) {}
   KhaanServerToServer( int id, bufferevent* be ) : Khaan( id, be ), m_serverId( 0 ), m_isGameServer( false ), m_isController( false ), m_isGateway( false )  {}

   bool	   OnDataReceived( unsigned char* data, int length );

   
   void     SetServerName( const string& name ) { m_serverName = name; }
   string   SetServerAddr( const string& addr ) { m_serverAddress = addr; }
   U32      SetServerId( U32 id) { m_serverId = id; }
   U16      SetServerName( U16 port) { m_serverPort = port; }
   bool     SetIsGameServer( bool isServer) { m_isGameServer = isServer; }
   bool     SetIsController( bool isController) { m_isController = isController; }
   bool     SetIsGateway( bool isGateWay ) { m_isGateway = isGateWay; }

   string   GetServerName() const { return m_serverName; }
   string   GetServerAddress() const { return m_serverAddress; }
   U32      GetServerId() const { return m_serverId; }
   U16      GetServerPort() const { return m_serverPort; }
   bool     IsGameServer() const { return m_isGameServer; }
   bool     IsController() const { return m_isController; }
   bool     IsGateway() const { return m_isGateway; }

protected:
   void  PreCleanup();

   void	         UpdateInwardPacketList();
   void           RequestUpdate();
   virtual bool   PassPacketOn( BasePacket* serverId, U32 connectionId );// this is good to override
   bool           HandleCommandFromGateway( BasePacket* packet, U32 connectionId );
   void           SaveOffServerIdentification( const PacketServerIdentifier* serverId );

   string      m_serverName;
   string      m_serverAddress;
   U32         m_serverId;
   U16         m_serverPort;
   bool        m_isGameServer;
   bool        m_isController;
   bool        m_isGateway;
};

//////////////////////////////////////////////////////////////////////////////////