#include <time.h>
#include <iostream>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/replace.hpp>

#include "../NetworkCommon/Utils/Utils.h"
#include "../NetworkCommon/Utils/TableWrapper.h"
#include "NewAccountQueryHandler.h"
#include "BlankUUIDQueryHandler.h"
#include "StatusUpdate.h"
#include "email.h"
#include "../NetworkCommon/Logging/server_log.h"

using namespace std;

const char* newAccountEmailAddress = "account_create@playdekgames.com";
const char* resetPasswordEmailAddress = "account_reset@playdekgames.com";

bool                            NewAccountQueryHandler::m_hasLoadedStringTable = false;
bool                            NewAccountQueryHandler::m_hasLoadedWeblinks = false;
const char*                     NewAccountQueryHandler::m_mailServer = "mail.playdekgames.com";
string                          NewAccountQueryHandler::m_linkToAccountCreated;
string                          NewAccountQueryHandler::m_linkToResetEmailConfirm;
string                          NewAccountQueryHandler::m_linkToResetPasswordConfirm;
string                          NewAccountQueryHandler::m_pathToConfirmationEmailFile;
string                          NewAccountQueryHandler::m_confirmationEmailTemplate;
string                          NewAccountQueryHandler::m_passwordResetEmailTemplate;
string                          NewAccountQueryHandler::m_emailResetEmailTemplate;
NewAccountQueryHandler::StringTableLookup               
                                 NewAccountQueryHandler::m_stringsTable;
map< stringhash, stringhash >   NewAccountQueryHandler::m_replacemetStringsLookup;

///////////////////////////////////////////////////////////////////////////////////////////

NewAccountQueryHandler::NewAccountQueryHandler( U32 id, Queryer* parent, string& query ) : ParentType( id, 20, parent ), 
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
      ParentType::Update( currentTime, m_isServicingNewAccounts );
   }
}

//---------------------------------------------------------------

void     NewAccountQueryHandler::CheckForNewAccounts()
{

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

         dbQuery->query = "SELECT * FROM string WHERE category='account' OR category='reset_password' OR category='change_email'";
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
         m_linkToResetPasswordConfirm = "http://accounts.playdekgames.com/reset_user_email_name_confirm.php";

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
   U32 queryType = static_cast< U32 >( dbResult->lookup );
   if( queryType != m_queryType && 
      queryType != m_loadStringsQueryType && 
      queryType != m_loadWebLinksQueryType && 
      queryType != m_olderEmailsQueryType &&
      queryType != StatusUpdate::QueryType_UserUpdateNewAccountKeyBeforeSendingEmail )
      return false;

   SetValueOnExit< bool >     setter( m_isServicingNewAccounts, false );// due to multiple exit points...

   if( queryType == m_loadStringsQueryType )
   {
      SaveStrings( dbResult );
      return true;
   }
   else if( queryType == m_loadWebLinksQueryType )
   {
      HandleWeblinks( dbResult );
      return true;
   }
   else if( queryType == m_olderEmailsQueryType || queryType == m_queryType )
   {
      PrepToSendUserEmail( dbResult );
      //time( &m_lastTimeStamp );// restart timer
   }
   else if( queryType == StatusUpdate::QueryType_UserUpdateNewAccountKeyBeforeSendingEmail )
   {
      EmailLookupIterator it = m_emailLookup.find( dbResult->meta );
      if( it != m_emailLookup.end() )
      {
         EmailToSend& emailDetails = it->second;
         // update playdek.user_temp_new_user set was_email_sent=was_email_sent+1, lookup_key='lkjasdfhlkjhadfs' where id='4' ;
         if( SendConfirmationEmail( emailDetails.email.c_str(), emailDetails.accountEmailAddress.c_str(), m_mailServer, emailDetails.bodyText.c_str(), emailDetails.subjectText.c_str(), "Playdek.com", emailDetails.linkPath.c_str() ) != 0 )
         {
            string   message = "ERROR: For new accounts, SendConfirmationEmail seems to be down. Socket connections are being rejected.";
            LogMessage( LOG_PRIO_ERR, message.c_str() );
            cout << endl << message << endl;
         }
         m_emailLookup.erase( it );
      }
      //time( &m_lastTimeStamp );// restart timer
   }

   // fix up any weird UUID problems
   // I realize that this breaks the essence of encapsulation, but it is far easier than many alternatives
   static_cast<StatusUpdate*>( m_parent )->DuplicateUUIDSearch();

   return true;
}

//---------------------------------------------------------------

bool     IsSpecialCaseEmailSoSendDefaultText( string& email )
{
   if( email.find ( "free.fr" ) != std::string::npos )
      return true;
   return false;
}

//---------------------------------------------------------------

void     NewAccountQueryHandler::PrepToSendUserEmail( const PacketDbQueryResult* dbResult )
{
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
      string lookup =            row[ TableUserTempNewUser::Column_lookup_key ];

      if( uuid == "NULL" || uuid == "null" || uuid == "0" ) 
         uuid.clear();
      if( lookup == "NULL" || lookup == "null" || lookup == "0" ) 
         lookup.clear();

      int languageId =           boost::lexical_cast< int >( row[ TableUserTempNewUser::Column_language_id ] );

      string message = "Sending new account confirmation to user: ";
      message += name;
      message += " at email: ";
      message += email;

      LogMessage( LOG_PRIO_INFO, message.c_str() );
      cout << message << endl;

      if( IsValidEmailAddress( email ) )
      {
         string subjectText = GetString( "email.new_account.welcome.subject", languageId ); //"Confirmation email";
         string bodyText = GetString( "email.new_account.welcome.body_text", languageId );//"Thank you for signing up with Playdek. Click this link to confirm your new account.";

         string userLookupKey = lookup;
         if( userLookupKey.size() == 0 )// do not regenerate the uuid
         {
            userLookupKey = GenerateUUID( GetCurrentMilliseconds() + static_cast<U32>( GenerateUniqueHash( name + email ) ) );
            userLookupKey += GenerateUUID( GetCurrentMilliseconds() ); // double long text
         }
         

         string linkPath = m_linkToAccountCreated;
         linkPath += "?key=";
         linkPath += userLookupKey;

         
         if( IsSpecialCaseEmailSoSendDefaultText( email ) == false && m_confirmationEmailTemplate.size() )
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

         if( m_blankUuidHandler && uuid.size() == 0 )
         {
            m_blankUuidHandler->UpdateUuidForTempUser( columnId, email );
         }
         EmailToSend emailDetails;
         emailDetails.accountEmailAddress = newAccountEmailAddress;
         emailDetails.bodyText = bodyText;
         emailDetails.email = email;
         emailDetails.linkPath = linkPath;
         emailDetails.subjectText = subjectText;
         emailDetails.userLookupKey = userLookupKey;
         m_emailLookup.insert( EmailLookupPair( userLookupKey, emailDetails ) );


         PacketDbQuery* dbQuery = new PacketDbQuery;
         dbQuery->id = 0;
         dbQuery->lookup = StatusUpdate::QueryType_UserUpdateNewAccountKeyBeforeSendingEmail;
         //dbQuery->customData = emailDetails;
         dbQuery->meta = userLookupKey;
         //dbQuery->isFireAndForget = true;

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
     /* if( count == 53 )
      {
         id = id;
      }*/

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
   string resetEmail = "reset_user_email_name_confirm.php";

   string pathToConfirmationEmailFile;
   string pathToResetPasswordEmailFile;
   string pathToResetUserEmailEmailFile;
   
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
      else if( key == "user_account.web_reset_password_confirmed" )
      {
         resetEmail = value;
      }
      else if( key == "user_account.account_created.confirmation_email_path" )
      {
         pathToConfirmationEmailFile = value;
      }
      else if( key == "user_account.password_reset.reset_email_path" )
      {
         pathToResetPasswordEmailFile = value;
      }
      else if( key == "user_account.email_reset.reset_user_email_path" )
      {
         pathToResetUserEmailEmailFile = value;
      }
   }

   //--------------------------------------------------
   
   // assemble the path
   m_linkToAccountCreated = begin;
   m_linkToResetPasswordConfirm = begin;
   m_linkToResetEmailConfirm = begin;

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
   if( accountCreated.size() < 3 )
   {
      cout << "Error: Account created link is too short" << endl;
      assert( accountCreated.size() > 2 );// minimal string size
   }
   
   m_linkToResetPasswordConfirm = m_linkToAccountCreated;
   m_linkToResetPasswordConfirm += resetPassword;
    
   m_linkToResetEmailConfirm = m_linkToAccountCreated;
   m_linkToResetEmailConfirm += resetEmail;
   
   m_linkToAccountCreated += accountCreated;  // make sure that in string additions, this comes last


   if( pathToConfirmationEmailFile.size() )
   {
      cout << "confirmation email file: " << pathToConfirmationEmailFile << endl;
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
   else
   {
      cout << "Success email: confirmation email file found" << endl;
   }

   //-------------------------------------------
   if( pathToResetPasswordEmailFile.size() )
   {
      cout << "reset password file: " << pathToResetPasswordEmailFile << endl;
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
   else
   {
      cout << "Success email: reset password email file found" << endl;
   }

   //-------------------------------------------
   if( pathToResetUserEmailEmailFile.size() )
   {
      cout << "reset email file: " << pathToResetUserEmailEmailFile << endl;
      m_emailResetEmailTemplate = OpenAndLoadFile( pathToResetUserEmailEmailFile );
   }
   else
   {
      cout << "Error email: pathToResetPasswordEmailFile not specified" << endl;
   }
   if( m_emailResetEmailTemplate.size() == 0 )
   {
      cout << "Error email: reset user_email email file not found" << endl;
   }
   else
   {
      cout << "Success email: reset user_email email file found" << endl;
   }
   //-------------------------------------------

   
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
