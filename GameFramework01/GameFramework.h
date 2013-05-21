#pragma once

#include <string>
using namespace std;

#include "../NetworkCommon/DataTypes.h"
#include "../NetworkCommon/Packets/BasePacket.h"

#include "GameCallbacks.h"

class GameFramework;
class DiplodocusGame;

//////////////////////////////////////////////////////////

class GameFramework
{
public:
   GameFramework( const char* gameName, const char* version = "0.04" );
   ~GameFramework();

   //----------- configuration -----------------------

      //------------ dafaults --------------------
      // DO NOT INVOKE THESE AFTER INVOKING THE RUN FUNCTION... they will have no effect
   void  SetupDefaultDatabaseConnection( const string& serverAddress, U16 port, const string& username,const string& password, const string& dbSchemaName );
   void  SetupDefaultGateway( U16 port );
   void  SetupDefaultChatConnection( const string& address, U16 port );
   void  SetupDefaultS2S( const string& address, U16 port );


   void  UseCommandlineOverrides( int argc, const char* argv[] );

   //----------- end configuration -------------------


   void     RegisterForIncomingData( GameCallbacks* basicGameServer ) { m_callbacksObject = basicGameServer; }
   bool     IsSetupProperly() const { if( m_callbacksObject == NULL ) return false; return true; }

   //----------------------------------------------

   bool     SendGameData( U32 connectionId, const MarshalledData* );

   //----------------------------------------------

   bool     Run();

private:
   GameCallbacks* m_callbacksObject;

   string         m_serverName;
   U32            m_serverId;
   string         m_version;

   DiplodocusGame*m_connectionManager;


   // default connection values
   U16            m_gatewayListenPort;

   U16            m_chatServerPort;
   string         m_chatServerAddress;

   U16            m_listenForS2SPort;
   string         m_listenForS2SAddress;

   U16            m_dbPort;
   string         m_dbIpAddress;
   string         m_dbUsername;
   string         m_dbPassword;
   string         m_dbSchema;
};

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////