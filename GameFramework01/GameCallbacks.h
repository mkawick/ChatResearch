// GameCallbacks.h
#pragma once

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

struct MarshalledData
{
   int         m_sizeOfData;
   U8*         m_data;
};

struct UserInfo
{
   string   username;
   string   uuid;
   string   apple_id;
   U32      connectionId;
};

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

class GameCallbacks
{
public:
   virtual bool   UserConnected( const UserInfo* info, U32 connectionId ) = 0;
   virtual bool   UserDisconnected( U32 connectionId ) = 0;

   virtual bool   UserInput( U32 connectionId, const MarshalledData* packet ) = 0;

   virtual bool   CommandFromOtherServer( const BasePacket* instructions ) = 0;
   

   //GameFramework* m_game;// convenience?
};

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
