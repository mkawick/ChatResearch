// KhaanConnector.h

#pragma once

#include "../NetworkCommon/NetworkIn/KhaanProtected.h"

//--------------------------------------------------------------

class KhaanConnector : public KhaanProtected
{
public:
   KhaanConnector( int id, bufferevent* be );
   ~KhaanConnector();

   const char* GetClassName() const { return "KhaanConnector"; }

protected:   
   bool     IsWhiteListedIn( const BasePacket* packet ) const;
};

//--------------------------------------------------------------