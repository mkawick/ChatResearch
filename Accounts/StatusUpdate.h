#pragma once

#include "../NetworkCommon/Utils/Utils.h"
#include "../NetworkCommon/Packets/BasePacket.h"
#include "../NetworkCommon/DataTypes.h"
#include "../NetworkCommon/ServerType.h"
#include "../NetworkCommon/ChainedArchitecture/ChainedThread.h"
#include "BlankUUIDQueryHandler.h"
#include "BlankUserProfileHandler.h"
#include "NewAccountQueryHandler.h"
#include "ProductEntryCreateBasedOnPlayHistory.h"
#include "ResetPasswordQueryHandler.h"
#include <map>

const int DefaultSleepTime = 30;


class PacketDbQuery;
class PacketDbQueryResult;


//-----------------------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////

class StatusUpdate : public Queryer
{
public:
   enum QueryType 
   {
      QueryType_UserCreateTempAccount = 1,
      QueryType_UserCheckForNewAccount,
      QueryType_UserFindBlankUUID,
      QueryType_UserFindBlankUserProfile,
      QueryType_UserUpdateNewAccountKeyBeforeSendingEmail,
      QueryType_LoadStrings,
      QueryType_LoadWeblinks,
      QueryType_AutoCreateUsers,
      QueryType_AutoCreateUsersInsertOrUpdate,
      QueryType_DeleteTempNewUserRecord,
      QueryType_ResendEmailToOlderAccounts,
      QueryType_MoveOlderAccountsRequest,
      QueryType_DeleteOlderAccountsRequest,
      QueryType_ResetPasswords,
      QueryType_DuplicateUUIDSearch,
      QueryType_LoadProductIds,
      //QueryType_FindEarliestPlayDateForProduct,
      QueryType_DoesUserHaveProductPurchase,
      QueryType_ProductEntryCreateBasedOnPlayHistory,
      QueryType_Hack
   };

public:
   StatusUpdate( const string& serverName, U32 serverId );
   ~StatusUpdate();

   bool     AddInputChainData( BasePacket* packet, U32 connectionId );
   bool     AddOutputChainData( BasePacket* packet, U32 connectionId );
   void     DuplicateUUIDSearch();

   void     EnableAddingUserProducts( bool enable ) { m_enableAddingUserProducts = enable; }
   

private:

   int      CallbackFunction();
   bool     AddQueryToOutput( PacketDbQuery* packet );

    //---------------------------------------------------------------

   


   /// the following is all related to email services
 /*  void     PreloadLanguageStrings();
   void     SaveStrings( const PacketDbQueryResult* dbResult );
   string   GetString( const string& stringName, int languageId );


   bool                 m_hasLoadedStringTable;
   StringTableLookup    m_stringsTable;
   map< stringhash, stringhash > m_replacemetStringsLookup;
   typedef pair< stringhash, stringhash > ReplacementPair;
   string   m_pathToConfirmationEmailFile;
   string   m_confirmationEmailTemplate;
   string   m_passwordResetEmailTemplate;
   void     ReplaceAllLookupStrings( string& bodyText, int languageId,  map< string, string >& specialStrings );

   void     PreloadWeblinks();
   void     HandleWeblinks( const PacketDbQueryResult* dbResult );
   string   m_linkToAccountCreated;
   string   m_linkToResetPasswordConfirm;
   bool     m_hasLoadedWeblinks;*/

   // TO BE REMOVED (once the login server does not rely on other services updating the new account creation)

   //void     CheckForNewAccounts();
   //void     HandleNewAccounts( const PacketDbQueryResult* dbResult );

   void     LookForFlaggedAutoCreateAccounts();
   void     HandleAutoCreateAccounts( const PacketDbQueryResult* dbResult );

   time_t   m_newAccountCreationTimer;
   int      m_newAccountTimeoutSeconds;
   time_t   m_checkOnautoCreateTimer;
   int      m_checkOnautoCreateTimeoutSeconds;

   time_t   m_checkOnOldEmailsTimer;
   int      m_checkOnOldEmailsTimeoutSeconds;

   time_t   m_expireOldAccountRequestsTimer;
   int      m_expireOldAccountRequestsTimeoutSeconds;

   bool     m_enableAddingUserProducts;

   //time_t   m_resetPasswordEmailTimer;
   //int      m_resetPasswordEmailTimeoutSeconds;


   void     ResendEmailToOlderAccounts();
   void     ResendEmailToOlderAccountsResult( PacketDbQueryResult* dbResult );

   void     Hack();
   void     HackResult( PacketDbQueryResult* dbResult );

   void     ExpireOldUserAccountRequests();
   
   void     DuplicateUUIDSearchResult( PacketDbQueryResult* dbResult );

 /*  void     CheckForResetPassword();
   void     HandleResetPassword( const PacketDbQueryResult* dbResult );*/

   BlankUUIDQueryHandler* m_blankUuidHandler;
   BlankUserProfileHandler* m_blankUserProfileHandler;
   NewAccountQueryHandler* m_newAccountHandler;
   ProductEntryCreateBasedOnPlayHistory* m_addProductEntryHandler;
   ResetPasswordQueryHandler* m_resetPasswordHandler;

   static const U32 timeoutBlankUserProfileTimer = 60;
   static const U32 timeoutBlankUUIDTimer = 60;
   static const U32 timeoutNewAccount = 48;
   static const U32 timeoutAddProductEntry = 45;
   static const U32 timeoutResetPassword = 30;
};
