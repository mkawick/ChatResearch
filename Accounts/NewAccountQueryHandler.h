// NewAccountQueryHandler.h
#pragma once

#include "../NetworkCommon/Packets/BasePacket.h"
#include "../NetworkCommon/Database/QueryHandler.h"
#include <map>
using namespace std;

class BlankUUIDQueryHandler;

static const bool isMailServiceEnabled = true;
extern const char* newAccountEmailAddress;
extern const char* resetPasswordEmailAddress;

///////////////////////////////////////////////////////////////////////////////////////////

struct EmailToSend
{
   string userLookupKey;
   string email;
   string accountEmailAddress;
   string bodyText;
   string subjectText;
   string linkPath;
};

///////////////////////////////////////////////////////////////////////////////////////////

class NewAccountQueryHandler : public QueryHandler< Queryer* >
{
public:
   typedef QueryHandler< Queryer* > ParentType;

public:
   NewAccountQueryHandler( U32 id, Queryer* parent, string& query );

   void     Update( time_t currentTime );

   void     SetBlankUUIDHandler( BlankUUIDQueryHandler* blankUUIDhandler ) { m_blankUuidHandler = blankUUIDhandler; }
   void     SetQueryTypeForLoadingStrings( U32 type ) { m_loadStringsQueryType = type; }
   void     SetQueryTypeForLoadingWeblinks( U32 type ) { m_loadWebLinksQueryType = type; }
   void     SetQueryTypeForOlderEmailsResent( U32 type ) { m_olderEmailsQueryType = type; }

   void     SetEmailDomain( const string& domain ); 
   void     EnableEmailSending( bool enabled = true ) { m_isEmailEnabled = enabled; }

   void     SetEmailPortOverride( U16 port ) { m_emailPortOverride = port; }
   void     SetEmailAuthUsername( const string& name ) { m_authenticatedEmailUsername = name; }
   void     SetEmailAuthPassword( const string& pwd ) { m_authenticatedEmailPassword = pwd; }

   bool     HandleResult( const PacketDbQueryResult* dbResult );
   void     UpdateUuidForUser( const string& userId, bool updateCreateAccountTableToo, const string& columnId );

   bool     IsReady() const { return m_hasLoadedStringTable & m_hasLoadedWeblinks; }

protected:

   void     PreloadLanguageStrings();
   void     SaveStrings( const PacketDbQueryResult* dbResult );
   void     CheckForNewAccounts();

   void     PrepToSendUserEmail( const PacketDbQueryResult* dbResult );
   

   BlankUUIDQueryHandler* m_blankUuidHandler;

   
   bool                 m_isServicingNewAccounts;
   bool                 m_isServicingStringLoading;
   bool                 m_isServicingWebLinks;
   bool                 m_isEmailEnabled;

   U32                  m_loadStringsQueryType;
   U32                  m_loadWebLinksQueryType;
   U32                  m_olderEmailsQueryType;

   typedef map< stringhash, vector< string > >     StringTableLookup;
   typedef pair< stringhash, vector< string > >    StringTableLookupPair;
   typedef pair< stringhash, stringhash >          ReplacementPair;

   typedef map< string, EmailToSend >              EmailLookupType;
   typedef pair< string, EmailToSend >             EmailLookupPair;
   typedef map< string, EmailToSend >::iterator    EmailLookupIterator;

   EmailLookupType      m_emailLookup;
   
   void     ReplaceAllLookupStrings( string& bodyText, int languageId,  map< string, string >& specialStrings );

   void     PreloadWeblinks();
   void     HandleWeblinks( const PacketDbQueryResult* dbResult );
   
   

protected:
   static string   GetString( const string& stringName, int languageId );

   static bool                            m_hasLoadedStringTable;
   static bool                            m_hasLoadedWeblinks;
   static string                          m_linkToAccountCreated;
   static string                          m_linkToResetPasswordConfirm;
   static string                          m_linkToResetEmailConfirm;
   static string                          m_pathToConfirmationEmailFile;
   static string                          m_confirmationEmailTemplate;
   static string                          m_passwordResetEmailTemplate;
   static string                          m_emailResetEmailTemplate;
   static StringTableLookup               m_stringsTable;
   static map< stringhash, stringhash >   m_replacemetStringsLookup;
   static string                          m_mailServer;

   static U16                             m_emailPortOverride;
   static string                          m_authenticatedEmailUsername;
   static string                          m_authenticatedEmailPassword;

   NewAccountQueryHandler();
   //~NewAccountQueryHandler();
};

///////////////////////////////////////////////////////////////////////////////////////////