#pragma once
#include "Deltadromeus.h"

class Scleromochlus : public Database::Deltadromeus
{
public:
   Scleromochlus();

   //-----------------------------------------------

   //void     SetConnectionInfo( const string& serverName, U16 port, const string& username,const string& password, const string& dbName, const string& schema );
   void     SetConnectionInfo( const string& serverName, U16 port, const string& username,const string& password, const string& dbSchema );

   //-----------------------------------------------
   Database::JobId    SendQuery( const string& query, int myId = 0, int senderReference = 0, const list<string>* stringsToEscape = NULL, bool isFireAndForget = false, bool isChainData = false, int extraLookupInfo = 0, string* meta = NULL, U32 serverId =0 );
   bool     AddInputChainData( BasePacket* packet, U32 chainId );// only use this interface for chained processing

protected:
   int      CallbackFunction();
};
