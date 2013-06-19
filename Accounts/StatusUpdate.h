#pragma once

#include "../NetworkCommon/Utils/Utils.h"
#include "../NetworkCommon/Packets/BasePacket.h"
#include "../NetworkCommon/DataTypes.h"
#include "../NetworkCommon/ServerType.h"
#include "../NetworkCommon/ChainedArchitecture/ChainedThread.h"
#include <map>

const int DefaultSleepTime = 30;


class PacketDbQuery;
class PacketDbQueryResult;


//-----------------------------------------------------------------------------------------

enum LanguageList // corresponds to the db-language table
{
   LanguageList_english = 1,
   LanguageList_spanish,
   LanguageList_french,
   LanguageList_german,
   LanguageList_italian,
   LanguageList_portuguese,
   LanguageList_russian,
   LanguageList_japanese,
   LanguageList_chinese
};


class StatusUpdate : public Threading::CChainedThread < BasePacket* >
{
   enum QueryType 
   {
      QueryType_UserCreateTempAccount = 1,
      QueryType_UserCheckForNewAccount,
      QueryType_UserFindBlankUUID,
      QueryType_LoadStrings,
      QueryType_LoadWeblinks,
      QueryType_AutoCreateUsers,
      QueryType_AutoCreateUsersInsertOrUpdate,
      QueryType_DeleteTempNewUserRecord
   };

public:
   StatusUpdate( const string& serverName, U32 serverId );
   ~StatusUpdate();

   bool     AddInputChainData( BasePacket* packet, U32 connectionId );
   bool     AddOutputChainData( BasePacket* packet, U32 connectionId );
   

private:

   int      CallbackFunction();
   bool     AddQueryToOutput( PacketDbQuery* packet );

    //---------------------------------------------------------------

   typedef map< stringhash, vector< string > >    StringTableLookup;
   typedef pair< stringhash, vector< string > >   StringTableLookupPair;


   /// the following is all related to email services
   void     PreloadLanguageStrings();
   void     SaveStrings( const PacketDbQueryResult* dbResult );
   string   GetString( const string& stringName, int languageId );


   bool                 m_hasLoadedStringTable;
   StringTableLookup    m_stringsTable;
   map< stringhash, stringhash > m_replacemetStringsLookup;
   typedef pair< stringhash, stringhash > ReplacementPair;
   string   m_pathToConfirmationEmailFile;
   string   m_confirmationEmailTemplate;
   void     ReplaceAllLookupStrings( string& bodyText, int languageId );

   void     PreloadWeblinks();
   void     HandleWeblinks( const PacketDbQueryResult* dbResult );
   string   m_linkToAccountCreated;
   bool     m_hasLoadedWeblinks;

   // TO BE REMOVED (once the login server does not rely on other services updating the new account creation)

   void     FillInUserAccountUUIDs();
   void     HandleBlankUUIDs( PacketDbQueryResult* dbResult );
   void     UpdateUuidForUser( const string& userId, bool updateCreateAccountTableToo, const string& columnId );
   void     CheckForNewAccounts();
   void     HandleNewAccounts( const PacketDbQueryResult* dbResult );

   void     LookForFlaggedAutoCreateAccounts();
   void     HandleAutoCreateAccounts( const PacketDbQueryResult* dbResult );

   time_t   m_checkOnBlankUuidTimer;
   int      m_checkOnBlankUuidTimeoutSeconds;
   time_t   m_newAccountCreationTimer;
   int      m_newAccountTimeoutSeconds;
   time_t   m_checkOnautoCreateTimer;
   int      m_checkOnautoCreateTimeoutSeconds;
};
