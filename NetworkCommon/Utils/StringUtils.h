// StringUtils.h

#pragma once

#include "../DataTypes.h"
#include <string>
#include <vector>
#include "Utils.h"

std::string          GenerateUUID( U32 xorValue = 0 );
U64                  GenerateUniqueHash( const std::string& str, int maxHexLen = TypicalMaxHexLen );

const std::string    itos(int n);

const std::string    ConvertStringToLower( const std::string& str );
std::string          ToHexString( U32 );
std::string          BufferToHexString( const U8* buffer, int numBytes );

const char*          ConvertToTrueFalseString( bool value ); 
bool                 ConvertToTrueFalse( const std::string& value );

std::string          Trim(const std::string& str, const std::string& whitespace = " \t");


// use case:
// const std::string foo = "    too much\t   \tspace\t\t\t  ";
// std::cout << foo << std::endl;
// std::cout << "[" << reduce(foo) << "]" << std::endl;
// std::cout << "[" << reduce(foo, "-") << "]" << std::endl;
//
// > [too much               space] 
// > [too much space]  
// > [too-much-space]  
std::string          Reduce(const std::string& str,
                         const std::string& fill = " ",
                         const std::string& whitespace = " \t");
unsigned int         split( const std::string& txt, std::vector< std::string >& strs, char ch = ' ');

void                 PrintDebugText( const char* text, int extraCr = 0 );

// losts of string utils
bool                 splitOnFirstFound( std::vector< std::string >& listOfStuff, const std::string& text, const char* delimiter = "=:" );
bool                 ParseListOfItems( std::vector< std::string >& listOfStuff, std::string text, const char* delimiter = "=:", const char* charsToRemove = NULL );
std::string          RemoveEnds( std::string s, const char* charsToStrip = "\"\'" );

void                 DumpBuffer( U8* buffer, int offset, int length );

template < class T >
bool ConvertFromString( const std::string& InputString, T& Value )
{
   Value = 0;
   //T tempValue = 0;
   int numConverted = 0;
   bool makeNegative = false;
   for(std::string::const_iterator it = InputString.begin() ; it < InputString.end(); ++it)
   {
      char c = *it;
      if( c>= '0' && c<='9' )
      {
         Value = Value * 10 + (c - '0' );
         numConverted ++;
      }
      else if( c == '-' && numConverted == 0 )
      {
         makeNegative = true;
      }
      else
      {
         return false;
      }
   }
   if( makeNegative == true )
   {
      Value *= -1;
   }
   return true;
}

template < class T >
bool ConvertToString( T value, std::string& InputString, int radix = 10 )
{
#ifdef _DEBUG
   T debugvalue=value;
#endif
   

   InputString.clear();
   if( value == 0 )
   {
      InputString = "0";
      return true;
   }
   bool positive = true;
   //if( boost::is_signed( value ) )
   //if( std::is_signed( value ) )
   {
      if( value < 0 )
      {
         positive = false;
      }
   }
   if( radix == 10 )
   {
      while( value != 0 )
      {
         char ins[] = {(char)( value % radix ),0};
         if( ins[0] < 0 )
         {
            ins[0] *= -1;
         }
         ins[0] += '0';
         std::string addition = ins;
         InputString += addition;
         value /= radix;
      }
   }
   else if( radix == 16 )
   {
      char lookup [] = "ABCDEF";
      while( value != 0 )
      {
         char ins[] = {(char)( value % radix ), 0};
         if( ins[0] < 0 )
         {
            ins[0] *= -1;
         }
         if( ins[0] < 10 )
         {
            ins[0] += '0';
         }
         else
         {
            ins[0] -= 10;
            ins[0] = lookup[ (int) ins[0] ];// cast for compiler warning only
         }
         
         std::string addition = ins;
         InputString += addition;
         
         value /= radix;
      }
   }

   if( positive == false )
   {
      const char* dash = "-";
      InputString.insert( (int)0, dash );
   }

   return true;
}