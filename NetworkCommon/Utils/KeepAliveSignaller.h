// KeepAliveSignaller.h
#pragma once

class BasePacket;

/////////////////////////////////////////////////////////////////////////////////

// the timing of this is a little confusing. If the signal is sent at the exact time
// then the client will timeout. E.g. client and server are set for 15 seconds,
// then the server sends the first, which the client records and starts a timer. When
// the server sends the second keep alive after 15 seconds, the client will timeout
// before the packet arrives. I have set the defaults to have the server try after 10
// secs and the client timeout after 12 secs.

class KeepAliveSignaller
{
public:
   KeepAliveSignaller() : 
         m_isValid( true ),
         m_isEnabled( false ),
         m_requiresKeepAliveSignal( false ),
         m_isServer( true ),
         m_timeLastSignalSent( 0 ),
         m_timeLastSignalReceived( 0 ),
         m_timeoutPeriod( 10 ),
         m_packetCounter( 0 ),
         m_isAwaitingKeepAliveReturn( false ),
         m_packetHandler( NULL )
   { }

   void  SetInvalid() { m_isValid = true; }
   void  ClearInvalid() { m_isValid = false; }
   void  ResetAfterDisconnect();
   void  Enable( bool enable );
   void  Set( ChainedInterface <BasePacket*>* signalled ) { m_packetHandler = signalled; }
   void  SetTimeout( int seconds ) { m_timeoutPeriod = seconds; }
   void  FunctionsAsServer( bool isServer );// either input or output. Khaan = input, Fruitidens = out
   
   bool  HasKeepAliveExpired() const;
   
   void  Update();
   bool  HandlePacket( const BasePacket* packetIn );
private:

   bool     CanDoAnyWork() const;

   bool     m_isValid;
   bool     m_isEnabled;
   bool     m_requiresKeepAliveSignal;
   bool     m_isServer;
   time_t   m_timeLastSignalSent;
   time_t   m_timeLastSignalReceived;
   int      m_timeoutPeriod;
   int      m_packetCounter;
   bool     m_isAwaitingKeepAliveReturn;
   ChainedInterface <BasePacket*>* m_packetHandler;
};

/////////////////////////////////////////////////////////////////////////////////
