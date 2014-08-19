// UserStatsMainThread.h
#pragma once

#include <deque>
#include <map>
using namespace std;

#include "../NetworkCommon/DataTypes.h"
#include "../NetworkCommon/Utils/CommandLineParser.h"
#include "../NetworkCommon/Utils/Utils.h"
#include "../NetworkCommon/NetworkIn/Khaan.h"
#include "../NetworkCommon/Database/Deltadromeus.h"

#include "../NetworkCommon/NetworkIn/Diplodocus.h"
#include "../NetworkCommon/NetworkIn/KhaanServerToServer.h"

class PacketDbQuery;
class PacketDbQueryResult;
///////////////////////////////////////////////////////////////////


class UserStatsMainThread : public Diplodocus< KhaanServerToServer >
{
public: 
   typedef Diplodocus< KhaanServerToServer > ChainedType;
public:
   UserStatsMainThread( const string& serverName, U32 serverId );
   ~UserStatsMainThread();

   void     Init();

   bool     AddInputChainData( BasePacket* packet, U32 connectionId );
   bool     AddOutputChainData( BasePacket* packet, U32 connectionId );
   bool     SendMessageToClient( BasePacket* packet, U32 connectionId );
   bool     SendGameData( U32 connectionId, int packetSize, const U8* packet );
   bool     AddQueryToOutput( PacketDbQuery* dbQuery );

   void     ServerWasIdentified( IChainedInterface* khaan );
   void     InputRemovalInProgress( IChainedInterface * chainedInput );

   DbHandle*   GetDbConnectionByType( Database::Deltadromeus::DbConnectionType type );

private:
   int      MainLoop_InputProcessing();
   int      MainLoop_OutputProcessing();
   void     UpdateDbResults();

   bool     HandlePacketFromClient( BasePacket* packet, U32 connectionId );
   bool     HandlePacketFromOtherServer( BasePacket* packet, U32 connectionId );// not thread safe
   bool     HandleLoginPacket( BasePacket* packet, U32 connectionId );
   bool     HandleUserStatPacket( BasePacket* packet, U32 connectionId );


   deque< PacketDbQueryResult* > m_dbQueries;
};

///////////////////////////////////////////////////////////////////