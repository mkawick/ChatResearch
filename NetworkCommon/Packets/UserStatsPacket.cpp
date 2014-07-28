// UserStatsPacket.cpp

#include "UserStatsPacket.h"

///////////////////////////////////////////////////////////////

bool  PacketUserStats::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );

   return true;
}

bool  PacketUserStats::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );

   return true;
}


///////////////////////////////////////////////////////////////

bool  PacketUserStats_TestUserStats::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketUserStats::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, userUuid );

   return true;
}

bool  PacketUserStats_TestUserStats::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketUserStats::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, userUuid );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketUserStats_RequestListOfUserStats::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketUserStats::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, whichGame );
   Serialize::In( data, bufferOffset, userUuid );

   return true;
}

bool  PacketUserStats_RequestListOfUserStats::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketUserStats::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, whichGame );
   Serialize::Out( data, bufferOffset, userUuid );

   return true;
}


///////////////////////////////////////////////////////////////

bool  PacketUserStats_RequestListOfUserStatsResponse::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketUserStats::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, userUuid );
   Serialize::In( data, bufferOffset, whichGame );
   Serialize::In( data, bufferOffset, stats );

   return true;
}

bool  PacketUserStats_RequestListOfUserStatsResponse::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketUserStats::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, userUuid );
   Serialize::Out( data, bufferOffset, whichGame );
   Serialize::Out( data, bufferOffset, stats );

   return true;
}

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

bool  PacketUserStats_RecordUserStats::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketUserStats::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, userUuid );
   Serialize::In( data, bufferOffset, whichGame );
   Serialize::In( data, bufferOffset, stats );

   return true;
}

bool  PacketUserStats_RecordUserStats::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketUserStats::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, userUuid );
   Serialize::Out( data, bufferOffset, whichGame );
   Serialize::Out( data, bufferOffset, stats );

   return true;
}


///////////////////////////////////////////////////////////////

bool  PacketUserStats_RecordUserStatsResponse::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketUserStats::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, userUuid );
   Serialize::In( data, bufferOffset, success );

   return true;
}

bool  PacketUserStats_RecordUserStatsResponse::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketUserStats::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, userUuid );
   Serialize::Out( data, bufferOffset, success );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketUserStats_ReportGameResult::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketUserStats::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, gameType );
   Serialize::In( data, bufferOffset, gameId );
   Serialize::In( data, bufferOffset, playerCount );
   int players = playerCount;
   if( players < 0 )
   {
      players = 0;
   }
   else if( players > k_maxPlayerCount )
   {
      players = k_maxPlayerCount;
   }
   for( int p = 0; p < players; ++p )
   {
      Serialize::In( data, bufferOffset, resultOrder[p] );
      Serialize::In( data, bufferOffset, playerFactions[p] );
   }
   Serialize::In( data, bufferOffset, forfeitFlags );

   return true;
}

bool  PacketUserStats_ReportGameResult::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketUserStats::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, gameType );
   Serialize::Out( data, bufferOffset, gameId );
   Serialize::Out( data, bufferOffset, playerCount );
   int players = playerCount;
   if( players < 0 )
   {
      players = 0;
   }
   else if( players > k_maxPlayerCount )
   {
      players = k_maxPlayerCount;
   }
   for( int p = 0; p < players; ++p )
   {
      Serialize::Out( data, bufferOffset, resultOrder[p] );
      Serialize::Out( data, bufferOffset, playerFactions[p] );
   }
   Serialize::Out( data, bufferOffset, forfeitFlags );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketUserStats_RequestGameFactionStats::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketUserStats::SerializeIn( data, bufferOffset );

   Serialize::In( data, bufferOffset, gameType );
   Serialize::In( data, bufferOffset, userId );

   return true;
}

bool  PacketUserStats_RequestGameFactionStats::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketUserStats::SerializeOut( data, bufferOffset );

   Serialize::Out( data, bufferOffset, gameType );
   Serialize::Out( data, bufferOffset, userId );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketUserStats_RequestGameProfile::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketUserStats::SerializeIn( data, bufferOffset );

   Serialize::In( data, bufferOffset, gameType );
   Serialize::In( data, bufferOffset, profileUserId );
   Serialize::In( data, bufferOffset, requestUserId );

   return true;
}

bool  PacketUserStats_RequestGameProfile::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketUserStats::SerializeOut( data, bufferOffset );

   Serialize::Out( data, bufferOffset, gameType );
   Serialize::Out( data, bufferOffset, profileUserId );
   Serialize::Out( data, bufferOffset, requestUserId );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketUserStats_RequestUserProfileStats::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketUserStats::SerializeIn( data, bufferOffset );

   Serialize::In( data, bufferOffset, gameType );
   Serialize::In( data, bufferOffset, profileUserId );

   return true;
}

bool  PacketUserStats_RequestUserProfileStats::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketUserStats::SerializeOut( data, bufferOffset );

   Serialize::Out( data, bufferOffset, gameType );
   Serialize::Out( data, bufferOffset, profileUserId );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketUserStats_RequestUserProfileStatsResponse::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketUserStats::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, profileUserId );
   Serialize::In( data, bufferOffset, gameType );
   Serialize::In( data, bufferOffset, userProfileStats );

   return true;
}

bool  PacketUserStats_RequestUserProfileStatsResponse::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketUserStats::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, profileUserId );
   Serialize::Out( data, bufferOffset, gameType );
   Serialize::Out( data, bufferOffset, userProfileStats );

   return true;
}

///////////////////////////////////////////////////////////////


