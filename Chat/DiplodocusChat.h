// DiplodocusChat.h

#pragma once

#include "../NetworkCommon/DataTypes.h"
#include "../NetworkCommon/Utils/CommandLineParser.h"
#include "../NetworkCommon/Utils/Utils.h"
#include "../NetworkCommon/NetworkIn/Khaan.h"
#include "../NetworkCommon/Database/Deltadromeus.h"
#include "../NetworkCommon/NetworkIn/Diplodocus.h"
#include "../NetworkCommon/ServerConstants.h"
#include "KhaanChat.h"
#include "UserConnection.h"
//#include "DiplodocusController.h"

class ChatChannelManager;
class DiplodocusServerToServer;
class PacketPrepareForUserLogin;
class PacketPrepareForUserLogout;
class PacketListOfUserProductsS2S;

const int ChatChannelManagerConnectionId = ConnectionIdExclusion.low;

////////////////////////////////////////////////////////////////////////////////////////

class DiplodocusChat : public Diplodocus< KhaanChat >
{
public: 
   typedef Diplodocus< KhaanChat > ChainedType;

public:
   DiplodocusChat( const string& serverName, U32 serverId );
   void     Init();

   void     ServerWasIdentified( IChainedInterface* khaan );

   bool     AddInputChainData( BasePacket* packet, U32 connectionId );
   // data going out can go only a few directions
   // coming from the DB, we can have a result or possibly a different packet meant for a single chat UserConnection
   // otherwise, coming from a UserConnection, to go out, it will already be packaged as a Gateway Wrapper and then 
   // we simply send it on.
   bool     AddOutputChainData( BasePacket* packet, U32 connectionId );
   bool     AddPacketFromUserConnection( BasePacket* packet, U32 connectionId );// not thread safe

   int      CallbackFunction();
   void     ChatChannelManagerNeedsUpdate() { m_chatChannelManagerNeedsUpdate = true; }

   void     TurnOnUpdates( bool onOff = true ) { m_inputsNeedUpdate = onOff; }

   //-----------------------------------------------------------------------------
   // ths folowing interface is for the Userconnections to use to communicate
   void     FinishedLogin( U32 connectionId, const string& uuidUser );
   void     FinishedLogout( U32 connectionId, const string& uuidUser );

   void     SetupGroup( const string& groupId, const KeyValueVector& userList );// as users load in, group list will be maintained.
   void     RemoveConnection( int connectionId );
 
   //bool     SendErrorReportToClient( PacketErrorReport::ErrorType error, int connectionId );
   
   void     InputConnected( IChainedInterface* chainedInput );
   void     InputRemovalInProgress( IChainedInterface* chainedInput );

   //-----------------------------------------------------------------------------

private:

   void     SetupForNewUserConnection( PacketPrepareForUserLogin* loginPacket );
   void     HandleUserDisconnection( PacketPrepareForUserLogout* logoutPacket );

   void     CleanupOldConnections();
   bool     HandleCommandFromGateway( BasePacket* packet, U32 connectionId );
   bool     HandlePacketFromOtherServer( BasePacket* packet, U32 connectionId );
   //bool     HandlePacketToOtherServer( BasePacket* packet, U32 connectionId );
   
   bool                                   m_inputsNeedUpdate;

   typedef pair< int, UserConnection* >   ConnectionPair;
   typedef map< int, UserConnection* >    ConnectionMap;
   typedef ConnectionMap::iterator        ConnectionMapIterator;

   typedef map< string, int >             UuidConnectionIdMap;
   typedef pair< string, int >            UuidConnectionIdPair;
   typedef map< int, string >             ConnectionIdUuidMap;
   typedef pair< int, string >            ConnectionIdUuidPair;

   ConnectionMap                          m_connectionMap;
   list< UserConnection* >                m_oldConenctions;

   UuidConnectionIdMap                    m_connectionLookup;// we are looking up the right side
   ConnectionIdUuidMap                    m_uuidLookup;

   deque< int >                           m_connectionsNeedingUpdate;

   //----------------------------------------------

   bool                                   m_chatChannelManagerNeedsUpdate;
   ChatChannelManager*                    m_chatChannelManager;
};

////////////////////////////////////////////////////////////////////////////////////////
