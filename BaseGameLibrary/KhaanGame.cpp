// KhaanGame.cpp

#include <iostream>

#include "KhaanGame.h"
#include "DiplodocusGame.h"
#include "../NetworkCommon/Packets/PacketFactory.h"
#include "../NetworkCommon/Packets/GamePacket.h"
#include "../NetworkCommon/Packets/ServerToServerPacket.h"

//-----------------------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////

int counter = 0;

bool	KhaanGame::OnDataReceived( unsigned char* data, int length )
{
   counter ++;

   //cout << " k# " << counter << endl;

   if( m_mainInterfacePtr == NULL )
   {
      SetupMainInterfacePointer();
   }
   assert( m_mainInterfacePtr != NULL ) ;

   TempStorage* ts = new TempStorage;
   memcpy( ts->data, data, length );
   ts->size = length;

   m_mutex.lock();
   m_toBeProcessed.push_back( ts );
   m_mutex.unlock();

   ThreadEvent te;
   te.type = ThreadEvent_NeedsService;
   te.identifier = m_chainId;
   m_mainInterfacePtr->PushInputEvent( &te );

   return true;
}

//---------------------------------------------------------------

void  KhaanGame :: SetupMainInterfacePointer()
{
   if( m_mainInterfacePtr )
      return;

   ChainLinkIteratorType itOutputs = m_listOfOutputs.begin();
   if( itOutputs != m_listOfOutputs.end() )// only ever one
   {
      const ChainLink& chain = *itOutputs++;
      m_mainInterfacePtr = chain.m_interface;
   }
}

int tracker = 0;
//---------------------------------------------------------------

bool   KhaanGame :: PassPacketOn( BasePacket* packetIn, U32 connectionId )
{
   // testing only, please remove
   if( packetIn->packetType == PacketType_GatewayWrapper )
   {
      PacketGatewayWrapper* wrapper = static_cast< PacketGatewayWrapper* >( packetIn );

      int temp = (int) (wrapper->pPacket->versionNumber);
      if( temp - tracker > 1 )
      {
         cout << "gap" << endl;
      }
      tracker = temp;

      //cout << " p# " << tracker  << endl;
   }
   m_mainInterfacePtr->AddInputChainData( packetIn, connectionId );
   return true;
}

//---------------------------------------------------------------

void  KhaanGame :: UpdateInwardPacketList()
{
   assert( m_mainInterfacePtr != NULL );

   while( m_toBeProcessed.size() )
   {
      m_mutex.lock();
      TempStorage* ts = m_toBeProcessed.front();
      m_toBeProcessed.pop_front();
      m_mutex.unlock();

      KhaanServerToServer::OnDataReceived( ts->data, ts->size );

      delete ts;

      //cout << "process" << endl;
   }
}
//-----------------------------------------------------------------------------------------
