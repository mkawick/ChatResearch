
#include "JSON.h"
#include <algorithm>
#include <iostream>
using namespace std;
using namespace JSON;
#pragma warning (disable: 4996 )

const int MAX_StringLength = 256;

bool	JSON::ParseOut( const string& input, const string& valueName, string& value )
{
	string namePlusColon = valueName;
	namePlusColon += ":";
	unsigned found = input.find( namePlusColon );
	if ( found == std::string::npos )
	{
		return false;// protocol demands a packet type
	}
	found += namePlusColon.size();// point to just after the string name
	while( input[found] == ' ' )
	{
		found ++;
	}
	int endOfValueName = input.find_first_of( ", }", found );// end of the string.

	char buffer[ MAX_StringLength ];
	int length = endOfValueName - found;
	input.copy( buffer, length, found );

	buffer[ length ] = 0;
	value = buffer;
	return true;
}

bool  JSON::Open( string& newMessage )
{
   if( newMessage[0] == '{' )// minimal validation
      return false;

   newMessage += '{';

   return true;
}

bool  JSON::Close( string& newMessage )
{
   int len = newMessage.size();
   if( len == 0 ||
      newMessage[0] == '}' )// minimal validation
      return false;

   newMessage += '}';

   return true;
}

bool  JSON::AddValue( string& newMessage, const string& name, int value, bool isContinued )
{
   newMessage += " ";
   newMessage += name;
   newMessage += ":";
   newMessage += value;
   if( isContinued )
      newMessage += ", ";

   return true;
}

bool  JSON::AddValue( string& newMessage, const string& name, const string& value, bool isContinued )
{
   newMessage += " ";
   newMessage += name;
   newMessage += ":'";
   newMessage += value;
   if( isContinued )
      newMessage += "', ";
   else
      newMessage += "' ";

   return true;
}

bool  JSON::AddValue( string& newMessage, const string& name, bool value, bool isContinued )
{
   newMessage += " ";
   newMessage += name;
   newMessage += ":";
   if( value == true )
      newMessage += "true";
   else if( value == false )
      newMessage += "false";
   if( isContinued )
      newMessage += ", ";

   return true;
}
