// DiplodocusGateway.cpp

#include "DiplodocusGateway.h"
#include "FruitadensGateway.h"
#include "../NetworkCommon/ServerConstants.h"
#include "../NetworkCommon/Packets/ServerToServerPacket.h"
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
                                          m_connectionIdTracker( 12 )
{
   SetSleepTime( 16 );// 30 fps
   SetSendHelloPacketOnLogin( true );
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
      PacketGatewayWrapper* wrapper = new PacketGatewayWrapper;
      wrapper->connectionId = connectionId;
      wrapper->pPacket = packet;

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

void     DiplodocusGateway::InputConnected( ChainedInterface * chainedInput )
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

void  DiplodocusGateway::InputRemovalInProgress( ChainedInterface * chainedInput )
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

bool DiplodocusGateway::PushPacketToProperOutput( BasePacket* packet )
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
      ChainedInterface* outputPtr = (*itOutput).m_interface;
      FruitadensGateway* fruity = static_cast< FruitadensGateway* >( outputPtr );
      if( fruity->AddOutputChainData( packet, -1 ) == true )
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
   SendServerIdentification();

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
      }
      else
      {
         khaan->DenyAllFutureData();
         connectionId = 0;
      }

      PacketLoginToClient* clientNotify = new PacketLoginToClient;
      clientNotify->wasLoginSuccessful = finishedLogin->wasLoginSuccessful;
      clientNotify->uuid = finishedLogin->uuid;
      clientNotify->username = finishedLogin->username;
      clientNotify->lastLogoutTime = finishedLogin->lastLogoutTime;
      clientNotify->connectionId = connectionId;

      delete finishedLogin;
      packet = clientNotify;
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

         SocketToConnectionMapIterator it = m_connectionToSocketMap.find( connectionId );
         if( it != m_connectionToSocketMap.end() )
         {
            
            ConnectionMapIterator connIt = m_connectionMap.find( connectionId );
            if( connIt != m_connectionMap.end() )
            {
               KhaanConnector* khaan = connIt->second;
               HandlePacketToKhaan( khaan, dataPacket );// all deletion and such is handled lower
            }

            else
            {
               factory.CleanupPacket( dataPacket );
            }
            //m_inputChainListMutex.lock();

         }
      }

      //m_mutex.lock();
      while( m_connectionsNeedingUpdate.size() > 0 )// this has the m_outputChainListMutex protection
      {
         int connectionId = m_connectionsNeedingUpdate.front();
         m_connectionsNeedingUpdate.pop_front();
         ConnectionMapIterator connIt = m_connectionMap.find( connectionId );
         if( connIt != m_connectionMap.end() )
         {
            KhaanConnector* khaan = connIt->second;
            khaan->Update();
         }

      }
      //m_mutex.unlock();
   }
   return 1;
}
//-----------------------------------------------------------------------------------------
