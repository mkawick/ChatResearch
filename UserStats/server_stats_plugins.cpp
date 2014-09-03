
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

   void Foodfight_StatsPluginInit( StatsPlugin *pluginData, MYSQL *mysqlStats );
   Foodfight_StatsPluginInit( &s_StatsPlugins[GameProductId_FOODFIGHT], mysqlStats );

   void Nightfall_StatsPluginInit( StatsPlugin *pluginData, MYSQL *mysqlStats );
   Nightfall_StatsPluginInit( &s_StatsPlugins[GameProductId_NIGHTFALL], mysqlStats );

   void PennyArcade_StatsPluginInit( StatsPlugin *pluginData, MYSQL *mysqlStats );
   PennyArcade_StatsPluginInit( &s_StatsPlugins[GameProductId_PENNYARCADE], mysqlStats );

   void Agricola_StatsPluginInit( StatsPlugin *pluginData, MYSQL *mysqlStats );
   Agricola_StatsPluginInit( &s_StatsPlugins[GameProductId_AGRICOLA], mysqlStats );

   void Fluxx_StatsPluginInit( StatsPlugin *pluginData, MYSQL *mysqlStats );
   Fluxx_StatsPluginInit( &s_StatsPlugins[GameProductId_FLUXX], mysqlStats );

   void TantoCuore_StatsPluginInit( StatsPlugin *pluginData, MYSQL *mysqlStats );
   TantoCuore_StatsPluginInit( &s_StatsPlugins[GameProductId_TANTOCUORE], mysqlStats );

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
