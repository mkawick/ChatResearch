// NewAccountQueryHandler.h
#pragma once

#include "../NetworkCommon/Packets/BasePacket.h"
#include "../NetworkCommon/Database/QueryHandler.h"
#include <map>
using namespace std;

class BlankUUIDQueryHandler;

static const bool isMailServiceEnabled = true;

///////////////////////////////////////////////////////////////////////////////////////////

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


///////////////////////////////////////////////////////////////////////////////////////////

class NewAccountQueryHandler : public QueryHandler
{

public:
   NewAccountQueryHandler( int id, Queryer* parent, string& query );

   void     Update( time_t currentTime );

   void     SetBlankUUIDHandler( BlankUUIDQueryHandler* blankUUIDhandler ) { m_blankUuidHandler = blankUUIDhandler; }
   void     SetQueryTypeForLoadingStrings( U32 type ) { m_loadStringsQueryType = type; }
   void     SetQueryTypeForLoadingWeblinks( U32 type ) { m_loadWebLinksQueryType = type; }
   void     SetQueryTypeForOlderEmailsResent( U32 type ) { m_olderEmailsQueryType = type; }


   bool     HandleResult( const PacketDbQueryResult* dbResult );
   void     UpdateUuidForUser( const string& userId, bool updateCreateAccountTableToo, const string& columnId );

   bool     IsReady() const { return m_hasLoadedStringTable & m_hasLoadedWeblinks; }

protected:

   void     PreloadLanguageStrings();
   void     SaveStrings( const PacketDbQueryResult* dbResult );
   void     CheckForNewAccounts();
   

   BlankUUIDQueryHandler* m_blankUuidHandler;

   
   bool                 m_isServicingNewAccounts;
   bool                 m_isServicingStringLoading;
   bool                 m_isServicingWebLinks;

   U32                  m_loadStringsQueryType;
   U32                  m_loadWebLinksQueryType;
   U32                  m_olderEmailsQueryType;

   typedef map< stringhash, vector< string > >    StringTableLookup;
   typedef pair< stringhash, vector< string > >   StringTableLookupPair;
   typedef pair< stringhash, stringhash >          ReplacementPair;
   
   void     ReplaceAllLookupStrings( string& bodyText, int languageId,  map< string, string >& specialStrings );

   void     PreloadWeblinks();
   void     HandleWeblinks( const PacketDbQueryResult* dbResult );
   string   m_linkToAccountCreated;
   string   m_linkToResetPasswordConfirm;
   

protected:
   static string   GetString( const string& stringName, int languageId );

   static bool                            m_hasLoadedStringTable;
   static bool                            m_hasLoadedWeblinks;
   static string                          m_pathToConfirmationEmailFile;
   static string                          m_confirmationEmailTemplate;
   static string                          m_passwordResetEmailTemplate;
   static StringTableLookup               m_stringsTable;
   static map< stringhash, stringhash >   m_replacemetStringsLookup;
   static const char*                     m_mailServer;

   NewAccountQueryHandler();
   //~NewAccountQueryHandler();
};

///////////////////////////////////////////////////////////////////////////////////////////