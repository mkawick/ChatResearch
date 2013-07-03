#include <time.h>
#include <iostream>
using namespace std;

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <iostream>
using namespace std;

#include "../NetworkCommon/Utils/Utils.h"
#include "../NetworkCommon/Utils/TableWrapper.h"
#include "ResetPasswordQueryHandler.h"
#include "email.h"


const char* resetPasswordEmailAddress = "account_reset@playdekgames.com";

ResetPasswordQueryHandler::ResetPasswordQueryHandler( int id, Queryer* parent, string& query ) : NewAccountQueryHandler( id, parent, query )
{
}

ResetPasswordQueryHandler::~ResetPasswordQueryHandler()
{
}

void     ResetPasswordQueryHandler::Update( time_t currentTime )
{
   if( m_hasLoadedStringTable == false || m_hasLoadedWeblinks == false )
      return;

   QueryHandler::Update( currentTime );
}

///////////////////////////////////////////////////////////////////////////////////////////

bool     ResetPasswordQueryHandler::HandleResult( const PacketDbQueryResult* dbResult )
{
   if( dbResult->lookup != m_queryType )
      return false;

   PasswordResetParser              enigma( dbResult->bucket );
   PasswordResetParser::iterator    it = enigma.begin();
   
   while( it != enigma.end() )
   {
      PasswordResetParser::row      row = *it++;
      string columnId =             row[ EmailAddressesOfPasswordsToResetTable::Column_id ];
      string email =                row[ EmailAddressesOfPasswordsToResetTable::Column_email ];
      string languageString =       row[ EmailAddressesOfPasswordsToResetTable::Column_language_id ];
      string Column_reset_key =     row[ EmailAddressesOfPasswordsToResetTable::Column_reset_key ];
      
      int languageId = 1;// english
      if( languageString != "NULL" )
      {
         languageId = boost::lexical_cast<int>( languageString );
      }

      if( IsValidEmailAddress( email ) )
      {
         string subjectText = GetString( "email.reset_password.subject", languageId ); //"Confirmation email";
         string bodyText = GetString( "email.reset_password.body_text", languageId );//"Thank you for signing up with Playdek. Click this link to confirm your new account.";

         string linkPath = m_linkToResetPasswordConfirm;
         linkPath += "?key=";
         linkPath += Column_reset_key;

         if( m_passwordResetEmailTemplate.size() )
         {
            string bodyText = m_passwordResetEmailTemplate;
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
         SendConfirmationEmail( email.c_str(), resetPasswordEmailAddress, m_mailServer, bodyText.c_str(), subjectText.c_str(), "Playdek.com", linkPath.c_str() );

         PacketDbQuery* dbQuery = new PacketDbQuery;
         dbQuery->id = 0;
         dbQuery->lookup = m_queryType;
         dbQuery->isFireAndForget = true;

         dbQuery->query = "UPDATE reset_password_keys SET was_email_sent=1 WHERE reset_key='";
         dbQuery->query += Column_reset_key;
         dbQuery->query += "'";

         m_parent->AddQueryToOutput( dbQuery );
      }
   }

   time( &m_lastTimeStamp );

   return true;
}

///////////////////////////////////////////////////////////////////////////////////////////
