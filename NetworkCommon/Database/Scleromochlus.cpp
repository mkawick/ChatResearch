#include "../ServerConstants.h"

#if PLATFORM == PLATFORM_WINDOWS
#pragma warning( disable:4996 )
#endif

#include "Scleromochlus.h"

struct DbJobMockData : public Database::DbJobBase
{
public:
   DbJobMockData( Database::JobId id, const std::string& query, U32 senderKey = 0, U32 senderIdentifier = 0 ) : 
      DbJobBase( id, query, senderKey, senderIdentifier ) {}

   //-----------------------------------------------------------

   bool        SubmitQuery( DbHandle* connection, const string& dbName )
   {
   }
};

Scleromochlus::Scleromochlus() : Deltadromeus()
{
   m_isConnected = true;
}

/*Scleromochlus::~Scleromochlus()
{
}*/

//------------------------------------------------------------

void     Scleromochlus::SetConnectionInfo( const string& serverName, U16 port, const string& username,const string& password, const string& dbSchema )
{
   m_serverName = serverName;
   m_port = port;
   m_username = username;
   m_password = password;
   m_dbSchema = dbSchema;
   //m_dbConnectionTypeBitField = type;
}

Database::JobId    Scleromochlus::SendQuery( const string& query, int myId, int senderReference, const list<string>* stringsToEscape, bool isFireAndForget, bool isChainData, int extraLookupInfo, string* meta, U32 serverId )
{
   return 0;
}

bool     Scleromochlus::AddInputChainData( BasePacket* packet, U32 chainId )
{
   return 0;
}

int      Scleromochlus::CallbackFunction()
{
   return 1;
}