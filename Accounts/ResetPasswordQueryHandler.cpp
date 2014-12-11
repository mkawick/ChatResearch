#include <time.h>
#include <iostream>

#include "AccountServer.h"
#include <boost/algorithm/string/replace.hpp>

#include "../NetworkCommon/Utils/Utils.h"
#include "../NetworkCommon/Utils/TableWrapper.h"
#include "ResetPasswordQueryHandler.h"
#include "email.h"
#include "../NetworkCommon/Logging/server_log.h"

using namespace std;

//////////////////////////////////////////////////////////////

class EmailAddressesOfPasswordsToResetTable
{
public:
   enum Columns
   {
      Column_id,
      Column_email,
      Column_language_id,
      Column_reset_key,
      Column_end
   };
};

typedef Enigmosaurus <EmailAddressesOfPasswordsToResetTable> PasswordResetParser;

//////////////////////////////////////////////////////////////

ResetPasswordQueryHandler::ResetPasswordQueryHandler( U32 id, Queryer* parent, string& query ) : NewAccountQueryHandler( id, parent, query ), m_isServicingResetPassword( false )
{
}

ResetPasswordQueryHandler::~ResetPasswordQueryHandler()
{
}

void     ResetPasswordQueryHandler::Update( time_t currentTime )
{
   if( m_hasLoadedStringTable == false || m_hasLoadedWeblinks == false )
      return;

   if( isMailServiceEnabled == true && m_isEmailEnabled == true )
   {
      ParentType::Update( currentTime, m_isServicingResetPassword );
   }
}

///////////////////////////////////////////////////////////////////////////////////////////

bool     ResetPasswordQueryHandler::HandleResult( const PacketDbQueryResult* dbResult )
{
   U32 queryType = static_cast< U32 >( dbResult->lookup );
   if( queryType != m_queryType )
      return false;

   SetValueOnExit< bool >           setter( m_isServicingResetPassword, false );// due to multiple exit points...

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
      if( languageString.size() > 0 && languageString != "NULL" )
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
            bodyText = m_passwordResetEmailTemplate;
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
         if( SendConfirmationEmail( email.c_str(), m_resetPasswordEmailAddress.c_str(), 
                                 m_mailServer.c_str(), 
                                 bodyText.c_str(), subjectText.c_str(), "Playdek.com", linkPath.c_str(),
                                 m_emailPortOverride,
                                 m_authenticatedEmailUsername.c_str(), m_authenticatedEmailPassword.c_str() ) != 0 )
         {
            string   message = "ERROR: For reset password, SendConfirmationEmail seems to be down. Socket connections are being rejected.";
            LogMessage( LOG_PRIO_ERR, message.c_str() );
            cout << endl << message << endl;
            time( &m_lastTimeStamp );// restart timer
            return false;
         }

         PacketDbQuery* dbQuery = new PacketDbQuery;
         dbQuery->id = 0;
         dbQuery->lookup = m_queryType;
         dbQuery->isFireAndForget = true;

         dbQuery->query = "UPDATE reset_password_keys SET was_email_sent=1 WHERE reset_key='";
         dbQuery->query += Column_reset_key;
         dbQuery->query += "'";

         m_parent->AddQueryToOutput( dbQuery );

         string message = "Resetting password account confirmation at email: ";
         message += email;

         LogMessage( LOG_PRIO_INFO, message.c_str() );
         cout << endl << message << endl;
      }
   }

   time( &m_lastTimeStamp );

   return true;
}

///////////////////////////////////////////////////////////////////////////////////////////
