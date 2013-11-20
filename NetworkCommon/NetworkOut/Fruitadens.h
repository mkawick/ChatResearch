// Fruitadens.h

#pragma once

#include "../DataTypes.h"
#include "../ChainedArchitecture/ChainedThread.h"
#include "../ServerType.h"
#include <string>

#if PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX

#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>

#endif

class BasePacket;
class PacketRerouteRequestResponse;

struct TempPacket
{
   U8    buffer [ MaxBufferSize ];
   int   size;
};

//-------------------------------------------------------------------------

class Fruitadens : public Threading::CChainedThread <BasePacket*>
{
public:
   Fruitadens( const char* name, bool processOnlyOneIncommingPacketPerLoop = false );
   ~Fruitadens();

   bool        AddOutputChainData( BasePacket* packet, U32 filingData );// standard code, no need to modify
   void        NotifyEndpointOfIdentification( const std::string& serverName, U32 serverId, U8 gameProductId, bool isGameServer, bool isController, bool requiresWrapper, bool isGateway );

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

   void           RequestRerouteInstructions();
   void           HandleRerouteRequestResult( PacketRerouteRequestResponse* response );

   virtual void   PostProcessInputPackets( int bytesRead );

   virtual bool   HandlePacketReceived( BasePacket* packetIn );

   virtual bool   SerializePacketOut( const BasePacket* packet );
   bool           SendPacket( const U8* buffer, int length ) const;

   virtual void   InitalConnectionCallback() {} // this will be invoked on every reconnect 

protected:

   virtual bool   FilterOutwardPacket( BasePacket* packet ) const { return true; }// true means acceptable

   void           AttemptConnection();

   int                  m_clientSocket;
   bool                 m_isConnected;
   bool                 m_hasFailedCritically;
   bool                 m_processOnlyOneIncommingPacketPerLoop;
   bool                 m_checkForReroute;
   U32                  m_connectedServerId;
   U8                   m_connectedGameProductId;

   sockaddr_in          m_ipAddress;
   U16                  m_port;

   bool                 m_awaitingReroute;
   string               m_rerouteAddress;
   U16                  m_reroutePort;

   ServerType           m_serverType;
   std::string          m_name;
   PacketQueue          m_packetsReadyToSend;
   U32                  m_serverId;
   U32                  m_numPacketsReceived; // tracking only.. no other purpose

   U32                  m_receiveBufferSize;
   U32                  m_receiveBufferOffset;
   U8*                  m_receiveBuffer;

   enum { OverflowBufferSize = 12*1024 };

   U8       m_overflowBuffer[ OverflowBufferSize ];
   int      m_bytesInOverflow;
};

//-------------------------------------------------------------------------
