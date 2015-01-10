#pragma once
// AnalyticsMainLoop.h

#include "../NetworkCommon/NetworkIn/Diplodocus.h"
#include "../NetworkCommon/NetworkOut/Fruitadens.h"
#include "../NetworkCommon/Database/QueryHandler.h"
#include "../NetworkCommon/Packets/AnalyticsPacket.h"

#include "ConnectorAnalytics.h"

#include <deque>
using namespace std;
#include "AnalyticsCommon.h"

// averaging
// time averaging (average value based on the gap time)
// highest watermark
// lowest watermark
// total
// std dev

//typedef map< string, PacketStat > HistoricalStats;

///////////////////////////////////////////////////////////////////

class DiplodocusStat : public Queryer, public Diplodocus< KhaanStat, Fruitadens > 
{
public:
   enum QueryType
   {
      QueryType_ExchangeRateLookup,
      QueryType_LoadAllProducts,
      QueryType_VerifyThatUserHasEnoughMoney,
      QueryType_PerformPurchase,
      QueryType_ProductLookup,
      QueryType_SalesLookup,
      QueryType_Count
   };

public:
   DiplodocusStat( const string& serverName, U32 serverId );
   ~DiplodocusStat();

   bool                    AddInputChainData( BasePacket* packet, U32 connectionId );
   bool                    AddOutputChainData( BasePacket* packet, U32 connectionId ) { return false; }

   bool                    AddQueryToOutput( PacketDbQuery* packet );
   void                    ServerWasIdentified( IChainedInterface* khaan );
private:
   bool                    HandlePacketFromOtherServer( BasePacket* packet, U32 connectionId );
   int                     CallbackFunction();
   
   void                    PeriodicWriteToDB();

   time_t                  m_lastDbWriteTimeStamp;
   static const int        timeoutDBWriteStatisics = 60 * 60;// one hour
   HistoricalAnalyisList   m_history;
};

///////////////////////////////////////////////////////////////////