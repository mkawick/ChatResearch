// KhaanConnector.h

#pragma once

#include "../NetworkCommon/DataTypes.h"
#include "../NetworkCommon/NetworkIn/khaan.h"

class DiplodocusLoadBalancer;

//--------------------------------------------------------------

class KhaanConnector :
   public Khaan
{
public:
   KhaanConnector( int id, bufferevent* be );
   ~KhaanConnector();

   void     SetGateway( DiplodocusLoadBalancer* loadBalancer ) { m_loadBalancer = loadBalancer; }
   bool	   OnDataReceived( unsigned char* data, int length );

private:
   
   bool  IsWhiteListedIn( const BasePacket* packet ) const;

   DiplodocusLoadBalancer*    m_loadBalancer;

   bool                       m_denyAllFutureData;
   bool                       m_markedToBeCleanedup;

};

//--------------------------------------------------------------