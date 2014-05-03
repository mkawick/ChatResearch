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

typedef int SocketType;
#else

typedef unsigned int SocketType;
#endif

class BasePacket;
class PacketRerouteRequestResponse;
class Fruitadens;

struct TempPacket
{
   U8    buffer [ MaxBufferSize ];
   int   size;
};

struct PacketHandlerInterface
{
   virtual bool  InitialConnectionCallback( const Fruitadens* ){ return false; }
   virtual bool  InitialDisconnectionCallback( const Fruitadens* ){ return false; }
   virtual bool  HandlePacketReceived( BasePacket* packetIn ){ return false; }
};

//-------------------------------------------------------------------------

class Fruitadens : public Threading::CChainedThread <BasePacket*>
{
public:
   Fruitadens( const char* name, bool processOnlyOneIncommingPacketPerLoop = false );
   ~Fruitadens();

   void        SetName( std::string& name ) { m_name = name; }
   std::string GetName() const { return m_name; }

   void        RegisterPacketHandlerInterface( PacketHandlerInterface* handler ) { m_packetHandlerInterface = handler; }

   bool        AddOutputChainData( BasePacket* packet, U32 filingData );// standard code, no need to modify
   
   void        SetConnectedServerType( ServerType type ) { m_serverType = type; }
   ServerType  GetConnectedServerType() const { return m_serverType; }

   bool        IsSocketValid() const { return m_clientSocket != SOCKET_ERROR; }
   bool        Connect( const char* serverName, int port );
   bool        Disconnect();

   bool        IsConnected() const { return m_isConnected; }
   void        SetServerUniqueId( U32 id ) { m_serverId = id; }

   virtual bool   NeedsProcessingTime() const { return true; }
   virtual void   HasBeenConnectedCallback();
   virtual void   HasBeenDisconnectedCallback();

protected:

   bool           SetupConnection( const char* serverName, int port );
   bool           CreateSocket();

   int            MainLoop_InputProcessing();
   int            MainLoop_OutputProcessing();

   virtual void   PostProcessInputPackets( int bytesRead );

   virtual bool   HandlePacketReceived( BasePacket* packetIn );

   virtual void   InitialConnectionCallback() {} // this will be invoked on every reconnect 

   virtual void   InheritedUpdate() {}

public:
   virtual bool   SerializePacketOut( const BasePacket* packet );
   bool           SendPacket( const U8* buffer, int length ) const;
protected:

   virtual bool   FilterOutwardPacket( BasePacket* packet ) const { return true; }// true means acceptable

   void           AttemptConnection();
   void           SocketHasDisconnectedDuringRecv( int error_number );

   SocketType           m_clientSocket;
   bool                 m_isConnected;
   bool                 m_hasFailedCritically;
   bool                 m_processOnlyOneIncommingPacketPerLoop;
   bool                 m_checkForReroute;
   U32                  m_connectedServerId;
   U8                   m_connectedGameProductId;

   sockaddr_in          m_ipAddress;
   U16                  m_port;

   ServerType           m_serverType;
   std::string          m_name;
   PacketQueue          m_packetsReadyToSend;
   U32                  m_serverId;
   U32                  m_numPacketsReceived; // tracking only.. no other purpose

   U32                  m_receiveBufferSize;
   U32                  m_receiveBufferOffset;
   U8*                  m_receiveBuffer;
   PacketHandlerInterface*    m_packetHandlerInterface;

   enum { OverflowBufferSize = 12*1024 };

   U8       m_overflowBuffer[ OverflowBufferSize ];
   int      m_bytesInOverflow;
};

//-------------------------------------------------------------------------

class FruitadensServer : public Fruitadens
{
public:
   FruitadensServer( const char* name, bool processOnlyOneIncommingPacketPerLoop = false );
   void        NotifyEndpointOfIdentification( const string& serverName, const string& serverAddress, U32 serverId, U16 serverPort, U8 gameProductId, bool isGameServer, bool isController, bool requiresWrapper, U8 gatewayType );

   void        InitialConnectionCallback();

   bool        IsGameServer() const { return m_localIsGameServer; }

protected:

   bool        PackageLocalServerIdentificationToSend();

   bool        m_areLocalIdentifyingParamsSet;
   string      m_localServerName;
   string      m_localIpAddress;
   U32         m_localServerId;
   U16         m_localServerPort;
   U8          m_localGameProductId;
   bool        m_localIsGameServer;
   bool        m_localIsController;
   bool        m_localRequiresWrapper;
   U8          m_gatewayType;
};

//-------------------------------------------------------------------------
