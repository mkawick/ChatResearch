// DiplodocusChat.h

#pragma once

#include "../NetworkCommon/Utils/CommandLineParser.h"
#include "../NetworkCommon/DataTypes.h"
#include "../NetworkCommon/Utils/Utils.h"
#include "../NetworkCommon/Database/Deltadromeus.h"

#include "../NetworkCommon/NetworkIn/Khaan.h"
#include "../NetworkCommon/NetworkIn/Diplodocus.h"
#include "../NetworkCommon/ServerConstants.h"

#include "KhaanGame.h"
#include "GameCallbacks.h"
#include "../NetworkCommon/Packets/GamePacket.h"
#include <list>
using namespace std;

class UserSession;
class PacketPrepareForUserLogin;
class PacketPrepareForUserLogout;

////////////////////////////////////////////////////////////////////////////////////////

class DiplodocusGame : public Diplodocus< KhaanGame >
{
public:
   DiplodocusGame( const string& serverName, U32 serverId, U8 gameProductId = 0 );

   void     RegisterCallbacks( GameCallbacks* callbacks ) { m_callbacks = callbacks; }

   // default input chain methods
   bool     AddInputChainData( BasePacket* packet, U32 connectionId );
   bool     AddOutputChainData( BasePacket* packet, U32 connectionId );
   bool     HandleCommandFromGateway( BasePacket* packet, U32 connectionId );
   
   //-----------------------------------------------------------------------------

   void     AddTimer( U32 timerId, U32 callbackTimeMs = 100 ); // timers must be unique
   void     InputRemovalInProgress( IChainedInterface* chainedInput );
   void     InputConnected( IChainedInterface* chainedInput );
   void     ServerWasIdentified( ChainedInterface* khaan );

   //-----------------------------------------------------------------------------
   void     SetDatabaseIdentification( const string& uuid ) { m_gameUuid = uuid; }
   bool     IsDatabaseIdentificationValid() const { return m_gameUuid.size() > 0; }

private:
   int      CallbackFunction();
   void	   UpdateAllConnections();
   void     CleanupOldConnections();

   void     UpdateAllTimers();

   bool     HandlePacketFromOtherServer( BasePacket* packet, U32 connectionId );
   bool     HandlePacketToOtherServer( BasePacket* packet, U32 connectionId );
   void     ConnectUser( const PacketPrepareForUserLogin* loginPacket );
   void     DisconnectUser( const PacketPrepareForUserLogout* logoutPacket );
   void     IsUserAllowedToUseThisProduct( const PacketListOfGames* packet );

   //U32                                    m_connectionIdGateway;
   deque< U32 >                           m_serversNeedingUpdate;
   GameCallbacks*                         m_callbacks;
   string                                 m_gameUuid;
   

   list< TimerInfo >                      m_timers;
};

////////////////////////////////////////////////////////////////////////////////////////
