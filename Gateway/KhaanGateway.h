// KhaanGateway.h

#pragma once

#include "../NetworkCommon/NetworkIn/KhaanProtected.h"

class MainGatewayThread;

//--------------------------------------------------------------

class KhaanGateway : public KhaanProtected
{
public:
   KhaanGateway( int id, bufferevent* be );
   ~KhaanGateway();
   const char* GetClassName() const { return "KhaanGateway"; }

   void     AuthorizeConnection();
   void     DenyAllFutureData();
   void     SetAdminLevelOperations( int level ) { m_adminLevel = level; }
   U8       GetLanguageId() const { return m_languageId; }
   void     SetLanguageId( U8 languageId );

   //void     SetGateway( MainGatewayThread* gateway ) { m_gateway = gateway; }
   void     ThrottleConnection( U32 timeoutMs ) { m_timeoutMs = timeoutMs; }

   void     SetLastGameConnectedTo( U8 gameId ) { m_gameId = gameId; }
   U8       GetLastGameConnectedTo() { return m_gameId; }

   bool	   Update();

private:
   
   bool     IsWhiteListedIn( const BasePacket* packet ) const;
   bool     HasPermission( const BasePacket* packet ) const;
   bool     HandleGatewayPackets( const BasePacket* packet ) const;
   bool     IsPacketSafe( const U8* data, int& offset);
   bool     IsHandshaking( const BasePacket* packetIn );
   bool     IsAuthorized() const { return m_authorizedConnection; }
   bool     TrackInwardPacketType( const BasePacket* packet ); // based on base class
   bool     TrackOutwardPacketType( const BasePacket* packet );
   void     SetupOutputDelayTimestamp();
   bool     ShouldDelayOutput();

   U32                  m_numPacketsReceivedBeforeAuth;
   U32                  m_randomNumberOfPacketsBeforeLogin;
   bool                 m_authorizedConnection;
   bool                 m_logoutPacketSent;
   int                  m_adminLevel;
   U8                   m_languageId;
   U32                  m_timeoutMs;
   U32                  m_lastSentToClientTimestamp;
   U8                   m_gameId;
   
   void  PreCleanup();
   //bool	OnDataReceived( const U8* data, int length );
   //bool  HandleInwardSerializedPacket( const U8* data, int& offset );
};

//--------------------------------------------------------------
