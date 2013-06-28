#pragma once

#include "KhaanServerToServer.h"
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
   DiplodocusServerToServer( const string& serverName, U32 serverId, U8 gameProductId = 0 );
   ~DiplodocusServerToServer();

   bool     AddInputChainData( BasePacket* packet, U32 connectionId );
   bool     AddOutputChainData( BasePacket* packet, U32 connectionId ) ;

   void     ClientConnectionFinishedAdding( BaseInputChainHandler* khaan );
   void     ClientConnectionIsAboutToRemove( BaseInputChainHandler* khaan );

   //---------------------------------------------------

   void     ServerWasIdentified( KhaanServerToServer* khaan );

private:

   void  CreateJob( const KhaanServerToServer* khaan, BasePacket* packet );
   int   CallbackFunction();

   deque<U32>     m_serversNeedingUpdate;
   U32            m_jobIdTracker;
};

//---------------------------------------------------------------
