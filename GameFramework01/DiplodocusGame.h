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

class UserSession;

////////////////////////////////////////////////////////////////////////////////////////

class DiplodocusGame : public Diplodocus< KhaanGame >
{
public:
   DiplodocusGame( const string& serverName, U32 serverId );

   bool     AddInputChainData( BasePacket* packet, U32 connectionId );
   bool     AddOutputChainData( BasePacket* packet, U32 connectionId );
   bool     IsConnectionValid( U32 connectionId ) const ;

   void     HandleCommandFromGateway( BasePacket* packet, U32 connectionId );

   void     RegisterCallbacks( GameCallbacks* callbacks ) { m_callbacks = callbacks; }
   

   //-----------------------------------------------------------------------------
   // ths folowing interface is for the Userconnections to use to communicate
   void     FinishedLogin( int connectionId, const string& uuidUser );
   void     FinishedLogout( int connectionId, const string& uuidUser );

   void     SetupGroup( const string& groupId, const KeyValueVector& userList );// as users load in, group list will be maintained.
   void     RemoveConnection( int connectionId );

   bool     SendErrorReportToClient( PacketErrorReport::ErrorType error, int connectionId );
   void     ClientConnectionIsAboutToRemove( KhaanGame* khaan );
   void     ClientConnectionFinishedAdding( KhaanGame* khaan );

   //-----------------------------------------------------------------------------
private:

   int      CallbackFunction();

   void     CleanupOldConnections();

   bool     HandlePacketFromOtherServer( BasePacket* packet, U32 connectionId );
   bool     HandlePacketToOtherServer( BasePacket* packet, U32 connectionId );
   void     ConnectUser( PacketPrepareForUserLogin* loginPacket );
   void     DisconnectUser( PacketPrepareForUserLogout* logoutPacket );

   typedef pair< int, UserSession* >      ConnectionPair;
   typedef map< int, UserSession* >       ConnectionMap;
   typedef map< string, int >             UuidConnectionIdMap;
   typedef pair< string, int >            UuidConnectionIdPair;
   typedef map< int, string >             ConnectionIdUuidMap;
   typedef pair< int, string >            ConnectionIdUuidPair;

   typedef ConnectionMap::iterator        ConnectionMapIterator;
   ConnectionMap                          m_connectionMap;

   UuidConnectionIdMap                    m_connectionLookup;// we are looking up the right side
   ConnectionIdUuidMap                    m_uuidLookup;

   deque< int >                           m_connectionsNeedingUpdate;

   GameCallbacks*                         m_callbacks;
};

////////////////////////////////////////////////////////////////////////////////////////
