// CreateAccountResultsAggregator.cpp

#include "../NetworkCommon/Utils/TableWrapper.h"
#include "CreateAccountResultsAggregator.h"
#include "DiplodocusLogin.h"

#include <boost/lexical_cast.hpp>

//////////////////////////////////////////////////////////

bool  CreateAccountResultsAggregator::HandleResult( const PacketDbQueryResult* dbResult )
{
   if( dbResult->id != m_connectionId ) // wrong result
      return false;

   m_numQueriesToAggregate--;

   // we really don't care about the results, but the error results are important
   switch( dbResult->lookup )
   {
   case DiplodocusLogin:: QueryType_LookupUserByUsernameOrEmail:
      {
         UserTable            enigma( dbResult->bucket );
         UserTable::iterator  it = enigma.begin();
         m_numUserRecordsMatching  = dbResult->bucket.bucket.size();
                  
         while( it != enigma.end() )
         {
            UserTable::row       row = *it++;

            string name =        row[ TableUser::Column_name ];
            string email =       row[ TableUser::Column_email ];
            string gamekit =     row[ TableUser::Column_user_gamekit_id_hash];
            string active =      row[ TableUser::Column_active];

            if( gamekit == m_gamekitHashId )
            {
               m_whichRecordMatched |= MatchingRecord_Gamekithash;
               m_userRecordMatchingGKHash = boost::lexical_cast< U32 > ( row[ TableUser::Column_id ] );
               m_emailForMatchingRecord_GamekitHashId = email;
            }
            if( name == m_username )
            {
               m_whichRecordMatched |= MatchingRecord_Name;
               m_userRecordMatchingName = boost::lexical_cast< U32 > ( row[ TableUser::Column_id ] );
            }
            if( email == m_useremail )
            {
               m_whichRecordMatched |= MatchingRecord_Email;
               m_userRecordMatchingEmail = boost::lexical_cast< U32 > ( row[ TableUser::Column_id ] );
            }
            if( active == "1" )
            {
               m_userRecordMatchingActive = boost::lexical_cast< U32 > ( row[ TableUser::Column_id ] );
            }
         }
      }
      return true;
   case DiplodocusLogin:: QueryType_LookupTempUserByUsernameOrEmail:
      {
         NewUsersTable              enigma( dbResult->bucket );
         UserTable::iterator        it = enigma.begin();
         m_numPendingUserRecordsMatching  = dbResult->bucket.bucket.size();

         while( it != enigma.end() )
         {
            NewUsersTable::row  row = *it++;
            
            string name =              row[ TableUserTempNewUser::Column_name ];
            string gkHash =            row[ TableUserTempNewUser::Column_gamekit_hash ];
            string email =             row[ TableUserTempNewUser::Column_email ];
            if( name == m_username )
            {
               m_whichRecordMatched |= MatchingRecord_Name;
               m_pendingUserRecordMatchingName = boost::lexical_cast< U32 > ( row[ TableUser::Column_id ] );
            }
            if( email == m_useremail )
            {
               m_whichRecordMatched |= MatchingRecord_Email;
               m_pendingUserRecordMatchingEmail = boost::lexical_cast< U32 > ( row[ TableUser::Column_id ] );
            }
            if( gkHash == m_gamekitHashId )
            {
               m_whichRecordMatched |= MatchingRecord_Gamekithash;
               m_pendingUserRecordMatchingGKHash = boost::lexical_cast< U32 > ( row[ TableUser::Column_id ] );
            }
         }
      }
      return true;
   case DiplodocusLogin:: QueryType_LookupUserNameForInvalidName:
      {
         if( dbResult->bucket.bucket.size() != 0 )
         {
            m_userNameIsInvalid = true;
         }
      }
      return true;
   }

   return false;
}

//---------------------------------------------

bool  CreateAccountResultsAggregator::IsDuplicateRecord() const
{
   if( m_userRecordMatchingEmail || m_userRecordMatchingName )
   {
      return true;
   }
   return false;
} 

//---------------------------------------------

bool     CreateAccountResultsAggregator::ShouldUpdateUserRecord() const 
{
   if( m_userRecordMatchingEmail || m_userRecordMatchingName ) 
   {
      return false;
   }

   if( m_userRecordMatchingGKHash > 0 ) // && m_userRecordMatchingGKHash == m_userRecordMatchingActive )
   {
      return true;
   }

   return false;
}

//---------------------------------------------

bool     CreateAccountResultsAggregator::ShouldUpdatePendingUserRecord() const 
{
   if( m_pendingUserRecordMatchingGKHash || m_pendingUserRecordMatchingEmail || m_pendingUserRecordMatchingName )
   {
      if( m_pendingUserRecordMatchingGKHash == m_pendingUserRecordMatchingEmail && // only when they match do we not update because they already have what the user needs
            m_pendingUserRecordMatchingGKHash == m_pendingUserRecordMatchingName )
         return false;

      return true; /// one is different so update
   }

   return false;
}

//---------------------------------------------

bool     CreateAccountResultsAggregator::HasFoundAnyMatchingRecords() const 
{ 
   if( m_numUserRecordsMatching > 0 || m_numPendingUserRecordsMatching > 0 ) 
      return true; 
   return false;
}  

//---------------------------------------------

bool     CreateAccountResultsAggregator::ShouldInsertNewUserRecord() const
{
   if( HasFoundAnyMatchingRecords() == false )
      return true;

   if( m_numUserRecordsMatching > 0 )
   {
      // if it possible that the gk hash matches but there is not an email set.. we want to create a new record and then update the existing record to clear the gk-hash
      if( m_userRecordMatchingGKHash && m_userRecordMatchingGKHash != m_userRecordMatchingEmail )
         return true;

      // in all other cases, do not create a new account.
      return false;
   }

   if( m_numPendingUserRecordsMatching )
   {
      return false;
     /* if( m_pendingUserRecordMatchingGKHash == m_pendingUserRecordMatchingEmail && // only when they match do we not update because they already have what the user needs
               m_pendingUserRecordMatchingGKHash == m_pendingUserRecordMatchingName )
            return false;*/

   }

   return false;
}

//---------------------------------------------

bool     CreateAccountResultsAggregator::IsMatching_GKHashRecord_DifferentFrom_UserEmail( const string& testEmailAddress ) const 
{
   // m_emailForMatchingRecord_GamekitHashId could be equal to NULL
   if( m_userRecordMatchingGKHash != 0 && 
      testEmailAddress != m_emailForMatchingRecord_GamekitHashId && 
      m_emailForMatchingRecord_GamekitHashId.size() && 
      m_userRecordMatchingGKHash != m_userRecordMatchingEmail )
      return true;

   return false;
}

//////////////////////////////////////////////////////////