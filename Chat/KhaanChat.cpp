#include "KhaanChat.h"

#include <iostream>

#include "../NetworkCommon/Packets/PacketFactory.h"
#include "../NetworkCommon/Packets/ServerToServerPacket.h"
#include "../NetworkCommon/Utils/CommandLineParser.h"

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
using boost::format;

#include "KhaanChat.h"
#include "DiplodocusChat.h"

///////////////////////////////////////////////////////////////////

//---------------------------------------------------------------

void  KhaanChat::PreStart()
{
   Khaan::PreStart();
   if( m_listOfOutputs.size() == 0 )
      return;

    cout << "Gateway has connected, name = '" << m_serverName << "' : " << m_serverId << endl;
}
//---------------------------------------------------------------

void   KhaanChat ::PreCleanup()
{
   cout << "Gateway has disconnected, name = '" << m_serverName << "' : " << m_serverId << endl;
   Khaan::PreCleanup();
   if( m_listOfOutputs.size() == 0 )
      return;
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////