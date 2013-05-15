#pragma once

#include <string>
using namespace std;

// json

namespace JSON
{
   bool	ParseOut( const string& input, const string& valueName, string& value );


   bool  Open( string& newMessage );
   bool  Close( string& newMessage );
   bool  AddValue( string& newMessage, const string& name, int value, bool isContinued = false );
   bool  AddValue( string& newMessage, const string& name, const string& value, bool isContinued = false );
   bool  AddValue( string& newMessage, const string& name, bool value, bool isContinued = false );
   //bool  JSON_AddValue( string& newMessage, ... , bool isContinued = false );
};

// json