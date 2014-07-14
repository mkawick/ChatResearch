// TournamentPacket.h
#pragma once

#include "BasePacket.h"
#include "PurchasePacket.h"

///////////////////////////////////////////////////////////////////

class TournamentInfo     //string userUuid.. will be sotred by id using uuid
{
public:
   TournamentInfo(){}
   TournamentInfo( const string& name, const string& uuid, const string& dateBegin, const string& dateEnd ): 
                  tournamentName( name ),
                  tournamentUuid( uuid ),
                  beginDate( dateBegin ),
                  endDate( dateEnd ),
                  timePerRound( 3 ),
                  timeUnitsPerRound( 1 )
                  {}// no initialization

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   void  Clear();

   string      tournamentName;
   UuidString  tournamentUuid;
   string      beginDate;
   string      endDate;
   int         timePerRound;
   int         timeUnitsPerRound;
  /* string   description;
   int      price;   /// lookup from products table
   bool     userCanEnter; // basically a first pass at approving a user's entry into the tournament. More of a first-pass denial.
   int      numberOfEntrantSlots;
   int      usersPerGame;
   int      gamesPerRound;
   int      pointsPerWin;
   int      pointsPerLoss;
   int      pointsForParticipating;
   int      pointsPerGamePlayed;
   int      playStyle;

   int      avatarId;
   bool     isOnline;*/
};

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

class PacketTournament : public BasePacket 
{
public:
   enum TournamentType
   {
      TournamentType_Base,
      TournamentType_TestNotification,

      TournamentType_RequestListOfTournaments,
      TournamentType_RequestListOfTournamentsResponse,

      TournamentType_RequestTournamentDetails,
      TournamentType_RequestTournamentDetailsResponse,

      TournamentType_RequestListOfTournamentEntrants,
      TournamentType_RequestListOfTournamentEntrantsResponse,

      TournamentType_UserRequestsEntryInTournament, 
      TournamentType_UserRequestsEntryInTournamentResponse,

      TournamentType_PurchaseTournamentEntry, // server to server request
      TournamentType_PurchaseTournamentEntryResponse,

      TournamentType_PurchaseTournamentEntryRefund, // server to server request
      TournamentType_PurchaseTournamentEntryRefundResponse,
   };
public:
   PacketTournament( int packet_type = PacketType_Tournament, int packet_sub_type = TournamentType_Base ) : BasePacket( packet_type, packet_sub_type ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;
};

///////////////////////////////////////////////////////////////////

class PacketTournament_TestNotification : public PacketTournament
{
public:
   PacketTournament_TestNotification() : PacketTournament( PacketType_Tournament, TournamentType_TestNotification ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   message;
   string   senderName;
   UuidString  senderUuid;
   int      type;
};


///////////////////////////////////////////////////////////////

class PacketTournament_RequestListOfTournaments : public PacketTournament
{
public:
   PacketTournament_RequestListOfTournaments() : PacketTournament( PacketType_Tournament, TournamentType_RequestListOfTournaments ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;
};

///////////////////////////////////////////////////////////////////

class PacketTournament_RequestListOfTournamentsResponse : public PacketTournament
{
public:
   PacketTournament_RequestListOfTournamentsResponse() : PacketTournament( PacketType_Tournament, TournamentType_RequestListOfTournamentsResponse ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   SerializedKeyValueVector< TournamentInfo >   tournaments;
};

///////////////////////////////////////////////////////////////

class PacketTournament_RequestTournamentDetails : public PacketTournament
{
public:
   PacketTournament_RequestTournamentDetails() : PacketTournament( PacketType_Tournament, TournamentType_RequestTournamentDetails ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   UuidString  tournamentUuid;
};

///////////////////////////////////////////////////////////////////

class PacketTournament_RequestTournamentDetailsResponse : public PacketTournament
{
public:
   PacketTournament_RequestTournamentDetailsResponse() : PacketTournament( PacketType_Tournament, TournamentType_RequestTournamentDetailsResponse ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   TournamentInfo tournament;
};

///////////////////////////////////////////////////////////////

class PacketTournament_RequestListOfEntrants : public PacketTournament
{
public:
   PacketTournament_RequestListOfEntrants() : PacketTournament( PacketType_Tournament, TournamentType_RequestListOfTournamentEntrants ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   UuidString  tournamentUuid;
};

///////////////////////////////////////////////////////////////////

class PacketTournament_RequestListOfEntrantsResponse : public PacketTournament
{
public:
   PacketTournament_RequestListOfEntrantsResponse() : PacketTournament( PacketType_Tournament, TournamentType_RequestListOfTournamentEntrantsResponse ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   SerializedKeyValueVector< string > entrants;
};

///////////////////////////////////////////////////////////////////

class PacketTournament_UserRequestsEntryInTournament : public PacketTournament
{
public:
   PacketTournament_UserRequestsEntryInTournament() : PacketTournament( PacketType_Tournament, TournamentType_UserRequestsEntryInTournament ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   UuidString                                      tournamentUuid;
   FixedString60                                   customDeck;
   SerializedVector< PurchaseServerDebitItem >     itemsToSpend;
};

///////////////////////////////////////////////////////////////////

class PacketTournament_UserRequestsEntryInTournamentResponse : public PacketTournament
{
public:
   PacketTournament_UserRequestsEntryInTournamentResponse() : PacketTournament( PacketType_Tournament, TournamentType_UserRequestsEntryInTournamentResponse ),
      result( PacketErrorReport::ErrorType_Purchase_Success ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   UuidString  tournamentUuid; 
   int         result;
};

///////////////////////////////////////////////////////////////

class PacketTournament_PurchaseTournamentEntry : public PacketTournament
{
public:
   PacketTournament_PurchaseTournamentEntry() : PacketTournament( PacketType_Tournament, TournamentType_PurchaseTournamentEntry ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   UuidString  userUuid;
   string      uniqueTransactionId; // fill this in with some unique value that you need

   SerializedVector< PurchaseServerDebitItem > itemsToSpend;
};

///////////////////////////////////////////////////////////////////

class PacketTournament_PurchaseTournamentEntryResponse : public PacketTournament
{
public:
   PacketTournament_PurchaseTournamentEntryResponse() : PacketTournament( PacketType_Tournament, TournamentType_PurchaseTournamentEntryResponse ),
               result( PacketErrorReport::ErrorType_Purchase_Success ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string      uniqueTransactionId; /// returned to the transaction server
   int         result;
};

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

class PacketTournament_PurchaseTournamentEntryRefund : public PacketTournament
{
public:
   PacketTournament_PurchaseTournamentEntryRefund() : PacketTournament( PacketType_Tournament, TournamentType_PurchaseTournamentEntryRefund ){  }
   
   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   UuidString  userUuid;
   string      uniqueTransactionId; // fill this in with some unique value that you need

   SerializedVector< PurchaseServerDebitItem > itemsToRefund;
};

///////////////////////////////////////////////////////////////////

class PacketTournament_PurchaseTournamentEntryRefundResponse : public PacketTournament
{
public:
   PacketTournament_PurchaseTournamentEntryRefundResponse() : PacketTournament( PacketType_Tournament, TournamentType_PurchaseTournamentEntryRefundResponse ),
               result( PacketErrorReport::ErrorType_Purchase_Success ){  }
   
   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string      uniqueTransactionId; /// returned to the transaction server
   int         result;
 
};

///////////////////////////////////////////////////////////////////
