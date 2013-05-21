// DiplodocusGateway.cpp

#include "DiplodocusGateway.h"
#include "FruitadensGateway.h"
#include "../NetworkCommon/ServerConstants.h"

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------

DiplodocusGateway::DiplodocusGateway( const string& serverName, U32 serverId ) : Diplodocus< KhaanConnector > ( serverName, serverId, ServerType_Gateway ),
                                          m_connectionIdTracker( 12 )
{
   SetSleepTime( 33 );// 30 fps
}

DiplodocusGateway::~DiplodocusGateway()
{
}

//-----------------------------------------------------------------------------------------

U32      DiplodocusGateway::GetNextConnectionId()
{
   m_connectionIdTracker ++;
   if( m_connectionIdTracker >= ConnectionIdExclusion.low &&  m_connectionIdTracker <= ConnectionIdExclusion.high )
   {
      m_connectionIdTracker = ConnectionIdExclusion.high + 1;
   }
   return m_connectionIdTracker;
}

//-----------------------------------------------------------------------------------------

bool  DiplodocusGateway::AddInputChainData( BasePacket* packet, U32 connectionId ) // coming from the client-side socket
{
   ConnectionMapIterator connIt = m_connectionMap.find( connectionId );
   if( connIt != m_connectionMap.end() )
   {
      PacketGatewayWrapper* wrapper = new PacketGatewayWrapper;
      wrapper->connectionId = connectionId;
      wrapper->pPacket = packet;

      //wrapper->serverType = ServerType_Chat;// we are cheating. we should look at the type of packet and route it appropriately.
      
      m_mutex.lock();
      m_packetsToBeSentInternally.push_back( wrapper );
      m_mutex.unlock();
      return true;
   }
   else
   {
      delete packet;// it dies here. we should log this and try to disconnect the user
      return false;
   }
}

//-----------------------------------------------------------------------------------------

void     DiplodocusGateway::ClientConnectionFinishedAdding( KhaanConnector* khaan )
{
   U32 newId = GetNextConnectionId();
   m_socketToConnectionMap.insert( SocketToConnectionPair( khaan->GetSocketId(), newId ) );
   m_connectionToSocketMap.insert( SocketToConnectionPair( newId, khaan->GetSocketId() ) );
   m_connectionMap.insert( ConnectionPair( newId, khaan ) );

   khaan->SetConnectionId( newId );

   khaan->SetGateway( this );
}

//-----------------------------------------------------------------------------------------

void  DiplodocusGateway::ClientConnectionIsAboutToRemove( KhaanConnector* khaan )
{
   int connectionId = khaan->GetConnectionId();
   int socketId = khaan->GetSocketId();

   PacketLogout* logout = new PacketLogout();// must be done before we clear the lists of ids
   //logout->serverType = ServerType_Chat;

   AddInputChainData( logout, connectionId );

   m_socketToConnectionMap.erase( socketId );
   m_connectionToSocketMap.erase( connectionId );
   m_connectionMap.erase( connectionId );
}

//-----------------------------------------------------------------------------------------

bool DiplodocusGateway::PushPacketToProperOutput( BasePacket* packet )
{
   Threading::MutexLock    locker( m_outputChainListMutex );

   ChainLinkIteratorType itOutput = m_listOfOutputs.begin();
   while( itOutput != m_listOfOutputs.end() )
   {
      ChainedInterface* outputPtr = (*itOutput).m_interface;
      FruitadensGateway* fruity = static_cast< FruitadensGateway* >( outputPtr );
      //if( fruity->GetConnectedServerType () == packet->serverType )
      {
         if( fruity->AddOutputChainData( packet, -1 ) == true )
         {
            return true;
         }
      }
      itOutput++;
   }
   return false;
}
//-----------------------------------------------------------------------------------------

int  DiplodocusGateway::ProcessInputFunction()// should be simple throughput, we just match the packet types coming in to the proper server
{
   SendServerIdentification();

   if( m_packetsToBeSentInternally.size() == 0 )
      return 0;

   Threading::MutexLock locker( m_mutex );
   while( m_packetsToBeSentInternally.size() )
   {
      BasePacket* packet = m_packetsToBeSentInternally.front();
      if( PushPacketToProperOutput( packet ) == false )
      {
         //assert( 0 );
         //return false;
         delete packet;
      }
      m_packetsToBeSentInternally.pop_front();
   }
   return 1;
}

//-----------------------------------------------------------------------------------------

bool  DiplodocusGateway::AddOutputChainData( BasePacket* packet, U32 serverType )
{
   // pass through only
   if( packet->packetType == PacketType_GatewayWrapper )
   {
      PacketGatewayWrapper* wrapper = static_cast< PacketGatewayWrapper* >( packet );
      int connectionId = wrapper->connectionId;
      BasePacket* dataPacket = wrapper->pPacket;
      delete wrapper;

      SocketToConnectionMapIterator it = m_connectionToSocketMap.find( connectionId );
      if( it != m_connectionToSocketMap.end() )
      {
         Threading::MutexLock locker( m_inputChainListMutex );
         ConnectionMapIterator connIt = m_connectionMap.find( connectionId );
         if( connIt != m_connectionMap.end() )
         {
            KhaanConnector* khaan = connIt->second;
            HandlePacketToKhaan( khaan, dataPacket );
         }
      }
   }
   else
   {
      assert( 0 );
      return false;
   }
   return true;
}

//-----------------------------------------------------------------------------------------

// assuming that everything is thread protected at this point
void  DiplodocusGateway::HandlePacketToKhaan( KhaanConnector* khaan, BasePacket* packet )
{
   if( packet->packetType == PacketType_Login && 
       packet->packetSubType == PacketLogin::LoginType_InformGatewayOfLoginStatus )
   {
      PacketLoginToGateway* finishedLogin = static_cast< PacketLoginToGateway* >( packet );
      if( finishedLogin->wasLoginSuccessful )
      {
         khaan->AuthorizeConnection();
      }
      else
      {
         khaan->DenyAllFutureData();
      }
      PacketLoginToClient* clientNotify = new PacketLoginToClient;
      clientNotify->wasLoginSuccessful = finishedLogin->wasLoginSuccessful;
      clientNotify->uuid = finishedLogin->uuid;
      clientNotify->username = finishedLogin->username;

      delete finishedLogin;
      packet = clientNotify;
   }

   khaan->AddOutputChainData( packet );
   m_connectionsNeedingUpdate.push_back( khaan->GetConnectionId() );
}

//-----------------------------------------------------------------------------------------

int   DiplodocusGateway::ProcessOutputFunction()
{
   // lookup packet info and pass it back to the proper socket if we can find it.
   if( m_connectionsNeedingUpdate.size() )
   {
      m_mutex.lock();
      while( m_connectionsNeedingUpdate.size() > 0 )
      {
         int connectionId = m_connectionsNeedingUpdate.front();
         m_connectionsNeedingUpdate.pop_front();
         m_inputChainListMutex.lock();

         ConnectionMapIterator connIt = m_connectionMap.find( connectionId );
         if( connIt != m_connectionMap.end() )
         {
            KhaanConnector* khaan = connIt->second;
            khaan->Update();
         }

         m_inputChainListMutex.unlock();
      }
      m_mutex.unlock();
   }
   return 1;
}
//-----------------------------------------------------------------------------------------
