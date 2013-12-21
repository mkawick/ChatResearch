// Fruitadens.cpp

#include "../ServerConstants.h"
#include "../Serialize.h"
#include "../Utils/Utils.h"

#include <iostream>
#include <iomanip>
#include <string>

#include <assert.h>
#include <boost/lexical_cast.hpp>

#include "Fruitadens.h"
#include "../Packets/BasePacket.h"
#include "../Packets/ServerToServerPacket.h"
#include "../Packets/PacketFactory.h"

#if PLATFORM != PLATFORM_WINDOWS
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#endif

#if PLATFORM == PLATFORM_MAC
#define MSG_NOSIGNAL SO_NOSIGPIPE
#elif PLATFORM == PLATFORM_WINDOWS
#define MSG_NOSIGNAL 0
#endif

using namespace std;

const int typicalSleepTime = 200;


//-----------------------------------------------------------------------------

Fruitadens :: Fruitadens( const char* name, bool processOnlyOneIncommingPacketPerLoop  ) : 
               Threading::CChainedThread <BasePacket*>( true, typicalSleepTime ),
               m_clientSocket( SOCKET_ERROR ),
               m_isConnected( false ),
               m_hasFailedCritically( false ),
               m_processOnlyOneIncommingPacketPerLoop( processOnlyOneIncommingPacketPerLoop ),
               m_checkForReroute( false ),
               m_connectedServerId( 0 ),
               m_connectedGameProductId( 0 ),
               m_port( 0 ),
               m_serverType( ServerType_General ),
               m_serverId( 0 ),
               m_numPacketsReceived( 0 ),
               m_receiveBufferOffset( 0 ), 
               m_packetHandlerInterface( NULL ),
               m_bytesInOverflow( 0 )
{
   m_name = name;
   memset( &m_ipAddress, 0, sizeof( m_ipAddress ) );

   m_receiveBufferSize = 1024 * 128;// 128k

   m_receiveBuffer = new U8[ m_receiveBufferSize ];
}

Fruitadens::~Fruitadens()
{
   Cleanup();
   delete [] m_receiveBuffer;
}

//-----------------------------------------------------------------------------
/*
void  Fruitadens :: SetupServerNotification( const string& serverName, const string& serverAddress, U32 serverId, U16 serverPort, U8 gameProductId, bool isGameServer, bool isController, bool requiresWrapper, bool isGateway )
{
   m_serverNotificationInfo.serverName = serverName;
   m_serverNotificationInfo.serverAddress = serverAddress;
   m_serverNotificationInfo.serverId = serverId;
   m_serverNotificationInfo.serverPort = serverPort;
   m_serverNotificationInfo.gameProductId = gameProductId;
   m_serverNotificationInfo.isController = isController;
   m_serverNotificationInfo.isGameServer = isGameServer;
   m_serverNotificationInfo.requiresWrapper = requiresWrapper;
   m_serverNotificationInfo.isGateway = isGateway;
}*/

//-----------------------------------------------------------------------------

//const string& serverName, const string& serverAddress, U32 serverId, U16 serverPort, bool isGameServer, bool isController, bool requiresWrapper )
/*void  Fruitadens :: NotifyEndpointOfIdentification()
{
   if( m_serverNotificationInfo.serverName.size() && 
      m_serverNotificationInfo.serverAddress.size() )
   {
      BasePacket* packet = NULL;
      PackageForServerIdentification( m_serverNotificationInfo.serverName, 
                                      m_serverNotificationInfo.serverAddress, 
                                      m_serverNotificationInfo.serverId, 
                                      m_serverNotificationInfo.serverPort, 
                                      m_serverNotificationInfo.gameProductId, 
                                      m_serverNotificationInfo.isGameServer, 
                                      m_serverNotificationInfo.isController, 
                                      m_serverNotificationInfo.requiresWrapper, 
                                      m_serverNotificationInfo.isGateway, &packet );
      AddOutputChainData( packet, 0 );
   }
}*/

void  Fruitadens :: NotifyEndpointOfIdentification( const string& serverName, const string& serverAddress, U32 serverId, U16 serverPort, U8 gameProductId, bool isGameServer, bool isController, bool requiresWrapper, bool isGateway )
{
   BasePacket* packet = NULL;
   PackageForServerIdentification( serverName, serverAddress, serverId, serverPort, gameProductId, isGameServer, isController, requiresWrapper, isGateway, &packet );
   AddOutputChainData( packet, 0 );
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
   cout << m_name << " connecting to " << serverName << " : " << port  << endl;
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
   closesocket( m_clientSocket );
   
   m_clientSocket = SOCKET_ERROR;
   m_isConnected = false;
   m_hasFailedCritically = false;

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
      m_clientSocket = SOCKET_ERROR;

      return false;
   }

   m_ipAddress.sin_family = AF_INET;
   m_ipAddress.sin_port = htons( port );
   m_ipAddress.sin_addr.s_addr = static_cast< U32 >( *(unsigned long*) host_entry->h_addr );

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
      m_clientSocket = SOCKET_ERROR;

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

   try
   {
      // this failure can happen for a lot of reasons, like the server hasn't been launched yet.
      if (connect( m_clientSocket, (sockaddr*)&m_ipAddress, sizeof(m_ipAddress)) == SOCKET_ERROR)
      {
   #if PLATFORM != PLATFORM_WINDOWS
         if( errno == EINPROGRESS )
         {
            return;
         }
   #endif
         notification += " failed to connect ";
         Log( notification.c_str() );
         return;
      }
   }
   catch(...)
   {
      m_hasFailedCritically = true;
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
      m_clientSocket = SOCKET_ERROR;
      m_hasFailedCritically = true;
      return;
   }

   m_isConnected = true;

   //NotifyEndpointOfIdentification();

   InitalConnectionCallback();
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

   memcpy( m_receiveBuffer, m_overflowBuffer, m_bytesInOverflow );
   m_receiveBufferOffset = m_bytesInOverflow;// save the offset
   m_bytesInOverflow = 0;

   int remainingBufferSize = m_receiveBufferSize - m_receiveBufferOffset;
   U8* buffer = m_receiveBuffer + m_receiveBufferOffset;

   int numBytesReceived = SOCKET_ERROR;
   do    
   {
      // pull everything off of the socket as quickly as possible to prevent packet loss 
      // witnessed during the development of the asset server. Over 1/3 of packets were lost.   
      numBytesReceived = static_cast< int >( recv( m_clientSocket, (char*) buffer, remainingBufferSize, NULL ) );

      if( numBytesReceived != SOCKET_ERROR )
      {
         buffer += numBytesReceived;
         remainingBufferSize -= numBytesReceived;
      }
   } while( numBytesReceived != SOCKET_ERROR && remainingBufferSize > 0 );

   

   // error checking
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
         m_clientSocket = SOCKET_ERROR;
         CreateSocket();
      }
   }
#endif


   // process the data in the queue
   PostProcessInputPackets( m_receiveBufferSize - remainingBufferSize );

   return 1;
}

//-----------------------------------------------------------------------------------------

void  Fruitadens::PostProcessInputPackets( int bytesRead )
{
   if( bytesRead < 1 )
   {
      return;
   }

   PacketFactory factory;
   int offset = 0;
   U16 size;

   while( offset < bytesRead )
   {
      int preOffset = offset;
      Serialize::In( m_receiveBuffer, offset, size );

      if( offset + size > bytesRead )
      {
         // copy remainder into temp buffer.
         m_bytesInOverflow = bytesRead - preOffset;
         memcpy( m_overflowBuffer, m_receiveBuffer + preOffset, m_bytesInOverflow );
         cout << "--- Overflow packets: " << m_bytesInOverflow << endl;
         return;
      }

      BasePacket* packetIn = NULL;
      if( factory.Parse( m_receiveBuffer, offset, &packetIn ) == true )
      {
         m_numPacketsReceived ++;
         HandlePacketReceived( packetIn );
      }
      else 
      {
         assert( 0 );// major problem
         offset = m_numPacketsReceived;// major failure here
      }
      if( m_processOnlyOneIncommingPacketPerLoop == true )
         break;
   }

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
   if( m_packetHandlerInterface && 
      m_packetHandlerInterface->HandlePacketReceived( packetIn ) == true )
      return true;

   PacketFactory factory;

   if( packetIn->packetType == PacketType_Base )
   { 
      // our basic bahavior is to ignore these initialization packets
      if(  packetIn->packetSubType == BasePacket::BasePacket_Hello )
      {
         factory.CleanupPacket( packetIn );
         return false;
      }
   }

   // special case... we handle server id directly, but we simply pass through for other s2s comms.
   if( packetIn->packetType == PacketType_ServerToServerWrapper )
   {
      PacketServerToServerWrapper* wrapper = static_cast< PacketServerToServerWrapper* >( packetIn );

      // ** note this reset
      packetIn = wrapper->pPacket;
      bool handled2SPacket = false;
      // ** note
      switch( packetIn->packetType )
      {
      case PacketType_ServerInformation:
         {
            PacketServerIdentifier* unwrappedPacket = static_cast< PacketServerIdentifier * > ( wrapper->pPacket );

            //if( m_connectedGameProductId != unwrappedPacket->gameProductId || m_connectedServerId != wrapper->serverId ) // prevent sups from reporting.
            {
               m_connectedServerId = wrapper->serverId;
               m_connectedGameProductId = unwrappedPacket->gameProductId;

               //std::string ip_txt( inet_ntoa( m_ipAddress.sin_addr ) );
               cout << endl;
               cout << "*********  Connected as client to " << unwrappedPacket->serverName << "  **************" << endl;
               cout << "    " << unwrappedPacket->serverAddress << " : " << static_cast<U32>( unwrappedPacket->serverPort ) << endl;
               cout << "    type " << static_cast<U32>( m_connectedGameProductId ) << " -- server ID = " << m_connectedServerId << endl;
               cout << "    isGame = " << boolalpha << unwrappedPacket->isGameServer << ", isController : " << unwrappedPacket->isController << noboolalpha << endl;
               cout << "**************************************************" << endl;
            }

            handled2SPacket = true;
         }
         break;
         // no other current needs
  /*    case BasePacket::ChatType_CreateChatChannelFromGameServerResponse: 
         {
            PacketChatCreateChatChannelFromGameServerResponse* unwrappedPacket = static_cast< PacketChatCreateChatChannelFromGameServerResponse * > ( wrapper->pPacket ); 
            if( unwrappedPacket->success )
            {
               CServerGameData* serverGameData = FindServerGameData( GAME_SELECT_SUMMONWAR, unwrappedPacket->gameId );
               serverGameData->SetUUID();
            }
            delete unwrappedPacket;
         }
         break;*/

      }


      if( handled2SPacket || packetIn == NULL ) 
      {
         factory.CleanupPacket( packetIn );
         return true;
      }
      // or we fall through
   }

   Threading::MutexLock locker( m_inputChainListMutex );
   // we don't do much interpretation here, we simply pass output data onto our output, which should be the DB or other servers.
   ChainLinkIteratorType itInput = m_listOfInputs.begin();
   while( itInput != m_listOfInputs.end() )// only one input currently supported.
   {
      ChainType* inputPtr = static_cast< ChainType*> ( (*itInput).m_interface );
      if( inputPtr->AddOutputChainData( packetIn ) == true )
      {
         return true;
      }
      itInput++;
   }
   
   factory.CleanupPacket( packetIn );
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

bool  Fruitadens :: SendPacket( const U8* buffer, int length ) const
{
   if( m_isConnected && ( length > 0 ) && m_clientSocket != SOCKET_ERROR )
   {
      int nBytes = SOCKET_ERROR;
      try
      {
         nBytes = static_cast< int >( send( m_clientSocket, reinterpret_cast<const char*>( buffer ), length, MSG_NOSIGNAL ) ); // we aren't handling sigpipe errors here so don't send them (crash/exit since we dont handle them)
      }
      catch(...)
      {
         nBytes = SOCKET_ERROR;
      }
      
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
