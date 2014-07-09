#pragma once

#include "KhaanServerToServer.h"
#include "Diplodocus.h"
#include <deque>
using namespace std;

//---------------------------------------------------------------

class ServerToServerJob
{
public:
   ServerToServerJob( U32 job_id, U32 server_id) : jobId( job_id ), serverId( server_id ) {}

   U32   jobId;
   U32   serverId;
};

//---------------------------------------------------------------

class DiplodocusServerToServer : public Diplodocus< KhaanServerToServer >
{
public: 
   typedef Diplodocus< KhaanServerToServer > ChainedType;
public:
   DiplodocusServerToServer( const string& serverName, U32 serverId, U8 gameProductId = 0, ServerType type = ServerType_Chat );
   ~DiplodocusServerToServer();

   bool     AddInputChainData( BasePacket* packet, U32 connectionId );
   bool     AddOutputChainData( BasePacket* packet, U32 connectionId ) ;

   void     InputConnected( IChainedInterface* khaan );
   void     InputRemovalInProgress( IChainedInterface* khaan );

   U32      FindServerIdByType( U32 type );
   //---------------------------------------------------

   void     ServerWasIdentified( IChainedInterface* khaan );

private:

   virtual void  CreateJob( const KhaanServerToServer* khaan, BasePacket* packet );

   //void	   UpdateAllConnections();
   void     SendJobsToUpperLayers();
   int      CallbackFunction();

   deque< PacketServerJobWrapper* >    m_unprocessedJobs;
   deque<U32>     m_serversNeedingUpdate;
   U32            m_jobIdTracker;
};

//---------------------------------------------------------------
