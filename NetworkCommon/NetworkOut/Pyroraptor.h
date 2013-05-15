// Pyroraptor.h

#pragma once

#include "../ChainedArchitecture/ChainedInterface.h"

class BasePacket;

/*namespace std
{
   class string;
}*/

class Pyroraptor : public ChainedInterface < BasePacket* >
{
public:
   
   enum Colors
   {
      ColorsText = 15, // white
      ColorsChatText = 14, // bright yellow
      ColorsFuschia = 13,
      ColorsError = 12, // bright red
      ColorsPrompt = 11, // cyan
      ColorsGreen = 10,
      ColorsBlue = 9,

      ColorsGray = 8,
      ColorsInstructions = 7, // light gray
      
      ColorsResponseText = 6, // dark yellow
      ColorsUsername = 5, // dark fuschia .. purple
      ColorsDarkRed = 4,
      ColorsGrayBlue = 3,
      ColorsNormal = 2, // green
      ColorsDarkBlue = 1,
      ColorsBlack = 0
   };
public:
   Pyroraptor();
   ~Pyroraptor();

   bool  Log( const char* text, bool endOfLine = true );
   bool  Log( const std::string& str, bool endOfLine = true );
   bool  Log( int, bool endOfLine = true );
   bool  Log( unsigned int, bool endOfLine = true );
   bool  Log( bool, bool endOfLine = true );
   void  SetConsoleColor( int color );
};
