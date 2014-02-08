// StatsCommon.cpp

//#include <UnitTest.h>
#include <conio.h>
#include <boost/random.hpp>
#include <boost/generator_iterator.hpp>

#include "../NetworkCommon/Packets/PacketFactory.h"
#include "../NetworkCommon/Packets/StatPacket.h"
#include "StatsCommon.h"

///////////////////////////////////////////////////

void  AddStatUtil( HistoricalStats& stats, PacketStat& stat )
{
   HistoricalStats::iterator it = stats.find( stat.statName );
   if( it == stats.end() )
   {
      pair< HistoricalStats::iterator, bool >& tempIt = stats.insert( StatInsert( string( stat.statName ), StatPacketList() ) );
      it = tempIt.first;
   }
   it->second.push_back( stat );
}

CalculatedStats  CalcStats( const StatPacketList& stats )
{
   double numEntries = static_cast< double >( stats.size() );
   if( numEntries == 0.0 )
      return CalculatedStats();

   float min = 1024*1024*1024; // a big number
   float max = -min; // a small number
   double accumulator = 0;
   float finalValue = 0;

   string minTime = "2120-12-12 00:00:00", maxTime = "1967-04-27 00:00:00";
   
   StatPacketList::const_iterator it = stats.begin();
   while( it != stats.end() )
   {
      float value = it->value;
      finalValue = value;
      const string& timestamp = it->timestamp;
      it++;

      accumulator += value;
      if( value < min )
         min = value;
      if( value > max )
         max = value;
      if( timestamp < minTime )
         minTime = timestamp;
      if( timestamp > maxTime )
         maxTime = timestamp;
   }

   // calc std dev
   double mean = accumulator / numEntries;
   double variance = 0;
   
   it = stats.begin();
   while( it != stats.end() )
   {
      double value = static_cast< double >( it->value );
      it++;

      double difference = value - mean;
      difference *= difference; // squared
      variance += difference;
   }

   variance /= numEntries; // this is the final value of the variance

   double stddev = sqrt( variance );

   CalculatedStats stat;//( *stats.begin() );
   stat.stdDev = static_cast< float >( stddev );
   stat.minValue = min;
   stat.maxValue = max;

   if( stats.begin()->statType == PacketStat::StatType_Accumulator )
   {
      stat.mean = static_cast< float >( mean );
   }
   else
   {
      stat.mean = 0;
   }
   stat.finalValue = finalValue;
   stat.statName = stats.begin()->statName;
   stat.beginTime = minTime;
   stat.endTime = maxTime;
   stat.numValues = static_cast< int >( numEntries );

   return stat;
}