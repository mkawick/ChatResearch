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
      UserStatsType_RequestUserProfileStats,
      UserStatsType_RequestUserProfileStatsResponse,
      UserStatsType_RequestGameProfile,
      UserStatsType_RequestGameFactionStats, // from client only

      UserStatsType_RecordUserStats, // from other servers only
      UserStatsType_RecordUserStatsResponse,
      UserStatsType_ReportGameResult,
   };
public:
   PacketUserStats( int packet_type = PacketType_UserStats, int packet_sub_type = UserStatsType_Base ) : BasePacket( packet_type, packet_sub_type ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;
};

///////////////////////////////////////////////////////////////////

class PacketUserStats_TestUserStats : public PacketUserStats
{
public:
   PacketUserStats_TestUserStats() : PacketUserStats( PacketType_UserStats, UserStatsType_TestUserStats ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   UuidString              userUuid;
};


///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////


class PacketUserStats_RequestListOfUserStats : public PacketUserStats
{
public:
   PacketUserStats_RequestListOfUserStats() : PacketUserStats( PacketType_UserStats, UserStatsType_RequestListOfUserStats ), whichGame( 0 ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   UuidString              userUuid;
   U8                      whichGame;
};


///////////////////////////////////////////////////////////////

class PacketUserStats_RequestListOfUserStatsResponse : public PacketUserStats
{
public:
   PacketUserStats_RequestListOfUserStatsResponse() : PacketUserStats( PacketType_UserStats, UserStatsType_RequestListOfUserStatsResponse ), whichGame( 0 ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   UuidString                          userUuid;
   U8                                  whichGame;
   SerializedKeyValueVector< string >  stats;
};


///////////////////////////////////////////////////////////////

class PacketUserStats_RecordUserStats : public PacketUserStats
{
public:
   PacketUserStats_RecordUserStats() : PacketUserStats( PacketType_UserStats, UserStatsType_RecordUserStats ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   UuidString                          userUuid;
   U8                                  whichGame;
   SerializedKeyValueVector< string >  stats;
};


///////////////////////////////////////////////////////////////

class PacketUserStats_RecordUserStatsResponse : public PacketUserStats
{
public:
   PacketUserStats_RecordUserStatsResponse() : PacketUserStats( PacketType_UserStats, UserStatsType_RecordUserStatsResponse ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   UuidString              userUuid;
   bool                    success;
};


///////////////////////////////////////////////////////////////


class PacketUserStats_ReportGameResult : public PacketUserStats
{
public:
   PacketUserStats_ReportGameResult() : PacketUserStats( PacketType_UserStats, UserStatsType_ReportGameResult ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   static const int k_maxPlayerCount = 8;

   int            gameType;
   U32            gameId;
   int            playerCount;
   unsigned int   resultOrder[k_maxPlayerCount];
   unsigned int   playerFactions[k_maxPlayerCount];
   unsigned int   forfeitFlags;
};


///////////////////////////////////////////////////////////////


class PacketUserStats_RequestGameFactionStats : public PacketUserStats
{
public:
   PacketUserStats_RequestGameFactionStats() : PacketUserStats( PacketType_UserStats, UserStatsType_RequestGameFactionStats ) {  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;


   int            gameType;
   unsigned int   userId;     // 0 = global stats
};


///////////////////////////////////////////////////////////////


class PacketUserStats_RequestGameProfile : public PacketUserStats
{
public:
   PacketUserStats_RequestGameProfile() : PacketUserStats( PacketType_UserStats, UserStatsType_RequestGameProfile ) {  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   int            gameType;
   unsigned int   profileUserId;
   unsigned int   requestUserId;
};


///////////////////////////////////////////////////////////////


class PacketUserStats_RequestUserProfileStats : public PacketUserStats
{
public:
   PacketUserStats_RequestUserProfileStats() : PacketUserStats( PacketType_UserStats, UserStatsType_RequestUserProfileStats ) {  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   int            gameType;
   unsigned int   profileUserId;
};


///////////////////////////////////////////////////////////////


class PacketUserStats_RequestUserProfileStatsResponse : public PacketUserStats
{
public:
   PacketUserStats_RequestUserProfileStatsResponse() : PacketUserStats( PacketType_UserStats, UserStatsType_RequestUserProfileStatsResponse ) {  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   unsigned int                        profileUserId;
   int                                 gameType;
   SerializedKeyValueVector< string >  userProfileStats;
};


///////////////////////////////////////////////////////////////



