// KhaanConnector.h

#pragma once

#include "../NetworkCommon/DataTypes.h"
#include "../NetworkCommon/NetworkIn/Khaan.h"

class DiplodocusLoadBalancer;

//--------------------------------------------------------------

class KhaanConnector :
   public Khaan
{
public:
   KhaanConnector( int id, bufferevent* be );
   ~KhaanConnector();

   void     SetGateway( DiplodocusLoadBalancer* loadBalancer ) { m_loadBalancer = loadBalancer; }
   bool	   OnDataReceived( const U8* data, int length );

private:
   
   const char* GetClassName() const { return "KhaanConnector"; }
   bool  IsWhiteListedIn( const BasePacket* packet ) const;

   DiplodocusLoadBalancer*    m_loadBalancer;

   bool                       m_denyAllFutureData;
   bool                       m_markedToBeCleanedup;

};

//--------------------------------------------------------------