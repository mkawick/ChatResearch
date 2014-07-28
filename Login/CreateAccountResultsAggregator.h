// CreateAccountResultsAggregator.h

#pragma once

#include <string>
using namespace std;

#include "../NetworkCommon/DataTypes.h"

class PacketDbQueryResult;

//////////////////////////////////////////////////////////

class CreateAccountResultsAggregator
{
public:
   enum     MatchingRecord
   {
      MatchingRecord_None,
      MatchingRecord_Name = 1,
      MatchingRecord_Email = 2,
      MatchingRecord_Gamekithash = 4
   };

   CreateAccountResultsAggregator( U32 _connectionId, const string& email, const string& _password, const string& _username, const string& _gamekitHashId, 
                                 const string& _deviceId, U8 _languageId, U8 _gameProductId, int numQueriesToComplete = 3 ) : 
         m_numQueriesToAggregate( numQueriesToComplete ), 
         m_numUserRecordsMatching( 0 ), 
         m_numPendingUserRecordsMatching( 0 ),
         m_connectionId( _connectionId ), 
         m_useremail( email ), 
         m_password( _password ), 
         m_username( _username ), 
         m_gamekitHashId( _gamekitHashId ), 
         m_deviceId( _deviceId ), 
         m_gameProductId( _gameProductId ),
         m_languageId( _languageId ),
         m_userNameIsInvalid( false ),
         m_whichRecordMatched( MatchingRecord_None ),
         m_userRecordMatchingGKHash( 0 ),
         m_userRecordMatchingName( 0 ),
         m_userRecordMatchingEmail( 0 ),
         m_userRecordMatchingActive( 0 ),
         m_pendingUserRecordMatchingGKHash( 0 ),
         m_pendingUserRecordMatchingName( 0 ),
         m_pendingUserRecordMatchingEmail( 0 )
         {}

   bool     HandleResult( const PacketDbQueryResult* dbResult );
   U32      GetConnectionId() const { return m_connectionId; }
   bool     IsComplete() const;
   bool     HasFoundAnyMatchingRecords() const;
   bool     HasFoundMatchingGKHashInUserTable() const;
   bool     HasMatchingGamekitHash() const { if( m_whichRecordMatched == MatchingRecord_Gamekithash ) return true; return false; }

   bool     GetMatchingRecordType( MatchingRecord recordType ) const { return (m_whichRecordMatched & recordType )? true: false; }
   //U32      Get

   // decision making
   bool     IsDuplicateUsernameOrEmailRecord() const;
   bool     ShouldUpdateUserRecord() const;
   bool     DoesGKPendingMatch_ButUsernameOrEmailAreNotSameRecord() const;
   bool     ShouldUpdatePendingUserRecord() const;
   bool     ShouldInsertNewUserRecord() const;

   bool     IsMatching_GKHashRecord_DifferentFrom_UserEmail( const string& testEmail ) const;
   
public:
   int            m_numQueriesToAggregate; // when it drops to 0, we are done.
   int            m_numUserRecordsMatching;
   int            m_numPendingUserRecordsMatching;
   U32            m_connectionId;
   string         m_useremail;
   string         m_password;
   string         m_username;
   string         m_gamekitHashId;
   string         m_deviceId;
   U8             m_gameProductId;
   U8             m_languageId;
   bool           m_userNameIsInvalid;
   
public:// we don't need accessors for all these
   U32            m_whichRecordMatched;

   U32            m_userRecordMatchingGKHash;
   U32            m_userRecordMatchingName;
   U32            m_userRecordMatchingEmail;
   U32            m_userRecordMatchingActive;

   U32            m_pendingUserRecordMatchingGKHash;
   U32            m_pendingUserRecordMatchingName;
   U32            m_pendingUserRecordMatchingEmail;

   string         m_emailForMatchingRecord_GamekitHashId;
};

//////////////////////////////////////////////////////////