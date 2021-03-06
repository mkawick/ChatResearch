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
   bool     GetErrorConditionSet() const { return m_isErrorState; }

   void     SetUserName( const char* ptr ) { m_userName = ptr; }
   const char* GetUserName() { return m_userName.c_str(); }
   void     SetUserEmail( const char* ptr ) { m_userEmail = ptr; }
   const char* GetUserEmail() { return m_userEmail.c_str(); }
   void     SetUserUuid( const char* ptr ) { m_userUuid = ptr; }
   const char* GetUserUuid() { return m_userUuid.c_str(); }

   //void     SetGateway( MainGatewayThread* gateway ) { m_gateway = gateway; }
   void     ThrottleConnection( U32 timeoutMs ) { m_timeoutMs = timeoutMs; }

   void     SetLastGameConnectedTo( U8 gameId ) { m_gameId = gameId; }
   U8       GetLastGameConnectedTo() { return m_gameId; }

   bool     SendImmediately( const BasePacket* logoutPacket );// does not cleanup
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

   FixedString80        m_userName;
   FixedString80        m_userEmail;
   FixedString16        m_userUuid;
   U32                  m_numPacketsReceivedBeforeAuth;
   U32                  m_randomNumberOfPacketsBeforeLogin;
   bool                 m_authorizedConnection;
   bool                 m_logoutPacketSent;
   int                  m_adminLevel;
   U8                   m_languageId;
   U32                  m_timeoutMs;
   U32                  m_lastSentToClientTimestamp;
   U8                   m_gameId;
   bool                 m_isErrorState;
   
   void  PreCleanup();
   //bool	OnDataReceived( const U8* data, int length );
   //bool  HandleInwardSerializedPacket( const U8* data, int& offset );
};

//--------------------------------------------------------------
