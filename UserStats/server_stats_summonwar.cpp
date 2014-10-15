
//#include "build_options.h"

//#include "server_gamedata.h"
//#include "server_database.h"
//#include "server_organizedplay.h"
//#include "server_log.h"
//#include "server_user.h"
//#include "server_notify.h"

//#include "summonwar_world.h"
//#include "summonwar_player.h"
//#include "summonwar_unit.h"
//#include "summonwar_database.h"
//#include "summonwar_startposition.h"
//#include "summonwar_lua_glue.h"
//#include "summonwar_rules_lua.h"
//#include "summonwar_rules_game.h"
//#include "summonwar_rules_selectionhints.h"

//#include "game_state_options.h"

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

#include "UserStatsMainThread.h"
#include "../NetworkCommon/Packets/UserStatsPacket.h"

#include <cstdlib>
#include <cstdio>

#include <mysql/mysql.h>

#if PLATFORM == PLATFORM_WINDOWS
#pragma warning ( disable: 4996 )
#endif


#define SUMMONWAR_MAX_FACTIONS   16

//static bool s_ProcessGameResults = true;

// Win count for each faction vs each other faction
static unsigned int s_FactionWins[SUMMONWAR_MAX_FACTIONS][SUMMONWAR_MAX_FACTIONS];

// Total number of plays of each faction
static unsigned int s_FactionGames[SUMMONWAR_MAX_FACTIONS];

static const char *SummonWar_GameName = "summonwar";

static MYSQL *s_pMysqlStats = NULL;


///////////////////////////////////////////////////////////////////////////////////////////////////////
////
////  TEMP!
////  copied from netmsg.h
////

#if defined(_MSC_VER)
typedef unsigned long long uint64_t;
#else
#include <stdint.h>
#endif

const unsigned int NETMSG_PLAYER_PROFILE        = 0x86868686;
const unsigned int NETMSG_PLAYER_FACTION        = 0x89898989;
const unsigned int NETMSG_GLOBAL_FACTION        = 0x8A8A8A8A;

#define GAME_PLAYER_NAME_BASIC   0

static const int MAX_USERNAME_LEN = 30;      // Bytes, not characters
static const unsigned int MAX_PLAYER_PARAMETER_BUFFER_SIZE = 32;

struct GamePlayerName {
   char displayName[MAX_USERNAME_LEN + 1];
   char gameKitHash[sizeof(uint64_t)];
   char type;
};

struct GamePlayerData {
   unsigned int   userID;
   unsigned short userState;
   unsigned short avatarIndex;
   //unsigned short userRating;
   GamePlayerName username;
   unsigned int playerParameterSize;
   unsigned char playerParametersBuffer[MAX_PLAYER_PARAMETER_BUFFER_SIZE];
};

struct CNetMsg {
protected:
	CNetMsg( unsigned int mType, unsigned int chan ) : msgType( mType ), channelID( chan ) {}
   
public:
	unsigned int	msgType;
	unsigned int	channelID;
};

struct CNetMsgUpdatePlayerProfile : public CNetMsg {
public:
	CNetMsgUpdatePlayerProfile() : CNetMsg( NETMSG_PLAYER_PROFILE, 0 ) {}
   
   GamePlayerData playerData;
   
   unsigned int   inProgressGames;
   unsigned int   completedGames;
   unsigned int   forfeits;
   unsigned int   wins[3];
   unsigned int   losses[3];

   unsigned int   wins_vs;
   unsigned int   losses_vs;
};

struct CNetMsgPlayerFactionStats : public CNetMsg {
public:
	CNetMsgPlayerFactionStats() : CNetMsg( NETMSG_PLAYER_FACTION, 0 ) {}
   
   static const int k_maxFactionCount = 16;
   
	unsigned int userID;
   
   unsigned short factionWins[k_maxFactionCount][k_maxFactionCount];
   unsigned short factionLosses[k_maxFactionCount][k_maxFactionCount];

};

struct CNetMsgGlobalFactionStats : public CNetMsg {
public:
	CNetMsgGlobalFactionStats() : CNetMsg( NETMSG_GLOBAL_FACTION, 0 ) {}
   
   static const int k_maxFactionCount = 16;
   
   unsigned int factionWins[k_maxFactionCount][k_maxFactionCount];
};

////
////  TEMP!
////
///////////////////////////////////////////////////////////////////////////////////////////////////////




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



/*
static unsigned int SummonWar_OnRetrieveRulesVersion()
{
   return summonwar::kRulesVersion;
}
*/

/*
static void SummonWar_OnStartGame(CServerGameData *pGameData)
{
   assert( sizeof(summonwar::GameParameters) < sizeof(pGameData->m_GameParametersBuffer) );
   const summonwar::GameParameters *pGameParameters = (const summonwar::GameParameters*)&pGameData->m_GameParametersBuffer;

   summonwar::CWorld *pNewWorld = new summonwar::CWorld( pGameParameters, pGameData->m_RulesVersion,
                                                         pGameData->m_RandomSeed, false, false );
   assert( pNewWorld != NULL );

   for( int p = 0; p < pGameData->m_PlayerCount; ++p )
   {
      CServerGamePlayerData *pPlayerData = &pGameData->m_PlayerData[p];
      assert( pPlayerData->userID > 0 );
      assert( pPlayerData->pServerUser != NULL );

      assert( sizeof(summonwar::PlayerParameters) <= sizeof(pPlayerData->playerParameterBuffer) );
      const summonwar::PlayerParameters *pPlayerParameters = (const summonwar::PlayerParameters*)&pPlayerData->playerParameterBuffer;

	   pNewWorld->AddPlayer( pPlayerData->userID, pPlayerData->pServerUser->GetName(), pPlayerParameters );
   }

   pGameData->AssignStateProcessGame( summonwarrules::CreateStateProcessGame(pNewWorld) );
   pGameData->m_pGameWorld = pNewWorld;
}
*/

/*
static void SummonWar_OnGameUpdate(CServerGameData *pGameData, bool &bGameComplete, bool &bForfeitPlayerTurn)
{
   bool bSendUpdateMsg = false;

#if USE_MYSQL_DATABASE
   const char *gameName = GameDatabaseName(pGameData->m_GameType);
#endif

   summonwar::CWorld *pWorld = dynamic_cast<summonwar::CWorld*>( pGameData->m_pGameWorld );
   if( pWorld != NULL )
   {
      if( pWorld->IsGameComplete() )
      {
         if( !s_ProcessGameResults )
         {
            return;
         }

         bGameComplete = true;

         if( pGameData->m_GameState != E_GAMESTATE_COMPLETED )
         {
            time_t currentTime = time(NULL);

            pGameData->m_GameState = E_GAMESTATE_COMPLETED;
            pGameData->m_CompletionTime = currentTime;
#if USE_MYSQL_DATABASE
            tm *pTimeVal = localtime( &currentTime );
            RunSqlGameQuery("UPDATE %s_games SET game_state=\'%d\', "
               "completion_time=\'%04d%02d%02d%02d%02d%02d\' WHERE game_id=\'%u\'",
               gameName, E_GAMESTATE_COMPLETED, 
               pTimeVal->tm_year+1900, pTimeVal->tm_mon+1, pTimeVal->tm_mday, pTimeVal->tm_hour, pTimeVal->tm_min, pTimeVal->tm_sec,
               pGameData->GetChannelID());
#endif
         }
         
         int currentDecisionPlayerIndex = 0;
         for( int p = 0; p < pGameData->m_GamePlayerCount; ++p )
         {
            if( pGameData->m_PlayerData[p].userState == E_USERSTATE_PLAYING )
            {
               if( !pGameData->m_PlayerData[p].pServerUser->GetServerConnection() )
               {
                  NotifyUser(pGameData->m_PlayerData[p].pServerUser->GetUserID(), GAME_SELECT_SUMMONWAR,
                     pGameData->GetChannelID(), gnTurn,
                     pGameData->m_PlayerData[1-p].pServerUser->GetName());
               }

               if( pGameData->m_PlayerData[p].gamePositionIndex < pGameData->m_MoveCount )
               {
                  currentDecisionPlayerIndex = pGameData->m_PlayerData[p].userID;
               }
            }
         }

         int resultIndex = pGameData->m_GamePlayerCount-2;
         if( resultIndex == 0 )
         {
            s_GamePlugins[pGameData->m_GameType].onGameFinished(pGameData, resultIndex);
         }


         bSendUpdateMsg = true;
         pGameData->m_CurrentRoundNumber = pWorld->GetRoundNumber();
         //pGameData->m_HonorPoolCount = 0;
         //pGameData->m_CurrentTurnPlayerIndex = 0;
         //pGameData->m_LastDecisionPlayerIndex = 0;
         //pGameData->m_LastDecisionPlayerTimer = 0;
         pGameData->m_CurrentDecisionPlayerIndex = currentDecisionPlayerIndex;


         if( pGameData->m_CurrentDecisionPlayerIndex != 0 &&
             pGameData->m_CurrentDecisionPlayerIndex != pGameData->m_LastDecisionPlayerIndex &&
             pGameData->m_NextMoveIndex >= pGameData->m_MoveCount )
         {
            time_t currentTime = time(NULL);

            if( pGameData->m_LastDecisionPlayerIndex != 0 && pGameData->m_StartPlayerTimer > 0 )
            {
               for( int p = 0; p < pGameData->m_GamePlayerCount; ++p )
			      {
		            if( pGameData->m_LastDecisionPlayerIndex == pGameData->m_PlayerData[p].userID )
                  {
                     unsigned int decisionTime = (unsigned int)(currentTime - pGameData->m_LastDecisionStartTime);
                     //LogMessage(LOG_PRIO_INFO, "         %s (user=%d) took %d seconds to play\n",
                     //   pGameData->m_PlayerData[p].pServerUser->GetName(), pGameData->m_PlayerData[p].userID, decisionTime);
                     //LogMessage(LOG_PRIO_INFO, "             Start  @ %s", ctime(&pGameData->m_LastDecisionStartTime) );
                     //LogMessage(LOG_PRIO_INFO, "             Finish @ %s", ctime(&currentTime) );

                     if( pGameData->m_PlayerData[p].userState != E_USERSTATE_FORFEIT )
                     {
                        unsigned int updatePlayerTimer = pGameData->m_PlayerData[p].playerTimer;
                        if( decisionTime > updatePlayerTimer )
                        {
                           updatePlayerTimer = 1;
                        }
                        else
                        {
                           updatePlayerTimer -= decisionTime;
                        } 
                        //LogMessage(LOG_PRIO_INFO, "            --- Player %d has %d seconds remaining\n", m_LastDecisionPlayerIndex, updatePlayerTimer);
                        pGameData->m_PlayerData[p].playerTimer = updatePlayerTimer;
//#if USE_MYSQL_DATABASE
//                     RunSqlGameQuery("UPDATE %s_players SET player_timer=\'%u\' WHERE user_id=\'%u\' AND game_id=\'%u\'",
//                        gameName, updatePlayerTimer, pGameData->m_PlayerData[p].userID, pGameData->GetChannelID());
//#endif
                     }
                     break;
                  }
               }
            }

            bSendUpdateMsg = true;
            if( pGameData->m_LastDecisionPlayerIndex != 0 )
            {
               pGameData->m_LastDecisionStartTime = currentTime;
               //LogMessage(LOG_PRIO_INFO, "CServerGameData::Update(%d) LastDecisionTime = %s\n",
               //   pGameData->GetChannelID(), ctime(&pGameData->m_LastDecisionStartTime) );
//#if USE_MYSQL_DATABASE
//               tm *pTimeVal = localtime( &currentTime );
//               RunSqlGameQuery("UPDATE %s_games SET decision_time='%04d%02d%02d%02d%02d%02d' WHERE game_id=\'%u\'", gameName,
//                  pTimeVal->tm_year+1900,pTimeVal->tm_mon+1,pTimeVal->tm_mday,
//                  pTimeVal->tm_hour,pTimeVal->tm_min,pTimeVal->tm_sec, pGameData->GetChannelID());
//#endif
            }
            pGameData->m_LastDecisionPlayerIndex = pGameData->m_CurrentDecisionPlayerIndex;

            if( pGameData->m_LastDecisionPlayerIndex != 0 && pGameData->m_StartPlayerTimer > 0  )
            {
               for( int p = 0; p < pGameData->m_GamePlayerCount; ++p )
			      {
		            if( pGameData->m_LastDecisionPlayerIndex == pGameData->m_PlayerData[p].userID )
                  {
                     //LogMessage(LOG_PRIO_INFO, "         Player %d has %d seconds\n", m_LastDecisionPlayerIndex, m_PlayerData[p].playerTimer);
                     pGameData->m_LastDecisionPlayerTimer = pGameData->m_PlayerData[p].playerTimer;
                     break;
                  }
               }
            }
         }
      }
      else
      {
         for( int p = 0; p < pGameData->m_GamePlayerCount; ++p )
			{
		      if( pGameData->m_CurrentDecisionPlayerIndex == pGameData->m_PlayerData[p].userID )
				{
					if( pGameData->m_PlayerData[p].userState == E_USERSTATE_FORFEIT )
               {
                  bForfeitPlayerTurn = true;
               }
					break;
				}
			}

         if( pGameData->m_CurrentRoundNumber != pWorld->GetRoundNumber() )
         {
            bSendUpdateMsg = true;
		      if( pGameData->m_CurrentRoundNumber == 0 && pWorld->GetRoundNumber() > 0 )
		      {
			      memset( pGameData->m_PlayerTurnOrder, 0, sizeof(pGameData->m_PlayerTurnOrder) );
			      for( unsigned int p = 0; p < pWorld->GetPlayerCount(); ++p )
			      {
				      summonwar::CPlayer *pPlayer = pWorld->GetPlayer( p );
				      for( int i = 0; i < pGameData->m_GamePlayerCount; ++i )
				      {
					      if( pPlayer->GetPlayerIndex() == pGameData->m_PlayerData[i].userID )
					      {
						      pGameData->m_PlayerTurnOrder[p] = i;
						      break;
					      }
				      }
			      }
		      }
            pGameData->m_CurrentRoundNumber = pWorld->GetRoundNumber();
         }

         summonwar::CPlayer *pTurnPlayer = pWorld->GetCurrentTurnPlayer();
         if( pTurnPlayer == NULL )
         {
            if( pGameData->m_CurrentTurnPlayerIndex != 0 )
            {
               bSendUpdateMsg = true;
               pGameData->m_CurrentTurnPlayerIndex = 0;
            }
         }
         else if( pGameData->m_CurrentTurnPlayerIndex != pTurnPlayer->GetPlayerIndex() )
         {
            bSendUpdateMsg = true;
            pGameData->m_CurrentTurnPlayerIndex = pTurnPlayer->GetPlayerIndex();
         }

         if( pGameData->m_CurrentDecisionPlayerIndex != 0 &&
             pGameData->m_CurrentDecisionPlayerIndex != pGameData->m_LastDecisionPlayerIndex &&
             pGameData->m_NextMoveIndex >= pGameData->m_MoveCount)
         {
            time_t currentTime = time(NULL);

            if( pGameData->m_LastDecisionPlayerIndex != 0 && pGameData->m_StartPlayerTimer > 0 )
            {
               for( int p = 0; p < pGameData->m_GamePlayerCount; ++p )
			      {
		            if( pGameData->m_LastDecisionPlayerIndex == pGameData->m_PlayerData[p].userID )
                  {
                     unsigned int decisionTime = (unsigned int)(currentTime - pGameData->m_LastDecisionStartTime);
                     //LogMessage(LOG_PRIO_INFO, "         %s (user=%d) took %d seconds to play\n",
                     //   pGameData->m_PlayerData[p].pServerUser->GetName(),
                     //   pGameData->m_PlayerData[p].userID, decisionTime);
                     //LogMessage(LOG_PRIO_INFO, "             Start  @ %s", ctime(&pGameData->m_LastDecisionStartTime) );
                     //LogMessage(LOG_PRIO_INFO, "             Finish @ %s", ctime(&currentTime) );
                     if( pGameData->m_PlayerData[p].userState != E_USERSTATE_FORFEIT )
                     {
                        unsigned int updatePlayerTimer = pGameData->m_PlayerData[p].playerTimer;
                        if( decisionTime > updatePlayerTimer )
                        {
                           updatePlayerTimer = 1;
                        }
                        else
                        {
                           updatePlayerTimer -= decisionTime;
                        } 
                        //LogMessage(LOG_PRIO_INFO, "            --- %s (user=%d) has %d seconds remaining\n",
                        //   pGameData->m_PlayerData[p].pServerUser->GetName(),
                        //   pGameData->m_PlayerData[p].userID, updatePlayerTimer);
                        pGameData->m_PlayerData[p].playerTimer = updatePlayerTimer;
#if USE_MYSQL_DATABASE
                        RunSqlGameQuery("UPDATE %s_players SET player_timer=\'%u\' WHERE user_id=\'%u\' AND game_id=\'%u\'",
                           gameName, updatePlayerTimer, pGameData->m_PlayerData[p].userID, pGameData->GetChannelID());
#endif
                     }
                     break;
                  }
               }
            }

            bSendUpdateMsg = true;
            if( pGameData->m_LastDecisionPlayerIndex != 0 )
            {
               pGameData->m_LastDecisionStartTime = currentTime;
               //LogMessage(LOG_PRIO_INFO, "CServerGameData::Update(%d) LastDecisionTime = %s\n",
               //   pGameData->GetChannelID(), ctime(&pGameData->m_LastDecisionStartTime) );
#if USE_MYSQL_DATABASE
               tm *pTimeVal = localtime( &currentTime );
               RunSqlGameQuery("UPDATE %s_games SET decision_time='%04d%02d%02d%02d%02d%02d' WHERE game_id=\'%u\'", gameName,
                  pTimeVal->tm_year+1900,pTimeVal->tm_mon+1,pTimeVal->tm_mday,
                  pTimeVal->tm_hour,pTimeVal->tm_min,pTimeVal->tm_sec, pGameData->GetChannelID());
#endif
            }
            pGameData->m_LastDecisionPlayerIndex = pGameData->m_CurrentDecisionPlayerIndex;

            if( pGameData->m_LastDecisionPlayerIndex != 0 && pGameData->m_StartPlayerTimer > 0  )
            {
               for( int p = 0; p < pGameData->m_GamePlayerCount; ++p )
			      {
		            if( pGameData->m_LastDecisionPlayerIndex == pGameData->m_PlayerData[p].userID )
                  {
                     unsigned int timeRemaining = pGameData->m_PlayerData[p].playerTimer;
                     unsigned int elapsedTime = (unsigned int)(currentTime - pGameData->m_LastDecisionStartTime);
                     timeRemaining -= elapsedTime;

                     //LogMessage(LOG_PRIO_INFO, "         %s (user=%d) has %d seconds to play\n",
                     //   pGameData->m_PlayerData[p].pServerUser->GetName(),
                     //   pGameData->m_PlayerData[p].userID, timeRemaining);
                     pGameData->m_LastDecisionPlayerTimer = pGameData->m_PlayerData[p].playerTimer;
                     break;
                  }
               }
            }

            if( !bForfeitPlayerTurn )
            {
               CServerUser *pNotifyUser = FindServerUserById(pGameData->m_CurrentDecisionPlayerIndex, FIND_USER_CACHED_ONLY);
               if (pNotifyUser && !pNotifyUser->IsConnected())
               {
                  for( int p = 0; p < pGameData->m_GamePlayerCount; ++p )
			         {
                     if( pGameData->m_PlayerData[p].userID != pGameData->m_CurrentDecisionPlayerIndex )
                     {
                        NotifyUser(pGameData->m_CurrentDecisionPlayerIndex, pGameData->m_GameType,
                                    pGameData->GetChannelID(), gnTurn,
                                    pGameData->m_PlayerData[p].pServerUser->GetName());
                        break;
                     }
                  }
               }
            }
         }
      }

      if( bSendUpdateMsg )
      {
	      CNetMsgUpdateGameState msgUpdateGameState( pGameData->GetChannelID() );
	      msgUpdateGameState.roundNumber = pGameData->m_CurrentRoundNumber;
	      msgUpdateGameState.turnPlayerIndex = pGameData->m_CurrentTurnPlayerIndex;
	      msgUpdateGameState.decisionPlayerIndex = pGameData->m_CurrentDecisionPlayerIndex;
	      memcpy( msgUpdateGameState.turnOrder, pGameData->m_PlayerTurnOrder, sizeof(msgUpdateGameState.turnOrder) );

         for( int p = 0; p < pGameData->m_GamePlayerCount; ++p )
			{
		      if( pGameData->m_CurrentDecisionPlayerIndex == pGameData->m_PlayerData[p].userID )
            {
               unsigned int elapsedTime = (unsigned int)(time(NULL) - pGameData->m_LastDecisionStartTime);
               if( pGameData->m_PlayerData[p].playerTimer > elapsedTime )
               {
                  msgUpdateGameState.playerTimers[p] = pGameData->m_PlayerData[p].playerTimer - elapsedTime;
               }
               else
               {
                  msgUpdateGameState.playerTimers[p] = 1;
               }
            }
            else
            {
               msgUpdateGameState.playerTimers[p] = pGameData->m_PlayerData[p].playerTimer;
            }
         }

	      std::vector<CChannelConnection>::iterator itt = pGameData->m_ServerConnectionList.begin();
	      for( ; itt != pGameData->m_ServerConnectionList.end(); ++itt )
	      {
		      itt->m_pChannelUser->SendTo( (const char*)&msgUpdateGameState, sizeof(CNetMsgUpdateGameState) );
	      }
      }

   }
}
*/

/*
static void SummonWar_OnHandleForfeitUserDecision(CServerGameData *pGameData, int forfeitPlayerIndex)
{
   const CGameStateOptions *pGameState = OBJECT_CAST<const CGameStateOptions*>( pGameData->GetCurrentActiveState() );
   int optionCount = pGameState->GetOptionCount();
   int optionIndex = optionCount-1;
   const CUserOption *pSelectOption = pGameState->GetOption( optionIndex );
   if( pSelectOption->GetSelectionHint() != summonwarrules::HINT_END_PHASE )
   {
      optionIndex = 0;
      pSelectOption = pGameState->GetOption( optionIndex );
   }
   //char buffer[128];
   //pSelectOption->DisplayOption( buffer, 128 );
   pGameData->ReceiveMove( pGameData->m_PlayerData[forfeitPlayerIndex].pServerUser,
                        pGameData->m_NextMoveIndex, pGameData->m_CurrentDecisionPlayerIndex,
                        optionIndex, pSelectOption->GetSelectionHint(), pSelectOption->GetSelectionID(), 0 );
}
*/


static void SummonWar_OnReportGameResult(unsigned int gameID, int playerCount, unsigned int *pResults, unsigned int *pFactions)
{
   /*
#if USE_MYSQL_DATABASE
   struct st_mysql *mysqlUser = GetMysqlUserConnection();
   const char *gameName = GameDatabaseName(GAME_SELECT_SUMMONWAR);
#endif
*/
   const char *gameName = SummonWar_GameName;

   printf( "  Receive ReportGameResult for %s game %d:", gameName, gameID );
   for( int p = 0; p < playerCount; ++p )
   {
      printf( "  %d(%d)", pResults[p], pFactions[p] );
   }
   printf( "\n" );
   
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


   // update global stats
   unsigned int winUser = pResults[0];
   unsigned int lossUser = pResults[1];
   int winFaction = pFactions[0];
   int lossFaction = pFactions[1];

   s_FactionWins[winFaction][lossFaction]++;
   s_FactionGames[winFaction]++;
   s_FactionGames[lossFaction]++;

//#if USE_MYSQL_DATABASE
   RunSqlStatsQuery("UPDATE stats_%s_faction_wins SET vs_faction_%d=vs_faction_%d+1 WHERE user_id=%d AND faction_id=%d",
      gameName, lossFaction, lossFaction, winUser, winFaction);
   if (mysql_affected_rows(s_pMysqlStats) <= 0)
   {
      if (!RunSqlStatsQuery("INSERT INTO stats_%s_faction_wins (user_id, faction_id, vs_faction_%d) VALUES (%d,%d,1)",
            gameName, lossFaction, winUser, winFaction))
      {
         //LogMessage(LOG_PRIO_ERR, "Could not update faction stats for user %ud", winUser);
      }
   }

   RunSqlStatsQuery("UPDATE stats_%s_faction_losses SET vs_faction_%d=vs_faction_%d+1 WHERE user_id=%d AND faction_id=%d",
      gameName, winFaction, winFaction, lossUser, lossFaction);
   if (mysql_affected_rows(s_pMysqlStats) <= 0)
   {
      if (!RunSqlStatsQuery("INSERT INTO stats_%s_faction_losses (user_id, faction_id, vs_faction_%d) VALUES (%d,%d,1)",
            gameName, winFaction, lossUser, lossFaction))
      {
         //LogMessage(LOG_PRIO_ERR, "Could not update faction stats for user %ud", lossUser);
      }
   }
//#endif

//#if USE_MYSQL_DATABASE
      unsigned int userWins = 0, oppWins = 0;

      unsigned int lowUser, highUser;
      if( winUser > lossUser )
      {
         lowUser = lossUser;
         highUser = winUser;
      }
      else
      {
         lowUser = winUser;
         highUser = lossUser;
      }

      bool bExisting = false;

      // update head to head stats
      char query[256];
      sprintf(query, "SELECT user_wins, opp_wins FROM stats_%s_vs WHERE user_id=\'%u\' AND opp_id=\'%u\'",
                  gameName, lowUser, highUser);

      mysql_query(s_pMysqlStats, query);
      MYSQL_RES *res = mysql_store_result(s_pMysqlStats);
      if( res )
      {
         MYSQL_ROW row = mysql_fetch_row(res);
         if( row )
         {
            int converted = 0;
   
            converted += sscanf(row[0], "%u", &userWins);
            converted += sscanf(row[1], "%u", &oppWins);
            if( converted == 2 )
            {
               //LogMessage(LOG_PRIO_DEBUG, "    Found VS stats (%d %d) %d-%d\n", lowUser, highUser, userWins, oppWins);
               if( lowUser == winUser )
               {
                  userWins++;
               }
               else
               {
                  oppWins++;
               }

               //LogMessage(LOG_PRIO_DEBUG, "    Update VS stats (%d %d) %d-%d\n", lowUser, highUser, userWins, oppWins);
               RunSqlStatsQuery("UPDATE stats_%s_vs SET user_wins=\'%u\', opp_wins=\'%u\' WHERE user_id=\'%u\' AND opp_id=\'%u\'",
                  gameName, userWins, oppWins, lowUser, highUser);
               bExisting = true;
            }
         }

         mysql_free_result(res);
     }

      if( !bExisting )
      {
         if( lowUser == winUser )
         {
            userWins++;
         }
         else
         {
            oppWins++;
         }

         //LogMessage(LOG_PRIO_DEBUG, "    Create VS stats (%d %d) %d-%d\n", lowUser, highUser, userWins, oppWins);
         RunSqlStatsQuery("INSERT INTO stats_%s_vs (user_id, opp_id, user_wins, opp_wins) "
                     "VALUES(\'%u\', \'%u\', \'%u\', \'%u\')",
                     gameName, lowUser, highUser, userWins, oppWins);
      }
//#endif

      //if( gameData->m_OrganizedPlayEventID != 0 )
      //{
      //   HandleOrganizedPlayMatchupResult( gameData->m_GameType, gameData->m_OrganizedPlayEventID,
      //                                    gameData->m_OrganizedPlayEventMatchupIndex,
      //                                    gameData->GetChannelID(),
      //                                    gameData->m_PlayerData[winnerIndex].userID );

      //}

}

static void SummonWar_OnReportUserForfeit(unsigned int userID, unsigned int gameID)
{
   /*
#if USE_MYSQL_DATABASE
   struct st_mysql *mysqlUser = GetMysqlUserConnection();
   const char *gameName = GameDatabaseName(GAME_SELECT_SUMMONWAR);
#endif
*/
   const char *gameName = SummonWar_GameName;

   printf( "  Receive ReportUserForfeit for %s user %d\n", gameName, userID );

   RunSqlStatsQuery("UPDATE stats_%s SET forfeits=forfeits+1 WHERE user_id=\'%u\'", gameName, userID );
}


static void OnRequestPlayerFactionStats_SummonWar(UserStatsMainThread *pUserStats, U32 connectionId, U32 gatewayId, unsigned int userId)
{
   //U32 userId = pServerConnection->GetUserInfo().id;
   //if( pMsg && pMsg->userID )
   //   userId = pMsg->userID;

	//CServerUser *pProfileUser = FindServerUserById( userId );
 //  if( pProfileUser == NULL )
 //  {
 //     LogMessage(LOG_PRIO_ERR, "User for player profile doesn't exist! (%d)\n", userId);
 //     return false;
 //  }

	CNetMsgPlayerFactionStats msgPlayerFactionStats;
	msgPlayerFactionStats.userID = userId;

   memset( &msgPlayerFactionStats.factionWins, 0, sizeof(msgPlayerFactionStats.factionWins) );
   memset( &msgPlayerFactionStats.factionLosses, 0, sizeof(msgPlayerFactionStats.factionLosses) );

   unsigned int total_wins = 0;
   unsigned int total_losses = 0;
//#if USE_MYSQL_DATABASE

   MYSQL_RES *res;
   MYSQL_ROW row;

   char query[256];

   //const char *gameName = GameDatabaseName(GAME_SELECT_SUMMONWAR);
   const char *gameName = SummonWar_GameName;

   sprintf(query, "SELECT * FROM stats_%s_faction_wins WHERE user_id=\'%u\'", gameName, userId);
   mysql_query(s_pMysqlStats, query);

   res = mysql_store_result(s_pMysqlStats);
   if( res )
   {
      if(mysql_num_fields(res) == SUMMONWAR_MAX_FACTIONS+2)
      {
         for (row = mysql_fetch_row(res); row; row = mysql_fetch_row(res))
         {
            unsigned int factionId;
            sscanf(row[1], "%u", &factionId);

            if( factionId < 0 || factionId >= SUMMONWAR_MAX_FACTIONS )
            {
               continue;
            }

            // store the wins for this faction
            for (int j = 0; j < SUMMONWAR_MAX_FACTIONS; ++j)
            {
               if( row[2+j] != NULL )
               {
                 unsigned int wins = 0;
                 wins = strtol(row[2+j], NULL, 10);
                 msgPlayerFactionStats.factionWins[factionId][j] = wins;
                 total_wins += wins;
               }
            }
         }
      }
      mysql_free_result(res);
   }

   sprintf(query, "SELECT * FROM stats_%s_faction_losses WHERE user_id=\'%u\'", gameName, userId);
   mysql_query(s_pMysqlStats, query);

   res = mysql_store_result(s_pMysqlStats);
   if( res )
   {
      if(mysql_num_fields(res) == SUMMONWAR_MAX_FACTIONS+2)
      {
         for (row = mysql_fetch_row(res); row; row = mysql_fetch_row(res))
         {
            unsigned int factionId;
            sscanf(row[1], "%u", &factionId);

            if( factionId < 0 || factionId >= SUMMONWAR_MAX_FACTIONS )
            {
               continue;
            }

            // store the wins for this faction
            for (int j = 0; j < SUMMONWAR_MAX_FACTIONS; ++j)
            {
               if( row[2+j] != NULL )
               {
                 unsigned int losses = 0;
                 losses = strtol(row[2+j], NULL, 10);
                 msgPlayerFactionStats.factionLosses[factionId][j] = losses;
                 total_losses += losses;
               }
            }
         }
      }
      mysql_free_result(res);
   }
//#endif

   //LogMessage(LOG_PRIO_DEBUG, "    Retrieve Player Faction Stats (userID=%d) (%d %d) -> (%d %d)\n",
   //         pProfileUser->GetUserID(), pProfileUser->m_Wins[0], pProfileUser->m_Losses[0], total_wins, total_losses);

	//pServerConnection->SendTo( (const char*)&msgPlayerFactionStats, sizeof(CNetMsgPlayerFactionStats) );
	//printf("Sending player faction stats...\n");
	//return true;

   pUserStats->SendGameData( connectionId, gatewayId, sizeof(msgPlayerFactionStats), reinterpret_cast<const U8*>(&msgPlayerFactionStats) );
}



/*
static void OnSendAdditionalLoginData_SummonWar( CServerConnection *pServerConnection )
{
   OnRequestPlayerFactionStats_SummonWar( pServerConnection, NULL );
}
*/



static void OnRequestGlobalFactionStats_SummonWar( UserStatsMainThread *pUserStats, U32 connectionId, U32 gatewayId )
{
	CNetMsgGlobalFactionStats msgGlobalFactionStats;
	memcpy(&msgGlobalFactionStats.factionWins, &s_FactionWins, sizeof(unsigned int)*16*16);

   pUserStats->SendGameData( connectionId, gatewayId, sizeof(msgGlobalFactionStats), reinterpret_cast<const U8*>(&msgGlobalFactionStats) );
	//pServerConnection->SendTo( (const char*)&msgGlobalFactionStats, sizeof(CNetMsgGlobalFactionStats) );
}



static void OnRequestGameProfile_SummonWar( UserStatsMainThread *pUserStats, U32 connectionId, U32 gatewayId,
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
   memset( msgUpdatePlayerProfile.playerData.username.displayName, 0,
      sizeof(msgUpdatePlayerProfile.playerData.username.displayName) );
   memset( msgUpdatePlayerProfile.playerData.username.gameKitHash, 0,
      sizeof(msgUpdatePlayerProfile.playerData.username.gameKitHash) );
   msgUpdatePlayerProfile.playerData.username.type = GAME_PLAYER_NAME_BASIC;

   char query[256];
   sprintf(query, "SELECT user_name FROM users WHERE user_id=\'%u\'", profileUserId);

   mysql_query(s_pMysqlStats, query);
   MYSQL_RES *user_res = mysql_store_result(s_pMysqlStats);
   if( user_res )
   {
      MYSQL_ROW user_row = mysql_fetch_row(user_res);
      if( user_row )
      {
         strcpy( msgUpdatePlayerProfile.playerData.username.displayName, user_row[0] );
      }
      mysql_free_result(user_res);
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

   pUserStats->SendGameData( connectionId, gatewayId, sizeof(msgUpdatePlayerProfile), reinterpret_cast<const U8*>(&msgUpdatePlayerProfile) );

   //return true;
}


bool OnRequestUserProfileStats_SummonWar(UserStatsMainThread *pUserStats, U32 connectionId, U32 gatewayId, unsigned int profileUserId )
{
   PacketUserStats_RequestUserProfileStatsResponse* response = new PacketUserStats_RequestUserProfileStatsResponse;
   response->profileUserId = profileUserId;
   response->gameType = GameProductId_SUMMONWAR;

   const char *gameName = SummonWar_GameName;

   char query[256];
   sprintf(query, "SELECT * FROM stats_%s WHERE user_id=\'%u\'", gameName, profileUserId);

   mysql_query(s_pMysqlStats, query);
   MYSQL_RES *res = mysql_store_result(s_pMysqlStats);
   if (res)
   {
      MYSQL_ROW row = mysql_fetch_row(res);
      if( row )
      {
         if( row[1] != NULL )
         {
            response->userProfileStats.insert( "completed", row[1] );
         }
         if( row[2] != NULL )
         {
            response->userProfileStats.insert( "wins_2p", row[2] );
         }
         if( row[3] != NULL )
         {
            response->userProfileStats.insert( "losses_2p", row[3] );
         }
         if( row[8] != NULL )
         {
            response->userProfileStats.insert( "forfeits", row[8] );
         }

         //LogMessage(LOG_PRIO_INFO, "Retrieved stats for user %d\n", pUser->GetUserID());
         //LogMessage(LOG_PRIO_INFO, "   (%d, %d, %d, %d, %d, %d, %d, %d)\n", pUser->m_CompletedGames, pUser->m_Wins[0],
         //   pUser->m_Losses[0], pUser->m_Wins[1], pUser->m_Losses[1], pUser->m_Wins[2], pUser->m_Losses[2], pUser->m_Forfeits );
      }
      mysql_free_result(res);
   }

   pUserStats->SendPacketToGateway( response, connectionId, gatewayId );
   return true;
}



/*
static bool SummonWar_ReceiveIncomingMessage( void *pConnection, const char *pRecvMsg, int msgSize )
{
	CServerConnection *pServerConnection = reinterpret_cast<CServerConnection*>( pConnection );
   assert( pServerConnection != NULL );

   const CNetMsg *pNetMsg = reinterpret_cast<const CNetMsg *>( pRecvMsg );

   if( pNetMsg->channelID == 0 )
   {
      switch (pNetMsg->msgType)
      {
		  case NETMSG_REQUEST_PLAYER_FACTION: return OnRequestPlayerFactionStats_SummonWar( pServerConnection, static_cast<const CNetMsgRequestPlayerFactionStats *>(pNetMsg) );
		  case NETMSG_REQUEST_GLOBAL_FACTION: return OnRequestGlobalFactionStats_SummonWar( pServerConnection, static_cast<const CNetMsgRequestGlobalFactionStats *>(pNetMsg) );
        case NETMSG_REQUEST_PROFILE:        return OnRequestPlayerProfile_SummonWar(pServerConnection, static_cast<const CNetMsgRequestPlayerProfile *>(pNetMsg));
      }
   }

   return false;
}
*/

static void SummonWar_Shutdown()
{
	//summonwar::ShutdownLua();
}




void SummonWar_StatsPluginInit( StatsPlugin *pluginData, MYSQL *mysqlStats )
{
   s_pMysqlStats = mysqlStats;

   memset(s_FactionWins, 0, sizeof(s_FactionWins));
   memset(s_FactionGames, 0, sizeof(s_FactionGames));

   pluginData->pGameName = SummonWar_GameName;
   //pluginData->onRetrieveRulesVersion = SummonWar_OnRetrieveRulesVersion;
   //pluginData->onStartGame = SummonWar_OnStartGame;
   //pluginData->onGameUpdate = SummonWar_OnGameUpdate;
   //pluginData->onHandleForfeitUserDecision = SummonWar_OnHandleForfeitUserDecision;
   pluginData->onReportGameResult = SummonWar_OnReportGameResult;
   pluginData->onReportUserForfeit = SummonWar_OnReportUserForfeit;

   pluginData->onRequestPlayerFactionStats = OnRequestPlayerFactionStats_SummonWar;
   pluginData->onRequestGlobalFactionStats = OnRequestGlobalFactionStats_SummonWar;
   pluginData->onRequestGameProfile = OnRequestGameProfile_SummonWar;
   pluginData->onRequestUserProfileStats = OnRequestUserProfileStats_SummonWar;
   //pluginData->onSendAdditionalLoginData = OnSendAdditionalLoginData_SummonWar;
   //pluginData->onReceiveIncomingMessage = SummonWar_ReceiveIncomingMessage;
   //pluginData->onShutdown = SummonWar_Shutdown;

   //summonwar::TLuaRegisterFunc summonwarRegisterFuncs[] = { summonwarrules::RegisterLua_summonwarrules };
   //unsigned int summonwarRegisterFuncCount = sizeof(summonwarRegisterFuncs)/sizeof(summonwarRegisterFuncs[0]);
	//summonwar::LoadLuaFunctions( summonwarRegisterFuncCount, summonwarRegisterFuncs, RESOURCE_DIR );

//#if USE_MYSQL_DATABASE
   //const char *gameName = GameDatabaseName(GAME_SELECT_SUMMONWAR);
   const char *gameName = SummonWar_GameName;
      
   for (int i = 0; i < SUMMONWAR_MAX_FACTIONS; ++i)
   {
      char query[2048] = "SELECT SUM(vs_faction_0) AS vs_faction_0";
      int qLen = strlen(query);
      for (int j = 1; j < SUMMONWAR_MAX_FACTIONS; ++j)
      {
         qLen += sprintf(query + qLen, ", SUM(vs_faction_%d) AS vs_faction_%d", j, j);
      }
      
      sprintf(query + qLen, " FROM stats_%s_faction_wins WHERE faction_id='%d'", gameName, i);

      mysql_query(s_pMysqlStats, query);

      MYSQL_RES *res = mysql_store_result(s_pMysqlStats);
      if( !res )
      {
         //LogMessage(LOG_PRIO_WARN, "Could not read faction stats for faction %d\n", i);
         continue;
      }

      if(mysql_num_fields(res) != SUMMONWAR_MAX_FACTIONS)
      {
         //LogMessage(LOG_PRIO_WARN, "Could not read faction stats for faction %d\n", i);
         mysql_free_result(res);
         continue;
      }

      MYSQL_ROW row = mysql_fetch_row(res);
      if (!row)
      {
         //LogMessage(LOG_PRIO_WARN, "Could not read faction stats for faction %d\n", i);
         mysql_free_result(res);
         continue;
      }
      
      // store the wins for this faction
      for (int j = 0; j < SUMMONWAR_MAX_FACTIONS; ++j)
      {
         unsigned int wins = 0;
         if( row[j] != NULL )
         {
            wins = strtol(row[j], NULL, 10);
         }
         s_FactionWins[i][j] = wins;
         s_FactionGames[i] += wins;
      }

      mysql_free_result(res);
   }

   // add up the losses for each faction
   for (int i = 0; i < SUMMONWAR_MAX_FACTIONS; ++i)
   {
      for (int j = 0; j < SUMMONWAR_MAX_FACTIONS; ++j)
      {
         s_FactionGames[i] += s_FactionWins[j][i];
      }
   }
//#endif

}

