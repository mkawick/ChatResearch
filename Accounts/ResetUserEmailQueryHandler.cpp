
#include <time.h>
#include <iostream>


#include "AccountServer.h"

#include <boost/algorithm/string/replace.hpp>


#include "../NetworkCommon/Utils/Utils.h"
#include "../NetworkCommon/Utils/TableWrapper.h"
#include "ResetUserEmailQueryHandler.h"
#include "email.h"
#include "../NetworkCommon/Logging/server_log.h"

using namespace std;

//////////////////////////////////////////////////////////////

class ResetUsernameEmailTable
{
public:
   enum Columns
   {
      Column_id,
      Column_uuid,
      Column_reset_key,
      Column_new_email,
      Column_new_username,
      //Column_was_email_sent,
      Column_time_last_email_sent,
      Column_language_id,
      Column_original_username,
      Column_original_email,
      Column_end
   };
};

typedef Enigmosaurus <ResetUsernameEmailTable> EmailUsernameEmailParser;

//////////////////////////////////////////////////////////////

ResetUserEmailQueryHandler::ResetUserEmailQueryHandler( U32 id, Queryer* parent, string& query ) : NewAccountQueryHandler( id, parent, query ), m_isServicingResetPassword( false )
{
}

ResetUserEmailQueryHandler::~ResetUserEmailQueryHandler()
{
}

void     ResetUserEmailQueryHandler::Update( time_t currentTime )
{
   if( m_hasLoadedStringTable == false || m_hasLoadedWeblinks == false )
      return;

   if( isMailServiceEnabled == true && m_isEmailEnabled == true )
   {
      ParentType::Update( currentTime, m_isServicingResetPassword );
   }
}

///////////////////////////////////////////////////////////////////////////////////////////

bool     ResetUserEmailQueryHandler::HandleResult( const PacketDbQueryResult* dbResult )
{
   U32 queryType = static_cast< U32 >( dbResult->lookup );
   if( queryType != m_queryType )
      return false;

   SetValueOnExit< bool >           setter( m_isServicingResetPassword, false );// due to multiple exit points...

   EmailUsernameEmailParser              enigma( dbResult->bucket );
   EmailUsernameEmailParser::iterator    it = enigma.begin();
   
   while( it != enigma.end() )
   {
      EmailUsernameEmailParser::row      row = *it++;
      string columnId =             row[ ResetUsernameEmailTable::Column_id ];
      string uuid =                 row[ ResetUsernameEmailTable::Column_uuid ];
      string reset_key =            row[ ResetUsernameEmailTable::Column_reset_key ];
      string new_email =            row[ ResetUsernameEmailTable::Column_new_email ];
      string new_username =         row[ ResetUsernameEmailTable::Column_new_username ];
     // string new_username =         row[ ResetUsernameEmailTable::Column_was_email_sent ];

      string time_last_email_sent = row[ ResetUsernameEmailTable::Column_time_last_email_sent ];
      string languageString =       row[ ResetUsernameEmailTable::Column_language_id ];
      string original_email =       row[ ResetUsernameEmailTable::Column_original_username ];
      string original_username =    row[ ResetUsernameEmailTable::Column_original_email ];

      
      int languageId = 1;// english
      if( languageString.size() > 0 && languageString != "NULL" )
      {
         languageId = boost::lexical_cast<int>( languageString );
      }

      // we need to pick the valid email
      string email = new_email;
      if( email.size() == 0 || email == "NULL" )
      {
         email = original_email;
      }

      if( IsValidEmailAddress( email ) )
      {
         string subjectText = GetString( "email.change_email.subject", languageId ); //"Confirmation email";
         string bodyText = GetString( "email.change_email.body_text", languageId );//"Thank you for signing up with Playdek. Click this link to confirm your new account.";

         string linkPath = m_linkToResetEmailConfirm;
         linkPath += "?key=";
         linkPath += reset_key;

         if( m_emailResetEmailTemplate.size() )
         {
            bodyText = m_emailResetEmailTemplate;
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
                                 m_authenticatedEmailUsername.c_str(), m_authenticatedEmailPassword.c_str()) != 0 )
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

         dbQuery->query = "UPDATE reset_user_email_name SET was_email_sent=1 WHERE id='";
         dbQuery->query += columnId;
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
