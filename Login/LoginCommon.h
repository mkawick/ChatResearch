// LoginCommon.h
#pragma once

#include "../NetworkCommon/DataTypes.h"

// the following is organized to bit pack well.. 
// there will be a lot of these objects per user, and most will be empty
#pragma pack (push, one_byte_pack,  1 )

//////////////////////////////////////////////////////////////////////////

struct UserLogin_ConnectionProductDetails
{
   UserLogin_ConnectionProductDetails() :  
            connectionId( 0 ),
            gatewayId( 0 ),
            gameProductId( 0 ),
            didLogin( false ),
            loginState( LoginState_NeverLoggedIn ),
            attemptedLoginCount( 0 ),
           /* isLoggingOut( false ),
            isReadyToBeCleanedUp( false ),*/
            loginTime( 0 ),
            logoutTime( 0 )
{
}

public: 

   void  ClearConnection() { connectionId = 0, gatewayId = 0; didLogin = false; loginState = LoginState_NeverLoggedIn; }
   bool  IncreaseLoginAttemptCount() { attemptedLoginCount ++; if( attemptedLoginCount > 3 ) return false; return true; }
   void  ClearLoginAttemptCount() { attemptedLoginCount  = 0; }

   enum LoginState
   {
      LoginState_NeverLoggedIn,
      LoginState_BeginningLogin,
      LoginState_IsLoggedIn,
      LoginState_IsLoggingOut,
      Loginstate_IsLoggedOut
   };

   U32         connectionId;
   U32         gatewayId;

   U8          gameProductId;
   bool        didLogin;
   LoginState  loginState;

   int         attemptedLoginCount;

   time_t      loginTime;
   time_t      logoutTime;
   time_t      lastLoggedInTime;

};

typedef UserLogin_ConnectionProductDetails LoginConnectionDetails;

//////////////////////////////////////////////////////////////////////////

#pragma pack( pop, one_byte_pack )