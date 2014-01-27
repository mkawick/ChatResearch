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

DiplodocusServerToServer::DiplodocusServerToServer( const string& serverName, U32 serverId, U8 gameProductId, ServerType type ) : Diplodocus< KhaanServerToServer >( serverName, serverId, gameProductId, type ), 
                                    m_jobIdTracker( 32 ) // 32 is a non-zero value useful for test only
{
   SetConnectionId( ServerToServerConnectionId );
}

DiplodocusServerToServer::~DiplodocusServerToServer()
{
}

//---------------------------------------------------------------

void  DiplodocusServerToServer::InputConnected( IChainedInterface* khaan ) 
{
  /* if( packet->packetType == PacketType_ServerJobWrapper )
   {
      HandlePacketFromOtherServer( packet, connectionId );
      return true;
   }*/
   // we can basically ignore this until we have more into like connection info
}

//---------------------------------------------------------------

void  DiplodocusServerToServer::InputRemovalInProgress( IChainedInterface* khaan )
{
   // remove all pending jobs.
}

//---------------------------------------------------------------

// this will always be data coming from other servers or at least from the outside in.
bool   DiplodocusServerToServer::AddInputChainData( BasePacket* packet, U32 connectionId ) 
{
   //cout << "DiplodocusServerToServer::AddInputChainData( " << endl;
   if( packet->packetType == PacketType_ServerToServerWrapper )
   {
      //HandleCommandFromGateway( packet, connectionId );
      PacketServerToServerWrapper* wrapper = static_cast< PacketServerToServerWrapper* >( packet );
      BasePacket* unwrappedPacket = wrapper->pPacket;
      U32         serverId = wrapper->serverId;

      //cout << "AddInputChainData start lock" << endl;
      Threading::MutexLock locker( m_mutex );
      //cout << "AddInputChainData finish lock" << endl;

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
      
      // potentially needed
      //m_serversNeedingUpdate.push_back( khaan->GetServerId() );

      return true;
   }

   return true;
}

//---------------------------------------------------------------

void  DiplodocusServerToServer::ServerWasIdentified( IChainedInterface* khaan )
{
   BasePacket* packet = NULL;
   PackageForServerIdentification( m_serverName, m_localIpAddress, m_serverId, m_listeningPort, m_gameProductId, m_isGame, m_isControllerApp, true, m_isGateway, &packet );
   ChainedType* localKhaan = static_cast< ChainedType* >( khaan );
   localKhaan->AddOutputChainData( packet, 0 );
   LockMutex();
   m_clientsNeedingUpdate.push_back( localKhaan->GetServerId() );
   UnlockMutex();

   // bubble this identifying info up to the next layer.

   KhaanServerToServer* ks2s = static_cast< KhaanServerToServer*> ( khaan );
   PacketServerIdentifier* idPacket = new PacketServerIdentifier;

   idPacket->serverName =     ks2s->GetServerName();
   idPacket->serverAddress =  ks2s->GetServerAddress();
   idPacket->serverId =       ks2s->GetServerId();
   idPacket->serverPort =     ks2s->GetServerPort();
   idPacket->gameInstanceId =  0;
   idPacket->isGameServer =   ks2s->IsGameServer();
   idPacket->isController =   ks2s->IsController();
   idPacket->isGateway   =    ks2s->IsGateway();
   CreateJob( ks2s, idPacket );
}

//---------------------------------------------------------------

void  DiplodocusServerToServer::CreateJob( const KhaanServerToServer* khaan, BasePacket* packet )
{
   int jobId = m_jobIdTracker++;

   PacketServerJobWrapper* wrapper = new PacketServerJobWrapper;
   wrapper->jobId = jobId;// only used currently for debugging. Could be used for a variety of things.
   wrapper->pPacket = packet;
   wrapper->serverId = khaan->GetServerId();

   ChainLinkIteratorType itOutputs = m_listOfOutputs.begin();
   if( itOutputs != m_listOfOutputs.end() )// we should only have one of these
   {
      if( static_cast< ChainType*> ( itOutputs->m_interface)->AddInputChainData( wrapper, khaan->GetServerId() ) == false )
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

      PacketServerToServerWrapper* s2swrapper = new PacketServerToServerWrapper;
      s2swrapper->serverId = connectionId;
      s2swrapper->pPacket = tempPacket->pPacket;// transfer packet locally
      //tempPacket->jobId
      delete packet;// cleanup the packet that contains the original info.

      ChainLinkIteratorType itInputs = m_listOfInputs.begin();
      while( itInputs != m_listOfInputs.end() )
      {
         ChainLink& chainedInput = *itInputs++;
         IChainedInterface* interfacePtr = chainedInput.m_interface;
         KhaanServerToServer* khaan = static_cast< KhaanServerToServer* >( interfacePtr );
         if( khaan->GetServerId() == connectionId )// 
         {
            // we will swallow this in either case and so we delete the packets if the khaan does not use it.
            if( khaan->AddOutputChainData( s2swrapper, 0 ) == true )
            {
               m_clientsNeedingUpdate.push_back( connectionId );
            }
            else
            {
               delete s2swrapper->pPacket;
               delete s2swrapper;
            }
            
            return true;
         }
      }

      // this instance of a S2S will not handle this packet.
      return false;
   }

   //assert( 0 );// we should not have arrived here.
   return false;
}

//---------------------------------------------------------------

int   DiplodocusServerToServer::CallbackFunction()
{
   // I would do this with a map, but we'll only ever have one or two of these.
   while( m_serversNeedingUpdate.size() )
   {
      // useful for storing the 
      U32 serverId = m_serversNeedingUpdate.front();
      m_serversNeedingUpdate.pop_front();

      LockMutex();
      ChainLinkIteratorType itInputs = m_listOfInputs.begin();
      while( itInputs != m_listOfInputs.end() )
      {
         ChainLink& chainedInput = *itInputs++;
         IChainedInterface* interfacePtr = chainedInput.m_interface;
         KhaanServerToServer* khaan = static_cast< KhaanServerToServer* >( interfacePtr );
         if( khaan->GetServerId() == serverId )
         {
            if( khaan->Update() == false )
            {
               m_serversNeedingUpdate.push_back( serverId );
            }
         }
      }
      UnlockMutex();
   }

   LockMutex();
   while( m_clientsNeedingUpdate.size() )
   {
      m_clientsNeedingUpdate.front();
      U32 serverId = m_clientsNeedingUpdate.front();
      m_clientsNeedingUpdate.pop_front();

      
      ChainLinkIteratorType itInputs = m_listOfInputs.begin();
      while( itInputs != m_listOfInputs.end() )
      {
         ChainLink& chainedInput = *itInputs++;
         IChainedInterface* interfacePtr = chainedInput.m_interface;
         KhaanServerToServer* khaan = static_cast< KhaanServerToServer* >( interfacePtr );
         if( khaan->GetChainedId() == serverId )
         {
            if( khaan->Update() == false )
            {
               m_clientsNeedingUpdate.push_back( serverId );// put this back in the queue
            }
         }
      }
   }
   UnlockMutex();

   UpdateAllConnections();

   return 1;
}
//---------------------------------------------------------------
//---------------------------------------------------------------
