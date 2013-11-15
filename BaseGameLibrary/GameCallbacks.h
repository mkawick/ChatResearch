// GameCallbacks.h
#pragma once

#include "../NetworkCommon/Packets/ContactPacket.h"

class GameFramework;
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

struct MarshalledData
{
   int         m_sizeOfData;
   const U8*   m_data;
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

   virtual bool   DataFromChat( U32 connectionId, const MarshalledData* packet ) { return false; }

   virtual bool   HandlePacketFromOtherServer( BasePacket* packet ){ return false; }

   // tournament stuff
   // the following may only remain as a testing interfaces
   virtual bool   UserWantsToJoinTournament( U32 connectionId, const string& tournamentUuid ) { return false; }
   virtual bool   UserWantsAListOfTournaments( U32 connectionId ) { return false; }
   
   // see PacketTournament_PurchaseTournamentEntryResponse
   virtual bool   UserWantsTournamentDetails( U32 connectionId, const string& tournamentUuid ) { return false; }
   virtual bool   UserWantsAListOfTournamentEntrants( U32 connectionId, const string& tournamentUuid ) { return false; }

   
protected:
   GameFramework& m_game;// convenience?
};

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
