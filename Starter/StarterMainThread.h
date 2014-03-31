// StarterMainThread.h
#pragma once


#include <map>
using namespace std;

#include "../NetworkCommon/DataTypes.h"
#include "../NetworkCommon/Utils/CommandLineParser.h"
#include "../NetworkCommon/Utils/Utils.h"
#include "../NetworkCommon/NetworkIn/Khaan.h"
#include "../NetworkCommon/Database/Deltadromeus.h"

#include "../NetworkCommon/NetworkIn/Diplodocus.h"
#include "../NetworkCommon/NetworkIn/KhaanServerToServer.h"

///////////////////////////////////////////////////////////////////

class StarterMainThread :   public Diplodocus< KhaanServerToServer >
{
public:
   StarterMainThread( const string& serverName, U32 serverId );
   ~StarterMainThread();

   bool     AddInputChainData( BasePacket* packet, U32 connectionId );
   bool     AddOutputChainData( BasePacket* packet, U32 connectionId );
   bool     SendMessageToClient( BasePacket* packet, U32 connectionId );

private:
   int      MainLoop_InputProcessing();
   int      MainLoop_OutputProcessing();
};

///////////////////////////////////////////////////////////////////