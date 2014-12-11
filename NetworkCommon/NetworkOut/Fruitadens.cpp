// Fruitadens.cpp

#include "../ServerConstants.h"
#include "../Packets/Serialize.h"
#include "../Utils/Utils.h"
#include "../Utils/StringUtils.h"
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

/*#if defined (CLIENT_ONLY)
const int defaultSocketTimeout = 10;
#else
const int defaultSocketTimeout = 15;
#endif*/

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
               m_bytesInOverflow( 0 ),
               m_extensiveLogging( false )
{
   m_chainedType = ChainedType_OutboundSocketConnector;
   m_name = name;
   memset( &m_ipAddress, 0, sizeof( m_ipAddress ) );

   m_receiveBufferSize = MaxBufferSize * 4;// 128k

   m_receiveBuffer = new U8[ m_receiveBufferSize ];

   m_keepAlive.Enable( false );
   m_keepAlive.Set( this );
   m_keepAlive.FunctionsAsServer( false );
   m_keepAlive.SetTimeout( 15 );
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
   {
      cout << "Fruitadens :: AddOutputChainData ... was filtered" << endl;
      return false;
   }

   //bool didLock = false;
   //if( m_outputChainListMutex.IsLocked() == false )
   {
      //didLock = true;
      //cout << "Fruitadens :: AddOutputChainData.. lock " << endl;
      m_outputChainListMutex.lock();
   }
   AddOutputChainDataNoLock( packet );
   
   //if( didLock == true )
   {
      //cout << "Fruitadens :: AddOutputChainData.. unlock " << endl;
      m_outputChainListMutex.unlock();
   }

   return true;
}

bool        Fruitadens :: AddOutputChainDataNoLock( BasePacket* packet )
{
   m_packetsReadyToSend.push_back( packet );
   return true;
}

//-----------------------------------------------------------------------------

bool  Fruitadens :: Connect( const char* serverName, int port )// work off of DNS
{
   LogMessage( LOG_PRIO_INFO, m_name.c_str(), " connecting to ", " : " );
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
      LogMessage( LOG_PRIO_INFO, notification.c_str() );

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
   LogMessage( LOG_PRIO_INFO, "Fruitadens :: CreateSocket()" );
   string portString = boost::lexical_cast<string>( m_port );
   string notification = "Client on port " + portString;

   m_clientSocket = static_cast< SocketType >( socket( AF_INET, SOCK_STREAM, 0 ) );
   if (m_clientSocket == SOCKET_ERROR)
   {
      notification += " cannot open a socket ";
      LogMessage( LOG_PRIO_INFO, notification.c_str() );

      m_hasFailedCritically = true;
      Cleanup();
      closesocket( m_clientSocket );
      m_clientSocket = SOCKET_ERROR;

      return false;
   }
   LogAllStateVars();
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
         if( m_extensiveLogging == true )
            LogMessage( LOG_PRIO_ERR, notification.c_str() );
         return;
      }
   }
   catch(...)
   {
      m_hasFailedCritically = true;
      return;
   }

   notification += " has connected ";
   LogMessage( LOG_PRIO_INFO, notification.c_str() );

   if( SetSocketToNonblock( m_clientSocket ) == false )
   {
      notification += " cannot set socket to non-blocking ";
      LogMessage( LOG_PRIO_ERR, notification.c_str() );

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

bool  Fruitadens :: HandleSendSocketErrorConditions( int numBytesReceived )
{
      // error checking
#if PLATFORM == PLATFORM_WINDOWS
   int error_number = WSAGetLastError();
   if( error_number != WSAEWOULDBLOCK )
   {
      if( error_number == WSAECONNRESET )
      {
         if( m_extensiveLogging == true )
            LogMessage( LOG_PRIO_ERR, "Fruitadens :: MainLoop_InputProcessing .. SocketHasDisconnectedDuringRecv" );

         SocketHasDisconnectedDuringRecv( error_number );
         return false;
      }
   }
#elif PLATFORM == PLATFORM_MAC || defined(ANDROID)
   if( numBytesReceived == -1 )
   {
      int error_number = errno;
      
      if( error_number != EINTR && errno != EWOULDBLOCK )//this error code for a non-blocking socket should be observed
          // common recv error which is commonly ignored
      {
         LogMessage( LOG_PRIO_ERR, "Fruitadens :: MainLoop_InputProcessing .. error_number: %d", error_number );
         SocketHasDisconnectedDuringRecv( error_number );
         return false;
      }
   }

#else
   if( numBytesReceived == -1 )
   {
      // this is normal and expected. We are polling the socket and for a non-blocking socket
      // -1 is the normal state. I retested this on 04Dec2014 on 10.16.4.33 using my simple_client
      // and this is how it works.
   }
   else if( numBytesReceived == 0 )
   {
      int error_number = errno;
      
    /*  fixme
      if( error_number == EWOULDBLOCK )//this error code for a non-blocking socket should be observed with 0 bytes received
      {
         LogMessage( LOG_PRIO_ERR, "Fruitadens :: MainLoop_InputProcessing .. normal disconnect: %d", error_number );
         SocketHasDisconnectedDuringRecv( error_number );
		   LogAllStateVars();
		   return false;
      }*/
      if( error_number != EINTR && errno != EWOULDBLOCK )
      {
         LogMessage( LOG_PRIO_ERR, "Fruitadens :: MainLoop_InputProcessing .. error_number: %d", error_number );
         SocketHasDisconnectedDuringRecv( error_number );
         return false;
      }
      
   }
#endif
   return true;
   
}

//-----------------------------------------------------------------------------

int   Fruitadens :: MainLoop_InputProcessing()
{
   if( m_extensiveLogging == true )
      LogMessage( LOG_PRIO_ERR, "Fruitadens :: MainLoop_InputProcessing enter" );
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
   int dupRemainingBufferSize = remainingBufferSize;
   U8* buffer = m_receiveBuffer + m_receiveBufferOffset;

   int numBytesReceived = SOCKET_ERROR;
   do    
   {
      if( m_extensiveLogging == true )
         LogMessage( LOG_PRIO_ERR, "Fruitadens :: MainLoop_InputProcessing .. recv" );

      // pull everything off of the socket as quickly as possible to prevent packet loss 
      // witnessed during the development of the asset server. Over 1/3 of packets were lost.   
      numBytesReceived = static_cast< int >( recv( m_clientSocket, (char*) buffer, remainingBufferSize, 0 ) );

      if( numBytesReceived != SOCKET_ERROR && numBytesReceived != 0 )
      {
         buffer += numBytesReceived;
         remainingBufferSize -= numBytesReceived;
      }
   } while( numBytesReceived != SOCKET_ERROR && numBytesReceived > 0 
            && remainingBufferSize > 0 );

   if( remainingBufferSize != dupRemainingBufferSize )
   {
      int numReceived = ( dupRemainingBufferSize ) - remainingBufferSize;
      if( m_extensiveLogging == true )
         LogMessage( LOG_PRIO_ERR, "Fruitadens :: MainLoop_InputProcessing .. data received %d", numReceived );
   }
   
   if( numBytesReceived > 0 ) // todo, remove this
      cout<< "Received " << numBytesReceived << " bytes" << endl;

   bool wasReceiveSuccessful = HandleSendSocketErrorConditions( numBytesReceived );

   if( wasReceiveSuccessful )
   {
      if( m_extensiveLogging == true )
         LogMessage( LOG_PRIO_ERR, "Fruitadens :: MainLoop_InputProcessing .. PostProcessInputPackets" );

      // process the data in the queue
      PostProcessInputPackets( m_receiveBufferSize - remainingBufferSize );

      if( m_extensiveLogging == true )
         LogMessage( LOG_PRIO_ERR, "Fruitadens :: MainLoop_InputProcessing exit" );
   }

   if( m_keepAlive.HasKeepAliveExpired() )
   {
      m_keepAlive.SetInvalid();
      SocketHasDisconnected();
   }
   else
   {
      m_keepAlive.Update();
   }

   return 1;
}

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------

void  Fruitadens::SocketHasDisconnectedDuringRecv( int error_number )
{
   
   //m_hasFailedCritically = true;
   LogMessage( LOG_PRIO_INFO, "***********************************************************" );
   LogMessage( LOG_PRIO_INFO, "Socket has been reset" );
   LogMessage( LOG_PRIO_INFO, "Socket error was: ", error_number );   
   LogMessage( LOG_PRIO_INFO, "attempting a reconnect" );
   LogMessage( LOG_PRIO_INFO, "***********************************************************" );
   SocketHasDisconnected();
}

void  Fruitadens::SocketHasDisconnected()
{
   LogMessage( LOG_PRIO_INFO, "***********************************************************" );
   LogMessage( LOG_PRIO_INFO, "Socket has been reset" );
   LogMessage( LOG_PRIO_INFO, "***********************************************************" );
   
   m_isConnected = false;
   closesocket( m_clientSocket );

   m_receiveBufferOffset = 0;
   m_keepAlive.ResetAfterDisconnect();

   InitialDisconnectionCallback();
   HasBeenDisconnectedCallback(); // tell higher layers
   m_clientSocket = SOCKET_ERROR;
   CreateSocket();
}

//-----------------------------------------------------------------------------------------
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
         LogMessage( LOG_PRIO_INFO, "--- Overflow packets: ", m_bytesInOverflow );
         return;
      }

      BasePacket* packetIn = NULL;
      int readSize = offset;
      bool parseResult = factory.Parse( m_receiveBuffer, readSize, &packetIn, m_networkVersionOverride );
      if( offset + size < readSize )
      {
         LogMessage( LOG_PRIO_INFO, "Super bad parsing error." );
      }

      if( parseResult == true )
      {
         int len = size;
         if( len > 16 )
            len = 16;
         DumpBuffer( m_receiveBuffer, offset, len );

         m_numPacketsReceived ++;
         if( m_extensiveLogging == true )
            LogMessage( LOG_PRIO_ERR, "Fruitadens :: MainLoop_InputProcessing .. HandlePacketReceived before" );
         
         if( m_keepAlive.HandlePacket( packetIn ) == false )
         {
            HandlePacketReceived( packetIn );
         }
      }
      else
      {
         if( m_extensiveLogging == true )
            LogMessage( LOG_PRIO_INFO, "Unknown packet type" );
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
   if( m_extensiveLogging )
   {
      LogMessage( LOG_PRIO_INFO, "Fruitadens: OutputLoop" );
   }

   if( m_isConnected == false )
   {
      return 0;
   }

   PacketQueue tempQueue = m_packetsReadyToSend;// already thread protected... m_outputChainListMutex
   m_packetsReadyToSend.clear();

   if( tempQueue.size() > 0 )
   {
      if( m_extensiveLogging )
      {
         LogMessage( LOG_PRIO_INFO, "MainLoop_OutputProcessing:sending packets: %u ", tempQueue.size() );
      }
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

   /*if( m_extensiveLogging == true )
      LogMessage( LOG_PRIO_ERR, "Fruitadens :: MainLoop_InputProcessing .. HandleS2SIdentitfyPacket before" );*/
   if( HandleS2SIdentitfyPacket( packetIn ) == true )
   {
      return true;
   }
/*   if( m_extensiveLogging == true )
      LogMessage( LOG_PRIO_ERR, "Fruitadens :: MainLoop_InputProcessing .. HandleS2SIdentitfyPacket after" );*/

   // this design does not require a lock. This function is called in all cases from 
   // within a threaded context and does not need additional locks.
   //if( m_inputChainListMutex.IsLocked() == false )
   //Threading::MutexLock locker( m_inputChainListMutex );
   // we don't do much interpretation here, we simply pass output data onto our output, which should be the DB or other servers.
   ChainLinkIteratorType itInput = m_listOfInputs.begin();
   while( itInput != m_listOfInputs.end() )// only one input currently supported.
   {
      ChainType* inputPtr = static_cast< ChainType*> ( (*itInput).m_interface );
      if( m_extensiveLogging == true )
         LogMessage( LOG_PRIO_ERR, "Fruitadens :: MainLoop_InputProcessing .. AddOutputChainData before" );
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

bool  	Fruitadens :: HandleS2SIdentitfyPacket( BasePacket* packetIn )
{
   return false;
}

//-----------------------------------------------------------------------------

void	Fruitadens :: LogAllStateVars()
{
   if( m_extensiveLogging == false )
      return;

	LogMessage( LOG_PRIO_INFO, "------------- Fruitadens states ----------------" );
	LogMessage( LOG_PRIO_INFO, "m_clientSocket                          %i", m_clientSocket );
	LogMessage( LOG_PRIO_INFO, "m_isConnected                           %i", m_isConnected );
	LogMessage( LOG_PRIO_INFO, "m_hasFailedCritically                   %i", m_hasFailedCritically );
	LogMessage( LOG_PRIO_INFO, "m_processOnlyOneIncommingPacketPerLoop  %i", m_processOnlyOneIncommingPacketPerLoop );
	LogMessage( LOG_PRIO_INFO, "m_checkForReroute                       %i", m_checkForReroute );
	LogMessage( LOG_PRIO_INFO, "m_isSettingUpConnection                 %i", m_isSettingUpConnection );
	LogMessage( LOG_PRIO_INFO, "m_connectedServerId                     %i", m_connectedServerId );
	
	LogMessage( LOG_PRIO_INFO, "m_connectedGameProductId                %i", m_connectedGameProductId );
	
	// m_ipAddress
	LogMessage( LOG_PRIO_INFO, "m_port                                  %u", (U32) m_port );
	LogMessage( LOG_PRIO_INFO, "m_serverType                            %u", (U32) m_serverType );
	LogMessage( LOG_PRIO_INFO, "m_name                                  %s", m_name.c_str() );
	LogMessage( LOG_PRIO_INFO, "m_packetsReadyToSend.size()             %u", (U32) m_packetsReadyToSend.size() );
	LogMessage( LOG_PRIO_INFO, "m_serverId                              %u",  m_serverId );
	LogMessage( LOG_PRIO_INFO, "m_numPacketsReceived                    %i", m_numPacketsReceived );
	
	LogMessage( LOG_PRIO_INFO, "m_receiveBufferSize                     %u", m_receiveBufferSize );
	LogMessage( LOG_PRIO_INFO, "m_receiveBufferOffset                   %u", m_receiveBufferOffset );
	LogMessage( LOG_PRIO_INFO, "m_networkVersionOverride                %u", (U32) m_networkVersionOverride );
	LogMessage( LOG_PRIO_INFO, "m_bytesInOverflow                       %i", m_bytesInOverflow );
	LogMessage( LOG_PRIO_INFO, "m_extensiveLogging                      %i", m_extensiveLogging );
	LogMessage( LOG_PRIO_INFO, "--------- end Fruitadens states ----------------" );
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

   if( m_extensiveLogging )
   {
      char stringBuffer[256];
      strcpy( stringBuffer, "packet={\n" );
      int numBytesToPrint = 16;
      if( numBytesToPrint > offset )
          numBytesToPrint = offset;

      for( int i=0; i< numBytesToPrint; i++ )
      {
         strcat( stringBuffer, ToHexString( buffer[i] ).c_str() );
         strcat( stringBuffer, " " );
      }

      strcat( stringBuffer, "\n}\n" );
      LogMessage( LOG_PRIO_INFO, stringBuffer );
   }

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
         if( m_extensiveLogging == true )
            LogMessage( LOG_PRIO_ERR, "Client: It wont go through sir!!" );
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
                              m_recentlyDisconnected( false ),
                              m_connectedServerPort( 0 ),
                              m_connectedIsGame( false ),
                              m_connectedIsController( false ),
                              m_connectedServerTime( 0 ),
                              m_disconnectedServerTime( 0 )
{
   //m_connectedServerIp;
   LogMessage( LOG_PRIO_INFO, "FruitadensServer:: %s", name );

   m_keepAlive.SetTimeout( 15 );
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
   if( m_extensiveLogging == true )
      LogMessage( LOG_PRIO_INFO, "FruitadensServer::InitialConnectionCallback" );
   
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
   if( m_extensiveLogging == true )
      LogMessage( LOG_PRIO_INFO, "FruitadensServer::InitialDisconnectionCallback" );

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
   //cout << "FruitadensServer::PackageLocalServerIdentificationToSend" << endl;

   if( m_extensiveLogging == true )
      LogMessage( LOG_PRIO_INFO, "FruitadensServer::PackageLocalServerIdentificationToSend" );

   if( m_areLocalIdentifyingParamsSet == false )
      return false;

   if( m_extensiveLogging == true )
      LogMessage( LOG_PRIO_INFO, "FruitadensServer::PackageLocalServerIdentificationToSend .. PackageForServerIdentification" );
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
   if( m_extensiveLogging == true )
      LogMessage( LOG_PRIO_INFO, "FruitadensServer::PackageForServerIdentification .. AddOutputChainData" );
   if( AddOutputChainDataNoLock( packet ) == false )
   {
      //if( m_extensiveLogging == true )
      LogMessage( LOG_PRIO_INFO, "FruitadensServer::PackageForServerIdentification .. packet was filtered" );
      PacketFactory factory;
      factory.CleanupPacket( packet );
   }

   if( m_extensiveLogging == true )
      LogMessage( LOG_PRIO_INFO, "FruitadensServer::PackageForServerIdentification .. complete" );
   return true;
}

//-------------------------------------------------------------------------

void   FruitadensServer::SocketHasDisconnectedDuringRecv( int error_number )
{
   time( &m_disconnectedServerTime );
   LogMessage( LOG_PRIO_INFO, "\n*********  Disconnected as client from %s  **************", m_connectedServerName.c_str() );
   LogMessage( LOG_PRIO_INFO, "    IP:PORT                     %s : %u", m_connectedServerIp.c_str(), m_connectedServerPort );
   LogMessage( LOG_PRIO_INFO, "    Time stamp:                 %s", GetDateInUTC().c_str() );
   LogMessage( LOG_PRIO_INFO, "    Amount of time connected(s):%d", static_cast<int>( difftime( m_disconnectedServerTime, m_connectedServerTime ) ) );
   LogMessage( LOG_PRIO_INFO, "    type                        %d", static_cast<U32>( m_connectedGameProductId ) );
   LogMessage( LOG_PRIO_INFO, "    server ID =                 %u", m_connectedServerId );
   LogMessage( LOG_PRIO_INFO, "    isGame = %s, isController = %s", ConvertToTrueFalseString( m_connectedIsController ), ConvertToTrueFalseString( m_connectedIsController  ) );
   LogMessage( LOG_PRIO_INFO, "**************************************************" );
         
   Fruitadens::SocketHasDisconnectedDuringRecv( error_number );
}

void	FruitadensServer :: LogAllStateVars()
{
   if( m_extensiveLogging == false )
      return;

	LogMessage( LOG_PRIO_INFO, "------------- FruitadensServer states ----------------" );
	Fruitadens :: LogAllStateVars();
	LogMessage( LOG_PRIO_INFO, "m_areLocalIdentifyingParamsSet  %i", m_areLocalIdentifyingParamsSet );
	LogMessage( LOG_PRIO_INFO, "m_localServerName               %s", m_localServerName.c_str() );
	LogMessage( LOG_PRIO_INFO, "m_localIpAddress                %s", m_localIpAddress.c_str() );
	LogMessage( LOG_PRIO_INFO, "m_externalIpAddress             %s", m_externalIpAddress.c_str() );
	LogMessage( LOG_PRIO_INFO, "m_localServerId                 %u", m_localServerId );
	LogMessage( LOG_PRIO_INFO, "m_localServerPort               %u", (U32) m_localServerPort );
	LogMessage( LOG_PRIO_INFO, "m_localGameProductId            %u", (U32) m_localGameProductId );
	LogMessage( LOG_PRIO_INFO, "m_gatewayType                   %i", (U32) m_gatewayType );
	LogMessage( LOG_PRIO_INFO, "m_localServerType               %i", (U32) m_localServerType );
	LogMessage( LOG_PRIO_INFO, "m_localIsGameServer             %i", m_localIsGameServer );
	LogMessage( LOG_PRIO_INFO, "m_areLocalIdentifyingParamsSet  %i", m_areLocalIdentifyingParamsSet );
	LogMessage( LOG_PRIO_INFO, "m_localIsController             %i", m_localIsController );
	LogMessage( LOG_PRIO_INFO, "m_localRequiresWrapper          %i", m_localRequiresWrapper );
	LogMessage( LOG_PRIO_INFO, "m_recentlyConnected             %i", m_recentlyConnected );
	LogMessage( LOG_PRIO_INFO, "m_recentlyDisconnected          %i", m_recentlyDisconnected );
	LogMessage( LOG_PRIO_INFO, "m_connectedServerName           %s", m_connectedServerName.c_str() );
	LogMessage( LOG_PRIO_INFO, "m_connectedServerIp             %s", m_connectedServerIp.c_str() );
	LogMessage( LOG_PRIO_INFO, "m_connectedServerPort           %u", (U32) m_connectedServerPort );
	LogMessage( LOG_PRIO_INFO, "m_connectedIsGame               %i", m_connectedIsGame );
	LogMessage( LOG_PRIO_INFO, "m_connectedIsController         %i", m_connectedIsController );
	LogMessage( LOG_PRIO_INFO, "m_connectedServerTime           %s", GetDateInUTC( m_connectedServerTime ).c_str() );
	LogMessage( LOG_PRIO_INFO, "m_disconnectedServerTime        %s", GetDateInUTC( m_disconnectedServerTime ).c_str() );

	LogMessage( LOG_PRIO_INFO, "--------- end FruitadensServer states ----------------" );
}

//-------------------------------------------------------------------------

bool  FruitadensServer :: HandleS2SIdentitfyPacket( BasePacket* packetIn )
{
   if( m_extensiveLogging == true )
      LogMessage( LOG_PRIO_INFO, "FruitadensServer::HandleS2SIdentitfyPacket " );
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
            m_connectedServerName = unwrappedPacket->serverName.c_str();
            m_connectedServerId = wrapper->serverId;
            m_connectedGameProductId = unwrappedPacket->gameProductId;
            m_connectedServerIp = unwrappedPacket->serverAddress.c_str();
            m_connectedServerPort = static_cast<U16>( unwrappedPacket->serverPort );
            m_connectedIsGame = unwrappedPacket->isGameServer;
            m_connectedIsController = unwrappedPacket->isController;
            time( &m_connectedServerTime );

            //std::string ip_txt( inet_ntoa( m_ipAddress.sin_addr ) );
            LogMessage( LOG_PRIO_INFO, "\n*********  Connected as client to %s  **************", m_connectedServerName.c_str() );
            LogMessage( LOG_PRIO_INFO, "    IP:PORT                     %s : %u", m_connectedServerIp.c_str(), m_connectedServerPort );
            LogMessage( LOG_PRIO_INFO, "    Time stamp:                 %s", GetDateInUTC().c_str() );
            LogMessage( LOG_PRIO_INFO, "    type                        %d", static_cast<U32>( m_connectedGameProductId ) );
            LogMessage( LOG_PRIO_INFO, "    server ID =                 %u", m_connectedServerId );
            LogMessage( LOG_PRIO_INFO, "    isGame = %s, isController = %s", ConvertToTrueFalseString( m_connectedIsController ), ConvertToTrueFalseString( m_connectedIsController  ) );
            LogMessage( LOG_PRIO_INFO, "**************************************************" );
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
