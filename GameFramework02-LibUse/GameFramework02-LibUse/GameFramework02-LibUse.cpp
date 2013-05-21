// GameFramework02-LibUse.cpp : Defines the entry point for the console application.
//

#include "..\..\Mber\ServerStack\BaseGameLibrary\GameFramework.h"
#include <iostream>
#include <conio.h>
using namespace std;
#pragma warning ( disable: 4996 )


///////////////////////////////////////////////////////////////////////////

class MyCallbackObj : public GameCallbacks
{
public:
   MyCallbackObj( GameFramework& game ) : GameCallbacks( game ) {}

   bool   UserConnected( const UserInfo* info, U32 connectionId ) 
   { 
      return false; 
   }

   bool   UserDisconnected( U32 connectionId ) 
   { 
      return false; 
   }

   bool   DataFromClient( U32 connectionId, const MarshalledData* packet )  
   { 
      m_game.SendGameData( connectionId, packet );
      return false; 
   }

   bool   UserConfirmedToOwnThisProduct( U32 connectionId, bool isConfirmed )
   { 
      if( isConfirmed == true )
      {
         m_game.InformClientWhoThisServerIs( connectionId );
      }

      return false; 
   }
};

//-------------------------------------------------------------------------

int main( int argc, const char* argv[] )
{
   GameFramework game( "Lords or Waterdeep", "low", "0.05" );

   game.SetupDefaultChatConnection( "localhost", 9602 );
   game.SetupDefaultS2S( "localhost", 24600 );

   game.UseCommandlineOverrides( argc, argv );

   game.SetDatabaseIdentification( "6af91cb05e53f915" );

   MyCallbackObj  callbacks( game );
   game.RegisterForIncomingData( &callbacks );
   if( game.IsSetupProperly() == true )
   {
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