// DiplodocusServerToServer.cpp

#include "../ServerConstants.h"
#include "../DataTypes.h"

#include "../Utils/CommandLineParser.h"
#include "../Utils/Utils.h"

#include "../NetworkIn/Khaan.h"
#include "../NetworkIn/Diplodocus.h"
#include "../Database/Deltadromeus.h"

#include "../Packets/ServerToServerPacket.h"
#include "DiplodocusServerToServer.h"

DiplodocusServerToServer::DiplodocusServerToServer( const string& serverName, U32 serverId ) : Diplodocus< KhaanServerToServer >( serverName, serverId, ServerType_Chat ), m_jobIdTracker( 32 ) // 32 is a non-zero value useful for test only
{
   SetConnectionId( ServerToServerConnectionId );
}

DiplodocusServerToServer::~DiplodocusServerToServer()
{
}

//---------------------------------------------------------------

void  DiplodocusServerToServer::ClientConnectionFinishedAdding( BaseInputChainHandler* khaan ) 
{
}

//---------------------------------------------------------------

void  DiplodocusServerToServer::ClientConnectionIsAboutToRemove( BaseInputChainHandler* khaan )
{
   // remove all pending jobs.
}

//---------------------------------------------------------------

// this will always be data coming from other servers or at least from the outside in.
bool   DiplodocusServerToServer::AddInputChainData( BasePacket* packet, U32 connectionId ) 
{
   if( packet->packetType == PacketType_ServerToServerWrapper )
   {
      //HandleCommandFromGateway( packet, connectionId );
      PacketServerToServerWrapper* wrapper = static_cast< PacketServerToServerWrapper* >( packet );
      BasePacket* unwrappedPacket = wrapper->pPacket;
      U32         serverId = wrapper->serverId;

      LockMutex();

      bool  found = false;
      KhaanServerToServer* khaan = NULL;
      ChainLinkIteratorType itInputs = m_listOfInputs.begin();
      while( itInputs != m_listOfInputs.end() )
      {
         ChainLink& chainedInput = *itInputs++;
	      ChainedInterface* interfacePtr = chainedInput.m_interface;
         khaan = static_cast< KhaanServerToServer* >( interfacePtr );

         if( serverId == khaan->GetServerId() )
         {
            found = true;
            break;
         }
      }
      if( found == false || khaan == NULL )
      {
         assert( 0 );
      }
      

      // create job for this packet including the serverId, a unique job id, and so on. Keep in mind that the connection may disappear
      // during the servicing of this job
      CreateJob( khaan, unwrappedPacket );
      
      // potentially needed
      //m_serversNeedingUpdate.push_back( khaan->GetServerId() );

      UnlockMutex();

      return true;
   }

   return true;
}

//---------------------------------------------------------------

void  DiplodocusServerToServer::ServerWasIdentified( KhaanServerToServer* khaan )
{
   BasePacket* packet = NULL;
   PackageForServerIdentification( m_serverName, m_serverId, m_isGame, m_isControllerApp, true, m_isGateway, &packet );
   khaan->AddOutputChainData( packet, 0 );
   m_serversNeedingUpdate.push_back( khaan->GetServerId() );
}

//---------------------------------------------------------------

void  DiplodocusServerToServer::CreateJob( const KhaanServerToServer* khaan, BasePacket* packet )
{
   int jobId = m_jobIdTracker++;

   PacketServerJobWrapper* wrapper = new PacketServerJobWrapper;
   wrapper->jobId = jobId;
   wrapper->pPacket = packet;
   wrapper->serverId = khaan->GetServerId();

   ChainLinkIteratorType itOutputs = m_listOfOutputs.begin();
   if( itOutputs != m_listOfOutputs.end() )// we should only have one of these
   {
      if( itOutputs->m_interface->AddInputChainData( wrapper, khaan->GetServerId() ) == false )
      {
         Log(" Bad S2S packet ", 4);
         delete packet;
         delete wrapper;
      }
   }
}

//---------------------------------------------------------------

bool  DiplodocusServerToServer::AddOutputChainData( BasePacket* packet, U32 connectionId ) 
{
   if( packet->packetType == PacketType_ServerJobWrapper )// this is ready for the destination server... send it on.
   {
      PacketServerJobWrapper* tempPacket = static_cast< PacketServerJobWrapper* >( packet );

      PacketServerToServerWrapper* wrapper = new PacketServerToServerWrapper;
      wrapper->serverId = connectionId;
      wrapper->pPacket = tempPacket->pPacket;// transfer packet locally
      delete packet;// cleanup the packet that contains the original info.

      ChainLinkIteratorType itInputs = m_listOfInputs.begin();
      while( itInputs != m_listOfInputs.end() )
      {
         ChainLink& chainedInput = *itInputs++;
         ChainedInterface* interfacePtr = chainedInput.m_interface;
         KhaanServerToServer* khaan = static_cast< KhaanServerToServer* >( interfacePtr );
         if( khaan->GetServerId() == connectionId )// 
         {
            // we will swallow this in either case and so we delete the packets if the khaan does not use it.
            if( khaan->AddOutputChainData( wrapper, 0 ) == true )
            {
               m_serversNeedingUpdate.push_back( connectionId );
            }
            else
            {
               delete wrapper->pPacket;
               delete wrapper;
            }
            
            return true;
         }
      }

      // this instance of a S2S will not handle this packet.
      return false;
   }

   assert( 0 );// we should not have arrived here.
   return false;
}

//---------------------------------------------------------------

int   DiplodocusServerToServer::CallbackFunction()
{
   // I would do this with a map, but we'll only ever have one or two of these.
   while( m_serversNeedingUpdate.size() )
   {
      U32 serverId = m_serversNeedingUpdate.front();
      m_serversNeedingUpdate.pop_front();

      LockMutex();
      ChainLinkIteratorType itInputs = m_listOfInputs.begin();
      while( itInputs != m_listOfInputs.end() )
      {
         ChainLink& chainedInput = *itInputs++;
         ChainedInterface* interfacePtr = chainedInput.m_interface;
         KhaanServerToServer* khaan = static_cast< KhaanServerToServer* >( interfacePtr );
         if( khaan->GetServerId() == serverId )
         {
            khaan->Update();
         }
      }
      UnlockMutex();
   }

   return 1;
}
//---------------------------------------------------------------
//---------------------------------------------------------------
