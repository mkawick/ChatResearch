
#if defined(_WIN32)

// Define _WINSOCKAPI_ so windows.h will not include old WinSock header.
#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_
#endif

#include <windows.h>
#include <winsock2.h>

#endif

#include <boost/lexical_cast.hpp>

#include "server_stats_plugins.h"
#include "server_rating_glicko2.h"

#include "UserStatsMainThread.h"
#include "../NetworkCommon/Packets/UserStatsPacket.h"

#include <cstdlib>
#include <cstdio>

#include <mysql/mysql.h>

#if PLATFORM == PLATFORM_WINDOWS
#pragma warning ( disable: 4996 )
#endif


static const char *Waterdeep_GameName = "waterdeep";

static MYSQL *s_pMysqlStats = NULL;




static bool RunSqlStatsQuery( const char *fmt, ...)
{
   char query[256];

   va_list args;
   va_start(args, fmt);

   vsnprintf(query, sizeof(query), fmt, args);

   va_end(args);

   int ret = mysql_query(s_pMysqlStats, query);
   if (ret != 0)
   {
      printf("Error %s (code %d) executing DB query: \"%s\"\n", mysql_error(s_pMysqlStats), ret, query);
      printf("        error: %s\n", mysql_error(s_pMysqlStats) );
      //LogMessage(LOG_PRIO_ERR, "Error %s (code %d) executing DB query: \"%s\"\n", mysql_error(s_pMysqlStats), ret, query);
      //LogMessage(LOG_PRIO_ERR, "        error: %s\n", mysql_error(s_pMysqlStats) );
      return false;
   }

   return true;
}



static void Waterdeep_OnReportGameResult(unsigned int gameID, int playerCount, unsigned int *pResults, unsigned int *pFactions)
{
   /*
#if USE_MYSQL_DATABASE
   struct st_mysql *mysqlUser = GetMysqlUserConnection();
   const char *gameName = GameDatabaseName(GAME_SELECT_SUMMONWAR);
#endif
*/
   const char *gameName = Waterdeep_GameName;

   /*
   summonwar::CWorld *pWorld = dynamic_cast<summonwar::CWorld*>( gameData->m_pGameWorld );
   bool bForfeit = false;
   int winnerIndex = -1;
   for( int p = 0; p < gameData->m_GamePlayerCount; ++p )
   {
      if( gameData->m_PlayerData[p].userState == E_USERSTATE_FORFEIT )
      {
         bForfeit = true;
      }
      else
      {
         summonwar::CPlayer *pPlayer = pWorld->GetPlayerByIndex(gameData->m_PlayerData[p].userID);
         if( !pPlayer->GetUnitList().empty() )
         {
            summonwar::CUnit *pFirstUnit = pPlayer->GetUnitList().front();
            if( pFirstUnit != NULL && pFirstUnit->IsTypeSummoner() )
            {
               winnerIndex = p;
            }
         }
      }
   }
   */


   /*
   if( winnerIndex >= 0 )
   {
      for( int r = 0; r < gameData->m_GamePlayerCount; ++r )
      {
         CServerUser *pPlayerUser = gameData->m_PlayerData[r].pServerUser;

         if( gameData->m_PlayerData[r].userState == E_USERSTATE_FORFEIT )
         {
            pPlayerUser->m_Losses[resultIndex]++;
#if USE_MYSQL_DATABASE
            RunSqlUserQuery("UPDATE stats_%s SET losses_%dp=\'%d\' WHERE user_id=\'%u\'",
                  gameName, gameData->m_GamePlayerCount, pPlayerUser->m_Losses[resultIndex], pPlayerUser->GetUserID());
#endif
         }
         else
         {
            if( r == winnerIndex )
            {
               if( bForfeit )
               {
                  if( pPlayerUser->m_CompletedGames < 10
                     || ((100*pPlayerUser->m_WinsByForfeit)/pPlayerUser->m_CompletedGames) < 20 )
                  {
                     pPlayerUser->m_Wins[resultIndex]++;
                     pPlayerUser->m_WinsByForfeit++;
                     pPlayerUser->m_CompletedGames++;
#if USE_MYSQL_DATABASE
                     RunSqlUserQuery("UPDATE stats_%s SET completed_games=\'%d\', wins_%dp=\'%d\', wins_by_forfeit=\'%d\' WHERE user_id=\'%u\'",
                        gameName, pPlayerUser->m_CompletedGames, gameData->m_GamePlayerCount, pPlayerUser->m_Wins[resultIndex],
                        pPlayerUser->m_WinsByForfeit, pPlayerUser->GetUserID());
#endif
                  }

               }
               else
               {
                  pPlayerUser->m_Wins[resultIndex]++;
                  pPlayerUser->m_CompletedGames++;
#if USE_MYSQL_DATABASE
                  RunSqlUserQuery("UPDATE stats_%s SET completed_games=\'%d\', wins_%dp=\'%d\' WHERE user_id=\'%u\'",
                     gameName, pPlayerUser->m_CompletedGames, gameData->m_GamePlayerCount, pPlayerUser->m_Wins[resultIndex], pPlayerUser->GetUserID());
#endif
               }
            }
            else
            {
               pPlayerUser->m_Losses[resultIndex]++;
               pPlayerUser->m_CompletedGames++;
#if USE_MYSQL_DATABASE
               RunSqlUserQuery("UPDATE stats_%s SET completed_games=\'%d\', losses_%dp=\'%d\' WHERE user_id=\'%u\'",
                  gameName, pPlayerUser->m_CompletedGames, gameData->m_GamePlayerCount, pPlayerUser->m_Losses[resultIndex], pPlayerUser->GetUserID());
#endif
            }
         }
      }
      */

   for( int r = 0; r < playerCount; ++r )
   {
      /*
      CServerUser *pPlayerUser = gameData->m_PlayerData[r].pServerUser;

      if( gameData->m_PlayerData[r].userState == E_USERSTATE_FORFEIT )
      {
         pPlayerUser->m_Losses[resultIndex]++;
#if USE_MYSQL_DATABASE
         RunSqlUserQuery("UPDATE stats_%s SET losses_%dp=\'%d\' WHERE user_id=\'%u\'",
               gameName, gameData->m_GamePlayerCount, pPlayerUser->m_Losses[resultIndex], pPlayerUser->GetUserID());
#endif
      }
      else
      */
      {
         if( r == 0 )
         {
            /*
            if( bForfeit )
            {
               if( pPlayerUser->m_CompletedGames < 10
                  || ((100*pPlayerUser->m_WinsByForfeit)/pPlayerUser->m_CompletedGames) < 20 )
               {
                  pPlayerUser->m_Wins[resultIndex]++;
                  pPlayerUser->m_WinsByForfeit++;
                  pPlayerUser->m_CompletedGames++;
#if USE_MYSQL_DATABASE
                  RunSqlUserQuery("UPDATE stats_%s SET completed_games=\'%d\', wins_%dp=\'%d\', wins_by_forfeit=\'%d\' WHERE user_id=\'%u\'",
                     gameName, pPlayerUser->m_CompletedGames, gameData->m_GamePlayerCount, pPlayerUser->m_Wins[resultIndex],
                     pPlayerUser->m_WinsByForfeit, pPlayerUser->GetUserID());
#endif
               }

            }
            else
            */
            {
//               pPlayerUser->m_Wins[resultIndex]++;
//               pPlayerUser->m_CompletedGames++;
               RunSqlStatsQuery("UPDATE stats_%s SET completed_games=completed_games+1, wins_%dp=wins_%dp+1 WHERE user_id=\'%u\'",
                  gameName, playerCount, playerCount, pResults[r] );
               if( mysql_affected_rows(s_pMysqlStats) <= 0 )
               {
                  if(!RunSqlStatsQuery("INSERT INTO stats_%s (user_id, completed_games, wins_%dp) VALUES (%d,1,1)",
                        gameName, playerCount, pResults[r]))
                  {
                     printf("Could not update faction stats for user %ud", pResults[r]);
                     //LogMessage(LOG_PRIO_ERR, "Could not update faction stats for user %ud", winUser);
                  }
               }
//#if USE_MYSQL_DATABASE
//               RunSqlUserQuery("UPDATE stats_%s SET completed_games=\'%d\', wins_%dp=\'%d\' WHERE user_id=\'%u\'",
//                  gameName, pPlayerUser->m_CompletedGames, gameData->m_GamePlayerCount, pPlayerUser->m_Wins[resultIndex], pPlayerUser->GetUserID());
//#endif
            }
         }
         else
         {
//            pPlayerUser->m_Losses[resultIndex]++;
//            pPlayerUser->m_CompletedGames++;
               RunSqlStatsQuery("UPDATE stats_%s SET completed_games=completed_games+1, losses_%dp=losses_%dp+1 WHERE user_id=\'%u\'",
                  gameName, playerCount, playerCount, pResults[r] );
               if( mysql_affected_rows(s_pMysqlStats) <= 0 )
               {
                  if(!RunSqlStatsQuery("INSERT INTO stats_%s (user_id, completed_games, losses_%dp) VALUES (%d,1,1)",
                        gameName, playerCount, pResults[r]))
                  {
                     printf("Could not update faction stats for user %ud",  pResults[r]);
                     //LogMessage(LOG_PRIO_ERR, "Could not update faction stats for user %ud", winUser);
                  }
               }
//#if USE_MYSQL_DATABASE
//            RunSqlUserQuery("UPDATE stats_%s SET completed_games=\'%d\', losses_%dp=\'%d\' WHERE user_id=\'%u\'",
//               gameName, pPlayerUser->m_CompletedGames, gameData->m_GamePlayerCount, pPlayerUser->m_Losses[resultIndex], pPlayerUser->GetUserID());
//#endif
         }
      }
   }


//#endif
}




/*
static void OnRequestGameProfile_SummonWar( UserStatsMainThread *pUserStats, U32 connectionId,
                                             unsigned int profileUserId, unsigned int requestUserId )
{
   //CServerUser *pProfileUser = FindServerUserById( pMsg->userID );
   //if( pProfileUser == NULL )
   //{
   //   LogMessage(LOG_PRIO_ERR, "User for player profile doesn't exist! (%d)\n", pMsg->userID);
   //   return false;
   //}

   CNetMsgUpdatePlayerProfile msgUpdatePlayerProfile;
   msgUpdatePlayerProfile.playerData.userID = profileUserId;
   msgUpdatePlayerProfile.playerData.userState = 0;
   msgUpdatePlayerProfile.playerData.avatarIndex = 1;
   //const char *gamekitId = pProfileUser->GetGameKitId();
   //if (gamekitId)
   //{
   //   STRCPY( msgUpdatePlayerProfile.playerData.username.name,
   //      sizeof(msgUpdatePlayerProfile.playerData.username.name), gamekitId);
   //   msgUpdatePlayerProfile.playerData.username.type = GAME_PLAYER_NAME_GAMEKIT;
   //}
   //else
   {
      //const char *username = pProfileUser->GetName();
      //STRCPY( msgUpdatePlayerProfile.playerData.username.displayName,
      //   sizeof(msgUpdatePlayerProfile.playerData.username.displayName), username );
      memset( msgUpdatePlayerProfile.playerData.username.displayName, 0,
         sizeof(msgUpdatePlayerProfile.playerData.username.displayName) );
      memset( msgUpdatePlayerProfile.playerData.username.gameKitHash, 0,
         sizeof(msgUpdatePlayerProfile.playerData.username.gameKitHash) );
      msgUpdatePlayerProfile.playerData.username.type = GAME_PLAYER_NAME_BASIC;
   }

   //unsigned int inProgressGames = 0;
   //std::vector<CServerChannel*>::const_iterator game_itt = pProfileUser->GetChannelList().begin();
   //for( ; game_itt != pProfileUser->GetChannelList().end(); ++game_itt )
   //{
   //   CServerGameData *pGame = dynamic_cast<CServerGameData*>( *game_itt );
   //   inProgressGames += (pGame != NULL && pGame->m_GameState == E_GAMESTATE_PLAYING);
   //}
   //msgUpdatePlayerProfile.inProgressGames = inProgressGames;
   msgUpdatePlayerProfile.inProgressGames = 0;

   msgUpdatePlayerProfile.completedGames = 0;
   msgUpdatePlayerProfile.forfeits = 0;
   memset( &msgUpdatePlayerProfile.wins, 0, sizeof(msgUpdatePlayerProfile.wins) );
   memset( &msgUpdatePlayerProfile.losses, 0, sizeof(msgUpdatePlayerProfile.losses) );

   const char *gameName = SummonWar_GameName;

   char query[256];
   sprintf(query, "SELECT * FROM stats_%s WHERE user_id=\'%u\'", gameName, profileUserId);

   mysql_query(s_pMysqlStats, query);
   MYSQL_RES *res = mysql_store_result(s_pMysqlStats);
   if (res)
   {
      MYSQL_ROW row = mysql_fetch_row(res);
      if (row)
      {
         sscanf(row[1], "%u", &msgUpdatePlayerProfile.completedGames);
         sscanf(row[2], "%u", &msgUpdatePlayerProfile.wins[0]);
         sscanf(row[3], "%u", &msgUpdatePlayerProfile.losses[0]);
         sscanf(row[4], "%u", &msgUpdatePlayerProfile.wins[1]);
         sscanf(row[5], "%u", &msgUpdatePlayerProfile.losses[1]);
         sscanf(row[6], "%u", &msgUpdatePlayerProfile.wins[2]);
         sscanf(row[7], "%u", &msgUpdatePlayerProfile.losses[2]);
         sscanf(row[8], "%u", &msgUpdatePlayerProfile.forfeits);
         //sscanf(row[9], "%u", &pUser->m_WinsByForfeit);

         //LogMessage(LOG_PRIO_INFO, "Retrieved stats for user %d\n", pUser->GetUserID());
         //LogMessage(LOG_PRIO_INFO, "   (%d, %d, %d, %d, %d, %d, %d, %d)\n", pUser->m_CompletedGames, pUser->m_Wins[0],
         //   pUser->m_Losses[0], pUser->m_Wins[1], pUser->m_Losses[1], pUser->m_Wins[2], pUser->m_Losses[2], pUser->m_Forfeits );
      }
      mysql_free_result(res);
   }


//#if USE_MYSQL_DATABASE
   msgUpdatePlayerProfile.wins_vs = 0;
   msgUpdatePlayerProfile.losses_vs = 0;

   if( requestUserId != 0 && profileUserId != requestUserId )
   {
      //unsigned int connectionID = pServerConnection->GetConnectedUser()->GetUserID();
      //unsigned int profileID = pProfileUser->GetUserID();
      //if( connectionID != profileID )
      {
         //struct st_mysql *mysqlUser = GetMysqlUserConnection();
         //const char *gameName = GameDatabaseName(GAME_SELECT_SUMMONWAR);

         unsigned int connectionWins = 0, profileWins = 0;

         unsigned int lowID, highID;
         if( requestUserId > profileUserId )
         {
            lowID = profileUserId;
            highID = requestUserId;
         }
         else
         {
            lowID = requestUserId;
            highID = profileUserId;
         }

         bool bExisting = false;

         // update head to head stats
         sprintf(query, "SELECT user_wins, opp_wins FROM stats_%s_vs WHERE user_id=\'%u\' AND opp_id=\'%u\'",
                     gameName, lowID, highID);

         mysql_query(s_pMysqlStats, query);
         MYSQL_RES *res = mysql_store_result(s_pMysqlStats);
         if( res )
         {
            MYSQL_ROW row = mysql_fetch_row(res);
            if( row )
            {
               int converted = 0;

               converted += sscanf(row[0], "%u", &connectionWins);
               converted += sscanf(row[1], "%u", &profileWins);
               if( converted == 2 )
               {
                  if( lowID == requestUserId )
                  {
                     msgUpdatePlayerProfile.wins_vs = connectionWins;
                     msgUpdatePlayerProfile.losses_vs = profileWins;
                  }
                  else
                  {
                     msgUpdatePlayerProfile.wins_vs = profileWins;
                     msgUpdatePlayerProfile.losses_vs = connectionWins;
                  }
               }
            }
            mysql_free_result(res);
         }
      }
   }
//#endif

   //pServerConnection->SendTo( (const char*)&msgUpdatePlayerProfile, sizeof(CNetMsgUpdatePlayerProfile) );

   pUserStats->SendGameData( connectionId, sizeof(msgUpdatePlayerProfile), reinterpret_cast<const U8*>(&msgUpdatePlayerProfile) );

   //return true;
}
*/


bool OnRequestUserProfileStats_Waterdeep(UserStatsMainThread *pUserStats, U32 connectionId, U32 gatewayId, unsigned int profileUserId )
{
   PacketUserStats_RequestUserProfileStatsResponse* response = new PacketUserStats_RequestUserProfileStatsResponse;
   response->profileUserId = profileUserId;
   response->gameType = GameProductId_WATERDEEP;

   const char *gameName = Waterdeep_GameName;

   char query[256];
   sprintf(query, "SELECT * FROM stats_%s WHERE user_id=\'%u\'", gameName, profileUserId);

   mysql_query(s_pMysqlStats, query);
   MYSQL_RES *res = mysql_store_result(s_pMysqlStats);
   if (res)
   {
      MYSQL_ROW row = mysql_fetch_row(res);
      if (row)
      {
         if( row[1] != NULL && row[2] != NULL && row[3] != NULL )
         {
            RatingGlicko2 glicko_rating;
            float rating = 0.0f, deviation = 0.0f, volatility = 0.0f;
            sscanf(row[1], "%f", &rating);
            sscanf(row[2], "%f", &deviation);
            sscanf(row[3], "%f", &volatility);

            glicko_rating.SetRating( rating );
            glicko_rating.SetDeviation( deviation );
            glicko_rating.SetVolatility( volatility );

            char rating_buffer[16];
            sprintf( rating_buffer, "%u", glicko_rating.GetDisplayRating() );
            response->userProfileStats.insert( "rating", rating_buffer );
         }

         if( row[4] != NULL )
         {
            response->userProfileStats.insert( "completed", row[4] );
         }
         if( row[5] != NULL )
         {
            response->userProfileStats.insert( "wins_2p", row[5] );
         }
         if( row[6] != NULL )
         {
            response->userProfileStats.insert( "losses_2p", row[6] );
         }
         if( row[7] != NULL )
         {
            response->userProfileStats.insert( "wins_3p", row[7] );
         }
         if( row[8] != NULL )
         {
            response->userProfileStats.insert( "losses_3p", row[8] );
         }
         if( row[9] != NULL )
         {
            response->userProfileStats.insert( "wins_4p", row[9] );
         }
         if( row[10] != NULL )
         {
            response->userProfileStats.insert( "losses_4p", row[10] );
         }
         if( row[11] != NULL )
         {
            response->userProfileStats.insert( "wins_5p", row[11] );
         }
         if( row[12] != NULL )
         {
            response->userProfileStats.insert( "losses_5p", row[12] );
         }
         if( row[13] != NULL )
         {
            response->userProfileStats.insert( "wins_6p", row[13] );
         }
         if( row[14] != NULL )
         {
            response->userProfileStats.insert( "losses_6p", row[14] );
         }
         if( row[15] != NULL )
         {
            response->userProfileStats.insert( "forfeits", row[15] );
         }

      }
      mysql_free_result(res);
   }

   pUserStats->SendPacketToGateway( response, connectionId, gatewayId );
   return true;
}






void Waterdeep_StatsPluginInit( StatsPlugin *pluginData, MYSQL *mysqlStats )
{
   s_pMysqlStats = mysqlStats;

   pluginData->pGameName = Waterdeep_GameName;

   //pluginData->onReportGameResult = SummonWar_OnReportGameResult;
   pluginData->onReportGameResult = NULL;
   pluginData->onReportUserForfeit = NULL;

   //pluginData->onRequestPlayerFactionStats = OnRequestPlayerFactionStats_SummonWar;
   pluginData->onRequestPlayerFactionStats = NULL;
   //pluginData->onRequestGlobalFactionStats = OnRequestGlobalFactionStats_SummonWar;
   pluginData->onRequestGlobalFactionStats = NULL;
   //pluginData->onRequestGameProfile = OnRequestGameProfile_SummonWar;
   pluginData->onRequestGameProfile = NULL;
   pluginData->onRequestUserProfileStats = OnRequestUserProfileStats_Waterdeep;


}

