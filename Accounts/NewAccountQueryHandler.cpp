#include <time.h>
#include <iostream>
using namespace std;

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <iostream>
using namespace std;

#include "../NetworkCommon/Utils/Utils.h"
#include "../NetworkCommon/Utils/TableWrapper.h"
#include "NewAccountQueryHandler.h"
#include "BlankUUIDQueryHandler.h"
#include "StatusUpdate.h"
#include "email.h"
#include "../NetworkCommon/Logging/server_log.h"


const char* newAccountEmailAddress = "account_create@playdekgames.com";

bool                            NewAccountQueryHandler::m_hasLoadedStringTable = false;
bool                            NewAccountQueryHandler::m_hasLoadedWeblinks = false;
const char*                     NewAccountQueryHandler::m_mailServer = "mail.playdekgames.com";
string                          NewAccountQueryHandler::m_linkToAccountCreated;
string                          NewAccountQueryHandler::m_linkToResetPasswordConfirm;
string                          NewAccountQueryHandler::m_pathToConfirmationEmailFile;
string                          NewAccountQueryHandler::m_confirmationEmailTemplate;
string                          NewAccountQueryHandler::m_passwordResetEmailTemplate;
NewAccountQueryHandler::StringTableLookup               
                                 NewAccountQueryHandler::m_stringsTable;
map< stringhash, stringhash >   NewAccountQueryHandler::m_replacemetStringsLookup;

///////////////////////////////////////////////////////////////////////////////////////////

NewAccountQueryHandler::NewAccountQueryHandler( U32 id, Queryer* parent, string& query ) : QueryHandler( id, 20, parent ), 
                     m_isServicingNewAccounts( false ),
                     m_isServicingStringLoading( false ),
                     m_isServicingWebLinks( false )
{
   m_loadStringsQueryType = 0;
   m_loadWebLinksQueryType = 0;
   m_olderEmailsQueryType = 0;
   m_queryString = query;
   time( &m_lastTimeStamp );
}

//---------------------------------------------------------------

void     NewAccountQueryHandler::Update( time_t currentTime )
{
   if( m_hasLoadedStringTable == false || m_hasLoadedWeblinks == false )
   {
      PreloadLanguageStrings();
      PreloadWeblinks();
      return;
   }
   if( isMailServiceEnabled == true )
   {
      QueryHandler::Update( currentTime, m_isServicingNewAccounts );
   }
}

//---------------------------------------------------------------

void     NewAccountQueryHandler::CheckForNewAccounts()
{
   

   time_t currentTime;
   time( &currentTime );

   
/*
   if( difftime( testTimer, m_lastTimeStamp ) >= m_periodicitySeconds ) 
   {
      //cout << "CheckForNewAccounts..." << endl;
      m_lastTimeStamp = testTimer;

      PacketDbQuery* dbQuery = new PacketDbQuery;
      dbQuery->id = 0;
      dbQuery->lookup = m_queryType;
      dbQuery->query = m_queryString;

      m_parent->AddQueryToOutput( dbQuery );
      m_isServicingNewAccounts = true;
   }*/
}

//---------------------------------------------------------------

void     NewAccountQueryHandler::PreloadLanguageStrings()
{
   if( m_hasLoadedStringTable == false && m_isServicingStringLoading == false )
   {
      time_t testTimer;
      time( &testTimer );
      int timeToTest = 10;

      if( difftime( testTimer, m_lastTimeStamp ) >= timeToTest )// wait until after we launch 10 seconds
      {
         m_isServicingStringLoading = true;
         m_lastTimeStamp = testTimer;// so that we don't check too often

         PacketDbQuery* dbQuery = new PacketDbQuery;
         dbQuery->id = 0;
         dbQuery->lookup = m_loadStringsQueryType;

         dbQuery->query = "SELECT * FROM string where category='account' or category='reset_password'";
         m_parent->AddQueryToOutput( dbQuery );

         //LogMessage( LOG_PRIO_INFO, "Accounts::PreloadLanguageStrings\n" );
      }
      return;
   }
}

//---------------------------------------------------------------

void     NewAccountQueryHandler::PreloadWeblinks()
{
   if( m_hasLoadedWeblinks == false && m_isServicingWebLinks == false )
   {
      time_t testTimer;
      time( &testTimer );
      int timeToTest = 10;

      if( difftime( testTimer, m_lastTimeStamp ) >= timeToTest )// wait until after we launch 10 seconds
      {
         m_isServicingWebLinks = true;
         m_hasLoadedWeblinks = true; // only ever load once.
         // because this table may not exist, we will set the default here

         m_linkToAccountCreated = "http://accounts.playdekgames.com/account_created.php";
         m_linkToResetPasswordConfirm = "http://accounts.playdekgames.com/reset_password_confirm.php";

         PacketDbQuery* dbQuery = new PacketDbQuery;
         dbQuery->id = 0;
         dbQuery->lookup = m_loadWebLinksQueryType;

         dbQuery->query = "SELECT * FROM config where category='Mber'";
         m_parent->AddQueryToOutput( dbQuery );

         //LogMessage( LOG_PRIO_INFO, "Accounts::PreloadWeblinks\n" );
      }
   }
}

//---------------------------------------------------------------

bool     NewAccountQueryHandler::HandleResult( const PacketDbQueryResult* dbResult )
{
   if( dbResult->lookup != m_queryType && 
      dbResult->lookup != m_loadStringsQueryType && 
      dbResult->lookup != m_loadWebLinksQueryType && 
      dbResult->lookup != m_olderEmailsQueryType )
      return false;

   SetValueOnExit< bool >     setter( m_isServicingNewAccounts, false );// due to multiple exit points...

   if( dbResult->lookup == m_loadStringsQueryType )
   {
      SaveStrings( dbResult );
      return true;
   }
   else if( dbResult->lookup == m_loadWebLinksQueryType )
   {
      HandleWeblinks( dbResult );
      return true;
   }

   NewUsersTable              enigma( dbResult->bucket );
   NewUsersTable::iterator    it = enigma.begin();
   
   while( it != enigma.end() )
   {
      NewUsersTable::row         row = *it++;
      string columnId =          row[ TableUserTempNewUser::Column_id ];
      string name =              row[ TableUserTempNewUser::Column_name ];
      string email =             row[ TableUserTempNewUser::Column_email ];
      string userId =            row[ TableUserTempNewUser::Column_user_id ];
      string uuid =              row[ TableUserTempNewUser::Column_uuid ];
      int languageId =           boost::lexical_cast< int >( row[ TableUserTempNewUser::Column_language_id ] );

      string message = "Sending new account confirmation to user: ";
      message += name;
      message += " at email: ";
      message += email;

      LogMessage( LOG_PRIO_INFO, message.c_str() );
      cout << message << endl;

      if( email.size() == 0 )// we can't send an email...
      {
         string message = "User does not have a valid email: ";
         message += name;
         LogMessage( LOG_PRIO_WARN, message.c_str() );
         cout << endl << message << endl;
         continue;
      }

      if( IsValidEmailAddress( email ) )
      {
         string subjectText = GetString( "email.new_account.welcome.subject", languageId ); //"Confirmation email";
         string bodyText = GetString( "email.new_account.welcome.body_text", languageId );//"Thank you for signing up with Playdek. Click this link to confirm your new account.";

         string userLookupKey = GenerateUUID( GetCurrentMilliseconds() + static_cast<U32>( GenerateUniqueHash( name + email ) ) );
         userLookupKey += GenerateUUID( GetCurrentMilliseconds() ); // double long text

         string linkPath = m_linkToAccountCreated;
         linkPath += "?key=";
         linkPath += userLookupKey;


         if( m_confirmationEmailTemplate.size() )
         {
            bodyText = m_confirmationEmailTemplate;
            map< string, string > specialStrings;
            specialStrings.insert( pair<string, string> ( "link-to-confirmation", linkPath ) );
            ReplaceAllLookupStrings( bodyText, languageId, specialStrings );
         }
         else
         {
            bodyText += "<a href='";
            bodyText += linkPath;
            bodyText += "'>Playdek.com</a>";
         }

         // update playdek.user_temp_new_user set was_email_sent=was_email_sent+1, lookup_key='lkjasdfhlkjhadfs' where id='4' ;
         if( SendConfirmationEmail( email.c_str(), newAccountEmailAddress, m_mailServer, bodyText.c_str(), subjectText.c_str(), "Playdek.com", linkPath.c_str() ) != 0 )
         {
            string   message = "ERROR: For new accounts, SendConfirmationEmail seems to be down. Socket connections are being rejected.";
            LogMessage( LOG_PRIO_ERR, message.c_str() );
            cout << endl << message << endl;
            time( &m_lastTimeStamp );// restart timer
            return false;
         }

         // it is likely that the new user does not have a UUID yet so we will add it to both tables
         //UpdateUuidForUser( userId, true, columnId );
         if( m_blankUuidHandler )
         {
            m_blankUuidHandler->UpdateUuidForUser( userId, true, columnId );
         }

         PacketDbQuery* dbQuery = new PacketDbQuery;
         dbQuery->id = 0;
         dbQuery->lookup = m_queryType;
         dbQuery->isFireAndForget = true;

         string queryString = "UPDATE user_temp_new_user SET was_email_sent=was_email_sent+1, lookup_key='";
         queryString += userLookupKey;
         queryString += "', time_last_confirmation_email_sent='";
         queryString += GetDateInUTC();
         queryString += "' WHERE id='";
         queryString += columnId;
         queryString += "';";
         dbQuery->query = queryString;

         m_parent->AddQueryToOutput( dbQuery );

         message = "new account confirmation to user: ";
         message += name;
         message += " at email: ";
         message += email;
         message += " ... lookup_key = ";
         message += userLookupKey;

         LogMessage( LOG_PRIO_INFO, message.c_str() );
         cout <<endl << message << endl;

         //cout << "Time sent: ";
         //PrintCurrentTime();
      }
      else
      {
         message = "new account confirmation failed due to invalid email address: ";
         message += name;
         message += " at email: '";
         message += email;
         //LogMessage( LOG_PRIO_INFO, message.c_str() );
         cout << message << endl;

         PacketDbQuery* dbQuery = new PacketDbQuery;
         dbQuery->id = 0;
         dbQuery->lookup = m_queryType;
         dbQuery->isFireAndForget = true;

         string queryString = "UPDATE user_temp_new_user SET flagged_as_invalid='1' WHERE id='";
         queryString += columnId;
         queryString += "';";
         dbQuery->query = queryString;

         m_parent->AddQueryToOutput( dbQuery );
      }
   }

   time( &m_lastTimeStamp );// restart timer

   // fix up any weird UUID problems
   // I realize that this breaks the essence of encapsulation, but it is far easier than many alternatives
   static_cast<StatusUpdate*>( m_parent )->DuplicateUUIDSearch();

   return true;
}

//---------------------------------------------------------------

void     NewAccountQueryHandler::SaveStrings( const PacketDbQueryResult* dbResult )
{
   cout << "strings saved :" << dbResult->bucket.bucket.size() << endl;

   StringTableParser              enigma( dbResult->bucket );
   StringTableParser::iterator    it = enigma.begin();
   int count = 0;
   m_hasLoadedStringTable = true;
   
   while( it != enigma.end() )
   {
      StringTableParser::row     row = *it++;

      string id =                row[ StringsTable::Column_id ];
      string stringName =        row[ StringsTable::Column_string ];
      string replacementstring = row[ StringsTable::Column_replaces ];

      stringhash lookupHash = GenerateUniqueHash( stringName );
      m_stringsTable.insert( StringTableLookupPair( lookupHash, row ) );
      count ++;
      if( count == 53 )
      {
         id = id;
      }

      if( replacementstring.size() )
      {
         stringhash replacementHash = GenerateUniqueHash( replacementstring );
         m_replacemetStringsLookup.insert( ReplacementPair( replacementHash, lookupHash ) );
      }
   }
}

//---------------------------------------------------------------

void     NewAccountQueryHandler::HandleWeblinks( const PacketDbQueryResult* dbResult )
{
   cout << "config saved :" << dbResult->bucket.bucket.size() << endl;

   ConfigParser              enigma( dbResult->bucket );
   ConfigParser::iterator    it = enigma.begin();

   string begin = "http://accounts.playdekgames.com";
   string middle = "";
   string accountCreated = "account_created.php";
   string resetPassword = "reset_password_confirm.php";

   string pathToConfirmationEmailFile;
   string pathToResetPasswordEmailFile;
   
   while( it != enigma.end() )
   {
      StringTableParser::row     row = *it++;

      string key =          row[ ConfigTable::Column_key ];
      string value =        row[ ConfigTable::Column_value ];

      if( key == "user_account.web_root" )
      {
         begin = value;
      }
      else if( key == "user_account.web_subdir" )
      {
         middle = value;
      }
      else if( key == "user_account.web_account_created" )
      {
         accountCreated = value;
      }
      else if( key == "user_account.web_reset_password_confirmed" )
      {
         resetPassword = value;
      }
      else if( key == "user_account.account_created.confirmation_email_path" )
      {
         pathToConfirmationEmailFile = value;
      }
      else if( key == "user_account.password_reset.reset_email_path" )
      {
         pathToResetPasswordEmailFile = value;
      }
   }

   //--------------------------------------------------
   
   // assemble the path
   m_linkToAccountCreated = begin;
   m_linkToResetPasswordConfirm = begin;

   char character = *m_linkToAccountCreated.rbegin();

   if( character != '/' )
   {
      m_linkToAccountCreated += "/";
   }
   if( middle.size() > 0 )
   {
      character = *middle.rbegin();
      m_linkToAccountCreated += middle;
      if( character != '/' )
      {
         m_linkToAccountCreated += "/";
      }
   }
   if( accountCreated.size() < 3 )
   {
      string str = "Config table does not contain a useful value for 'user_account.web_account_created'; db query failed";
      //Log( str );
   }
   assert( accountCreated.size() > 2 );// minimal string size
   m_linkToResetPasswordConfirm = m_linkToAccountCreated;
   m_linkToAccountCreated += accountCreated;
   m_linkToResetPasswordConfirm += resetPassword;

   if( pathToConfirmationEmailFile.size() )
   {
      m_confirmationEmailTemplate = OpenAndLoadFile( pathToConfirmationEmailFile );
   }
   else
   {
      cout << "Error email: pathToConfirmationEmailFile not specified" << endl;
   }
   if( m_confirmationEmailTemplate.size() == 0 )
   {
      cout << "Error email: confirmation email file not found" << endl;
   }

   if( pathToResetPasswordEmailFile.size() )
   {
      m_passwordResetEmailTemplate = OpenAndLoadFile( pathToResetPasswordEmailFile );
   }
   else
   {
      cout << "Error email: pathToResetPasswordEmailFile not specified" << endl;
   }
   if( m_passwordResetEmailTemplate.size() == 0 )
   {
      cout << "Error email: reset password email file not found" << endl;
   }
}

//---------------------------------------------------------------

//---------------------------------------------------------------

string   NewAccountQueryHandler::GetString( const string& stringName, int languageId )
{
   stringhash lookupHash = GenerateUniqueHash( stringName );
   StringTableLookup::iterator it = m_stringsTable.find( lookupHash );

   if( it != m_stringsTable.end() )
   {
      string defaultText = it->second[ StringsTable::Column_english ];

      string tempString = "";
      switch( languageId )
      {
      case LanguageList_spanish:
         tempString = it->second[ StringsTable::Column_spanish ];
         break;
      case LanguageList_french:
         tempString = it->second[ StringsTable::Column_french ];
         break;
      case LanguageList_german:
         tempString = it->second[ StringsTable::Column_german ];
         break;
      case LanguageList_italian:
         tempString = it->second[ StringsTable::Column_italian ];
         break;
      case LanguageList_portuguese:
         tempString = it->second[ StringsTable::Column_german ];
         break;
      case LanguageList_russian:
         tempString = it->second[ StringsTable::Column_russian ];
         break;
      case LanguageList_japanese:
         tempString = it->second[ StringsTable::Column_japanese ];
         break;
      case LanguageList_chinese:
         tempString = it->second[ StringsTable::Column_chinese ];
         break;
      }

      if( tempString != "" && tempString != "NULL" )
         return tempString;
      return defaultText;
   }
   else
   {
      return string();
   }
}

//---------------------------------------------------------------

void     NewAccountQueryHandler::ReplaceAllLookupStrings( string& bodyText, int languageId,  map< string, string >& specialStrings )
{
   vector<string> dictionary = CreateDictionary( bodyText );
   map< string, string > replacements;

   vector<string>::iterator it = dictionary.begin(); 
   while( it != dictionary.end() )
   {
      string lookupString = *it++;

      string actualString;
      map< string, string >::iterator foundIter = specialStrings.find( lookupString );
      if( foundIter != specialStrings.end() )
      {
         actualString = foundIter->second;
         //actualString += "\r\n";// keeping the strings from going too long ... this doesn't work on Apple's email system.
      }
      else
      {
         actualString = GetString( lookupString, languageId );
         actualString += "\r\n";// keeping the strings from going too long
      }
      //if( actualString.size() )
      {
         string replaceString = "%";
         replaceString += lookupString;
         replaceString += "%";
         boost::replace_first( bodyText, replaceString, actualString );
      }
   }
}

///////////////////////////////////////////////////////////////////////////////////////////
