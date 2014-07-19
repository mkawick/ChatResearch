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
   Serialize::In( data, bufferOffset, userUuid );

   return true;
}

bool  PacketUserStats_RecordUserStats::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketUserStats::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, userUuid );
   Serialize::Out( data, bufferOffset, userUuid );

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
