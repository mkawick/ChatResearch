// Fruitadens.h

#pragma once

#include "../DataTypes.h"
#include "../ChainedArchitecture/ChainedThread.h"
#include "../ServerType.h"
#include <string>

class BasePacket;
//-------------------------------------------------------------------------

class Fruitadens : public Threading::CChainedThread <BasePacket*>
{
public:
   Fruitadens( const char* name );

   bool        AddOutputChainData( BasePacket* packet, U32 filingData );// standard code, no need to modify
   void        NotifyEndpointOfIdentification( const std::string& serverName, U32 serverId, bool isGameServer, bool isController, bool requiresWrapper = true, bool isGateway = false );

   void        SetConnectedServerType( ServerType type ) { m_serverType = type; }
   ServerType  GetConnectedServerType() const { return m_serverType; }

   bool        Connect( const char* serverName, int port );
   bool        Disconnect();

   bool        IsConnected() const { return m_isConnected; }
   void        SetServerUniqueId( U32 id ) { m_serverId = id; }

protected:

   bool           SetupConnection( const char* serverName, int port );
   bool           CreateSocket();

   int            ProcessInputFunction();
   int            ProcessOutputFunction();
   virtual bool   HandlePacketReceived( BasePacket* packetIn );

   virtual bool   SerializePacketOut( const BasePacket* packet );
   bool           SendPacket( const U8* buffer, int length );

protected:

   virtual bool   FilterOutwardPacket( BasePacket* packet ) const { return true; }// true means acceptable

   void           AttemptConnection();

   int               m_clientSocket;
   bool              m_isConnected;
   bool              m_hasFailedCritically;
   U32               m_connectedServerId;
   sockaddr_in       m_ipAddress;
   U16               m_port;
   ServerType        m_serverType;
   std::string       m_name;
   PacketQueue       m_packetsReadyToSend;
   U32               m_serverId;
};

//-------------------------------------------------------------------------
