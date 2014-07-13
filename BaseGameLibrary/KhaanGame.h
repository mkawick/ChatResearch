#pragma once
// KhaanGame.h

#include "../NetworkCommon/NetworkIn/KhaanServerToServer.h"
#include "../NetworkCommon/ServerConstants.h"

class PacketServerIdentifier;

struct TempStorage
{
   U8 data[ MaxBufferSize ];
   int size;
};

///////////////////////////////////////////////////////////////

class KhaanGame : public KhaanServerToServer
{
public:
   KhaanGame() : KhaanServerToServer(){}//, m_mainInterfacePtr( NULL )  {}
   KhaanGame( int id, bufferevent* be ) : KhaanServerToServer( id, be ) {}//, m_mainInterfacePtr( NULL ) {}

   const char* GetClassName() const { return "KhaanGame"; }
   //bool  OnDataReceived( unsigned char* data, int length );
private:

   //void  UpdateInwardPacketList();
   //bool  PassPacketOn( BasePacket* serverId, U32 connectionId );

   //void  SetupMainInterfacePointer();

   //ChainedInterface* m_mainInterfacePtr;
   //deque< TempStorage* > m_toBeProcessed;
   //Threading::Mutex m_mutex;
};

///////////////////////////////////////////////////////////////
