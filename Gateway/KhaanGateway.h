// KhaanGateway.h

#pragma once

#include "../NetworkCommon/DataTypes.h"
#include "../NetworkCommon/NetworkIn/Khaan.h"

class MainGatewayThread;

//--------------------------------------------------------------

class KhaanGateway : public Khaan
{
public:
   KhaanGateway( int id, bufferevent* be );
   ~KhaanGateway();

   void     AuthorizeConnection();
   void     ForceShutdown();
   void     DenyAllFutureData();
   void     SetAdminLevelOperations( int level ) { m_adminLevel = level; }
   U8       GetLanguageId() const { return m_languageId; }
   void     SetLanguageId( U8 languageId );

   void     SetGateway( MainGatewayThread* gateway ) { m_gateway = gateway; }
   void     ThrottleConnection( U32 timeoutMs ) { m_timeoutMs = timeoutMs; }

   void     SetLastGameConnectedTo( U8 gameId ) { m_gameId = gameId; }
   U8       GetLastGameConnectedTo() { return m_gameId; }

   bool	   Update();

private:
   
   bool  IsWhiteListedIn( const BasePacket* packet ) const;
   bool  HasPermission( const BasePacket* packet ) const;
   bool  HandleGatewayPackets( const BasePacket* packet ) const;
   bool  IsPacketSafe( unsigned char* data, int& offset);
   bool  IsHandshaking( const BasePacket* packetIn );
   bool  TrackInwardPacketType( const BasePacket* packet ); // based on base class
   bool  TrackOutwardPacketType( const BasePacket* packet );
   void  SetupOutputDelayTimestamp();
   bool  ShouldDelayOutput();

   U32                  m_numPacketsReceivedBeforeAuth;
   U32                  m_randomNumberOfPacketsBeforeLogin;
   bool                 m_authorizedConnection;
   bool                 m_denyAllFutureData;
   bool                 m_logoutPacketSent;
   bool                 m_isExpectingMoreDataInPreviousPacket;
   int                  m_expectedBytesReceivedSoFar;
   int                  m_expectedBytes;
   int                  m_adminLevel;
   U8                   m_languageId;
   MainGatewayThread*   m_gateway;
   U32                  m_timeoutMs;
   U32                  m_lastSentToClientTimestamp;
   U8                   m_gameId;
   U8                   m_tempBuffer[ MaxBufferSize ];

   void  PreCleanup();
   bool	OnDataReceived( unsigned char* data, int length );
   bool  HandleInwardSerializedPacket( U8* data, int& offset );
};

//--------------------------------------------------------------
