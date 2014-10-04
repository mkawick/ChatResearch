// DiplodocusServerToServer.cpp

#include <assert.h>

#include "../ServerConstants.h"

#if PLATFORM == PLATFORM_WINDOWS
#pragma warning( disable:4996 )
#endif

#include "../DataTypes.h"

#include "../Utils/CommandLineParser.h"
#include "../Utils/Utils.h"
#include "../Packets/ServerToServerPacket.h"

#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
using boost::format;

#include "../NetworkIn/Khaan.h"
#include "../NetworkIn/Diplodocus.h"
#include "../Database/Deltadromeus.h"

#include "DiplodocusServerToServer.h"

DiplodocusServerToServer::DiplodocusServerToServer( const string& serverName, U32 serverId, U8 gameProductId, ServerType type ) : Diplodocus< KhaanServerToServer >( serverName, serverId, gameProductId, type ), 
                                    m_jobIdTracker( 32 ) // 32 is a non-zero value useful for test only
{
   m_chainedType = ChainedType_AlternateThreadContainer;
   SetConnectionId( ServerToServerConnectionId );
   SetSleepTime( 100 );
}

DiplodocusServerToServer::~DiplodocusServerToServer()
{
}


//////////////////////////////////////////////////////////////////////////

void  DiplodocusServerToServer::InputConnected( IChainedInterface* chainedInput ) 
{
   cout << "DiplodocusServerToServer::InputConnected" << endl;
   KhaanServerToServer* khaan = static_cast< KhaanServerToServer* >( chainedInput );
   MarkConnectionAsNeedingUpdate( khaan->GetChainedId() );
}


//////////////////////////////////////////////////////////////////////////

void  DiplodocusServerToServer::InputRemovalInProgress( IChainedInterface* chainedInput )
{
   KhaanServerToServer* khaan = static_cast< KhaanServerToServer* >( chainedInput );

   SetupClientWaitingToBeRemoved( khaan );

   string currentTime = GetDateInUTC();
   string printer = "Client disconnection at time:" + currentTime + " from " + inet_ntoa( khaan->GetIPAddress().sin_addr );
   LogMessage( LOG_PRIO_INFO, printer.c_str() );

   LogMessage( LOG_PRIO_INFO, "** InputRemovalInProgress" );
}

//////////////////////////////////////////////////////////////////////////

U32   DiplodocusServerToServer::FindServerIdByType( U32 type )
{
   ChainLinkIteratorType itInputs = m_listOfInputs.begin();
   while( itInputs != m_listOfInputs.end() )
   {
      ChainLink& chainedInput = *itInputs++;

      IChainedInterface* interfacePtr = chainedInput.m_interface;
      KhaanServerToServer* khaan = static_cast< KhaanServerToServer* >( interfacePtr );
      if( khaan->GetServerType() == type )
         return khaan->GetServerId();
   }
   return 0;
}


//////////////////////////////////////////////////////////////////////////

// this will always be data coming from other servers or at least from the outside in.
bool   DiplodocusServerToServer::AddInputChainData( BasePacket* packet, U32 connectionId ) 
{
   cout << "DiplodocusServerToServer::AddInputChainData" << endl;
   //LogMessage( LOG_PRIO_INFO, "DiplodocusServerToServer::AddInputChainData( " );
   if( packet->packetType == PacketType_ServerToServerWrapper )
   {
      //HandleCommandFromGateway( packet, connectionId );
      PacketServerToServerWrapper* wrapper = static_cast< PacketServerToServerWrapper* >( packet );
      BasePacket* unwrappedPacket = wrapper->pPacket;
      U32         serverId = wrapper->serverId;

      //LogMessage( LOG_PRIO_INFO, "AddInputChainData start lock" );
      Threading::MutexLock locker( m_mutex );
      //LogMessage( LOG_PRIO_INFO, "AddInputChainData finish lock" );

      bool  found = false;
      KhaanServerToServer* khaan = NULL;
      ChainLinkIteratorType itInputs = m_listOfInputs.begin();
      while( itInputs != m_listOfInputs.end() )
      {
         ChainLink& chainedInput = *itInputs++;
	      IChainedInterface* interfacePtr = chainedInput.m_interface;
         khaan = static_cast< KhaanServerToServer* >( interfacePtr );

         if( serverId == khaan->GetServerId() )
         {
            found = true;
            break;
         }
      }
      if( found == false || khaan == NULL )
      {
         //assert( 0 );
         return false;
      }
      

      // create job for this packet including the serverId, a unique job id, and so on. Keep in mind that the connection may disappear
      // during the servicing of this job
      CreateJob( khaan, unwrappedPacket );

      return true;
   }

   return true;
}


//////////////////////////////////////////////////////////////////////////

void  DiplodocusServerToServer::ServerWasIdentified( IChainedInterface* khaan )
{
   cout << "DiplodocusServerToServer::ServerWasIdentified" << endl;
   BasePacket* packet = NULL;
   PackageForServerIdentification( m_serverName, m_localIpAddress, m_externalIpAddress, m_serverId, m_serverType, m_listeningPort, m_gameProductId, m_isGame, m_isControllerApp, true, m_gatewayType, &packet );
   ChainedType* localKhaan = static_cast< ChainedType* >( khaan );
   
   LockMutex();
   m_idPacketsToHandle.push_back( PacketHolder( localKhaan->GetChainedId(), packet ) );
   UnlockMutex();
}

//////////////////////////////////////////////////////////////////////////

void  DiplodocusServerToServer::HandleInputPackets()
{
   LockMutex();
   deque< PacketHolder >  tempQueue = m_idPacketsToHandle;
   m_idPacketsToHandle.clear();
   UnlockMutex();

   U32 numPacketsToProcess = tempQueue.size();
   if( numPacketsToProcess== 0 )
   {
      return;
   }

   cout << "DiplodocusServerToServer::HandleInputPackets::numPacketsToProcess = " << numPacketsToProcess << endl;

   PacketFactory factory;
   while( tempQueue.size() )
   {
      PacketHolder holder = tempQueue.front();
      tempQueue.pop_front();

      m_inputChainListMutex.lock();
      KhaanServerToServer* localKhaan = NULL;
      ChainLinkIteratorType itInputs = m_listOfInputs.begin();
      while( itInputs != m_listOfInputs.end() )
      {
         ChainLink& chainedInput = *itInputs++;

         IChainedInterface* interfacePtr = chainedInput.m_interface;
         KhaanServerToServer* tempKhaan = static_cast< KhaanServerToServer* >( interfacePtr );
         if( tempKhaan->GetChainedId() == holder.chainIdOfInputConnection )
         {
            localKhaan = tempKhaan;
            break;
         }
      }
      m_inputChainListMutex.unlock();
      if( localKhaan == NULL )
      {
         factory.CleanupPacket( holder.pPacket );
         continue;
      }
    /*  //LogMessage( LOG_PRIO_INFO, "DiplodocusServerToServer::ServerWasIdentified .. return data to new client" );
      LogMessage( LOG_PRIO_INFO, "    m_serverName:     %s", m_serverName.c_str() );
      LogMessage( LOG_PRIO_INFO, "    m_localIpAddress: %s", m_localIpAddress.c_str() );
      LogMessage( LOG_PRIO_INFO, "    m_serverId:       %u", m_serverId );
      LogMessage( LOG_PRIO_INFO, "    m_listeningPort:  %u", m_listeningPort );
      LogMessage( LOG_PRIO_INFO, "    m_serverType:     %u", m_serverType );
      LogMessage( LOG_PRIO_INFO, "    m_isControllerApp:%d", m_isControllerApp );*/
      
      //LogMessage( LOG_PRIO_INFO, "DiplodocusServerToServer::ServerWasIdentified .. AddOutputChainData" );
      
      localKhaan->AddOutputChainData( holder.pPacket, 0 );
      //LogMessage( LOG_PRIO_INFO, "DiplodocusServerToServer::ServerWasIdentified .. MarkConnectionAsNeedingUpdate" );
      
      MarkConnectionAsNeedingUpdate( holder.chainIdOfInputConnection );

      // bubble this identifying info up to the next layer.

      PacketServerIdentifier* idPacket = new PacketServerIdentifier;

      idPacket->serverName =        localKhaan->GetServerName();
      idPacket->serverAddress =     localKhaan->GetServerAddress();
      idPacket->serverId =          localKhaan->GetServerId();
      idPacket->serverPort =        localKhaan->GetServerPort();
      idPacket->serverType =        localKhaan->GetServerType();
      idPacket->gameInstanceId =    0;
      idPacket->isGameServer =      localKhaan->IsGameServer();
      idPacket->isController =      localKhaan->IsController();
      idPacket->gatewayType   =     localKhaan->GetGatewayType();
      idPacket->externalIpAddress = localKhaan->GetExternalIpAddress();
      
      //LogMessage( LOG_PRIO_INFO, "DiplodocusServerToServer::ServerWasIdentified .. CreateJob" );
      
      CreateJob( localKhaan, idPacket );
      //LogMessage( LOG_PRIO_INFO, "DiplodocusServerToServer::ServerWasIdentified .. exit" );
   }

}

//////////////////////////////////////////////////////////////////////////

void  DiplodocusServerToServer::CreateJob( const KhaanServerToServer* khaan, BasePacket* packet )
{
   int jobId = m_jobIdTracker++;

   PacketServerJobWrapper* wrapper = new PacketServerJobWrapper;
   wrapper->jobId = jobId;// only used currently for debugging. Could be used for a variety of things.
   wrapper->pPacket = packet;
   wrapper->serverId = khaan->GetServerId();

   LockMutex();
   m_unprocessedJobs.push_back( wrapper );
   UnlockMutex();
}

void     DiplodocusServerToServer::SendJobsToUpperLayers()
{
   LockMutex();   
      deque< PacketServerJobWrapper* > queueOfJobs =   m_unprocessedJobs;
      m_unprocessedJobs.clear();
   UnlockMutex();

   PacketFactory factory;
   while( queueOfJobs.size() )
   {
      PacketServerJobWrapper* wrapper = queueOfJobs.front();
      queueOfJobs.pop_front();

      ChainLinkIteratorType itOutputs = m_listOfOutputs.begin();
      if( itOutputs != m_listOfOutputs.end() )// we should only have one of these
      {
         if( static_cast< ChainType*> ( itOutputs->m_interface)->AddInputChainData( wrapper, wrapper->serverId ) == false )
         {
            Log(" Bad S2S packet ", 4);
            BasePacket* packet = static_cast< BasePacket* >( wrapper );
            factory.CleanupPacket( packet );
         }
      }
      else
      {
         LogMessage( LOG_PRIO_ERR, "Bad Dipl_S2S setup" );
         assert( 0 );
      }

   }
}

//////////////////////////////////////////////////////////////////////////

bool  DiplodocusServerToServer::AddOutputChainData( BasePacket* packet, U32 connectionId ) 
{
   if( packet->packetType == PacketType_ServerJobWrapper )// this is ready for the destination server... send it on.
   {
      PacketServerJobWrapper* tempPacket = static_cast< PacketServerJobWrapper* >( packet );

      PacketServerToServerWrapper* s2swrapper = new PacketServerToServerWrapper;
      s2swrapper->serverId = connectionId;
      s2swrapper->pPacket = tempPacket->pPacket;// transfer packet locally
      delete packet;// cleanup the packet that contains the original info.

      LogMessage( LOG_PRIO_INFO, "DiplodocusServerToServer::AddOutputChainData: .. prep s2s wrapper" );
      
      ChainLinkIteratorType itInputs = m_listOfInputs.begin();
      while( itInputs != m_listOfInputs.end() )
      {
         ChainLink& chainedInput = *itInputs++;
         IChainedInterface* interfacePtr = chainedInput.m_interface;
         KhaanServerToServer* khaan = static_cast< KhaanServerToServer* >( interfacePtr );
         if( khaan->GetServerId() == connectionId )// 
         {
            LogMessage( LOG_PRIO_INFO, "khaan->GetServerId() == connectionId: %u", connectionId );
   
            // we will swallow this in either case and so we delete the packets if the khaan does not use it.
            if( khaan->AddOutputChainData( s2swrapper, 0 ) == true )
            {
               LogMessage( LOG_PRIO_INFO, "khaan->AddOutputChainData success" );
               MarkConnectionAsNeedingUpdate( khaan->GetChainedId() );
            }
            else
            {
               delete s2swrapper->pPacket;
               delete s2swrapper;
            }
            
            return true;
         }
      }
      LogMessage( LOG_PRIO_INFO, "failed to handle packet" );

      // this instance of a S2S will not handle this packet.
      return false;
   }

   //assert( 0 );// we should not have arrived here.
   return false;
}

//////////////////////////////////////////////////////////////////////////

int   DiplodocusServerToServer::CallbackFunction()
{
   CleanupOldClientConnections( "KhaanServerToServer" );

   HandleInputPackets();
   UpdateAllConnections( "KhaanServerToServer" );
   SendJobsToUpperLayers();

   return 1;
}

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
