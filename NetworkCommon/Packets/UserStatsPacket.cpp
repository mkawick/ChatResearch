// UserStatsPacket.cpp

#include "UserStatsPacket.h"

///////////////////////////////////////////////////////////////

bool  PacketUserStats::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );

   return true;
}

bool  PacketUserStats::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );

   return true;
}


///////////////////////////////////////////////////////////////

bool  PacketUserStats_TestUserStats::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketUserStats::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, userUuid, minorVersion );

   return true;
}

bool  PacketUserStats_TestUserStats::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketUserStats::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, userUuid, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketUserStats_RequestListOfUserStats::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketUserStats::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, whichGame, minorVersion );
   Serialize::In( data, bufferOffset, userUuid, minorVersion );

   return true;
}

bool  PacketUserStats_RequestListOfUserStats::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketUserStats::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, whichGame, minorVersion );
   Serialize::Out( data, bufferOffset, userUuid, minorVersion );

   return true;
}


///////////////////////////////////////////////////////////////

bool  PacketUserStats_RequestListOfUserStatsResponse::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketUserStats::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, userUuid, minorVersion );
   Serialize::In( data, bufferOffset, whichGame, minorVersion );
   Serialize::In( data, bufferOffset, stats, minorVersion );

   return true;
}

bool  PacketUserStats_RequestListOfUserStatsResponse::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketUserStats::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, userUuid, minorVersion );
   Serialize::Out( data, bufferOffset, whichGame, minorVersion );
   Serialize::Out( data, bufferOffset, stats, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

bool  PacketUserStats_RecordUserStats::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketUserStats::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, userUuid, minorVersion );
   Serialize::In( data, bufferOffset, whichGame, minorVersion );
   Serialize::In( data, bufferOffset, stats, minorVersion );

   return true;
}

bool  PacketUserStats_RecordUserStats::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketUserStats::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, userUuid, minorVersion );
   Serialize::Out( data, bufferOffset, whichGame, minorVersion );
   Serialize::Out( data, bufferOffset, stats, minorVersion );

   return true;
}


///////////////////////////////////////////////////////////////

bool  PacketUserStats_RecordUserStatsResponse::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketUserStats::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, userUuid, minorVersion );
   Serialize::In( data, bufferOffset, success, minorVersion );

   return true;
}

bool  PacketUserStats_RecordUserStatsResponse::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketUserStats::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, userUuid, minorVersion );
   Serialize::Out( data, bufferOffset, success, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketUserStats_ReportGameResult::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketUserStats::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, gameType, minorVersion );
   Serialize::In( data, bufferOffset, gameId, minorVersion );
   Serialize::In( data, bufferOffset, playerCount, minorVersion );
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
      Serialize::In( data, bufferOffset, resultOrder[p], minorVersion );
      Serialize::In( data, bufferOffset, playerFactions[p], minorVersion );
   }
   Serialize::In( data, bufferOffset, forfeitFlags, minorVersion );

   return true;
}

bool  PacketUserStats_ReportGameResult::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketUserStats::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, gameType, minorVersion );
   Serialize::Out( data, bufferOffset, gameId, minorVersion );
   Serialize::Out( data, bufferOffset, playerCount, minorVersion );
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
      Serialize::Out( data, bufferOffset, resultOrder[p], minorVersion );
      Serialize::Out( data, bufferOffset, playerFactions[p], minorVersion );
   }
   Serialize::Out( data, bufferOffset, forfeitFlags, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketUserStats_RequestGameFactionStats::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketUserStats::SerializeIn( data, bufferOffset, minorVersion );

   Serialize::In( data, bufferOffset, gameType, minorVersion );
   Serialize::In( data, bufferOffset, userId, minorVersion );

   return true;
}

bool  PacketUserStats_RequestGameFactionStats::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketUserStats::SerializeOut( data, bufferOffset, minorVersion );

   Serialize::Out( data, bufferOffset, gameType, minorVersion );
   Serialize::Out( data, bufferOffset, userId, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketUserStats_RequestGameProfile::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketUserStats::SerializeIn( data, bufferOffset, minorVersion );

   Serialize::In( data, bufferOffset, gameType, minorVersion );
   Serialize::In( data, bufferOffset, profileUserId, minorVersion );
   Serialize::In( data, bufferOffset, requestUserId, minorVersion );

   return true;
}

bool  PacketUserStats_RequestGameProfile::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketUserStats::SerializeOut( data, bufferOffset, minorVersion );

   Serialize::Out( data, bufferOffset, gameType, minorVersion );
   Serialize::Out( data, bufferOffset, profileUserId, minorVersion );
   Serialize::Out( data, bufferOffset, requestUserId, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketUserStats_RequestUserProfileStats::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketUserStats::SerializeIn( data, bufferOffset, minorVersion );

   Serialize::In( data, bufferOffset, gameType, minorVersion );
   Serialize::In( data, bufferOffset, profileUserId, minorVersion );

   return true;
}

bool  PacketUserStats_RequestUserProfileStats::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketUserStats::SerializeOut( data, bufferOffset, minorVersion );

   Serialize::Out( data, bufferOffset, gameType, minorVersion );
   Serialize::Out( data, bufferOffset, profileUserId, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketUserStats_RequestUserProfileStatsResponse::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketUserStats::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, profileUserId, minorVersion );
   Serialize::In( data, bufferOffset, gameType, minorVersion );
   Serialize::In( data, bufferOffset, userProfileStats, minorVersion );

   return true;
}

bool  PacketUserStats_RequestUserProfileStatsResponse::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketUserStats::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, profileUserId, minorVersion );
   Serialize::Out( data, bufferOffset, gameType, minorVersion );
   Serialize::Out( data, bufferOffset, userProfileStats, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////


