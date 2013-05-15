// KhaanConnector.h

#pragma once

#include "../NetworkCommon/DataTypes.h"
#include "../NetworkCommon/NetworkIn/khaan.h"

class DiplodocusGateway;

//--------------------------------------------------------------

class KhaanConnector : public Khaan
{
public:
   KhaanConnector( int id, bufferevent* be );
   ~KhaanConnector();

   void     AuthorizeConnection() { m_authorizedConnection = true; }
   void     DenyAllFutureData() { m_denyAllFutureData = true; }
   U32      GetConnectionId() const { return m_connectionId; }
   void     SetConnectionId( U32 id ) { m_connectionId = id; }

   void     SetGateway( DiplodocusGateway* gateway ) { m_gateway = gateway; }
   //bool     AddOutputChainData( BasePacket* packet, int filingData = -1 );// already built in base class. Completely encapsulated.

private:
   
   bool  IsWhiteListedIn( const BasePacket* packet ) const;

   U32                  m_connectionId;
   U32                  m_numPacketsReceivedBeforeAuth;
   bool                 m_authorizedConnection;
   bool                 m_denyAllFutureData;
   DiplodocusGateway*   m_gateway;

   void  PreCleanup();
   bool	OnDataReceived( unsigned char* data, int length );
};

//--------------------------------------------------------------
