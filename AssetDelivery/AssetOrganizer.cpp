
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

#pragma warning (disable: 4996)
#include <boost/format.hpp>
#include <boost/tokenizer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/range/algorithm/remove_if.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/date_time.hpp>
using namespace boost::posix_time;
namespace bt = boost::posix_time;


#include "../NetworkCommon/ServerConstants.h"
#include "../NetworkCommon/DataTypes.h"
#include "AssetOrganizer.h"

#include "../NetworkCommon/Utils/Utils.h"

// found here:
//http://stackoverflow.com/questions/3786201/parsing-of-date-time-from-string-boost


AssetOrganizer::AssetOrganizer() : m_isInitializedProperly( false ), m_lastFileLoadedTime( 0 ), m_allFilesAreNowLoaded( false )
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
/*
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
   "iphone",
   "android",
   "androidtablet",
   "pc",
   "mac",
   "vita",
   "xbox",
   "blackberry",
   "wii"
};
*/

const char* assetPriotityType[] = {
   "default",
   "primary",
   "secondary"
};

//////////////////////////////////////////////////////////////////////////

AssetDefinition:: AssetDefinition(): isLoaded( false ), isLayout( false ), productId( 0 ), platform( 0 ), fileData( NULL ), fileSize( 0 ), compressionType( 0 ), version( "0.5" ), priority( 0 )
{
}

AssetDefinition::~AssetDefinition()
{
   delete [] fileData;
}

void  AssetDefinition:: SetupHash()
{
   if( name.size() > 0 )
   {
      ConvertToString( GenerateUniqueHash( name, 15 ), hash );
   }
   else
   {
      ConvertToString( GenerateUniqueHash( path, 15 ), hash );
   }
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
      isLoaded = true;
   }
   else
   {
      return false;
   }

   
   return true;
}

bool  AssetDefinition:: IsDefinitionComplete()
{
   if( isLayout == false )
   {
      if( productId == 0 || path.size() == 0 )
         return false;
   }
   else //if( isLayout == true )
   {
      if( path.size() == 0 || platform == 0 )
         return false;
   }
   return true;
}

void  AssetDefinition:: Print()
{
   cout << "[asset] " << endl;
   const char* formatString = "%-15s%-15s";
   cout << boost::format( formatString ) % "Name: " % name << endl;
   cout << boost::format( formatString ) % "path: " % path << endl;
   cout << boost::format( formatString ) % "version: " % version << endl;
   cout << boost::format( formatString ) % "hash: " % hash << endl;
   if( beginTime.size() )
   {
      cout << boost::format( formatString ) % "beginTime: " % beginTime << endl ;
   }
   if( endTime.size() )
   {
      cout << boost::format( formatString ) % "endTime: " % endTime << endl ;
   }
   cout << boost::format( formatString ) % "productId: " % FindProductName( productId ) << endl ;
   cout << boost::format( formatString ) % "platform: " % FindPlatformName( platform ) << endl ;

   cout << endl;
}

//////////////////////////////////////////////////////////////////////////

int   Findpriority( const string& value )
{
   int numAssetTypes = sizeof( assetPriotityType ) / sizeof( assetPriotityType[0] );
   for( int i=0; i< numAssetTypes; i++ )
   {
      if( value == assetPriotityType[i] )
      {
         return i;
      }
   }
   return 0;
}

//////////////////////////////////////////////////////////////////////////

bool  splitOnFirstFound( vector< string >& listOfStuff, string text, const char* delimiter = "=:" )
{
   size_t position = text.find_first_of( delimiter );
   if( position != string::npos )
   {
      std::string substr1 = text.substr( 0, position );
      std::string substr2 = text.substr( position+1, std::string::npos );// assuming only one
      listOfStuff.push_back( substr1 );
      listOfStuff.push_back( substr2 );
      return true;
   }
   else
   {
      listOfStuff.push_back( text );
      return false;
   }
}

bool  ParseListOfItems( vector< string >& listOfStuff, string text, const char* delimiter = "=:", const char* charsToRemove = NULL )
{
   //text.erase( boost::remove_if( text.begin(), text.end(), "[]{}"), text.end() );
   if( charsToRemove )
   {
      text.erase(remove_if(text, boost::is_any_of( charsToRemove )), text.end());
   }

   string separator1( "" );//dont let quoted arguments escape themselves
   string separator2( delimiter );//split on = and :
   string separator3( "\"\'" );//let it have quoted arguments


   boost::escaped_list_separator<char> els( separator1, separator2, separator3 );
   boost::tokenizer<boost::escaped_list_separator<char> > tokens( text, els );

   for (boost::tokenizer<boost::escaped_list_separator<char> >::iterator i(tokens.begin());
      i!=tokens.end(); ++i) 
   {
      listOfStuff.push_back(*i);
   }

   if( listOfStuff.size() > 0 )
      return true;

   return false;
}

string RemoveEnds( std::string &s, const char* charsToStrip = "\"\'" )
{
   int len = strlen( charsToStrip );

   for( int i=0; i<len; i++ )
   {
      while( *s.begin() == charsToStrip[i] )
      {
         s = s.substr(1, s.size());
      }
      while( *s.rbegin() == charsToStrip[i] )
      {
         s = s.substr(0, s.size()-1);
      }
      //s.erase(remove( s.begin(), s.end(), charsToStrip[i] ),s.end());
   }

   return s;
}

//////////////////////////////////////////////////////////////////////////

bool  FillInAsset( string& line, AssetDefinition& asset )
{
   vector< string > listOfStuff;
   splitOnFirstFound( listOfStuff, line );

   if( listOfStuff.size() != 0 )
   {
      const string& potentionalKey = ConvertStringToLower( listOfStuff[ 0 ] );
      const string& value = RemoveEnds( ConvertStringToLower( listOfStuff[ 1 ] ) );
      const string originalValue = RemoveEnds( listOfStuff[ 1 ] );
      

      if( listOfStuff.size() == 2 )// simplest case
      {
         if( potentionalKey == "product" )
         {
            asset.productId = FindProductId( value.c_str() );
            if( asset.productId != 0 ) // bad id
               return true;
         }
         else if( potentionalKey == "priority" )
         {
            asset.priority = boost::lexical_cast< int >( value );//Findpriority( value );
            return true;
         }
         else if( potentionalKey == "platform" )
         {
            asset.platform = FindPlatformId( value.c_str() );
            if( asset.platform != 0 ) // bad id
               return true;
         }
         else if( potentionalKey == "version" )
         {
            asset.version = value;
            return true;
         }
         else if( potentionalKey == "path" || potentionalKey == "file" )
         {
            asset.path = originalValue;
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
         else if( potentionalKey == "filters" )
         {
            vector< string > tempList;
            return ParseListOfItems( asset.filters, value, ",", "[]{}" );
         }
         else if( potentionalKey == "notes" )
         {
            return true;// ignore notes
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

bool  FillInLayout( string& line, AssetDefinition& asset )
{
   vector< string > listOfStuff;
   splitOnFirstFound( listOfStuff, line );

   if( listOfStuff.size() != 0 )
   {
      const string& potentionalKey = ConvertStringToLower( listOfStuff[ 0 ] );
      const string& value = RemoveEnds( ConvertStringToLower( listOfStuff[ 1 ] ) );
      const string originalValue = RemoveEnds( listOfStuff[ 1 ] );
      

      if( listOfStuff.size() == 2 )// simplest case
      {
         if( potentionalKey == "path" || potentionalKey == "file" )
         {
            asset.path = originalValue;
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
         else if( potentionalKey == "name" )
         {
            asset.name = value;
            return true;
         }
         else if( potentionalKey == "platform" )
         {
            asset.platform = FindPlatformId( value.c_str() );
            if( asset.platform != 0 ) // bad id
               return true;
         }
         else if( potentionalKey == "version" )
         {
            asset.version = value;
            return true;
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
         else if( potentionalKey == "notes" )
         {
            return true;// ignore notes
         }
         else if( potentionalKey == "payload" )
         {
            asset.payload = value;
            return true;
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

std::istream& safeGetline(std::istream& is, std::string& t)
{
    t.clear();

    // The characters in the stream are read one-by-one using a std::streambuf.
    // That is faster than reading them one-by-one using the std::istream.
    // Code that uses streambuf this way must be guarded by a sentry object.
    // The sentry object performs various tasks,
    // such as thread synchronization and updating the stream state.

    std::istream::sentry se(is, true);
    std::streambuf* sb = is.rdbuf();

    for(;;) {
        int c = sb->sbumpc();
        switch (c) {
        case '\n':
            return is;
        case '\r':
            if(sb->sgetc() == '\n')
                sb->sbumpc();
            return is;
        case EOF:
            // Also handle the case when the last line has no line ending
            if(t.empty())
                is.setstate(std::ios::eofbit);
            return is;
        default:
            t += (char)c;
        }
    }
}

//////////////////////////////////////////////////////////////////////////

bool  AssetOrganizer::ParseNextAsset( ifstream& infile, int& lineCount )
{
   string line;
   AssetDefinition asset;
   while( safeGetline( infile, line ) )
   {
      lineCount++;
      bool result = FillInAsset( line, asset );
      if( result == false )
         return false;

      if( asset.IsDefinitionComplete() == true )// we will skip right over version and platform so be careful
      {
         asset.SetupHash();
         //asset.Print();
         AddAssetDefinition( asset );
         return true;
      }
      
   }
   return false;
}

//////////////////////////////////////////////////////////////////////////

bool  AssetOrganizer::ParseNextLayout( ifstream& infile, int& lineCount )
{
   string line;
   AssetDefinition asset;
   asset.isLayout = true; // <<<<< important

   while( safeGetline(infile, line) )
   {
      lineCount++;
      bool result = FillInAsset( line, asset );
      if( result == false )
         return false;

      if( asset.IsDefinitionComplete() == true )// we will skip right over version and platform so be careful
      {
         asset.SetupHash();
         //asset.Print();
         AddAssetDefinition( asset );
         return true;
      }
      
   }
   return false;
}

//////////////////////////////////////////////////////////////////////////

bool  AssetOrganizer::ReplaceExistingAssetBasedOnHash( AssetDefinition& asset )
{
   AssetVectorIterator it = m_assets.begin();
   while( it != m_assets.end() )
   {
      AssetDefinition& testAsset = *it;

      if( testAsset.productId == asset.productId )
      {
         if( testAsset.hash == asset.hash || 
            ( testAsset.name.size() > 0 && testAsset.name == asset.name )// these are the same item...
             )
         {
            if( testAsset.version > asset.version )
            {
               // toss the new one since it is an older version
            }
            else
            {
               *it = asset;// simple replacement
               return true;
            }
         }
         
      }
      it++;
   }
   return false;
}

//////////////////////////////////////////////////////////////////////////

void  AssetOrganizer::AddAssetDefinition( AssetDefinition& asset )
{
   if( ReplaceExistingAssetBasedOnHash( asset ) == true )
      return;


   if( asset.priority )
   {
      if( m_assets.size() == 0 )
      {
         m_assets.push_back( asset );
      }
      else
      {
         bool didInsert = false;
         AssetVectorIterator it = m_assets.begin();
         while( it != m_assets.end() )
         {
            if( asset.priority > it->priority )
            {
               m_assets.insert( it, asset );
               didInsert = true;
               break;
            }
            it++;
         }
         if( didInsert == false )
         {
            m_assets.push_back( asset );
         }
      }
   }
   else
   {
      m_assets.push_back( asset );
   }
}

//////////////////////////////////////////////////////////////////////////
/*
string ExePath() {
    char buffer[MAX_PATH];
    GetCurrentDirectoryA( MAX_PATH, buffer );
    string::size_type pos = string( buffer ).find_last_of( "\\/" );
    return string( buffer ).substr( 0, pos);
}*/


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
   while( safeGetline( infile, line ) )// could be spaces, etc
   {
      lineCount ++;
      if( line == "[asset]" )
      {
         bool result = ParseNextAsset( infile, lineCount );
         if( result == false )
         {
            cout << "**********************************************" << endl;
            cout << "Error in asset file reading line " << lineCount << endl;
            cout << "**********************************************" << endl;
            errorCode = true;
            break;
         }
      }

      if( line == "[layout]" )
      {
         bool result = ParseNextLayout( infile, lineCount );
         if( result == false )
         {
            cout << "**********************************************" << endl;
            cout << "Error in asset file reading line " << lineCount << endl;
            cout << "**********************************************" << endl;
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
      else //if( m_assets.size() == 0 )
      {
         m_allFilesAreNowLoaded = true;
      }
   }

	return m_isInitializedProperly;
}

//////////////////////////////////////////////////////////////////////////

bool  AssetOrganizer::GetListOfAssets( U8 productId, int platformId, vector< string >& listOfAssetsByHash ) const
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

bool  AssetOrganizer::GetListOfAssets( int platformId, const set< string >& listOfFilters, vector< string >& listOfAssetsByHash ) const
{
   listOfAssetsByHash.clear();

   // do we need to organize the assets by product id? There probably won't be enough but we can group them by product if we end up with a lot of assets.
   vector< AssetDefinition >::const_iterator it = m_assets.begin();
   while( it != m_assets.end() )
   {
      if( it->platform == platformId )// first match product
      {
         if( listOfFilters.size() != 0 )
         {
            bool filterFound = false;
            // this solution found here: 
            // http://stackoverflow.com/questions/14284444/how-to-find-matching-string-in-a-list-of-strings
            vector< string >::const_iterator filterIt = it->filters.begin();
            while( filterIt != it->filters.end() )
            {
               if( listOfFilters.find( *filterIt ) == listOfFilters.end() )
               {
                  filterFound = true;
                  break;
               }
               filterIt++;
            }
            if( filterFound == false )
            {
               listOfAssetsByHash.push_back( it->hash );
            }
         }
         else
         {
            listOfAssetsByHash.push_back( it->hash );
         }
      }
      it++;
   }

   if( listOfAssetsByHash.size() > 0 )
      return true;
   return false;
}

//////////////////////////////////////////////////////////////////////////

void  AssetOrganizer::Update()
{
   if( m_assets.size() == 0 || m_allFilesAreNowLoaded == true || m_isInitializedProperly == false )
      return;

   time_t currentTime;
   time( &currentTime );

   if( difftime( currentTime, m_lastFileLoadedTime ) >= 0.3 ) // 1/3rd of a second
   {
      m_lastFileLoadedTime = currentTime;

      vector< AssetDefinition >::iterator it = m_assets.begin();
      while( it != m_assets.end() )
      {
         if( it->IsLoaded() == false )
         {
            it->LoadFile();
            return;
         }
         else
         {
            it++;
         }
      }
      m_allFilesAreNowLoaded = true;
   }
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
   /*vector< AssetDefinition >::iterator it = m_assets.begin();
   while( it != m_assets.end() )
   {
      if( it->LoadFile() == false )
      {
         cout << "Critical failure loading asset : " << it->path << endl;
         return false;
      }

      it++;
   }*/
   return true;
}

//////////////////////////////////////////////////////////////////////////
