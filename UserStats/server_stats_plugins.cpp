
#include "server_stats_plugins.h"

#include "../NetworkCommon/ServerConstants.h"

StatsPlugin s_StatsPlugins[k_statsPluginCount];

void InitStatsPlugins( MYSQL *mysqlStats )
{
   memset( s_StatsPlugins, 0, sizeof(s_StatsPlugins) );

   void Ascension_StatsPluginInit( StatsPlugin *pluginData, MYSQL *mysqlStats );
   Ascension_StatsPluginInit( &s_StatsPlugins[GameProductId_ASCENSION], mysqlStats );

   void SummonWar_StatsPluginInit( StatsPlugin *pluginData, MYSQL *mysqlStats );
   SummonWar_StatsPluginInit( &s_StatsPlugins[GameProductId_SUMMONWAR], mysqlStats );

   void Agricola_StatsPluginInit( StatsPlugin *pluginData, MYSQL *mysqlStats );
   Agricola_StatsPluginInit( &s_StatsPlugins[GameProductId_AGRICOLA], mysqlStats );

   void Waterdeep_StatsPluginInit( StatsPlugin *pluginData, MYSQL *mysqlStats );
   Waterdeep_StatsPluginInit( &s_StatsPlugins[GameProductId_WATERDEEP], mysqlStats );
}

void ShutdownGamePlugins()
{
   for( int g = 0; g < k_statsPluginCount; ++g )
   {
      if( s_StatsPlugins[g].onShutdown != NULL )
      {
         s_StatsPlugins[g].onShutdown();
      }
   }
}
