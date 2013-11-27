#include "DiplodocusLoadBalancer.h"

#include "../NetworkCommon/Packets/ServerToServerPacket.h"

///////////////////////////////////////////////////////////////////

DiplodocusLoadBalancer::DiplodocusLoadBalancer( const string& serverName, U32 serverId ): Diplodocus< KhaanConnector >( serverName, serverId, 0,  ServerType_LoadBalancer ), m_connectionIdTracker( 100 )
{
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
   //khaan->SendThroughLibEvent( true );

   Threading::MutexLock locker( m_outputChainListMutex );
   m_connectionsNeedingUpdate.push_back( newId );
}
//---------------------------------------------------------------

bool  DiplodocusLoadBalancer::HandlePacketFromOtherServer( BasePacket* packet, U32 connectionId )// not thread safe
{
   if( packet->packetType != PacketType_ServerJobWrapper )
   {
      return false;
   }

   PacketServerJobWrapper* wrapper = static_cast< PacketServerJobWrapper* >( packet );
   BasePacket* unwrappedPacket = wrapper->pPacket;
   U32  serverIdLookup = wrapper->serverId;

   bool success = false;

 /*  if( unwrappedPacket->packetType == PacketType_Login )
   {
      switch( unwrappedPacket->packetSubType )
      {
      case PacketLogin::LoginType_PrepareForUserLogin:
         ConnectUser( static_cast< PacketPrepareForUserLogin* >( unwrappedPacket ) );
         return true;

      case PacketLogin::LoginType_PrepareForUserLogout:
         DisconnectUser( static_cast< PacketPrepareForUserLogout* >( unwrappedPacket ) );
         return true;

      case PacketLogin::LoginType_ListOfProductsS2S:
         StoreUserProductsOwned( static_cast< PacketListOfUserProductsS2S* >( unwrappedPacket ) );
         return true;
      }
   }
   else if( unwrappedPacket->packetType == PacketType_Tournament )
   {
      switch( unwrappedPacket->packetSubType )
      {
      case PacketTournament::TournamentType_PurchaseTournamentEntry:
         {
            return HandlePurchaseRequest( static_cast< PacketTournament_PurchaseTournamentEntry* >( unwrappedPacket ), serverIdLookup );
         }
         break;
      }
      return false;
   }*/

   return false;
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

 /*  PacketLogout* logout = new PacketLogout();// must be done before we clear the lists of ids
   logout->wasDisconnectedByError = true;
   //logout->serverType = ServerType_Chat;

   AddInputChainData( logout, connectionId );*/

   m_socketToConnectionMap.erase( socketId );
   m_connectionToSocketMap.erase( connectionId );
   m_connectionMap.erase( connectionId );
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
   return false;
}

//-----------------------------------------------------------------------------------------

void     DiplodocusLoadBalancer::HandleRerouteRequest( U32 connectionId )
{
   ConnectionMapIterator connIt = m_connectionMap.find( connectionId );
   if( connIt != m_connectionMap.end() )
   {
      PacketRerouteRequestResponse* response = new PacketRerouteRequestResponse;
      list< GatewayInfo >::iterator it = m_gatewayRoutes.begin();
      while( it != m_gatewayRoutes.end() )
      {
         PacketRerouteRequestResponse::Address address;
         address.address = it->address;
         address.port = it->port;
         address.name = "gateway";
         address.whichLocationId = PacketRerouteRequestResponse::LocationId_Gateway; // these will need to vary

         response->locations.push_back( address );
         
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

///////////////////////////////////////////////////////////////////
