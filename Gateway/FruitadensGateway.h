#pragma once

#include "../NetworkCommon/NetworkOut/Fruitadens.h"

#include "GatewayCommon.h"

class MainGatewayThread;
//-----------------------------------------------------------------------------

class FruitadensGateway : public FruitadensServer
{
public:
   FruitadensGateway( const char* name );
   ~FruitadensGateway();
   const char* GetClassName() const { return "FruitadensGateway"; }

   void     Enable() { m_isEnabled = true; }
   void     Disable() { m_isEnabled = false; }
   bool     IsEnabled() const { return m_isEnabled; }

   bool     AcceptsPacketType( U32 type ) const;
   bool     IsReadyToTrack() const { return m_gateway != NULL; }

private:

   bool     FilterOutwardPacket( BasePacket* packet ) const;
   int      MainLoop_OutputProcessing();

   void     PreprocessPacketsForOutput( BasePacket* packet );
   void     PostProcessInputPackets( int bytesRead );

   int      MainLoop_InputProcessing();
   void     FindGateway();

   MainGatewayThread*   m_gateway;
   bool                 m_isEnabled;
   
};

//-----------------------------------------------------------------------------