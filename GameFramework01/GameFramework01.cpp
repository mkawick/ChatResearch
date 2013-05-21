// GameFramework01.cpp : Defines the entry point for the console application.
//

#include "GameFramework.h"
#include <iostream>
#include <conio.h>
using namespace std;
#pragma warning ( disable: 4996 )

///////////////////////////////////////////////////////////////////////////

class MyCallbackObj : public GameCallbacks
{
public:
   bool   UserConnected( const UserInfo* info, U32 connectionId ) 
   { 
      return false; 
   }

   bool   UserDisconnected( U32 connectionId ) 
   { 
      return false; 
   }

   bool   UserInput( U32 connectionId, const MarshalledData* packet )  
   { 
      return false; 
   }

   bool   CommandFromOtherServer( const BasePacket* instructions ) 
   { 
      return false; 
   }
};

//-------------------------------------------------------------------------

int main( int argc, const char* argv[] )
{
   GameFramework game( "low" );

   game.SetupDefaultChatConnection( "localhost", 9602 );

   game.UseCommandlineOverrides( argc, argv );

   if( game.IsSetupProperly() == true )
   {
      MyCallbackObj callbacks;
      game.RegisterForIncomingData( &callbacks );
      game.Run();
   }
   else
   {
      cout << "Errors in setup " << endl;
      getch();
   }

	return 0;
}

///////////////////////////////////////////////////////////////////////////
