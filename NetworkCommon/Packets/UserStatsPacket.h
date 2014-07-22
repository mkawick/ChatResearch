// UserStatsPacket.h
#pragma once

#include "BasePacket.h"
#include "PurchasePacket.h"


///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

class PacketUserStats : public BasePacket 
{
public:
   enum UserStatsType
   {
      UserStatsType_Base,
      UserStatsType_TestUserStats,

      UserStatsType_RequestListOfUserStats, // from client only
      UserStatsType_RequestListOfUserStatsResponse,

      UserStatsType_RecordUserStats, // from other servers only
      UserStatsType_RecordUserStatsResponse,
   };
public:
   PacketUserStats( int packet_type = PacketType_UserStats, int packet_sub_type = UserStatsType_Base ) : BasePacket( packet_type, packet_sub_type ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;
};

///////////////////////////////////////////////////////////////////

class PacketUserStats_TestUserStats : public PacketUserStats
{
public:
   PacketUserStats_TestUserStats() : PacketUserStats( PacketType_UserStats, UserStatsType_TestUserStats ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   UuidString              userUuid;
};


///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////


class PacketUserStats_RequestListOfUserStats : public PacketUserStats
{
public:
   PacketUserStats_RequestListOfUserStats() : PacketUserStats( PacketType_UserStats, UserStatsType_RequestListOfUserStats ), whichGame( 0 ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   UuidString              userUuid;
   U8                      whichGame;
};


///////////////////////////////////////////////////////////////

class PacketUserStats_RequestListOfUserStatsResponse : public PacketUserStats
{
public:
   PacketUserStats_RequestListOfUserStatsResponse() : PacketUserStats( PacketType_UserStats, UserStatsType_RequestListOfUserStatsResponse ), whichGame( 0 ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   UuidString                          userUuid;
   U8                                  whichGame;
   SerializedKeyValueVector< string >  stats;
};


///////////////////////////////////////////////////////////////

class PacketUserStats_RecordUserStats : public PacketUserStats
{
public:
   PacketUserStats_RecordUserStats() : PacketUserStats( PacketType_UserStats, UserStatsType_RecordUserStats ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   UuidString                          userUuid;
   U8                                  whichGame;
   SerializedKeyValueVector< string >  stats;
};


///////////////////////////////////////////////////////////////

class PacketUserStats_RecordUserStatsResponse : public PacketUserStats
{
public:
   PacketUserStats_RecordUserStatsResponse() : PacketUserStats( PacketType_UserStats, UserStatsType_RecordUserStatsResponse ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   UuidString              userUuid;
   bool                    success;
};


///////////////////////////////////////////////////////////////
