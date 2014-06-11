//DiplodocusLoadBalancer.cpp

#include <string>
using namespace std;

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include "../NetworkCommon/Utils/CommandLineParser.h"
#include "DiplodocusLoadBalancer.h"

#include "../NetworkCommon/Packets/ServerToServerPacket.h"

///////////////////////////////////////////////////////////////////

DiplodocusLoadBalancer::DiplodocusLoadBalancer( const string& serverName, U32 serverId ): Diplodocus< KhaanConnector >( serverName, serverId, 0,  ServerType_LoadBalancer ), m_connectionIdTracker( 100 )
{
   time( &m_timestampStatsPrint );
   m_timestampSelectPreferredGateway = m_timestampStatsPrint;
}

DiplodocusLoadBalancer::~DiplodocusLoadBalancer()
{
}

///////////////////////////////////////////////////////////////////

void  DiplodocusLoadBalancer::AddGatewayAddress( const string& address, U16 port )
{
   assert( port > 0 && address.size() > 5 );

   list< GatewayInfo >::iterator it = m_gatewayRoutes.begin();
   while( it != m_gatewayRoutes.end() )
   {
      if( it->address == address && 
          it->port == port )
      {
         cout << "Duplicate entry for gateway connection found" << endl;
         assert( 0 );
      }
      it++;
   }

   m_gatewayRoutes.push_back( GatewayInfo( address, port ) );
}

///////////////////////////////////////////////////////////////////

U32      DiplodocusLoadBalancer::GetNextConnectionId()
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

int   DiplodocusLoadBalancer::MainLoop_InputProcessing()
{
   // m_inputChainListMutex.lock   // see CChainedThread<Type>::CallbackFunction()
   CommonUpdate();

   OutputCurrentStats();

   SelectPreferredGateways();

   return 1;
}

//-----------------------------------------------------------------------------------------

void     DiplodocusLoadBalancer::OutputCurrentStats()
{
   time_t currentTime;
   time( &currentTime );

   if( difftime( currentTime, m_timestampStatsPrint ) >= timeoutStatsPrint ) 
   {
      m_timestampStatsPrint = currentTime;

      cout << "********** current stats ************" << endl;
      list< GatewayInfo >::iterator it = m_gatewayRoutes.begin();
      int numConnected = 0;
      while( it != m_gatewayRoutes.end() )
      {
         if( it->isConnected )
         {
            numConnected++;
         }
         it++;
      }

      cout << "Num connected servers = " << numConnected << endl;
      cout << "----------------------------" << endl;
      it = m_gatewayRoutes.begin();
      while( it != m_gatewayRoutes.end() )
      {
         GatewayInfo& gi = *it++;
         if( gi.isConnected )
         {
            cout << " addr:       " << gi.address << endl;
            cout << " port:       " << gi.port << endl;
            cout << " server id:  " << gi.serverId << endl;
            cout << " load:       " << gi.currentLoad << endl;
            cout << " max load:   " << gi.maxLoad << endl;
            cout << " tolerance:  " << gi.loadTolerance << endl;
            if( it != m_gatewayRoutes.end() )// separator
               cout << "........................" << endl;
         }
      }
      cout << "----------------------------" << endl;
   }
}

//-----------------------------------------------------------------------------------------

bool gatewayCompare( GatewayInfo* i, GatewayInfo* j ) 
{ 
   return ( i->currentLoad < j->currentLoad ); 
}

//-----------------------------------------------------------------------------------------

void     DiplodocusLoadBalancer::SelectPreferredGateways()
{
   time_t currentTime;
   time( &currentTime );

   if( difftime( currentTime, m_timestampSelectPreferredGateway ) >= timeoutSelectPreferredGateway ) 
   {
      m_timestampSelectPreferredGateway = currentTime;

      if( m_gatewayRoutes.size() < 1 )
      {
         return;
      }

      // gateways do not come and go, so threaded protections are not needed, plus this is invoked from MainLoop_InputProcessing  which has its own
      vector< GatewayInfo* > sortedRoutes;
      list< GatewayInfo >::iterator it = m_gatewayRoutes.begin();
      while( it != m_gatewayRoutes.end() )
      {
         GatewayInfo& gi = *it++;
         if( gi.isConnected == true && gi.type == GatewayInfo::Type_Normal ) // clear all of these flags
         {
            gi.isPreferred = false;
            sortedRoutes.push_back( &gi );
         }
      }

      std::sort( sortedRoutes.begin(), sortedRoutes.end(), gatewayCompare );

      vector< GatewayInfo* >::iterator itSorted = sortedRoutes.begin();
      while( itSorted != sortedRoutes.end() )
      {
         GatewayInfo* gi = *itSorted++;
         if( gi->currentLoad < gi->maxLoad + gi->loadTolerance )// this could be improved a lot, but this will suffice for our purposes.
         {
            gi->isPreferred = true;
            break;
         }
      }
   }
}


//-----------------------------------------------------------------------------------------

void     DiplodocusLoadBalancer::InputConnected( IChainedInterface * chainedInput )
{
   KhaanConnector* khaan = static_cast< KhaanConnector* >( chainedInput );
   string currentTime = GetDateInUTC();
   cout << "Accepted connection at time:" << currentTime << " from " << inet_ntoa( khaan->GetIPAddress().sin_addr ) << endl;
   //PrintText( "** InputConnected" , 1 );
   U32 newId = GetNextConnectionId();
   m_socketToConnectionMap.insert( SocketToConnectionPair( khaan->GetSocketId(), newId ) );
   m_connectionToSocketMap.insert( SocketToConnectionPair( newId, khaan->GetSocketId() ) );
   m_connectionMap.insert( ConnectionPair( newId, khaan ) );

   khaan->SetConnectionId( newId );

   khaan->SetGateway( this );

   Threading::MutexLock locker( m_outputChainListMutex );
   m_connectionsNeedingUpdate.push_back( newId );
}

//-----------------------------------------------------------------------------------------

void     DiplodocusLoadBalancer::InputRemovalInProgress( IChainedInterface * chainedInput )
{
   KhaanConnector* khaan = static_cast< KhaanConnector* >( chainedInput );
   string currentTime = GetDateInUTC();
   cout << "Client disconnection at time:" << currentTime << " from " << inet_ntoa( khaan->GetIPAddress().sin_addr ) << endl;

   //PrintText( "** InputRemovalInProgress" , 1 );
   int connectionId = khaan->GetConnectionId();
   int socketId = khaan->GetSocketId();

   m_socketToConnectionMap.erase( socketId );
   m_connectionToSocketMap.erase( connectionId );
   m_connectionMap.erase( connectionId );
}

//---------------------------------------------------------------

bool  DiplodocusLoadBalancer::HandlePacketFromOtherServer( BasePacket* packet, U32 connectionId )// not thread safe
{
   if( packet->packetType != PacketType_ServerJobWrapper )
   {
      return false;
   }

   PacketFactory factory;
   PacketServerJobWrapper* wrapper = static_cast< PacketServerJobWrapper* >( packet );
   BasePacket* unwrappedPacket = wrapper->pPacket;
   U32  serverIdLookup = wrapper->serverId;
   serverIdLookup = serverIdLookup;

   bool success = false;

   if(  unwrappedPacket->packetType == PacketType_ServerInformation )
   {
      switch( unwrappedPacket->packetSubType )
      {
      case  PacketServerConnectionInfo::PacketServerIdentifier_TypicalInfo:
         NewServerConnection( static_cast< const PacketServerIdentifier* >( unwrappedPacket ) );
         success = true;
         break;
      case  PacketServerConnectionInfo::PacketServerIdentifier_Disconnect:
         ServerDisconnected( static_cast< const PacketServerDisconnect* >( unwrappedPacket ) );
         success = true;
         break;
      case  PacketServerConnectionInfo::PacketServerIdentifier_ConnectionInfo:
         ServerInfoUpdate( static_cast< const PacketServerConnectionInfo* >( unwrappedPacket ) );
         success = true;
         break;
      }
   }

   if( success == true )
   {
      factory.CleanupPacket( packet );
   }

   return success;
}


//-----------------------------------------------------------------------------------------

bool     DiplodocusLoadBalancer::AddInputChainData( BasePacket* packet, U32 connectionId )
{
   //PrintText( "AddInputChainData", 1);
   ConnectionMapIterator connIt = m_connectionMap.find( connectionId );
   if( connIt != m_connectionMap.end() )
   {
      if( packet->packetType == PacketType_Base )
      {
         if( packet->packetSubType == BasePacket::BasePacket_RerouteRequest )
         {
            HandleRerouteRequest( connectionId );
            return false;// delete the original data
         }
      }
   }
   if( packet->packetType == PacketType_ServerJobWrapper )
   {
      HandlePacketFromOtherServer( packet, connectionId );
      return true;
   }
   return false;
}

//-----------------------------------------------------------------------------------------

bool  IsOnSameNetwork( const string& myNetwork, const string& potentialMatch )
{
   std::size_t  found= myNetwork.find( '.' );
   if (found == std::string::npos)
   {
      std::cout << "IsOnSameNetwork::Bad network IP Address: " << myNetwork << '\n';
      return false;
   }
   found ++;// advance
   found= myNetwork.find( '.', found );
   if( found == std::string::npos )
   {
      std::cout << "IsOnSameNetwork::Bad network IP Address: " << myNetwork << '\n';
      return false;
   }

   int compareValue = strncmp( myNetwork.c_str(), potentialMatch.c_str(), found );
   if( compareValue == 0 )
      return true;
   return false;
}

//-----------------------------------------------------------------------------------------

void     DiplodocusLoadBalancer::HandleRerouteRequest( U32 connectionId )
{
   ConnectionMapIterator connIt = m_connectionMap.find( connectionId );
   if( connIt != m_connectionMap.end() )
   {
      cout << "Telling user about the following connection destinations" << endl;
      connIt->second->GetIPAddress();
      list< GatewayInfo >::iterator it = m_gatewayRoutes.begin();

      bool useLocalAddress = true;
      if( IsOnSameNetwork( it->address, inet_ntoa( connIt->second->GetIPAddress().sin_addr ) ) )
         useLocalAddress = true;
      else
         useLocalAddress = false;

      PacketRerouteRequestResponse* response = new PacketRerouteRequestResponse;
      //bool  hasFoundViableGateway = false;

      while( it != m_gatewayRoutes.end() )
      {
         if( it->isConnected == true )
         {
            PacketRerouteRequestResponse::Address address;
            if( useLocalAddress )
               address.address = it->address;
            else
               address.address = it->externalIpAddress;

            address.port = it->port;

            cout << "addr( " << address.address << " ) : port( " << address.port << " )" << endl;
            
            if( it->type == GatewayInfo::Type_Normal )
            {
               address.name = "normal gateway";
               address.whichLocationId = PacketRerouteRequestResponse::LocationId_Gateway;
            }
            else
            {
               address.name = "asset gateway";
               address.whichLocationId = PacketRerouteRequestResponse::LocationId_Asset; 
            }

            response->locations.push_back( address );
         }
         it++;
      }

      KhaanConnector* khaan = connIt->second;
      HandlePacketToKhaan( khaan, response );// all deletion and such is handled lower
   }
}


//-----------------------------------------------------------------------------------------

// assuming that everything is thread protected at this point
void     DiplodocusLoadBalancer::HandlePacketToKhaan( KhaanConnector* khaan, BasePacket* packet )
{
   //PrintText( "HandlePacketToKhaan" );
   U32 connectionId = khaan->GetConnectionId();
   khaan->AddOutputChainData( packet );

   ConnectionIdQueue::iterator it = m_connectionsNeedingUpdate.begin();
   while( it != m_connectionsNeedingUpdate.end() )
   {
      if( *it++ == connectionId )
         return;
   }
   m_connectionsNeedingUpdate.push_back( connectionId );
}


//-----------------------------------------------------------------------------------------

int   DiplodocusLoadBalancer::MainLoop_OutputProcessing()
{
   // mutex is locked already

   // lookup packet info and pass it back to the proper socket if we can find it.
   if( m_connectionsNeedingUpdate.size() )
   {
      //PrintText( "MainLoop_OutputProcessing" );

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
               //moreTimeNeededQueue.push_back( connectionId );
            }
         }
      }
   }
   return 1;
}

///////////////////////////////////////////////////////////////////

list< GatewayInfo >::iterator 
DiplodocusLoadBalancer::FindGateway( const string& ipAddress, U16 port, U32 serverId )
{
   list< GatewayInfo >::iterator it = m_gatewayRoutes.begin();
   while( it != m_gatewayRoutes.end() )
   {
      if( it->address == ipAddress )
      {
         if( serverId == 0 && port == 0 )
         {
            assert( 0 );
         }
         else if( port && it->port == port )
         {
            return it;
         }
         else if( serverId && it->serverId == serverId )
         {
            return it;
         }
      }
      if( it->externalIpAddress == ipAddress ) 
         return it;
      it++;
   }

   return m_gatewayRoutes.end();
}

///////////////////////////////////////////////////////////////////

void     DiplodocusLoadBalancer::NewServerConnection( const PacketServerIdentifier* gatewayInfo )
{
   list< GatewayInfo >::iterator it = FindGateway( gatewayInfo->serverAddress, gatewayInfo->serverPort, gatewayInfo->serverId );// we may not have the serverId already stored
   if( it != m_gatewayRoutes.end() )
   {
      it->serverId = gatewayInfo->serverId;
      it->port = gatewayInfo->serverPort;
      it->externalIpAddress = gatewayInfo->externalIpAddress;
      it->isVerified = true;
      it->isConnected = true;
      return;
   }

   // clearly we don't have this gateway in our list. let's add it. We really don't need to remove these tho.
   AddGatewayAddress( gatewayInfo->serverAddress, gatewayInfo->serverPort );
   it = FindGateway( gatewayInfo->serverAddress, gatewayInfo->serverPort );
   it->serverId = gatewayInfo->serverId;
   it->externalIpAddress = gatewayInfo->externalIpAddress;
   it->port = gatewayInfo->serverPort;
   it->isVerified = true;
   it->isConnected = true;
   if( gatewayInfo->gatewayType == PacketServerIdentifier::GatewayType_None )
   {
      it->type = GatewayInfo::Type_None;
   }
   else if( gatewayInfo->gatewayType == PacketServerIdentifier::GatewayType_Normal )
   {
      it->type = GatewayInfo::Type_Normal;
   }
   else if( gatewayInfo->gatewayType == PacketServerIdentifier::GatewayType_Asset )
   {
      it->type = GatewayInfo::Type_Asset;
   }
   else
   {
      cout << "Error: gateway type unknown" << endl;
      assert(0);
   }
}

///////////////////////////////////////////////////////////////////

void     DiplodocusLoadBalancer::ServerDisconnected( const PacketServerDisconnect* gatewayInfo )
{
   list< GatewayInfo >::iterator it = FindGateway( gatewayInfo->serverAddress, 0, gatewayInfo->serverId );
   if( it != m_gatewayRoutes.end() )
   {
      it->isConnected = false;
   }
}

///////////////////////////////////////////////////////////////////

void     DiplodocusLoadBalancer::ServerInfoUpdate( const PacketServerConnectionInfo* gatewayInfo )
{
   list< GatewayInfo >::iterator it = FindGateway( gatewayInfo->serverAddress, 0, gatewayInfo->serverId );
   if( it != m_gatewayRoutes.end() )
   {
      it->currentLoad = gatewayInfo->currentLoad;
   }
}

///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////