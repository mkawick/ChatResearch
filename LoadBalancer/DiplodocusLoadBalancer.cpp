//DiplodocusLoadBalancer.cpp

#include <string>
using namespace std;

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include "../NetworkCommon/Utils/CommandLineParser.h"
#include "DiplodocusLoadBalancer.h"

#include "../NetworkCommon/Packets/ServerToServerPacket.h"

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

DiplodocusLoadBalancer::DiplodocusLoadBalancer( const string& serverName, U32 serverId ): 
   Diplodocus< KhaanConnector >( serverName, serverId, 0,  ServerType_LoadBalancer ), 
   m_distributedConnectionIdPoint( 1001 ),
   m_numConnectionIdsToDistrubute( 40 ),
   m_connectionIdTracker( 100 )   
{
   time( &m_timestampStatsPrint );
   m_timestampSelectPreferredGateway = m_timestampStatsPrint;
}

///////////////////////////////////////////////////////////////////

DiplodocusLoadBalancer::~DiplodocusLoadBalancer()
{
}

///////////////////////////////////////////////////////////////////
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
         LogMessage( LOG_PRIO_ERR, "Duplicate entry for gateway connection found ( %s:%u )", address.c_str(), port );
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

///////////////////////////////////////////////////////////////////

int       DiplodocusLoadBalancer::CallbackFunction()
{
   CommonUpdate();

   OutputCurrentStats();

   SelectPreferredGateways();

   CleanupOldClientConnections( "KhaanConnector" );
   UpdateAllConnections( "KhaanConnector" );
   return 1;
}

///////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////

bool gatewayCompare( GatewayInfo* i, GatewayInfo* j ) 
{ 
   return ( i->currentLoad < j->currentLoad ); 
}

///////////////////////////////////////////////////////////////////

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


///////////////////////////////////////////////////////////////////

void     DiplodocusLoadBalancer::InputConnected( IChainedInterface * chainedInput )
{
   KhaanConnector* khaan = static_cast< KhaanConnector* >( chainedInput );
   string currentTime = GetDateInUTC();
   LogMessage( LOG_PRIO_INFO, "Accepted connection at time: %s from %s, ", currentTime.c_str(), inet_ntoa( khaan->GetIPAddress().sin_addr ) );
   //PrintText( "** InputConnected" , 1 );

   U32 newId = GetNextConnectionId();
 /*  m_inputChainListMutex.lock();
   
   //m_socketToConnectionMap.insert( SocketToConnectionPair( khaan->GetSocketId(), newId ) );
   //m_connectionToSocketMap.insert( SocketToConnectionPair( newId, khaan->GetSocketId() ) );
   m_connectionMap.insert( ConnectionPair( newId, khaan ) );
   m_inputChainListMutex.unlock();*/

   khaan->SetConnectionId( newId );

   khaan->SetMainOutput( this );

   MarkConnectionAsNeedingUpdate( khaan->GetChainedId() );
  /* m_mutex.lock();
   m_connectionsNeedingUpdate.push_back( newId );
   m_mutex.unlock();*/
}

///////////////////////////////////////////////////////////////////

void     DiplodocusLoadBalancer::InputRemovalInProgress( IChainedInterface * chainedInput )
{
   KhaanConnector* khaan = static_cast< KhaanConnector* >( chainedInput );

   SetupClientWaitingToBeRemoved( khaan );

   string currentTime = GetDateInUTC();
   string printer = "Client disconnection at time:" + currentTime + " from " + inet_ntoa( khaan->GetIPAddress().sin_addr );
   LogMessage( LOG_PRIO_INFO, printer );

   LogMessage( LOG_PRIO_INFO, "** InputRemovalInProgress" );
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
      case  PacketServerConnectionInfo::PacketServerIdentifier_GatewayRequestLB_ConnectionIds:
         RequestConnectionIds( static_cast< const PacketServerToServer_GatewayRequestLB_ConnectionIds* >( unwrappedPacket ) );
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


///////////////////////////////////////////////////////////////////

bool     DiplodocusLoadBalancer::AddInputChainData( BasePacket* packet, U32 connectionId )
{
   if( packet->packetType == PacketType_Base )
   {
      if( packet->packetSubType == BasePacket::BasePacket_RerouteRequest )
      {
         HandleRerouteRequest( connectionId );
         return false;// delete the original data
      }
   }
   if( packet->packetType == PacketType_ServerJobWrapper )
   {
      HandlePacketFromOtherServer( packet, connectionId );
      return true;
   }
   return false;
}

///////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////


#define HACK_FOR_MULTIPLE_GATEWAY_TESTING

#ifdef HACK_FOR_MULTIPLE_GATEWAY_TESTING
int indexForRotatingGatewayIndex = 1;
#else
int indexForRotatingGatewayIndex = -1;
#endif // HACK_FOR_MULTIPLE_GATEWAY_TESTING

int   DiplodocusLoadBalancer::IncrementRotatingGatewayIndex()
{
#ifdef HACK_FOR_MULTIPLE_GATEWAY_TESTING
   m_inputChainListMutex.lock();
   //int numGatewayRoutes = m_gatewayRoutes.size();
   int numGatewayRoutes = 0;
   list< GatewayInfo >::iterator it = m_gatewayRoutes.begin();
   while( it != m_gatewayRoutes.end() )
   {
      if( it->type == GatewayInfo::Type_Normal )
         numGatewayRoutes++;
      it++;
   }
   m_inputChainListMutex.unlock();

   //offsetIndex = 1;// never select the first one
   if( indexForRotatingGatewayIndex >= numGatewayRoutes )
      indexForRotatingGatewayIndex = 0;

   return indexForRotatingGatewayIndex++;
#endif // HACK_FOR_MULTIPLE_GATEWAY_TESTING

   return 0;
}

DiplodocusLoadBalancer::ChainLinkIteratorType 
DiplodocusLoadBalancer::FindInputByConnectionId( U32 connectionId )
{
   ChainLinkIteratorType itInputs = m_listOfInputs.begin();
   while( itInputs != m_listOfInputs.end() )// only one output currently supported.
   {
      const KhaanConnector* khaan = static_cast< const KhaanConnector* >( (*itInputs).m_interface );
      if( khaan->GetConnectionId() == connectionId )
      {
         return itInputs;
      }
      itInputs++;
   }
   return itInputs;
}

///////////////////////////////////////////////////////////////////

void     DiplodocusLoadBalancer::HandleRerouteRequest( U32 connectionId )
{
   int offsetIndex = IncrementRotatingGatewayIndex();

   bool  hasNormal = false; // we're going to store one of each
   bool  hasAsset = false;
   {
      ChainLinkIteratorType itInputs = FindInputByConnectionId( connectionId );
      if( itInputs == m_listOfInputs.end() )
      {
         return;
      }
      KhaanConnector* khaan = static_cast< KhaanConnector* >( (*itInputs).m_interface );
      const char* ipAddressOfClient = inet_ntoa( khaan->GetIPAddress().sin_addr );

      LogMessage( LOG_PRIO_INFO, "Telling user about the following connection destinations" );
      //connIt->second->GetIPAddress();
      list< GatewayInfo >::iterator it = m_gatewayRoutes.begin();
      PacketRerouteRequestResponse* response = new PacketRerouteRequestResponse;

      if( it != m_gatewayRoutes.end() )
      {
         while( it != m_gatewayRoutes.end() )
         {
#ifdef HACK_FOR_MULTIPLE_GATEWAY_TESTING
            if( offsetIndex != 0 && 
               it->type == GatewayInfo::Type_Normal 
               )
            {
               offsetIndex--;
               it++;
               continue;
            }
#endif
            if( it->isConnected == true )
            {
               bool  shouldPushAddress = false;
               PacketRerouteRequestResponse::Address address;

               if( IsOnSameNetwork( it->address, ipAddressOfClient ) == true )
                  address.address = it->address;
               else
                  address.address = it->externalIpAddress;

               address.port = it->port;

               cout << "addr( " << address.address << " ) : port( " << address.port << " ) : type(";
               if( it->type == GatewayInfo::Type_Normal )
                  cout << " main";
               else
                  cout << " asset";
               cout << ")" << endl;
               
               if( it->type == GatewayInfo::Type_Normal )
               {
                  if( hasNormal == false )
                  {
                     shouldPushAddress = true;
                     hasNormal = true;
                     address.name = "normal gateway";
                     address.whichLocationId = PacketRerouteRequestResponse::LocationId_Gateway;
                  }
               }
               else
               {
                  if( hasAsset == false )
                  {
                     shouldPushAddress = true;
                     hasAsset = true;
                     address.name = "asset gateway";
                     address.whichLocationId = PacketRerouteRequestResponse::LocationId_Asset; 
                  }
               }

               if( shouldPushAddress )
               {
                  response->locations.push_back( address );
               }
               if( hasNormal && hasAsset )
                  break;
            }
            it++;
         }
      }

      HandlePacketToKhaan( khaan, response );// all deletion and such is handled lower
   }
}


///////////////////////////////////////////////////////////////////

// assuming that everything is thread protected at this point
void     DiplodocusLoadBalancer::HandlePacketToKhaan( KhaanConnector* khaan, BasePacket* packet )
{
   //PrintText( "HandlePacketToKhaan" );
   //U32 connectionId = khaan->GetConnectionId();
   khaan->AddOutputChainData( packet );

   MarkConnectionAsNeedingUpdate( khaan->GetChainedId() );

  /* Threading::Mutex locker( m_mutex ); // because of return
   //MarkConnectionAsNeedingUpdate
   ConnectionIdQueue::iterator it = m_connectionsNeedingUpdate.begin();
   while( it != m_connectionsNeedingUpdate.end() )
   {
      if( *it++ == connectionId )
         return;
   }
   m_connectionsNeedingUpdate.push_back( connectionId );*/
}


///////////////////////////////////////////////////////////////////

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

list< GatewayInfo >::iterator 
DiplodocusLoadBalancer::FindGateway( U32 serverId )
{
   list< GatewayInfo >::iterator it = m_gatewayRoutes.begin();
   while( it != m_gatewayRoutes.end() )
   {
      if( serverId && it->serverId == serverId )
      {
         return it;
      }
    
      it++;
   }

   return m_gatewayRoutes.end();
}

///////////////////////////////////////////////////////////////////

void     DiplodocusLoadBalancer::NewServerConnection( const PacketServerIdentifier* gatewayInfo )
{
   LogMessage( LOG_PRIO_INFO, "NewServerConnection packet: %s, %u, %u", gatewayInfo->serverAddress.c_str(), gatewayInfo->serverPort, gatewayInfo->serverId );

   list< GatewayInfo >::iterator it = FindGateway( gatewayInfo->serverAddress, gatewayInfo->serverPort, gatewayInfo->serverId );// we may not have the serverId already stored
   if( it != m_gatewayRoutes.end() )
   {
      it->serverId = gatewayInfo->serverId;
      it->port = gatewayInfo->serverPort;
      it->externalIpAddress = gatewayInfo->externalIpAddress.c_str();
      it->isVerified = true;
      it->isConnected = true;
      LogMessage( LOG_PRIO_INFO, "Gateway reconnected");

      return;
   }

   LogMessage( LOG_PRIO_INFO, "New Gateway connection");
   // clearly we don't have this gateway in our list. let's add it. We really don't need to remove these tho.
   AddGatewayAddress( gatewayInfo->serverAddress, gatewayInfo->serverPort );
   it = FindGateway( gatewayInfo->serverAddress, gatewayInfo->serverPort );
   it->serverId = gatewayInfo->serverId;
   it->externalIpAddress = gatewayInfo->externalIpAddress.c_str();
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
   LogMessage( LOG_PRIO_INFO, "ServerDisconnected packet: %s, %u", gatewayInfo->serverAddress.c_str(), gatewayInfo->serverId );

   list< GatewayInfo >::iterator it = FindGateway( gatewayInfo->serverAddress, 0, gatewayInfo->serverId );
   if( it != m_gatewayRoutes.end() )
   {
      it->isConnected = false;
   }
}

///////////////////////////////////////////////////////////////////

void     DiplodocusLoadBalancer::ServerInfoUpdate( const PacketServerConnectionInfo* gatewayInfo )
{
   LogMessage( LOG_PRIO_INFO, "ServerInfoUpdate packet: %s, %u", gatewayInfo->serverAddress.c_str(), gatewayInfo->serverId );

   list< GatewayInfo >::iterator it = FindGateway( gatewayInfo->serverAddress, 0, gatewayInfo->serverId );
   if( it != m_gatewayRoutes.end() )
   {
      it->currentLoad = gatewayInfo->currentLoad;
   }
}

///////////////////////////////////////////////////////////////////

void     DiplodocusLoadBalancer::RequestConnectionIds( const PacketServerToServer_GatewayRequestLB_ConnectionIds* gatewayInfo )
{
   list< GatewayInfo >::iterator it = FindGateway( gatewayInfo->serverAddress, 0, gatewayInfo->serverId );
   if( it != m_gatewayRoutes.end() )
   {
      if( it->serverId == gatewayInfo->serverId )
      {
         cout << "Requesting connection ids. values( " << m_distributedConnectionIdPoint << ":" << m_numConnectionIdsToDistrubute << ")" << endl;
         PacketServerToServer_GatewayRequestLB_ConnectionIdsResponse* response = new PacketServerToServer_GatewayRequestLB_ConnectionIdsResponse;
         if( m_distributedConnectionIdPoint >= ConnectionIdExclusion.low &&  
            m_distributedConnectionIdPoint <= ConnectionIdExclusion.high )
         {
            m_distributedConnectionIdPoint = ConnectionIdExclusion.high + 1;
         }
         if( m_distributedConnectionIdPoint + m_numConnectionIdsToDistrubute >= ConnectionIdExclusion.low &&  
            m_distributedConnectionIdPoint + m_numConnectionIdsToDistrubute <= ConnectionIdExclusion.high )
         {
            m_distributedConnectionIdPoint = ConnectionIdExclusion.high + 1;
         }
         response->beginningId = m_distributedConnectionIdPoint;
         response->countId = m_numConnectionIdsToDistrubute;
         m_distributedConnectionIdPoint += m_numConnectionIdsToDistrubute;

         PackageAndSendToOtherServer( response, gatewayInfo->serverId );
         return;
      }
   }

}

///////////////////////////////////////////////////////////////////

bool     DiplodocusLoadBalancer::PackageAndSendToOtherServer( BasePacket* packet, U32 serverId )
{
   PacketServerJobWrapper* wrapper = new PacketServerJobWrapper;
   wrapper->pPacket = packet;
   wrapper->serverId = serverId;

   //m_chatServer->AddOutputChainData( wrapper, serverId );
   return HandlePacketToOtherServer( wrapper, serverId );
}


///////////////////////////////////////////////////////////////////
