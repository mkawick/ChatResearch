// GameCallbacks.h
#pragma once

class GameFramework;
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

struct MarshalledData
{
   int         m_sizeOfData;
   const U8*   m_data;
};

struct UserInfo
{
   string   username;
   string   uuid;
   string   apple_id;
   U32      connectionId;
   U8       gameProductId;
   bool     active;
   string   email;
   string   passwordHash;
   string   id;
};

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

class GameCallbacks
{
public:
   GameCallbacks( GameFramework& game ) : m_game ( game ) {}
   virtual void   Initialize() {}

   virtual bool   UserConnected( const UserInfo* info, U32 connectionId ) = 0;
   virtual bool   UserDisconnected( U32 connectionId, bool errorDisconnect ) = 0;

   virtual bool   DataFromClient( U32 connectionId, const MarshalledData* packet ) = 0;
   //virtual bool   CommandFromOtherServer( const BasePacket* instructions ) = 0;

   virtual bool   UserConfirmedToOwnThisProduct( U32 connectionId, bool isConfirmed ) = 0;

   virtual bool   TimerCallback( U32 timerId, time_t& currentTime ) = 0;

   //virtual bool   DbQueryComplete( int queryType, char* result, bool success );

   virtual bool   DataFromChat( U32 connectionId, const MarshalledData* packet ) { return false; };
   
protected:
   GameFramework& m_game;// convenience?
};

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
