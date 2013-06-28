// DiplodocusChat.h

#pragma once

#include "../NetworkCommon/Utils/CommandLineParser.h"
#include "../NetworkCommon/DataTypes.h"
#include "../NetworkCommon/Utils/Utils.h"
#include "../NetworkCommon/Database/Deltadromeus.h"

#include "../NetworkCommon/NetworkIn/Khaan.h"
#include "../NetworkCommon/NetworkIn/Diplodocus.h"
#include "../NetworkCommon/ServerConstants.h"

#include "../NetworkCommon/Packets/GamePacket.h"

#include "KhaanGame.h"
class UserSession;
//#include "UserConnection.h"

//class ChatChannelManager;

////////////////////////////////////////////////////////////////////////////////////////

class DiplodocusGame : public Diplodocus< KhaanGame >
{
public:
   DiplodocusGame( const string& serverName, U32 serverId, U8 gameProductId = 0 );

   bool     AddInputChainData( BasePacket* packet, U32 connectionId );
   bool     AddOutputChainData( BasePacket* packet, U32 connectionId );
   bool     AddPacketFromUserConnection( BasePacket* packet, U32 connectionId );// not thread safe

   int      CallbackFunction();

   bool     HandleCommandFromGateway( BasePacket* packet, U32 connectionId );
   

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

   //UserConnection*   FindUser( const string& uuid );

private:
   void     CleanupOldConnections();

   bool     HandlePacketFromOtherServer( BasePacket* packet, U32 connectionId );
   bool     HandlePacketToOtherServer( BasePacket* packet, U32 connectionId );
   void     ConnectUser( const PacketPrepareForUserLogin* loginPacket );
   void     DisconnectUser( const PacketPrepareForUserLogout* logoutPacket );
   void     IsUserAllowedToUseThisProduct( const PacketListOfGames* packet );

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

   //----------------------------------------------

   U32                                    m_connectionIdChatChannelManager;
   //bool                                   m_chatChannelManagerNeedsUpdate;
   //ChatChannelManager*                    m_chatChannelManager;
};

////////////////////////////////////////////////////////////////////////////////////////
