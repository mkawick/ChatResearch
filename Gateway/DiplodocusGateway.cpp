// DiplodocusGateway.cpp

#include "DiplodocusGateway.h"
#include "FruitadensGateway.h"
#include "../NetworkCommon/ServerConstants.h"
#include "../NetworkCommon/Packets/BasePacket.h"
#include "../NetworkCommon/Packets/ServerToServerPacket.h"
#include "../NetworkCommon/Packets/LoginPacket.h"
#include "../NetworkCommon/Packets/PacketFactory.h"


//#define VERBOSE
void  PrintText( const char* text, int extraCr = 0 )
{
#if defined (VERBOSE)

   cout << text << endl;
   while( --extraCr >= 0 )
   {
      cout << endl;
   }
#endif
}

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------

DiplodocusGateway::DiplodocusGateway( const string& serverName, U32 serverId ) : Diplodocus< KhaanConnector > ( serverName, serverId, 0, ServerType_Gateway ),
                                          m_connectionIdTracker( 12 ),
                                          m_printPacketTypes( false ),
                                          m_reroutePort( 0 )
{
   SetSleepTime( 16 );// 30 fps
   SetSendHelloPacketOnLogin( true );
   
   time( &m_timestampSendConnectionStatisics );
}

DiplodocusGateway::~DiplodocusGateway()
{
}

//-----------------------------------------------------------------------------------------

U32      DiplodocusGateway::GetNextConnectionId()
{
   m_inputChainListMutex.lock();
   m_connectionIdTracker ++;
   if( m_connectionIdTracker >= ConnectionIdExclusion.low &&  m_connectionIdTracker <= ConnectionIdExclusion.high )
   {
      m_connectionIdTracker = ConnectionIdExclusion.high + 1;
   }
   U32 returnValue = m_connectionIdTracker;

   m_inputChainListMutex.unlock();
   return returnValue;
}

//-----------------------------------------------------------------------------------------

bool  DiplodocusGateway::AddInputChainData( BasePacket* packet, U32 connectionId ) // coming from the client-side socket
{
   PrintText( "AddInputChainData", 1);
   ConnectionMapIterator connIt = m_connectionMap.find( connectionId );
   if( connIt != m_connectionMap.end() )
   {
     /* if( packet->packetType == PacketType_Base )
      {
         if( packet->packetSubType == BasePacket::BasePacket_RerouteRequest )
         {
            HandleReroutRequest( connectionId );
            return false;// delete the original data
         }
      }*/

      PacketGatewayWrapper* wrapper = new PacketGatewayWrapper;
      wrapper->SetupPacket( packet, connectionId );

      if( m_printPacketTypes )
      {
         cout << "Packet to servers: " << (int)packet->packetType << ":" << (int)packet->packetSubType << endl;
      }

      //wrapper->serverType = ServerType_Chat;// we are cheating. we should look at the type of packet and route it appropriately.
      
      m_inputChainListMutex.lock();
      m_packetsToBeSentInternally.push_back( wrapper );
      m_inputChainListMutex.unlock();
      return true;
   }
   else
   {
      delete packet;// it dies here. we should log this and try to disconnect the user
      return false;
   }
}

//-----------------------------------------------------------------------------------------
/*
void     DiplodocusGateway::HandleReroutRequest( U32 connectionId )
{
   ConnectionMapIterator connIt = m_connectionMap.find( connectionId );
   if( connIt != m_connectionMap.end() )
   {
      PacketRerouteRequestResponse* response = new PacketRerouteRequestResponse;
      if( IsRerouoting() == true )
      {
         PacketRerouteRequestResponse::Address address;
         address.address = m_rerouteAddress;
         address.port = m_reroutePort;
         address.name = "gateway";
         address.whichLocationId = PacketRerouteRequestResponse::LocationId_Gateway;

         response->locations.push_back( address );
      }
      KhaanConnector* khaan = connIt->second;
      HandlePacketToKhaan( khaan, response );// all deletion and such is handled lower
   }
}*/

//-----------------------------------------------------------------------------------------

void     DiplodocusGateway::InputConnected( IChainedInterface * chainedInput )
{
   KhaanConnector* khaan = static_cast< KhaanConnector* >( chainedInput );
   string currentTime = GetDateInUTC();
   cout << "Accepted connection at time:" << currentTime << " from " << inet_ntoa( khaan->GetIPAddress().sin_addr ) << endl;
   PrintText( "** InputConnected" , 1 );
   U32 newId = GetNextConnectionId();
   m_socketToConnectionMap.insert( SocketToConnectionPair( khaan->GetSocketId(), newId ) );
   m_connectionToSocketMap.insert( SocketToConnectionPair( newId, khaan->GetSocketId() ) );
   m_connectionMap.insert( ConnectionPair( newId, khaan ) );

   khaan->SetConnectionId( newId );

   khaan->SetGateway( this );
   //khaan->SendThroughLibEvent( true );

   Threading::MutexLock locker( m_outputChainListMutex );
   m_connectionsNeedingUpdate.push_back( newId );
}

//-----------------------------------------------------------------------------------------

void     DiplodocusGateway::InputRemovalInProgress( IChainedInterface * chainedInput )
{
   KhaanConnector* khaan = static_cast< KhaanConnector* >( chainedInput );
   string currentTime = GetDateInUTC();
   cout << "Client disconnection at time:" << currentTime << " from " << inet_ntoa( khaan->GetIPAddress().sin_addr ) << endl;

   PrintText( "** InputRemovalInProgress" , 1 );
   int connectionId = khaan->GetConnectionId();
   int socketId = khaan->GetSocketId();

   PacketLogout* logout = new PacketLogout();// must be done before we clear the lists of ids
   logout->wasDisconnectedByError = true;
   //logout->serverType = ServerType_Chat;

   AddInputChainData( logout, connectionId );

   m_socketToConnectionMap.erase( socketId );
   m_connectionToSocketMap.erase( connectionId );
   m_connectionMap.erase( connectionId );
}

//-----------------------------------------------------------------------------------------

void     DiplodocusGateway::PrintPacketTypes( bool printingOn ) 
{
   m_printPacketTypes = printingOn; 
   cout << "Client side packet printing is ";
   if( m_printPacketTypes == true )
   {
      cout << "enabled" << endl;
   }
   else
   {
      cout << "disabled" << endl;
   }
}

//-----------------------------------------------------------------------------------------

void     DiplodocusGateway::SetupReroute( const string& address, U16 port )
{
   m_rerouteAddress = address;
   m_reroutePort = port;

   // todo, add support for rerouting to other servers like login

   // todo, add dynamic rerouting allowing an admin to login and 
}

//-----------------------------------------------------------------------------------------

bool     DiplodocusGateway::PushPacketToProperOutput( BasePacket* packet )
{
   PrintText( "PushPacketToProperOutput", 1 );

   BaseOutputContainer tempOutput;
   // create new scope
   {
      Threading::MutexLock    locker( m_outputChainListMutex );// quickly copy the list before doing more serious work
      ChainLinkIteratorType itOutput = m_listOfOutputs.begin();
      while( itOutput != m_listOfOutputs.end() )
      {
         tempOutput.push_back( *itOutput++ );
      }
   }

   ChainLinkIteratorType itOutput = tempOutput.begin();
   while( itOutput != tempOutput.end() )
   {
      IChainedInterface* outputPtr = (*itOutput).m_interface;
      FruitadensGateway* fruity = static_cast< FruitadensGateway* >( outputPtr );
      U32 unusedParam = -1;
      if( fruity->AddOutputChainData( packet, unusedParam ) == true )
      {
         return true;
      }
      itOutput++;
   }
   
   return false;
}

//-----------------------------------------------------------------------------------------

int  DiplodocusGateway::ProcessInputFunction()
{
   // m_inputChainListMutex.lock   // see CChainedThread<Type>::CallbackFunction()
   CommonUpdate();

   SendStatsToLoadBalancer();

   if( m_packetsToBeSentInternally.size() == 0 )
      return 0;

   PrintText( "ProcessInputFunction" );

   PacketFactory factory;
   while( m_packetsToBeSentInternally.size() )
   {
      BasePacket* packet = m_packetsToBeSentInternally.front();
      if( PushPacketToProperOutput( packet ) == false )
      {
         factory.CleanupPacket( packet );
      }
      m_packetsToBeSentInternally.pop_front();
   }
   return 1;
}

//-----------------------------------------------------------------------------------------

void  DiplodocusGateway::SendStatsToLoadBalancer()
{
   time_t currentTime;
   time( &currentTime );

   if( difftime( currentTime, m_timestampSendConnectionStatisics ) >= timeoutSendConnectionStatisics ) 
   {
      m_timestampSendConnectionStatisics = currentTime;
      int num = m_connectedClients.size();

      bool statsSent = false;
      ChainLinkIteratorType itOutput = m_listOfOutputs.begin();
      while( itOutput != m_listOfOutputs.end() )
      {
         IChainedInterface* outputPtr = (*itOutput).m_interface;
         FruitadensGateway* fruity = static_cast< FruitadensGateway* >( outputPtr );
         if( fruity->GetConnectedServerType() == ServerType_LoadBalancer )
         {
            PacketServerConnectionInfo* packet = new PacketServerConnectionInfo;
            packet->currentLoad = num;
            packet->serverAddress = GetIpAddress();
            packet->serverId = GetServerId();

            U32 unusedParam = -1;
            if( fruity->AddOutputChainData( packet, unusedParam ) == true )
            {
               statsSent = true;
               PrintText( "SendStatsToLoadBalancer" ); 
               return;
            }
            else
            {
               delete packet;
            }
         }
         itOutput++;
      }
   }
}



//-----------------------------------------------------------------------------------------

bool  DiplodocusGateway::AddOutputChainData( BasePacket* packet, U32 serverType )
{
   PrintText( "AddOutputChainData" ); 

   // pass through only
   if( packet->packetType == PacketType_GatewayWrapper )
   {
      m_outputChainListMutex.lock();
      m_outputTempStorage.push_back( packet );
      m_outputChainListMutex.unlock();
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
   PrintText( "HandlePacketToKhaan" );
   U32 connectionId = khaan->GetConnectionId();
   if( packet->packetType == PacketType_Login && 
       packet->packetSubType == PacketLogin::LoginType_InformGatewayOfLoginStatus )
   {
      PacketLoginToGateway* finishedLogin = static_cast< PacketLoginToGateway* >( packet );
      if( finishedLogin->wasLoginSuccessful )
      {
         khaan->AuthorizeConnection();
         khaan->SetAdminLevelOperations( finishedLogin->adminLevel );
      }
      else
      {
         khaan->DenyAllFutureData();
         connectionId = 0;
      }

      PacketLoginToClient* clientNotify = new PacketLoginToClient;
      clientNotify->wasLoginSuccessful = finishedLogin->wasLoginSuccessful;
      clientNotify->uuid = finishedLogin->uuid;
      clientNotify->userName = finishedLogin->userName;
      clientNotify->lastLogoutTime = finishedLogin->lastLogoutTime;
      clientNotify->connectionId = connectionId;
      clientNotify->loginKey = finishedLogin->loginKey;

      delete finishedLogin;
      packet = clientNotify;
   }

   if( m_printPacketTypes )
   {
      cout << "Packet to client: " << (int)packet->packetType << ":" << (int)packet->packetSubType << endl;
   }
   khaan->AddOutputChainData( packet );

   // this is a performance improvement to prevent duplicate entries in this deque.
   //Threading::MutexLock locker( m_mutex );
   ConnectionIdQueue::iterator it = m_connectionsNeedingUpdate.begin();
   while( it != m_connectionsNeedingUpdate.end() )
   {
      if( *it++ == connectionId )
         return;
   }
   m_connectionsNeedingUpdate.push_back( connectionId );
}

//-----------------------------------------------------------------------------------------

int   DiplodocusGateway::ProcessOutputFunction()
{
   // mutex is locked already

   

   // lookup packet info and pass it back to the proper socket if we can find it.
   if( m_connectionsNeedingUpdate.size() || m_outputTempStorage.size() )
   {
      PrintText( "ProcessOutputFunction" );

      PacketFactory factory;
      while( m_outputTempStorage.size() )
      {
         PacketGatewayWrapper* wrapper = static_cast< PacketGatewayWrapper* >( m_outputTempStorage.front() );
         m_outputTempStorage.pop_front();
         int connectionId = wrapper->connectionId;
         BasePacket* dataPacket = wrapper->pPacket;
         delete wrapper;
         bool  handled = false;

         SocketToConnectionMapIterator it = m_connectionToSocketMap.find( connectionId );
         if( it != m_connectionToSocketMap.end() )
         {
            
            ConnectionMapIterator connIt = m_connectionMap.find( connectionId );
            if( connIt != m_connectionMap.end() )
            {
               KhaanConnector* khaan = connIt->second;
               HandlePacketToKhaan( khaan, dataPacket );// all deletion and such is handled lower
               handled = true;
            }
         }
         if( handled == false )
         {
            factory.CleanupPacket( dataPacket );
         }
      }

      ConnectionIdQueue moreTimeNeededQueue;
      while( m_connectionsNeedingUpdate.size() > 0 )// this has the m_outputChainListMutex protection
      {
         int connectionId = m_connectionsNeedingUpdate.front();
         m_connectionsNeedingUpdate.pop_front();
         ConnectionMapIterator connIt = m_connectionMap.find( connectionId );
         if( connIt != m_connectionMap.end() )
         {
            KhaanConnector* khaan = connIt->second;
            bool didFinish = khaan->Update();
            if( didFinish == false )
            {
               moreTimeNeededQueue.push_back( connectionId );
            }
         }
      }

      m_connectionsNeedingUpdate = moreTimeNeededQueue; // copy 
   }
   return 1;
}
//-----------------------------------------------------------------------------------------
