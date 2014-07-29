
#ifndef _H_SERVER_STATS_PLUGINS_
#define _H_SERVER_STATS_PLUGINS_

#if defined(_WIN32)
// Define _WINSOCKAPI_ so windows.h will not include old WinSock header.
#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_
#endif
#include <windows.h>
#include <winsock2.h>
#endif

#include <mysql/mysql.h>

#include "../NetworkCommon/DataTypes.h"

class UserStatsMainThread;

struct StatsPlugin
{
   const char *pGameName;
   void (*onReportGameResult)(unsigned int gameID, int playerCount, unsigned int *pResults, unsigned int *pFactions);
   void (*onRequestPlayerFactionStats)(UserStatsMainThread *pUserStats, U32 connectionId, unsigned int userId);
   void (*onRequestGlobalFactionStats)(UserStatsMainThread*, U32 connectionId);
   void (*onRequestGameProfile)(UserStatsMainThread *pUserStats, U32 connectionId,
                                             unsigned int profileUserId, unsigned int requestUserId );
   bool (*onRequestUserProfileStats)(UserStatsMainThread *pUserStats, U32 connectionId, unsigned int profileUserId );
   
   void (*onShutdown)();
};
static const int k_statsPluginCount = 32;
extern StatsPlugin s_StatsPlugins[k_statsPluginCount];

void InitStatsPlugins( MYSQL *mysqlStats );
void ShutdownStatsPlugins();

inline const char* StatsDatabaseName(int gameType)
{
   return s_StatsPlugins[gameType].pGameName;
}

#endif
