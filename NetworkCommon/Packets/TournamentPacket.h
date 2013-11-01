// TournamentPacket.h
#pragma once

#include "BasePacket.h"


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

   string   tournamentName;
   string   tournamentUuid;
   string   beginDate;
   string   endDate;
   int      timePerRound;
   int      timeUnitsPerRound;
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

      TournamentType_EnterUserInTournament, // server to server request
      TournamentType_EnterUserInTournamentResponse,
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
   string   senderUuid;
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

///////////////////////////////////////////////////////////////////

class PacketTournament_UserRequestsEntryInTournament : public PacketTournament
{
public:
   PacketTournament_UserRequestsEntryInTournament() : PacketTournament( PacketType_Tournament, TournamentType_UserRequestsEntryInTournament ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string      tournamentUuid;
};

///////////////////////////////////////////////////////////////////

class PacketTournament_UserRequestsEntryInTournamentResponse : public PacketTournament
{
public:
   enum  TournamentPurchase_Result
   {
      TournamentPurchase_Result_Success,
      TournamentPurchase_Result_NotEnoughMoney,
      TournamentPurchase_Result_TransactionFailed,
      TournamentPurchase_Result_TooManyPlayers,
      TournamentPurchase_Result_DateIsWrong,
      TournamentPurchase_Result_RequirementsNotMet,
      TournamentPurchase_Result_TournamentClosed
   };
public:
   PacketTournament_UserRequestsEntryInTournamentResponse() : PacketTournament( PacketType_Tournament, TournamentType_UserRequestsEntryInTournamentResponse ),
               result( TournamentPurchase_Result_Success ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string      tournamentUuid; 
   int         result;
};
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////

class PacketTournament_EnterUserInTournament : public PacketTournament
{
public:
   PacketTournament_EnterUserInTournament() : PacketTournament( PacketType_Tournament, TournamentType_EnterUserInTournament ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string      userUuid;
   string      tournamentUuid;
   string      uniqueTransactionId; // fill this in with some unique value that you need
   int         gameId;
   int         numTicketsRequired;
};

///////////////////////////////////////////////////////////////////

class PacketTournament_EnterUserInTournamentResponse : public PacketTournament
{
public:
   enum  TournamentPurchase_Result
   {
      TournamentPurchase_Result_Success,
      TournamentPurchase_Result_NotEnoughMoney,
      TournamentPurchase_Result_TransactionFailed,
   };
public:
   PacketTournament_EnterUserInTournamentResponse() : PacketTournament( PacketType_Tournament, TournamentType_EnterUserInTournament ),
               result( TournamentPurchase_Result_Success ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string      uniqueTransactionId; /// returned to the transaction server
   int         result;
};

///////////////////////////////////////////////////////////////////
