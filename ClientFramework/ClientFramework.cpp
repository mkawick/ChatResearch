// ClientFramework.cpp : Defines the entry point for the console application.
//

#include "NetworkLayer.h"
#include <conio.h>
#pragma warning (disable: 4996)

//////////////////////////////////////////////////////////////////////////////////

class Notifications : public UserNetworkEventNotifier
{
   void  UserLogin( bool success ) {};
   void  UserLogout() {};

   void  UserDemographics( const string& username, const Demographics& userDemographics ) {};
   void  UserWinLoss( const string& username, const WinLoss& userWinLoss ) {};

   void  GameData( U16 length, const U8* const rawdata ) {};
};

//////////////////////////////////////////////////////////////////////////////////


int main( int argc, const char* argv[] )
{

   Notifications notify;

   NetworkLayer network;
   network.Init();

   network.RegisterCallbackInterface( & notify );

   //.. do stuff

   network.RequestLogin( "mickey", "password" );


   getch();
   network.Exit();

   
	return 0;
}

//////////////////////////////////////////////////////////////////////////////////
