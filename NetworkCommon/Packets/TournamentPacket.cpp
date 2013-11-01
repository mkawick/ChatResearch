// TournamentPacket.cpp

#include "TournamentPacket.h"


///////////////////////////////////////////////////////////////


bool  TournamentInfo::SerializeIn( const U8* data, int& bufferOffset )
{
   Serialize::In( data, bufferOffset, tournamentName );
   Serialize::In( data, bufferOffset, tournamentUuid );
   Serialize::In( data, bufferOffset, beginDate );
   Serialize::In( data, bufferOffset, endDate );
   Serialize::In( data, bufferOffset, timePerRound );
   Serialize::In( data, bufferOffset, timeUnitsPerRound );

   return true;
}

bool  TournamentInfo::SerializeOut( U8* data, int& bufferOffset ) const
{
   Serialize::Out( data, bufferOffset, tournamentName );
   Serialize::Out( data, bufferOffset, tournamentUuid );
   Serialize::Out( data, bufferOffset, beginDate );
   Serialize::Out( data, bufferOffset, endDate );
   Serialize::Out( data, bufferOffset, timePerRound );
   Serialize::Out( data, bufferOffset, timeUnitsPerRound );

   return true;
}


///////////////////////////////////////////////////////////////

bool  PacketTournament::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );

   return true;
}

bool  PacketTournament::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketTournament_RequestListOfTournaments::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketTournament::SerializeIn( data, bufferOffset );

   return true;
}

bool  PacketTournament_RequestListOfTournaments::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketTournament::SerializeOut( data, bufferOffset );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketTournament_RequestListOfTournamentsResponse::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketTournament::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, tournaments );

   return true;
}

bool  PacketTournament_RequestListOfTournamentsResponse::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketTournament::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, tournaments );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketTournament_UserRequestsEntryInTournament::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketTournament::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, tournamentUuid );

   return true;
}

bool  PacketTournament_UserRequestsEntryInTournament::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketTournament::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, tournamentUuid );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketTournament_UserRequestsEntryInTournamentResponse::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketTournament::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, tournamentUuid );
   Serialize::In( data, bufferOffset, result );

   return true;
}

bool  PacketTournament_UserRequestsEntryInTournamentResponse::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketTournament::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, tournamentUuid );
   Serialize::Out( data, bufferOffset, result );

   return true;
}

///////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////

bool  PacketTournament_EnterUserInTournament::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketTournament::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, userUuid );
   Serialize::In( data, bufferOffset, tournamentUuid );
   Serialize::In( data, bufferOffset, uniqueTransactionId );
   Serialize::In( data, bufferOffset, gameId );
   Serialize::In( data, bufferOffset, numTicketsRequired );

   return true;
}

bool  PacketTournament_EnterUserInTournament::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketTournament::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, userUuid );
   Serialize::Out( data, bufferOffset, tournamentUuid );
   Serialize::Out( data, bufferOffset, uniqueTransactionId );
   Serialize::Out( data, bufferOffset, gameId );
   Serialize::Out( data, bufferOffset, numTicketsRequired );

   return true;
}

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

bool  PacketTournament_EnterUserInTournamentResponse::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketTournament::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, uniqueTransactionId );
   Serialize::In( data, bufferOffset, result );

   return true;
}

bool  PacketTournament_EnterUserInTournamentResponse::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketTournament::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, uniqueTransactionId );
   Serialize::Out( data, bufferOffset, result );

   return true;
}

///////////////////////////////////////////////////////////////