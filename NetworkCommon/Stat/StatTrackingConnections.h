// StatTrackingConnections.h
#pragma once

#include "../Packets/BasePacket.h"
#include "../Packets/AnalyticsPacket.h"

#include "../NetworkOut/Fruitadens.h"
#include "../ChainedArchitecture/ChainedInterface.h"

///////////////////////////////////////////////////////////////

class StatTrackingConnections
{
public:
   enum StatTracking
   {
      StatTracking_BadPacketVersion,
      StatTracking_UserBlocked,
      StatTracking_ForcedDisconnect,
      StatTracking_UserLoginSuccess,
      StatTracking_UserLogoutSuccess,
      StatTracking_UserAverageTimeOnline,
      StatTracking_UserTotalTimeOnline,
      StatTracking_GamePacketsSentToGame,
      StatTracking_GamePacketsSentToClient,
      StatTracking_NumUsersOnline,
      StatTracking_UserTotalCount,
      StatTracking_SuccessfulLogins,
      StatTracking_PurchaseMade,
      StatTracking_UserRelogin,

      StatTracking_UsersPerGame,
      StatTracking_UsersAverageTimePerGame,
      StatTracking_UsersPlayedMultipleGames,
      StatTracking_UsersLostConnection,
      StatTracking_UniquesUsersPerDay,

      StatTracking_ChatNumberOfChatsSentPerHour,
      StatTracking_ChatNumberOfChannelChatsSentPerHour,
      StatTracking_ChatNumberOfP2PChatsSentPerHour,
      StatTracking_ChatNumberOfChatChannelChangesPerHour,
      
      StatTracking_ContactNumberSearchesForUserPerformed,
      StatTracking_ContactNumInvitesSentPerDay,
      StatTracking_ContactAcceptedInvitesPerDay,
      StatTracking_ContactRejectedInvitesPerDay,
   };

   StatTrackingConnections();

   Threading::Mutex     m_statMutex;
   deque< PacketAnalytics* > m_stats;
   time_t         m_timeoutSendStatServerStats;
   static const U32 timeoutSendStatServerStats = 60;

   //void           TrackStats( StatTracking stat, float value );
   void           TrackCountStats( const string& serverName, int ServerId, StatTracking stat, float value, int sub_category );
   void           TrackStats( const string& serverName, int ServerId, const string& statName, U16 stat, float value, PacketAnalytics::StatType type );

   template <typename type >
   void           SendStatsToStatServer( std::list<type>&, const string& serverName, U32 serverId, ServerType m_serverType );

private:
   void           SendStatsToStatServer( Fruitadens*, const string& serverName, U32 serverId, ServerType m_serverType );// meant to be invoked periodically
   void           GetStatString( U16 statId, PacketAnalytics* packet );
};


//-----------------------------------------------------------------------------------------

template <typename type >
void     StatTrackingConnections::SendStatsToStatServer( std::list< type >& listOfOutputs, const string& serverName, U32 serverId, ServerType serverType )
{
   typedef std::list< type > ListType;
   typedef typename ListType::iterator iterator;

   time_t currentTime;
   time( &currentTime );

   if( difftime( currentTime, m_timeoutSendStatServerStats ) >= timeoutSendStatServerStats ) 
   {
      m_timeoutSendStatServerStats = currentTime;
      Fruitadens* fruity = NULL;

      iterator itOutput = listOfOutputs.begin();
      while( itOutput != listOfOutputs.end() )
      {
         IChainedInterface* outputPtr = (*itOutput).m_interface;
         fruity = static_cast< Fruitadens* >( outputPtr );
         if( fruity->GetConnectedServerType() == ServerType_Analytics )
         {
            break;
         }
         else
         {
            fruity = NULL;
         }
         itOutput++;
      }

      StatTrackingConnections::SendStatsToStatServer( fruity, serverName, serverId, serverType );
   }
}

///////////////////////////////////////////////////////////////