// Fruitadens.cpp

#include "../ServerConstants.h"
#include "../Packets/Serialize.h"
#include "../Utils/Utils.h"
#include "../NetworkUtils.h"

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
               m_isSettingUpConnection( false ),
               m_connectedServerId( 0 ),
               m_connectedGameProductId( 0 ),
               m_port( 0 ),
               m_serverType( ServerType_General ),
               m_serverId( 0 ),
               m_numPacketsReceived( 0 ),
               m_receiveBufferOffset( 0 ),
               m_networkVersionOverride( NetworkVersionMinor ),
               m_packetHandlerInterface( NULL ),
               m_bytesInOverflow( 0 )
               
{
   m_name = name;
   memset( &m_ipAddress, 0, sizeof( m_ipAddress ) );

   m_receiveBufferSize = MaxBufferSize * 4;// 128k

   m_receiveBuffer = new U8[ m_receiveBufferSize ];
}

Fruitadens::~Fruitadens()
{
   Cleanup();
   delete [] m_receiveBuffer;
}

//-----------------------------------------------------------------------------

bool        Fruitadens :: AddOutputChainData( BasePacket* packet, U32 filingData )
{
   if( FilterOutwardPacket( packet ) == false )
      return false;

   m_outputChainListMutex.lock();
   m_packetsReadyToSend.push_back( packet );
   m_outputChainListMutex.unlock();

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

   //m_packetsReadyToSend.clear();
   
   m_clientSocket = SOCKET_ERROR;
   m_isConnected = false;
   m_hasFailedCritically = false;

   return true;
}

//-----------------------------------------------------------------------------

void   Fruitadens :: HasBeenConnectedCallback() 
{
   if( m_packetHandlerInterface )
   {
      m_packetHandlerInterface->InitialConnectionCallback( this );
   }
}
void   Fruitadens :: HasBeenDisconnectedCallback() 
{
   if( m_packetHandlerInterface )
   {
      m_packetHandlerInterface->InitialDisconnectionCallback( this );
   }
}


//-----------------------------------------------------------------------------

bool  Fruitadens :: SetupConnection( const char* serverName, int port )
{
   if( m_isSettingUpConnection == true ) // preventing reentrant code.. threading protections.
      return false;

   m_isSettingUpConnection = true;
   SetValueOnExit< bool > setter( m_isSettingUpConnection, false );

   Pause();
   m_port = port;

   string portString = boost::lexical_cast<string>( m_port );
   string notification = "Client on port " + portString;

   if( CreateSocket() == false )
   {
      return false;
   }

   m_isConnected = false;
   m_hasFailedCritically = false;

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

   Resume();

   return true;
}

//-----------------------------------------------------------------------------

bool  Fruitadens :: CreateSocket()
{
   string portString = boost::lexical_cast<string>( m_port );
   string notification = "Client on port " + portString;

   m_clientSocket = static_cast< SocketType >( socket( AF_INET, SOCK_STREAM, 0 ) );
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

   HasBeenConnectedCallback();
   InitialConnectionCallback();
}

//-----------------------------------------------------------------------------

int   Fruitadens :: MainLoop_InputProcessing()
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
   int error_number = WSAGetLastError();
   if( error_number != WSAEWOULDBLOCK )
   {
      if( error_number == WSAECONNRESET )
      {
         SocketHasDisconnectedDuringRecv( error_number );
      }
   }
#else
   if( numBytesReceived == -1 )
   {
      int error_number = errno;
      if( error_number != EINTR && errno != EWOULDBLOCK ) // common recv error which is commonly ignored
      {
         SocketHasDisconnectedDuringRecv( error_number );
      }
   }
#endif


   // process the data in the queue
   PostProcessInputPackets( m_receiveBufferSize - remainingBufferSize );

   return 1;
}

//-----------------------------------------------------------------------------------------

void  Fruitadens::SocketHasDisconnectedDuringRecv( int error_number )
{
   m_isConnected = false;
   //m_hasFailedCritically = true;
   cout << "***********************************************************" << endl;
   cout << "Socket has been reset" << endl;
   cout << "Socket error was: " << hex << error_number << dec << endl;   
   cout << "attempting a reconnect" << endl;
   cout << "***********************************************************" << endl;
   closesocket( m_clientSocket );

   InitialDisconnectionCallback();
   HasBeenDisconnectedCallback(); // tell higher layers
   m_clientSocket = SOCKET_ERROR;
   CreateSocket();
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
      Serialize::In( m_receiveBuffer, offset, size, m_networkVersionOverride );

      if( offset + size > bytesRead )
      {
         // copy remainder into temp buffer.
         m_bytesInOverflow = bytesRead - preOffset;
         memcpy( m_overflowBuffer, m_receiveBuffer + preOffset, m_bytesInOverflow );
         cout << "--- Overflow packets: " << m_bytesInOverflow << endl;
         return;
      }

      BasePacket* packetIn = NULL;
      int readSize = offset;
      bool parseResult = factory.Parse( m_receiveBuffer, readSize, &packetIn, m_networkVersionOverride );
      if( offset + size < readSize )
      {
         cout << "Super bad parsing error." << endl;
      }

      if( parseResult == true )
      {
         m_numPacketsReceived ++;
         HandlePacketReceived( packetIn );
      }
      else
      {
         cout << "Unknown packet type" << endl;
      }
    /*  else 
      {
         assert( 0 );// major problem
         offset = m_numPacketsReceived;// major failure here
      }*/
      offset += size; // ignore the read size because this could be smaller 
      // than the number of bytes provided by the underlying transport.
      if( m_processOnlyOneIncommingPacketPerLoop == true )
         break;
   }

}

//-----------------------------------------------------------------------------------------

int  Fruitadens::MainLoop_OutputProcessing()
{
   if( m_isConnected == false )
   {
      return 0;
   }

   PacketQueue tempQueue = m_packetsReadyToSend;// already thread protected... m_outputChainListMutex
   m_packetsReadyToSend.clear();

   if( tempQueue.size() > 0 )
   {
      PacketFactory factory;      
      while( tempQueue.size() )
      {
         BasePacket* packet = tempQueue.front();
         tempQueue.pop_front();
         
         SerializePacketOut( packet );

         factory.CleanupPacket( packet );
      }
   }

   InheritedUpdate();

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

   if( HandleS2SIdentitfyPacket( packetIn ) == true )
   {
      return true;
   }

   Threading::MutexLock locker( m_inputChainListMutex );
   // we don't do much interpretation here, we simply pass output data onto our output, which should be the DB or other servers.
   ChainLinkIteratorType itInput = m_listOfInputs.begin();
   while( itInput != m_listOfInputs.end() )// only one input currently supported.
   {
      ChainType* inputPtr = static_cast< ChainType*> ( (*itInput).m_interface );
      if( inputPtr->AddOutputChainData( packetIn, m_connectedServerId ) == true )
      {
         return true;
      }
      itInput++;
   }
   
   factory.CleanupPacket( packetIn );
   return false;
}

//-----------------------------------------------------------------------------

bool  Fruitadens :: HandleS2SIdentitfyPacket( BasePacket* packetIn )
{
   return false;
}

//-----------------------------------------------------------------------------

bool  Fruitadens :: SerializePacketOut( const BasePacket* packet )
{
   U8 buffer[ MaxBufferSize ];
   
   int   offset = 2;
   
   if( m_networkVersionOverride != NetworkVersionMinor )
   {
      BasePacket* p = const_cast<BasePacket*>( packet );// yes, I know that this is awful... normally. Not so bad here. I am working aorund the compiler.
      p->versionNumberMinor = m_networkVersionOverride;
      p->SerializeOut( buffer, offset, m_networkVersionOverride );
   }
   else
   {
      packet->SerializeOut( buffer, offset, m_networkVersionOverride );
   }
   U16   size = offset-2;
   int   headerOffset = 0;
   Serialize::Out( buffer, headerOffset, size, m_networkVersionOverride );

   return SendPacket( buffer, offset );
}

//-----------------------------------------------------------------------------

bool  Fruitadens :: SendPacket( const U8* buffer, int length ) const
{
   if( m_isConnected && ( length > 0 ) && m_clientSocket != SOCKET_ERROR )
   {
/*//#ifdef TwoByteProtocol
      U16 len = static_cast< U16 >( length );
      try
      {
         nBytes = static_cast< int >( send( m_clientSocket, reinterpret_cast<const char*>( &len ), sizeof( U16 ), MSG_NOSIGNAL ) ); // we aren't handling sigpipe errors here so don't send them (crash/exit since we dont handle them)
      }
      catch(...)
      {
         nBytes = SOCKET_ERROR;
      }
//#endif*/

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

FruitadensServer :: FruitadensServer( const char* name, bool processOnlyOneIncommingPacketPerLoop ) : 
                              Fruitadens( name, processOnlyOneIncommingPacketPerLoop ),
                              m_areLocalIdentifyingParamsSet( false ),
                              m_localServerId( 0 ),
                              m_localServerPort( 0 ),
                              m_localGameProductId( 0 ),
                              m_gatewayType( 0 ),
                              m_localIsGameServer( false ),
                              m_localIsController( false ),
                              m_localRequiresWrapper( true ),
                              m_recentlyConnected( false ),
                              m_recentlyDisconnected( false )
{
}

//-----------------------------------------------------------------------------

void     FruitadensServer :: NotifyEndpointOfIdentification( const string& serverName, const string& serverAddress, U32 serverId, U8 serverType, U16 serverPort, U8 gameProductId, bool isGameServer, bool isController, bool requiresWrapper, U8 gatewayType, const string& externalIpAddress )
{
   m_areLocalIdentifyingParamsSet = true;
   m_localServerName = serverName;
   m_localIpAddress = serverAddress;
   m_localServerId = serverId;
   m_localServerPort = serverPort;
   m_localServerType = (ServerType) serverType;
   m_localGameProductId = gameProductId;
   m_localIsGameServer = isGameServer;
   m_localIsController = isController;
   m_localRequiresWrapper = requiresWrapper;
   m_gatewayType = gatewayType;
   m_externalIpAddress = externalIpAddress;

   //PackageLocalServerIdentificationToSend();
}

//-----------------------------------------------------------------------------

void     FruitadensServer::InitialConnectionCallback()
{
   ChainLinkIteratorType   itInputs = m_listOfInputs.begin();
   while( itInputs != m_listOfInputs.end() )
   {
      IChainedInterface* inputPtr = itInputs->m_interface;
      inputPtr->OutputConnected( this );
      itInputs++;
   }

   PackageLocalServerIdentificationToSend();

   m_recentlyDisconnected = false;
   m_recentlyConnected = true;
}

//-----------------------------------------------------------------------------

void     FruitadensServer::InitialDisconnectionCallback()
{
   ChainLinkIteratorType   itInputs = m_listOfInputs.begin();
   while( itInputs != m_listOfInputs.end() )
   {
      IChainedInterface* inputPtr = itInputs->m_interface;
      inputPtr->OutputRemovalInProgress( this );
      itInputs++;
   }

   m_recentlyDisconnected = true;
   m_recentlyConnected = false;
}

//-------------------------------------------------------------------------

bool     FruitadensServer::PackageLocalServerIdentificationToSend()
{
   if( m_areLocalIdentifyingParamsSet == false )
      return false;

   BasePacket* packet = NULL;
   PackageForServerIdentification( m_localServerName, 
                                   m_localIpAddress, 
                                   m_externalIpAddress,
                                   m_localServerId, 
                                   m_localServerType,
                                   m_localServerPort, 
                                   m_localGameProductId, 
                                   m_localIsGameServer, 
                                   m_localIsController, 
                                   m_localRequiresWrapper, 
                                   m_gatewayType, 
                                   &packet );
   if( AddOutputChainData( packet, 0 ) == false )
   {
      PacketFactory factory;
      factory.CleanupPacket( packet );
   }

   return true;
}

//-------------------------------------------------------------------------

bool  FruitadensServer :: HandleS2SIdentitfyPacket( BasePacket* packetIn )
{
   // special case... we handle server id directly, but we simply pass through for other s2s comms.
   if( packetIn->packetType == PacketType_ServerToServerWrapper )
   {
      PacketServerToServerWrapper* wrapper = static_cast< PacketServerToServerWrapper* >( packetIn );

      // ** note this reset
      BasePacket* contentPacket = wrapper->pPacket;
      bool handled2SPacket = false;
      // ** note
      if( contentPacket->packetType == PacketType_ServerInformation && 
            contentPacket->packetSubType == PacketServerConnectionInfo::PacketServerIdentifier_TypicalInfo )
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

      if( handled2SPacket || contentPacket == NULL ) 
      {
         PacketFactory factory;
         factory.CleanupPacket( contentPacket );
         return true;
      }
   }
   return false;
}

//-------------------------------------------------------------------------
