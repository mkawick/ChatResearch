// KhaanGateway.h

#pragma once

#include "../NetworkCommon/DataTypes.h"
#include "../NetworkCommon/NetworkIn/Khaan.h"

class DiplodocusGateway;

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

   void     SetGateway( DiplodocusGateway* gateway ) { m_gateway = gateway; }

private:
   
   bool  IsWhiteListedIn( const BasePacket* packet ) const;
   bool  HasPermission( const BasePacket* packet ) const;
   bool  HandleGatewayPackets( const BasePacket* packet ) const;
   bool  IsPacketSafe( unsigned char* data, int& offset);
   bool  IsHandshaking( const BasePacket* packetIn );
   bool  TrackInwardPacketType( const BasePacket* packet ); // based on base class
   bool  TrackOutwardPacketType( const BasePacket* packet );

   U32                  m_numPacketsReceivedBeforeAuth;
   U32                  m_randomNumberOfPacketsBeforeLogin;
   bool                 m_authorizedConnection;
   bool                 m_denyAllFutureData;
   bool                 m_logoutPacketSent;
   int                  m_adminLevel;
   U8                   m_languageId;
   DiplodocusGateway*   m_gateway;

   void  PreCleanup();
   bool	OnDataReceived( unsigned char* data, int length );
};

//--------------------------------------------------------------
