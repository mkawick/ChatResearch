// ConnectionToUser.h

#include "../NetworkCommon/DataTypes.h"
#include "ProductInfo.h"
#include <string>
#include <vector>
#include <map>
using namespace std; 

//////////////////////////////////////////////////////////////////////////

struct ConnectionToUser
{
   enum LoginStatus
   {
      LoginStatus_Pending,
      LoginStatus_Invalid,
      LoginStatus_LoggedIn,
      LoginStatus_Hacker
   };

   ConnectionToUser( const string& name, const string& pword, const string& key );

   void  AddProductFilterName( const string& text );
   int   FindProductFilterName( const string& text ); 

   //----------------------------------
   string                  id;
   string                  username;
   string                  passwordHash;
   string                  email;
   string                  userUuid;
   string                  loginKey;
   string                  lastLoginTime;
   //U32                  

   vector< string >        productFilterNames;
   vector< ProductInfo >   productsWaitingForInsertionToDb;
   LoginStatus             status;
   U8                      gameProductId;
   bool                    active;
   time_t                  loggedOutTime;

   int                     adminLevel;
   bool                    showWinLossRecord;
   bool                    marketingOptOut;
   bool                    showGenderProfile;
   
   map< U32, ConnectionToUser> adminUserData; // if you are logged in as an admin, you may be querying other users.

   
};

//////////////////////////////////////////////////////////////////////////
