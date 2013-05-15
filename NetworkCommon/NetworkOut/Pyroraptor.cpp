// Pyroraptor.cpp

#include "Pyroraptor.h"

#include <string>
#include <boost/lexical_cast.hpp>
#include <iostream>
using namespace std;

Pyroraptor::Pyroraptor() : ChainedInterface< BasePacket* >()
{
}

Pyroraptor::~Pyroraptor()
{
}

bool Pyroraptor :: Log( int value, bool endOfLine )
{
   Log( boost::lexical_cast<string>( value ), endOfLine );
   return true;
}

bool  Pyroraptor :: Log( unsigned int value, bool endOfLine )
{
   Log( boost::lexical_cast<string>( value ), endOfLine );
   return true;
}

bool  Pyroraptor :: Log( bool value, bool endOfLine )
{
   cout << std::boolalpha << value << std::noboolalpha;
   if( endOfLine )
       cout << endl;
   return true;
}

bool  Pyroraptor :: Log( const char* text, bool endOfLine )
{
   cout << text;
   if( endOfLine )
       cout << endl;
   return true;
}

bool  Pyroraptor :: Log( const string& str, bool endOfLine ) 
{ 
   return Log(str.c_str(), endOfLine); 
}


void  Pyroraptor :: SetConsoleColor( int color )
{
   // change the text color
   HANDLE hConsole;
   hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
   SetConsoleTextAttribute( hConsole, color );
}

