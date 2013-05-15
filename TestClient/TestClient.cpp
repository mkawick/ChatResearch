// TestClient.cpp : Defines the entry point for the console application.
//

//#include <winsock2.h>
#include "FruitadensClientChat.h"

#include "../NetworkCommon/Utils/CommandLineParser.h"
#include "../NetworkCommon/Utils/Utils.h"

#include <boost/algorithm/string.hpp>

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

using namespace std;

#include <assert.h>
#include <conio.h>
#define _CRT_SECURE_NO_DEPRECATE
#pragma warning( disable: 4996 )

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

class ConsoleStrategy
{
   enum Strategy
   {
      Strategy_Text,
      Strategy_Login,
      Strategy_ChatAdmin,
      Strategy_HackerMode,
      Strategy_Game
   };
public:
   ConsoleStrategy( FruitadensClientChat& communications, Pyroraptor& pyro ) : 
                     m_strategy( Strategy_Login ), 
                     m_communications( communications ), 
                     m_pyro( pyro ) {}

   void     Run();

private:

   bool     IsExit( const string& text );
   bool     IsInstructions( const string& text );
   bool     ChangeStrategy( const string& text );
   void     PrintInstructions();
   void     OutputCommand( const char* command, const char* instructions, bool isEnabled );
   //bool     GetText( const string& prompt, const string& textBack );


   bool     Login( const string& text );
   bool     Logout( const string& text );//?
   bool     Text( const string& text );
   bool     Chat( const string& text );
   bool     Hacker( const string& text );
   bool     Gameplay( const string& text );

   void     PrintCurrentStrategy();
      //------------------------------------
   void     RunRandomHack( int bufferLength );
   void     RunLoginHack( const string& username );
   void     RunBadStringHack();
   void     RunHeaderHack();
   void     RunRepeatHack( int numPackets );

      //------------------------------------
   Strategy                m_strategy;
   FruitadensClientChat&   m_communications;
   Pyroraptor&             m_pyro;
};

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

int main(int argc, char* argv[])
{
 /*  Pyroraptor pyro;
   for( int i=0; i<16; i++ )
   {
      pyro.SetConsoleColor( i );
      cout << "this is a test of colors " << i << endl;
   }*/

   Pyroraptor pyro;
   pyro.SetConsoleColor( Pyroraptor::ColorsNormal );
   cout << endl << "    Client communications app." << endl << endl;
   cout << "     version 0.04 " << endl;
   cout << " ------------------------------------------- " << endl;
   pyro.SetConsoleColor( Pyroraptor::ColorsNormal );

   CommandLineParser    parser( argc, argv );
   int port = 9600;
   string serverName = "localhost";
   serverName = "chat.mickey.playdekgames.com";

   parser.FindValue( "server.port", port );
   parser.FindValue( "server.name", serverName );


   InitializeSockets();
   FruitadensClientChat fruity;
   fruity.Connect( serverName.c_str(), 9600 );

   
   fruity.AddOutputChain( &pyro );

   int key = 0;
   while( fruity.IsConnected() == false )
   {
      Sleep( 20 );
      int isHit = kbhit();
      if( isHit != 0 )
      {
         key = getch();
         if( key == 27 )
            exit( 1 );
      }
   }
   fruity.FinalFixup();

   ConsoleStrategy strategy( fruity, pyro );
   strategy.Run();

   ShutdownSockets();

   pyro.SetConsoleColor( Pyroraptor::ColorsNormal );
   cout << " ------------------------------------------- " << endl;
   cout << endl << endl;
   cout << endl << "app has exited" << endl << "press any key to exit" << endl;
   getch();

	return 0;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//------------------------------------------------------------------------

bool  GetCommand( string originalText, string& command, char delineator = '/' )
{
	int length = originalText.size();
	int position = originalText.find_first_of( delineator );
	if( position == string::npos )
	{
		return false;
	}
	while( position < length && originalText[ position ] == delineator )// allowing users to enter too many slashes by accident
	{
		position++;
	}

	if( position >= static_cast<int>( originalText.size() ) ) 
	{
		return false;
	}
   int nextPosition = originalText.find_first_of( ' ', position );// go until we find a space. treat that as the channel marker.
	char buffer[MAX_PATH];
	char newText[MAX_PATH];// todo, needs to be much longer
	newText[0] = 0;
	if( nextPosition == string::npos ) 
	{
		int copyLength = length - position;
		originalText.copy( buffer, copyLength, position );
		buffer[ copyLength ] = 0;
	} 
	else 
	{
		int copyLength = nextPosition - position;
		originalText.copy( buffer, nextPosition - position, position );
		buffer[ copyLength ] = 0;

		copyLength = nextPosition - position;
		originalText.copy( newText, length - nextPosition, nextPosition +1 );
		newText[ copyLength ] = 0;
	}
	originalText = newText;
	command = buffer;
   return true;
}

//------------------------------------------------------------------------

void  ConsoleStrategy::Run()
{
   while( 1 )
   {
      m_pyro.SetConsoleColor( Pyroraptor::ColorsNormal );
      PrintCurrentStrategy();
      m_pyro.SetConsoleColor( Pyroraptor::ColorsInstructions );
      PrintInstructions();
      m_pyro.SetConsoleColor( Pyroraptor::ColorsPrompt );
      string username = m_communications.GetUsername();
      if( username.size() > 0 )
      {
         m_pyro.Log( username.c_str(), false );
      }
      string channel = m_communications.GetCurrentChannel();
      if( channel.size() > 0 )
      {
         if( username.size() > 0 )
         {
            m_pyro.Log( ":", false );
         }
         m_pyro.Log( channel.c_str(), false );
      }
      m_pyro.Log( ">", false );
      m_pyro.SetConsoleColor( Pyroraptor::ColorsText );

      std::string    text;
      std::getline( std::cin, text );

      string command;
      if( GetCommand( text, command ) == true )
      {
         bool isExit = IsExit( command );
         if( isExit == true )
         {
            m_communications.Logout();
            break;
         }
         if( IsInstructions( command ) == true )
         {
            //PrintInstructions();
            // the instructions 'auto print'
         }
         else if( ChangeStrategy( command ) == true )
         {
            m_pyro.Log( "  ** mode changed ** " );
         }
         else
         {
            m_pyro.SetConsoleColor( Pyroraptor::ColorsError );
            m_pyro.Log( "Unrecognized command, try again ");
         }
      }
      else
      {
         switch( m_strategy )
         {
         case Strategy_Text:
            Text( text );
            break;
         case Strategy_Login:
            Login( text );
            break;
         case Strategy_ChatAdmin:
            Chat(text );
            break;
         case Strategy_HackerMode:
            Hacker( text );
            break;
         case Strategy_Game:
            Gameplay( text );
            break;
         }
      }
   }
}

//------------------------------------------------------------------------

void  ConsoleStrategy::OutputCommand( const char* command, const char* instructions, bool isEnabled )
{
   if( isEnabled )
   {
      m_pyro.SetConsoleColor( Pyroraptor::ColorsChatText );
   }
   else
   {
      m_pyro.SetConsoleColor( Pyroraptor::ColorsResponseText );
   }
   m_pyro.Log( "     ", false );
   m_pyro.Log( command, false );
   m_pyro.Log( "  ", false );
   if( isEnabled )
   {
      m_pyro.SetConsoleColor( Pyroraptor::ColorsInstructions );
   }
   else
   {
      m_pyro.SetConsoleColor( Pyroraptor::ColorsGrayBlue );
   }
   m_pyro.Log( instructions );
}

//------------------------------------------------------------------------

void     ConsoleStrategy::PrintInstructions()
{
   switch( m_strategy )
   {
   case Strategy_Text:
      OutputCommand( "{/chat=chat admin mode, /game=game mode, /hack=hacker mode, /exit=exit app}", " ", true );
      OutputCommand( "*", "x = changes chat channel to x (e.g. '* stars': channel ->'stars')", true );
      OutputCommand( "groups", "= lists all of your groups", true );
      OutputCommand( "friends", "= lists all of your friends", true );
       break;
   case Strategy_Login:
      OutputCommand( "{/hack=hacker mode, /exit=exit app}", " ", true );
      OutputCommand( " ", "Enter username. Password is not used. ( e.g. >mickey )", true );
      break;
   case Strategy_ChatAdmin:
      OutputCommand( "{/text=normal mode, /game=game mode, /exit=exit app}", " ", true );
      OutputCommand( "delete", "x = delete group", true );
      OutputCommand( "create", "x = create new empty group, conflicts will be ignored (todo)", true );
      OutputCommand( "invite", "x to y = invite a user by uuid (syntax matters) *", false );
      OutputCommand( "add", "x to y = add user to group (syntax matters)", true ); 
      OutputCommand( "remove", "x from y = remove user from group (syntax matters)", true ); 
      OutputCommand( "history", "x = get all chat for a group", true );
      OutputCommand( "list", "= list all group names and Ids", false );
      OutputCommand( "chatters", "x = get all chat contributers for a group", true );
      OutputCommand( "members", "x = get all chat members for a group", true );
      OutputCommand( "reload", "= loads all channels into chat server", true );
      OutputCommand( "all_channels", "= sends complete list of channels to client", true );
      OutputCommand( "all_users", "= sends complete list of users to client", true );
      break;
   case Strategy_HackerMode:
      OutputCommand( "{/text=normal mode, /exit=exit app}", " ", true );
      OutputCommand( "random x", "= random packet of x len. if no x, then random length", true );
      OutputCommand( "login x", "= send login again for x", true );
      OutputCommand( "bad_string", "= send a string packet with bad string size info", true );
      OutputCommand( "header", "= packet with valid header but super large", true );
      OutputCommand( "repeat", "x = repeated chats in a row.. long text", true );
      break;
   case Strategy_Game:
      OutputCommand( "create", "x = create new game, new name will be returned", false );
      OutputCommand( "invite", "x to y = invite a user by uuid (syntax matters)", false );
      OutputCommand( "accept", "x = accept invitation", false );
      //OutputCommand( "add", "x to y = add user to game (syntax matters)", false );
      OutputCommand( "add", "x to y = add user to game (syntax matters)", false );
      OutputCommand( "quit", " current game ", false );
      OutputCommand( "delete", " current game ", false );
      OutputCommand( "list", " list of games ", false );
      // advance the game turn
      // login to exiting game
      break;
   }

   m_pyro.SetConsoleColor( Pyroraptor::ColorsNormal );
   m_pyro.Log( "__________________________________________________" );
}

//------------------------------------------------------------------------

bool     ConsoleStrategy::IsExit( const string& text )
{
   if( boost::iequals( text, "exit" ) == true )// caseless compare
   {
      return true;
   }
   return false;
}

//------------------------------------------------------------------------

bool     ConsoleStrategy::IsInstructions( const string& text )
{
   if( boost::iequals( text, "instructions" ) == true )// caseless compare
   {
      return true;
   }
   if( boost::iequals( text, "commands" ) == true )// caseless compare
   {
      return true;
   }
   if( boost::iequals( text, "options" ) == true )// caseless compare
   {
      return true;
   }
   return false;
}

//------------------------------------------------------------------------

bool     ConsoleStrategy::ChangeStrategy( const string& text )
{
   switch( m_strategy )
   {
   case Strategy_Text:
      {
         if( boost::iequals( text, "chat" ) )
         {
            m_strategy = Strategy_ChatAdmin;
            return true;
         }
         else if( boost::iequals( text, "hack" ) )
         {
            m_strategy = Strategy_HackerMode;
            return true;
         }
         else if( boost::iequals( text, "game" ) )
         {
            m_strategy = Strategy_Game;
            return true;
         }
      // nothing else is aceptable
      }
   case Strategy_Login:// you must login first
         if( boost::iequals( text, "hack" ) )
         {
            m_strategy = Strategy_HackerMode;
            return true;
         }
      break;
   case Strategy_ChatAdmin:
      {
         if( boost::iequals( text, "text" ) )
         {
            m_strategy = Strategy_Text;
            return true;
         }
         else if( boost::iequals( text, "game" ) )
         {
            m_strategy = Strategy_Game;
            return true;
         }
      // nothing else is aceptable
      }
      break;
   case Strategy_HackerMode:
      {
         if( boost::iequals( text, "text" ) )
         {
            m_strategy = Strategy_Text;
            return true;
         }
      // nothing else is aceptable
      }
      break;
   case Strategy_Game:
      {
         if( boost::iequals( text, "text" ) )
         {
            m_strategy = Strategy_Text;
            return true;
         }
         if( boost::iequals( text, "chat" ) )
         {
            m_strategy = Strategy_ChatAdmin;
            return true;
         }
      // nothing else is aceptable
      }
      break;
   }
   return false;
}

//------------------------------------------------------------------------

bool     ConsoleStrategy::Login( const string& username )
{
   string password = "password";
   if( m_communications.Login( username, password ) == false )
   {
      m_pyro.SetConsoleColor( Pyroraptor::ColorsError );
      m_pyro.Log( "Client: Login went badly!" );
      m_pyro.Log( "Try again!" );
   }
   else
   {
      m_strategy = Strategy_Text;
      m_pyro.SetConsoleColor( Pyroraptor::ColorsNormal );
      m_pyro.Log( "Client: login info sent to login server" );
   }
   return false;
}

//------------------------------------------------------------------------

bool     ConsoleStrategy::Logout( const string& text )
{
   return false;
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------

bool     ConsoleStrategy::Text( const string& text )
{
   if( text.size() == 0 )
   {
      return false;
   }

   string chatChannel;
   string test = Trim( text );
   if( test[0] == '*' )
   {
      const string::size_type begin = test.find_first_not_of( '*' );
      if( begin > test.size() )// normally -1 is returned if there is no following text
      {
         m_pyro.SetConsoleColor( Pyroraptor::ColorsError );
         m_pyro.Log( "Cannot change channel.. no name specified" );
         return false;
      }

      chatChannel = test.substr( begin, test.size() + 1 );

      chatChannel = Trim( chatChannel );

      if( m_communications.ChangeChannel( chatChannel ) == false )
      {
         m_pyro.SetConsoleColor( Pyroraptor::ColorsError );
         m_pyro.Log( "Problem changing channel" );
         return false;
      }
      return true;
   }

   //-------------------------------
   vector< string > stringList;
   boost::split( stringList, test, boost::is_any_of("\t "));

   CommandLineParser parse( stringList ) ;
   if( parse.IsKeywordFirst( "groups" ) == true )
   {
      m_pyro.SetConsoleColor( Pyroraptor::ColorsText );
      m_pyro.Log( "Dumping list of groups" );
      m_communications.DumpChatChannels();
      return true;
   }
   if( parse.IsKeywordFirst( "friends" ) == true )
   {
      m_pyro.SetConsoleColor( Pyroraptor::ColorsText );
      m_pyro.Log( "Dumping list of friends" );
      m_communications.DumpFriends();
      return true;
   }

   if( m_communications.SendMessage( text ) == false )
   {
      m_pyro.SetConsoleColor( Pyroraptor::ColorsError );
      m_pyro.Log( "Problem with sending the message" );
      return false;
   }
   return true;
}

//------------------------------------------------------------------------

bool     ConsoleStrategy::Chat( const string& text )
{
   vector< string > stringList;
   boost::split( stringList, text, boost::is_any_of("\t "));

   CommandLineParser parse( stringList ) ;

   string chatChannel, userUuid;
   if( parse.FindValue( "create", chatChannel ) == true )
   {
      if( chatChannel.size() == 0 )
      {
         m_pyro.SetConsoleColor( Pyroraptor::ColorsError );
         m_pyro.Log( "You must provide a chat channel name" );
         return false;
      }
      m_communications.CreateChatChannel( chatChannel );
      return true;
   }
   //--------------------------------
   if( parse.FindValue( "delete", chatChannel ) == true )
   {
      if( chatChannel.size() == 0 )
      {
         m_pyro.SetConsoleColor( Pyroraptor::ColorsError );
         m_pyro.Log( "You must provide a chat channel name" );
         return false;
      }
      string channelUuid = m_communications.FindChatChannel( chatChannel );
      if( channelUuid.size() == 0 )
      {
         m_pyro.SetConsoleColor( Pyroraptor::ColorsError );
         m_pyro.Log( "The chat channel that you provided cannot be found. Try loading all." );
         return false;
      }
      m_communications.DeleteChatChannel( channelUuid );
      return true;
   }
   //--------------------------------
   if( parse.FindValue( "invite", userUuid ) == true )
   {
      if( userUuid.size() == 0 )
      {
         m_pyro.SetConsoleColor( Pyroraptor::ColorsError );
         m_pyro.Log( "You must provide a user uuid" );
         return false;
      }
      parse.FindValue( "to", chatChannel );
      if( chatChannel.size() == 0 )
      {
         m_pyro.SetConsoleColor( Pyroraptor::ColorsError );
         m_pyro.Log( "You must provide a user id and chat channel uuid" );
         return false;
      }
      m_communications.InviteUserToChat( userUuid, chatChannel );
      return true;
   }
   //--------------------------------
   string username;
   if( parse.FindValue( "add", username ) == true )
   {
      if( username.size() == 0 )
      {
         m_pyro.SetConsoleColor( Pyroraptor::ColorsError );
         m_pyro.Log( "You must provide a user name" );
         return false;
      }
      parse.FindValue( "to", chatChannel );
      if( chatChannel.size() == 0 )
      {
         m_pyro.SetConsoleColor( Pyroraptor::ColorsError );
         m_pyro.Log( "You must provide a user name and chat channel uuid" );
         return false;
      }
      string channelUuid = m_communications.FindChatChannel( chatChannel );
      userUuid = m_communications.FindFriend( username );
      if( channelUuid.size() == 0 || userUuid.size() == 0 )
      {
         m_pyro.SetConsoleColor( Pyroraptor::ColorsError );
         m_pyro.Log( "The username or chat channel that you provided cannot be found. Try loading all." );
         return false;
      }
      m_communications.AddUserToChat( userUuid, channelUuid );
      return true;
   }
   //--------------------------------
   if( parse.FindValue( "remove", username ) == true )
   {
      if( username.size() == 0 )
      {
         m_pyro.SetConsoleColor( Pyroraptor::ColorsError );
         m_pyro.Log( "You must provide a user uuid" );
         return false;
      }
      parse.FindValue( "from", chatChannel );
      if( chatChannel.size() == 0 )
      {
         m_pyro.SetConsoleColor( Pyroraptor::ColorsError );
         m_pyro.Log( "You must provide a user id and chat channel name" );
         return false;
      }
      string channelUuid = m_communications.FindChatChannel( chatChannel );
      userUuid = m_communications.FindFriend( username );
      if( channelUuid.size() == 0 || userUuid.size() == 0 )
      {
         m_pyro.SetConsoleColor( Pyroraptor::ColorsError );
         m_pyro.Log( "The username or chat channel that you provided cannot be found. Try loading all." );
         return false;
      }
      m_communications.RemoveUserFromChat( userUuid, channelUuid );
      return true;
   }
   //--------------------------------
   if( parse.FindValue( "history", chatChannel ) == true )
   {
      if( chatChannel.size() == 0 )
      {
         m_pyro.SetConsoleColor( Pyroraptor::ColorsError );
         m_pyro.Log( "You must provide a chat channel uuid" );
         return false;
      }
      string channelUuid = m_communications.FindChatChannel( chatChannel );
      if( channelUuid.size() == 0 )
      {
         m_pyro.SetConsoleColor( Pyroraptor::ColorsError );
         m_pyro.Log( "The chat channel that you provided cannot be found. Try loading all." );
         return false;
      }
      m_communications.RequestChatHistory( channelUuid );
      return true;
   }
   //--------------------------------
   if( parse.FindValue( "list" ) == true )
   {
      m_communications.RequestAllChatChannels( chatChannel );
      return true;
   }
   //--------------------------------
   if( parse.FindValue( "chatters", chatChannel ) == true )
   {
      if( chatChannel.size() == 0 )
      {
         m_pyro.SetConsoleColor( Pyroraptor::ColorsError );
         m_pyro.Log( "You must provide a chat channel name" );
         return false;
      }
      string channelUuid = m_communications.FindChatChannel( chatChannel );
      if( channelUuid.size() == 0 )
      {
         m_pyro.SetConsoleColor( Pyroraptor::ColorsError );
         m_pyro.Log( "The chat channel that you provided cannot be found. Try loading all." );
         return false;
      }
      m_communications.RequestChatters( channelUuid );
      return true;
   }
   //--------------------------------
   if( parse.FindValue( "members", chatChannel ) == true )
   {
      if( chatChannel.size() == 0 )
      {
         m_pyro.SetConsoleColor( Pyroraptor::ColorsError );
         m_pyro.Log( "You must provide a chat channel uuid" );
         return false;
      }
      string channelUuid = m_communications.FindChatChannel( chatChannel );
      if( channelUuid.size() == 0 )
      {
         m_pyro.SetConsoleColor( Pyroraptor::ColorsError );
         m_pyro.Log( "The chat channel that you provided cannot be found. Try loading all." );
         return false;
      }
      m_communications.RequestMembers( channelUuid );
      return true;
   }
   //--------------------------------
   if( parse.FindValue( "reload", chatChannel ) == true )
   {
      m_communications.LoadAllChannels();
      return true;
   }
   //--------------------------------
   if( parse.FindValue( "all_channels", chatChannel ) == true )
   {
      m_communications.RequestFullListOfChatChannels();
      return true;
   }
   //--------------------------------
   if( parse.FindValue( "all_users", chatChannel ) == true )
   {
      m_communications.RequestFullListOfUsers();
      return true;
   }
   return false;
}

//------------------------------------------------------------------------

bool     ConsoleStrategy::Hacker( const string& text )
{
   vector< string > stringList;
   boost::split( stringList, text, boost::is_any_of("\t "));

   CommandLineParser parse( stringList ) ;

   int requestedLength = 0;
   string username;
   if( parse.FindValue( "random", requestedLength ) == true )
   {
      RunRandomHack( requestedLength );
      return true;
   }
   if( parse.FindValue( "login", username ) == true )
   {
      RunLoginHack( username );
      return true;
   }
   if( parse.FindValue( "bad_string", requestedLength ) == true )
   {
      RunBadStringHack();
      return true;
   }
   if( parse.FindValue( "header", requestedLength ) == true )
   {
      RunHeaderHack();
      return true;
   }
   if( parse.FindValue( "repeat", requestedLength ) == true )
   {
      RunRepeatHack( requestedLength );
      return true;
   }

   return false;
}

//------------------------------------------------------------------------

bool     ConsoleStrategy::Gameplay( const string& text )
{
   vector< string > stringList;
   boost::split( stringList, text, boost::is_any_of("\t "));

   CommandLineParser parse( stringList ) ;

   string game;
   //--------------------------------
   if( parse.FindValue( "create", game ) == true )
   {
      if( game.size() == 0 )
      {
         m_pyro.SetConsoleColor( Pyroraptor::ColorsError );
         m_pyro.Log( "You must provide a game name" );
         return false;
      }
   /*   string channelUuid = m_communications.FindChatChannel( chatChannel );
      if( channelUuid.size() == 0 )
      {
         m_pyro.SetConsoleColor( Pyroraptor::ColorsError );
         m_pyro.Log( "The chat channel that you provided cannot be found. Try loading all." );
         return false;
      }
      m_communications.RequestMembers( channelUuid );*/
      m_communications.CreateGame( game );
      return true;
   }
   return false;
}

//------------------------------------------------------------------------

void     ConsoleStrategy::RunRandomHack( int bufferLength )
{
   if( bufferLength == 0 )
   {
      bufferLength = rand() % 1014 + 10; // 10-1024
   }

   U8* buffer = new U8[ bufferLength ];

   m_pyro.SetConsoleColor( Pyroraptor::ColorsText );
   m_pyro.Log( "Random data being sent" );
   m_pyro.Log( "length is ", false );
   m_pyro.SetConsoleColor( Pyroraptor::ColorsNormal );
   m_pyro.Log( bufferLength );

   
   int width = 24;
   cout << hex;
   for( int i=0; i<bufferLength; i++ )
   {
      U8 val = rand()%256;
      buffer[i] = val;
      if( i < 256 )// stop printing after a while 
      {
         if( i>0 && i % 25 == 0 )
         {
            cout << endl;
         }
         if( val < 16 )
            cout << "0";
         cout << (int) val << " ";
      }
      
   }
   cout << dec << endl;
   m_communications.SendRawData( buffer, bufferLength );
   delete [] buffer;

   m_pyro.Log( "Random data has been sent" );
}

//------------------------------------------------------------------------

void     ConsoleStrategy::RunLoginHack( const string& username )
{
   m_pyro.SetConsoleColor( Pyroraptor::ColorsText );
   m_pyro.Log( "Logging in as user ", false );
   m_pyro.Log( username );

   m_communications.Login( username, "password" );

   m_pyro.Log( "Log in complete " );
}

//------------------------------------------------------------------------

void     ConsoleStrategy::RunBadStringHack()
{
   assert( 0 );
}

//------------------------------------------------------------------------

void     ConsoleStrategy::RunHeaderHack()
{
   int bufferLength = 65536;
   U8* buffer = new U8[ bufferLength ];

   PacketFriendsListRequest packet;
   int offset = 0;
   packet.SerializeOut( buffer, offset );

   m_communications.SendRawData( buffer, bufferLength );

   delete buffer;
}

//------------------------------------------------------------------------

void     ConsoleStrategy::RunRepeatHack( int numPackets )
{
   m_pyro.SetConsoleColor( Pyroraptor::ColorsText );
   m_pyro.Log( "Repeat hack for num packets = ", false );
   m_pyro.Log( numPackets );

   string message = "Now is the time for all good men to come to the aid of their country.";
   m_pyro.Log( "Message sent: ", false );

   m_pyro.Log( message );

   for( int i=0; i< numPackets; i++) 
   {
      m_communications.SendMessage( message );
   }
}

//------------------------------------------------------------------------

void     ConsoleStrategy::PrintCurrentStrategy()
{
   switch( m_strategy )
   {
   case Strategy_Text:
      m_pyro.Log( "text_mode" );
      break;
   case Strategy_Login:
      m_pyro.Log( "login_mode" );
      break;
   case Strategy_ChatAdmin:
      m_pyro.Log( "chat_admin_mode" );
      break;
   case Strategy_HackerMode:
      m_pyro.Log( "hacker_mode" );
      break;
   case Strategy_Game:
      m_pyro.Log( "game mode" );
      break;
   }
}

//------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////
