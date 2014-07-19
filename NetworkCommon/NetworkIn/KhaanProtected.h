// KhaanProtected.h
#pragma once


#include "../DataTypes.h"
#include "Khaan.h"

typedef std::deque< U32 >               ConnectionIdQueue;

//////////////////////////////////////////////////////////////////////

class KhaanProtected : public Khaan
{
public:
   KhaanProtected( int id, bufferevent* be );
   ~KhaanProtected();

   bool	   OnDataReceived( const U8* data, int length );
   void     SetMainOutput( ChainedInterface* chain ) { m_mainOutputChain = chain; }
   //----------------------------------------------

protected:   
   virtual bool  IsWhiteListedIn( const BasePacket* packet ) const = 0;
   virtual void  DenyAllFutureData();
   virtual bool  IsHandshaking( const BasePacket* packet ) { return false; }
   virtual bool  HasPermission( const BasePacket* packet ) const { return true; }
   virtual void  SetupOutputDelayTimestamp() {}

   virtual bool  IsAuthorized() const { return true; }
   virtual bool  IsPacketSafe( const U8* data, int& offset) { return true; }

   bool  HandleInwardSerializedPacket( const U8* data, int& offset );
   void  SendPacketToApp( BasePacket* packet );

   bool                       m_denyAllFutureData;
   bool                       m_markedToBeCleanedup;
   ChainedInterface*          m_mainOutputChain;
};

//////////////////////////////////////////////////////////////////////
