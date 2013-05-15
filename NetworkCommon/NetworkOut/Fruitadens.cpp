// Fruitadens.cpp

#include "../ServerConstants.h"

#include <iostream>

#include <string>
using namespace std;
#include <assert.h>
#include <boost/lexical_cast.hpp>

#include "Fruitadens.h"
#include "../Packets/BasePacket.h"
#include "../Packets/ServerToServerPacket.h"
#include "../Packets/PacketFactory.h"


//-----------------------------------------------------------------------------

Fruitadens :: Fruitadens( const char* name ) : CChainedThread( true, 200 ),
               m_clientSocket( 0 ),
               m_isConnected( false ),
               m_port( 0 ),
               m_hasFailedCritically( false ),
               m_serverType( ServerType_General )
{
   m_name = name;
   memset( &m_ipAddress, 0, sizeof( m_ipAddress ) );
}

//-----------------------------------------------------------------------------

void  Fruitadens :: NotifyEndpointOfIdentification( const string& serverName, U32 serverId, bool isGameServer, bool isController, bool requiresWrapper )
{
   PacketServerIdentifier* serverIdPacket = new PacketServerIdentifier;
   serverIdPacket->serverName = serverName;
   serverIdPacket->serverId = serverId;
   serverIdPacket->isGameServer = false;
   serverIdPacket->isController = false;

   if( requiresWrapper )
   {
      PacketServerToServerWrapper* wrapper = new PacketServerToServerWrapper;
      wrapper->serverId = serverId;
      wrapper->pPacket = serverIdPacket;
      AddOutputChainData( wrapper, 0 );
   }
   else
   {
      AddOutputChainData( serverIdPacket, 0 );
   }
}

//-----------------------------------------------------------------------------

bool        Fruitadens :: AddOutputChainData( BasePacket* packet, U32 filingData )
{
   if( FilterOutwardPacket( packet ) == false )
      return false;

   m_mutex.lock();
   m_packetsReadyToSend.push_back( packet );
   m_mutex.unlock();

   return true;
}

//-----------------------------------------------------------------------------

bool  Fruitadens :: Connect( const char* serverName, int port )// work off of DNS
{
   if( SetupConnection( serverName, port ) == false )
   {
      assert( 0 );
      return false;
   }
   return true;
}

//-----------------------------------------------------------------------------

bool  Fruitadens :: Disconnect()
{
   Cleanup();
   closesocket( m_clientSocket );

   return true;
}

//-----------------------------------------------------------------------------

bool  Fruitadens :: SetupConnection( const char* serverName, int port )
{
   Pause();
   m_port = port;

   string portString = boost::lexical_cast<string>( m_port );
   string notification = "Client on port " + portString;

   if( CreateSocket() == false )
   {
      return false;
   }

   struct hostent *host_entry;
   if ((host_entry = gethostbyname( serverName )) == NULL)
   {
      notification += " cannot resolve the hostname ";
      Log( notification.c_str() );

      m_hasFailedCritically = true;
      Cleanup();
      closesocket( m_clientSocket );
      m_clientSocket = 0;
   }

   m_ipAddress.sin_family = AF_INET;
   m_ipAddress.sin_port = htons( port );
   m_ipAddress.sin_addr.s_addr = *(unsigned long*) host_entry->h_addr;

   AttemptConnection();

   Resume();

   return true;
}

//-----------------------------------------------------------------------------

bool  Fruitadens :: CreateSocket()
{
   string portString = boost::lexical_cast<string>( m_port );
   string notification = "Client on port " + portString;

   m_clientSocket = socket( AF_INET, SOCK_STREAM, 0 );
   if (m_clientSocket == SOCKET_ERROR)
   {
      notification += " cannot open a socket ";
      Log( notification.c_str() );

      m_hasFailedCritically = true;
      Cleanup();
      closesocket( m_clientSocket );
      m_clientSocket = 0;

      return false;
   }
   return true;
}

//-----------------------------------------------------------------------------

void  Fruitadens :: AttemptConnection()
{
   if( m_isConnected == true || m_hasFailedCritically == true )
   {
      return;
   }

   string portString = boost::lexical_cast<string>( m_port );
   string notification = "Client on port " + portString;
   // this failure can happen for a lot of reasons, like the server hasn't been launched yet.
   if (connect( m_clientSocket, (sockaddr*)&m_ipAddress, sizeof(m_ipAddress)) == SOCKET_ERROR)
   {
      notification += " failed to connect ";
      Log( notification.c_str() );
      return;
   }

   notification += " has connected ";
   Log( notification.c_str() );

   if( SetSocketToNonblock( m_clientSocket ) == false )
   {
      notification += " cannot set socket to non-blocking ";
      Log( notification.c_str() );

      Cleanup();
      closesocket( m_clientSocket );
      m_clientSocket = 0;
      m_hasFailedCritically = true;
      return;
   }

   m_isConnected = true;
}

//-----------------------------------------------------------------------------

int   Fruitadens :: ProcessInputFunction()
{
   if( m_isConnected == false )
   {
      AttemptConnection();
      if( m_isConnected  == false )
      {
         return 0;
      }
   }
   U8 buffer[ MaxBufferSize ];

   while( 1 )
   {
      int numBytes = recv( m_clientSocket, (char*) buffer, MaxBufferSize, NULL );
	   if( numBytes == SOCKET_ERROR)
	   {
#if PLATFORM == PLATFORM_WINDOWS
         U32 error = WSAGetLastError();
         if( error != WSAEWOULDBLOCK )
         {
            if( error == WSAECONNRESET )
            {
               m_isConnected = false;
               //m_hasFailedCritically = true;
               cout << "***********************************************************" << endl;
               cout << "Socket error was: " << hex << error << dec << endl;
               cout << "Socket has been reset" << endl;
               cout << "attempting a reconnect" << endl;
               cout << "***********************************************************" << endl;
               closesocket( m_clientSocket );
               CreateSocket();
            }
         }
#endif
         break;
      }
      else
      {
         PacketFactory factory;
         int offset = 0;
         while( offset < numBytes )// there might be multiple packets in the same recv.
         {
            BasePacket* packetIn;
            if( factory.Parse( buffer, offset, &packetIn ) == true )
            {
               HandlePacketReceived( packetIn );
            }
            else 
            {
               offset = numBytes;// jump to the end
            }
         }
      }
   }

   return 1;
}


//-----------------------------------------------------------------------------------------

int  Fruitadens::ProcessOutputFunction()
{
   if( m_isConnected == false )
   {
      return 0;
   }

   if( m_packetsReadyToSend.size() > 0 )
   {
      m_mutex.lock();
      while( m_packetsReadyToSend.size() )
      {
         BasePacket* packet = m_packetsReadyToSend.front();
         SerializePacketOut( packet );
         m_packetsReadyToSend.pop_front();

     /*    if( packet->packetType == PacketType_GatewayWrapper )
         {
            PacketGatewayWrapper* wrapper = static_cast< PacketGatewayWrapper* >( packet );
            delete wrapper->pPacket;
         }*/
         delete packet;// can we move this out of the lock area?
      }
      m_mutex.unlock();
   }

   return 0;
}

//-----------------------------------------------------------------------------------------

bool  Fruitadens::HandlePacketReceived( BasePacket* packetIn )
{
   Threading::MutexLock locker( m_inputChainListMutex );
   // we don't do much interpretation here, we simply pass output data onto our output, which should be the DB or other servers.
   ChainLinkIteratorType itInput = m_listOfInputs.begin();
   while( itInput != m_listOfInputs.end() )// only one input currently supported.
   {
      ChainedInterface* inputPtr = (*itInput).m_interface;
      if( inputPtr->AddOutputChainData( packetIn ) == true )
      {
         return true;
      }
      itInput++;
   }
   
   delete packetIn;

   return false;
}

//-----------------------------------------------------------------------------

bool  Fruitadens :: SerializePacketOut( const BasePacket* packet )
{
   U8 buffer[ MaxBufferSize ];
   int offset = 0;

   packet->SerializeOut( buffer, offset );
   return SendPacket( buffer, offset );
}

//-----------------------------------------------------------------------------

bool  Fruitadens :: SendPacket( const U8* buffer, int length )
{
   if( m_isConnected && ( length > 0 ) )
   {
      int nBytes = send( m_clientSocket, reinterpret_cast<const char*>( buffer ), length, 0 );
      if( nBytes == SOCKET_ERROR || nBytes < length )
      {
         Log( "Client: It wont go through sir!!" );
         return false;
      }
   }
   else
   {
      return false;
   }

   return true;
}


//-----------------------------------------------------------------------------
