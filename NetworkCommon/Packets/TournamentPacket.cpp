// TournamentPacket.cpp

#include "TournamentPacket.h"


///////////////////////////////////////////////////////////////


bool  TournamentInfo::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   Serialize::In( data, bufferOffset, tournamentName, minorVersion );
   Serialize::In( data, bufferOffset, tournamentUuid, minorVersion );
   Serialize::In( data, bufferOffset, beginDate, minorVersion );
   Serialize::In( data, bufferOffset, endDate, minorVersion );
   Serialize::In( data, bufferOffset, timePerRound, minorVersion );
   Serialize::In( data, bufferOffset, timeUnitsPerRound, minorVersion );

   return true;
}

bool  TournamentInfo::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   Serialize::Out( data, bufferOffset, tournamentName, minorVersion );
   Serialize::Out( data, bufferOffset, tournamentUuid, minorVersion );
   Serialize::Out( data, bufferOffset, beginDate, minorVersion );
   Serialize::Out( data, bufferOffset, endDate, minorVersion );
   Serialize::Out( data, bufferOffset, timePerRound, minorVersion );
   Serialize::Out( data, bufferOffset, timeUnitsPerRound, minorVersion );

   return true;
}

void  TournamentInfo::Clear()
{
   tournamentName.clear();
   tournamentUuid.clear();
   beginDate.clear();
   endDate.clear();
   timePerRound = 0;
   timeUnitsPerRound = 0;
}

///////////////////////////////////////////////////////////////

bool  PacketTournament::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );

   return true;
}

bool  PacketTournament::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );

   return true;
}


///////////////////////////////////////////////////////////////

bool  PacketTournament_TestNotification::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, message, minorVersion );
   Serialize::In( data, bufferOffset, senderName, minorVersion );
   Serialize::In( data, bufferOffset, senderUuid, minorVersion );
   Serialize::In( data, bufferOffset, type, minorVersion );

   return true;
}

bool  PacketTournament_TestNotification::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, message, minorVersion );
   Serialize::Out( data, bufferOffset, senderName, minorVersion );
   Serialize::Out( data, bufferOffset, senderUuid, minorVersion );
   Serialize::Out( data, bufferOffset, type, minorVersion );

   return true;
}


///////////////////////////////////////////////////////////////

bool  PacketTournament_RequestListOfTournaments::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketTournament::SerializeIn( data, bufferOffset, minorVersion );

   return true;
}

bool  PacketTournament_RequestListOfTournaments::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketTournament::SerializeOut( data, bufferOffset, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketTournament_RequestListOfTournamentsResponse::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketTournament::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, tournaments, minorVersion );

   return true;
}

bool  PacketTournament_RequestListOfTournamentsResponse::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketTournament::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, tournaments, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketTournament_UserRequestsEntryInTournament::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketTournament::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, tournamentUuid, minorVersion );
   Serialize::In( data, bufferOffset, itemsToSpend, minorVersion );
   //itemsToSpend.SerializeIn( data, bufferOffset, minorVersion );

   return true;
}

bool  PacketTournament_UserRequestsEntryInTournament::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketTournament::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, tournamentUuid, minorVersion );
   Serialize::Out( data, bufferOffset, itemsToSpend, minorVersion );
   //itemsToSpend.SerializeOut( data, bufferOffset, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketTournament_UserRequestsEntryInTournamentResponse::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketTournament::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, tournamentUuid, minorVersion );
   Serialize::In( data, bufferOffset, result, minorVersion );

   return true;
}

bool  PacketTournament_UserRequestsEntryInTournamentResponse::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketTournament::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, tournamentUuid, minorVersion );
   Serialize::Out( data, bufferOffset, result, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////

bool  PacketTournament_PurchaseTournamentEntry::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketTournament::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, userConnectionId, minorVersion );
   Serialize::In( data, bufferOffset, userGatewayId, minorVersion );
   Serialize::In( data, bufferOffset, userUuid, minorVersion );
   Serialize::In( data, bufferOffset, uniqueTransactionId, minorVersion );
   //itemsToSpend.SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, itemsToSpend, minorVersion );

   return true;
}

bool  PacketTournament_PurchaseTournamentEntry::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketTournament::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, userConnectionId, minorVersion );
   Serialize::Out( data, bufferOffset, userGatewayId, minorVersion );
   Serialize::Out( data, bufferOffset, userUuid, minorVersion );
   Serialize::Out( data, bufferOffset, uniqueTransactionId, minorVersion );
   //itemsToSpend.SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, itemsToSpend, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

bool  PacketTournament_PurchaseTournamentEntryResponse::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketTournament::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, uniqueTransactionId, minorVersion );
   Serialize::In( data, bufferOffset, result, minorVersion );

   return true;
}

bool  PacketTournament_PurchaseTournamentEntryResponse::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketTournament::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, uniqueTransactionId, minorVersion );
   Serialize::Out( data, bufferOffset, result, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////

bool  PacketTournament_PurchaseTournamentEntryRefund::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketTournament::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, userUuid, minorVersion );
   Serialize::In( data, bufferOffset, uniqueTransactionId, minorVersion );
   //itemsToRefund.SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, itemsToRefund, minorVersion );

   return true;
}

bool  PacketTournament_PurchaseTournamentEntryRefund::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketTournament::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, userUuid, minorVersion );
   Serialize::Out( data, bufferOffset, uniqueTransactionId, minorVersion );
   //itemsToRefund.SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, itemsToRefund, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

bool  PacketTournament_PurchaseTournamentEntryRefundResponse::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketTournament::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, uniqueTransactionId, minorVersion );
   Serialize::In( data, bufferOffset, result, minorVersion );

   return true;
}

bool  PacketTournament_PurchaseTournamentEntryRefundResponse::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketTournament::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, uniqueTransactionId, minorVersion );
   Serialize::Out( data, bufferOffset, result, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////