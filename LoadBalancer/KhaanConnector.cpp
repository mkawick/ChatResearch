// KhaanConnector.cpp

#include <string>
using namespace std;

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include "../NetworkCommon/Utils/CommandLineParser.h"

#include "KhaanConnector.h"
#include "DiplodocusLoadBalancer.h"

#include "../NetworkCommon/Packets/LoginPacket.h"
#include "../NetworkCommon/Packets/PacketFactory.h"

//-----------------------------------------------------------------------------------------

KhaanConnector::KhaanConnector( int id, bufferevent* be ): 
      KhaanProtected( id, be )
{
}

//-----------------------------------------------------------------------------------------


KhaanConnector::~KhaanConnector()
{
}

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------

bool  KhaanConnector::IsWhiteListedIn( const BasePacket* packet ) const
{
   switch( packet->packetType )
   {
   case  PacketType_Base:
      {
         if( packet->packetSubType == BasePacket::BasePacket_RerouteRequest )
         {
            return true;
         }
         return false;
      }
   }

   return false;
}

//-----------------------------------------------------------------------------------------