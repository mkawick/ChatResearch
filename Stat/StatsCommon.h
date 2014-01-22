#pragma once

#include <map>
#include <deque>
#include <list>
using namespace std;

///////////////////////////////////////////////////

class    CalculatedStats
{
public:
   CalculatedStats() : mean( 0 ), finalValue( 0 ), minValue( 0 ), maxValue( 0 ), numValues( 0 ), stdDev( 0 )
   {}

   string   statName;
   float    mean;
   float    finalValue;
   float    minValue;
   float    maxValue;
   int      numValues;
   float    stdDev;
   
   string   beginTime;
   string   endTime;
};

///////////////////////////////////////////////////

typedef list< PacketStat >              StatPacketList;
typedef map< string, StatPacketList >   HistoricalStats;
typedef pair< string, StatPacketList >  StatInsert;

///////////////////////////////////////////////////

void  AddStatUtil( HistoricalStats& stats, PacketStat& stat );
CalculatedStats  CalcStats( const StatPacketList& stats );