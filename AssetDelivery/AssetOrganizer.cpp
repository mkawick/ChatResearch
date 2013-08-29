
// AssetOrganizer.cpp 
//

#include <stdio.h>
#include <direct.h>
#include <windows.h>
#include <conio.h>

#include <iostream>
#include <fstream> 
#include <string>
using namespace std;

#include <boost/format.hpp>
#include <boost/tokenizer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time.hpp>
using namespace boost::posix_time;
namespace bt = boost::posix_time;
#pragma warning (disable: 4996)

#include "../NetworkCommon/DataTypes.h"
#include "AssetOrganizer.h"

#include "../NetworkCommon/Utils/Utils.h"

// found here:
//http://stackoverflow.com/questions/3786201/parsing-of-date-time-from-string-boost


AssetOrganizer::AssetOrganizer() : m_isInitializedProperly( false )
{
}

AssetOrganizer::~AssetOrganizer()
{
}


string ConvertStringToStandardTimeFormat(const std::string& timeString )
{
   const char* timeFormats [] = 
   {
      "%Y/%m/%d %H:%M:%S",
      "%Y/%m/%d %H:%M:%S",
      "%d.%m.%Y %H:%M:%S",
      "%Y-%m-%d",
      "%d-%m-%Y"
   };

   const size_t timeFormatsCount = sizeof(timeFormats)/sizeof(timeFormats[0]);

   bt::ptime pt;
   bool success = false;
   for(size_t i=0; i<timeFormatsCount; ++i)
   {
      std::istringstream is( timeString );
      is.imbue( std::locale(std::locale::classic(), new bt::time_input_facet( timeFormats[i] )) );
      is >> pt;
      if(pt != bt::ptime()) 
      {
         success = true;
         break;
      }
   }

   if( success )
   {
      return boost::posix_time::to_simple_string( pt );
   }

   return string();
}

//////////////////////////////////////////////////////////////////////////

const char* productNames [] = {
   "",
   "ascension",
   "dominion",
   "thunderstone",
   "wowcmg",
   "summonwar",
   "foodfight",
   "nightfall",
   "pennyarcade",
   "infinitecity",
   "agricola",
   "fluxx",
   "smashup"
};

const char* platformStrings[] = {
   "",
   "ios",
   "android",
   "pc",
   "mac",
   "blackberry"
};

//////////////////////////////////////////////////////////////////////////

AssetDefinition:: AssetDefinition(): productId( 0 ), platform( 0 ), fileData( NULL ), fileSize( 0 ), compressionType( 0 )
{
}

AssetDefinition::~AssetDefinition()
{
   delete [] fileData;
}

bool  AssetDefinition:: LoadFile()
{
   ifstream file ( path.c_str(), ios::in|ios::binary|ios::ate);
   if (file.is_open())
   {
      fileSize = file.tellg();
      fileData = new U8 [fileSize];
      file.seekg (0, ios::beg);
      file.read ( reinterpret_cast< char* >( fileData ), fileSize);
      file.close();
   }
   else
   {
      return false;
   }

   if( name.size() > 0 )
   {
      ConvertToString( GenerateUniqueHash( name ), hash );
   }
   else
   {
      ConvertToString( GenerateUniqueHash( path ), hash );
   }
   return true;
}

bool  AssetDefinition:: IsDefinitionComplete()
{
   if( productId == 0 || path.size() == 0 )
      return false;
   return true;
}

void  AssetDefinition:: Print()
{
   cout << "[asset] " << endl;
   const char* formatString = "%-15s%-15s";
   cout << boost::format( formatString ) % "Name: " % name << endl;
   cout << boost::format( formatString ) % "path: " % path << endl;
   cout << boost::format( formatString ) % "version: " % version << endl;
   if( beginTime.size() )
   {
      cout << boost::format( formatString ) % "beginTime: " % beginTime << endl ;
   }
   if( endTime.size() )
   {
      cout << boost::format( formatString ) % "endTime: " % endTime << endl ;
   }
   cout << boost::format( formatString ) % "productId: " % productNames[ productId ] << endl ;
   cout << boost::format( formatString ) % "platform: " % platformStrings[ platform ]<< endl ;

   cout << endl;
}

//////////////////////////////////////////////////////////////////////////

int   FindProductName( const string& value )
{
   int numProductNames = sizeof( productNames ) / sizeof( productNames[0] );
   for( int i=0; i< numProductNames; i++ )
   {
      if( value == productNames[i] )
      {
         return i;
      }
   }
   return 0;
}

//////////////////////////////////////////////////////////////////////////

int   FindPlatformName( const string& value )
{
   int numPlatformNames = sizeof( platformStrings ) / sizeof( platformStrings[0] );
   for( int i=0; i< numPlatformNames; i++ )
   {
      if( value == platformStrings[i] )
      {
         return i;
      }
   }
   return 0;
}

//////////////////////////////////////////////////////////////////////////

bool  FillInAsset( string& line, AssetDefinition& asset )
{
   vector< string > listOfStuff;

   string separator1("");//dont let quoted arguments escape themselves
   string separator2("=:");//split on = and :
   string separator3("\"\'");//let it have quoted arguments


   boost::escaped_list_separator<char> els( separator1, separator2, separator3 );
   boost::tokenizer<boost::escaped_list_separator<char> > tokens( line, els );

   for (boost::tokenizer<boost::escaped_list_separator<char> >::iterator i(tokens.begin());
      i!=tokens.end();++i) 
   {
      listOfStuff.push_back(*i);
   }

   if( listOfStuff.size() != 0 )
   {
      const string& potentionalKey = ConvertStringToLower( listOfStuff[ 0 ] );
      const string& value = ConvertStringToLower( listOfStuff[ 1 ] );
      if( listOfStuff.size() == 2 )// simplest case
      {
         if( potentionalKey == "product" )
         {
            asset.productId = FindProductName( value );
            if( asset.productId != 0 ) // bad id
               return true;
         }
         else if( potentionalKey == "platform" )
         {
            asset.platform = FindPlatformName( value );
            if( asset.platform != 0 ) // bad id
               return true;
         }
         else if( potentionalKey == "version" )
         {
            asset.version = value;
            return true;
         }
         else if( potentionalKey == "path" )
         {
            asset.path = value;
            FILE* test = fopen( asset.path.c_str(), "rb" );
            if( test != NULL )
            {
               fclose( test );
               return true;
            }
            else
            {
               cout << "Invalid file :  " << asset.path << endl;
            }
         }
         else if( potentionalKey == "begintime" ) 
         {
            asset.beginTime = ConvertStringToStandardTimeFormat( value );
            return true;
         }
         else if( potentionalKey == "endtime" ) 
         {
            asset.endTime = ConvertStringToStandardTimeFormat( value );
            return true;
         }
         else if( potentionalKey == "payload" )
         {
            asset.payload = value;
            return true;
         }
         else if( potentionalKey == "name" )
         {
            asset.name = value;
            return true;
         }
         else
         {
            return false; // no other usable keys
         }
      }
      else
      {
         return false; // no support for one or three things on a line.
      }
   }

   return false;
}

//////////////////////////////////////////////////////////////////////////

bool  AssetOrganizer::ParseNextAsset( ifstream& infile, int& lineCount )
{
   string line;
   AssetDefinition asset;
   while (std::getline(infile, line))
   {
      lineCount++;
      bool result = FillInAsset( line, asset );
      if( result == false )
         return false;

      if( asset.IsDefinitionComplete() == true )// we will skip right over version and platform so be careful
      {
         //asset.Print();
         m_assets.push_back( asset );
         return true;
      }
      
   }
   return false;
}

//////////////////////////////////////////////////////////////////////////

string ExePath() {
    char buffer[MAX_PATH];
    GetCurrentDirectoryA( MAX_PATH, buffer );
    string::size_type pos = string( buffer ).find_last_of( "\\/" );
    return string( buffer ).substr( 0, pos);
}

//////////////////////////////////////////////////////////////////////////

bool  AssetOrganizer::Init( const string& assetManifestFile )
{
   m_isInitializedProperly = false;

   ifstream infile( assetManifestFile.c_str() );
   string line;
   if (!infile) 
   { 
      std::cerr << "Error opening file!\n"; 
      return 1; 
   }

   bool errorCode = false;
   int lineCount = 0;
   while( std::getline( infile, line ) )// could be spaces, etc
   {
      lineCount ++;
      if( line == "[asset]" )
      {
         bool result = ParseNextAsset( infile, lineCount );
         if( result == false )
         {
            cout << "Error in asset file reading line " << lineCount << endl;
            errorCode = true;
            break;
         }
      }
      
   }

   if( errorCode == false )
   {
      if( m_assets.size() > 0 )
      {
         AssetVectorIterator it = m_assets.begin();
         while( it != m_assets.end() )
         {
            (*it).Print();
            it++;
         }
         m_isInitializedProperly = LoadAllFiles();
      }
   }

	return m_isInitializedProperly;
}

//////////////////////////////////////////////////////////////////////////

bool  AssetOrganizer::GetListOfAssets( U8 productId, vector< string >& listOfAssetsByHash ) const
{
   listOfAssetsByHash.clear();

   // do we need to organize the assets by product id? There probably won't be enough but we can group them by product if we end up with a lot of assets.
   vector< AssetDefinition >::const_iterator it = m_assets.begin();
   while( it != m_assets.end() )
   {
      if( it->productId == productId )
      {
         listOfAssetsByHash.push_back( it->hash );
      }
      it++;
   }

   if( listOfAssetsByHash.size() > 0 )
      return true;
   return false;
}


//////////////////////////////////////////////////////////////////////////

bool  AssetOrganizer::FindByHash( const string& hash, AssetDefinition const *& asset ) const
{
   asset = NULL;

   vector< AssetDefinition >::const_iterator it = m_assets.begin();
   while( it != m_assets.end() )
   {
      if( it->hash == hash )
      {
         asset = & (*it );// yes, grab the address of the instance
         return true;
      }
      it++;
   }
   return false;
}

//////////////////////////////////////////////////////////////////////////

bool  AssetOrganizer::LoadAllFiles()
{
   vector< AssetDefinition >::iterator it = m_assets.begin();
   while( it != m_assets.end() )
   {
      if( it->LoadFile() == false )
      {
         cout << "Critical failure loading asset : " << it->path << endl;
         return false;
      }

      it++;
   }
   return true;
}

//////////////////////////////////////////////////////////////////////////
