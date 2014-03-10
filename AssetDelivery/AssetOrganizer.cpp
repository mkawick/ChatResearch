
// AssetOrganizer.cpp 
//

#include <iostream>       // std::cout, std::ios
#include <sstream>        // std::istringstream
#include <ctime>          // std::tm
#include <locale>         // std::locale, std::time_get, std::use_facet

#include <fstream> 
#include <string>
using namespace std;

#include <sys/stat.h>
#include <time.h>


#include "../NetworkCommon/ServerConstants.h"
#include "../NetworkCommon/DataTypes.h"

#include "../NetworkCommon/Utils/Utils.h"

#if PLATFORM == PLATFORM_WINDOWS
#pragma warning (disable: 4996)
#endif

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include "AssetOrganizer.h"

// found here:
//http://stackoverflow.com/questions/3786201/parsing-of-date-time-from-string-boost

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

bool IsStringBracketed( const std::string& s, const char* bracketPairs )
{
   int len = static_cast< int >( strlen( bracketPairs ) ) / 2;

   for( int i=0; i<len; i+=2 )
   {
      if( *s.begin() == bracketPairs[i] && 
         *s.rbegin() == bracketPairs[i+1] )
         return true;
   }

   return false;
}

//////////////////////////////////////////////////////////////////////////

bool IsBracketedTag( const string& line, const char* bracketPairs ) // asset, layout
{
   // no spaces allowed
   if (line.find(' ') != std::string::npos)
   {
       return false;
   }

   // brackets on the ends.
   if( IsStringBracketed( line, bracketPairs ) == false )
      return false;
   return true;
}

//////////////////////////////////////////////////////////////////////////

AssetOrganizer::AssetOrganizer() : m_isInitializedProperly( false ), m_allFilesAreNowLoaded( false ), m_lastFileLoadedTime( 0 ), m_assetFileModificationTime( 0 )
{
   time( &m_checkFileChangesTime );
   //m_assets.reserve( 120 );
}

AssetOrganizer::~AssetOrganizer()
{
}

template <class T>
T stoa(const std::string& s)
{
   T t;
   std::istringstream iss(s);
   if (!(iss >t))
   throw "Can't convert";
   return t;
}

string ConvertStringToStandardTimeFormat(const std::string& timeString )
{
    // time struct to parse date into
    std::tm datetime;  // zero initialized
    memset( &datetime, 0, sizeof( datetime ) );

    int year, month, day, hour, minute, second;
    sscanf( timeString.c_str(), "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second );
    datetime.tm_sec = second;
    datetime.tm_min = minute;
    datetime.tm_hour = hour;
    datetime.tm_mday = day;
    datetime.tm_mon = month;
    datetime.tm_year = year;
    //return mktime(datetime);

    char buffer[100];
    sprintf (buffer, "%4d-%02d-%02d %02d:%02d:%02d", datetime.tm_year, datetime.tm_mon, datetime.tm_mday, datetime.tm_hour, datetime.tm_min, datetime.tm_sec );

       
    return string( buffer );
}

//////////////////////////////////////////////////////////////////////////

const char* assetPriotityType[] = {
   "default",
   "primary",
   "secondary"
};

//////////////////////////////////////////////////////////////////////////

AssetDefinition:: AssetDefinition() :  isLoaded( false ), 
                                       isLayout( false ), 
                                       isOptional( false ), 
                                       productId( 0 ), 
                                       platform( 0 ), 
                                       priority( 0 ),
                                       fileModificationTime( 0 ),
                                       version( "0.8" ),
                                       fileData( NULL ), 
                                       fileSize( 0 ),
                                       compressionType( 0 )
{
}

AssetDefinition:: AssetDefinition( const AssetDefinition& assetDefn ) : isLoaded( assetDefn.isLoaded ), 
                                       isLayout( assetDefn.isLayout ), 
                                       isOptional( assetDefn.isOptional ), 
                                       productId( assetDefn.productId ), 
                                       platform( assetDefn.platform ), 
                                       priority( assetDefn.priority ),
                                       fileModificationTime( assetDefn.fileModificationTime ),
                                       version( assetDefn.version ),
                                       path( assetDefn.path ),
                                       beginTime( assetDefn.beginTime ),
                                       endTime( assetDefn.endTime ),
                                       payload( assetDefn.payload ),
                                       name( assetDefn.name ),
                                       hash( assetDefn.hash ),
                                       category( assetDefn.category ),
                                       filters( assetDefn.filters ),
                                       gates( assetDefn.gates ),
                                       fileData( NULL ), 
                                       fileSize( assetDefn.fileSize ),
                                       compressionType( assetDefn.compressionType )
{
   if( assetDefn.fileData )
   {
      fileData = new U8[ assetDefn.fileSize ];
      memcpy( fileData, assetDefn.fileData, assetDefn.fileSize );
   }

}

AssetDefinition::~AssetDefinition()
{
   delete [] fileData;
}

AssetDefinition& AssetDefinition:: operator = ( const AssetDefinition& assetDefn )
{
   isLoaded = assetDefn.isLoaded;
   isLayout = assetDefn.isLayout; 
   isOptional = assetDefn.isOptional; 
   productId = assetDefn.productId; 
   platform = assetDefn.platform; 
   priority = assetDefn.priority;
   fileModificationTime = assetDefn.fileModificationTime;
   version = assetDefn.version;
   path = assetDefn.path;
   beginTime = assetDefn.beginTime;
   endTime = assetDefn.endTime;
   payload = assetDefn.payload;
   name = assetDefn.name;
   hash = assetDefn.hash;
   category = assetDefn.category;
   filters = assetDefn.filters;
   gates = assetDefn.gates;
   fileSize = assetDefn.fileSize;
   compressionType = assetDefn.compressionType;

   if( assetDefn.fileData )
   {
      fileData = new U8[ assetDefn.fileSize ];
      memcpy( fileData, assetDefn.fileData, assetDefn.fileSize );
   }

   return *this;
}

void  AssetDefinition:: SetupHash()
{
   if( name.size() > 0 )
   {
      string lowerCaseString = ConvertStringToLower( name );
      ConvertToString( GenerateUniqueHash( lowerCaseString ), hash );
   }
   else
   {
      string lowerCaseString = ConvertStringToLower( name );
      ConvertToString( GenerateUniqueHash( lowerCaseString ), hash );
   }

   if( hash.size() > TypicalMaxHexLenForNetworking )// limit
   {
      hash = hash.substr( hash.size()-TypicalMaxHexLenForNetworking, TypicalMaxHexLenForNetworking); 
   }
}

bool  AssetDefinition:: LoadFile()
{
   ifstream file ( path.c_str(), ios::in|ios::binary|ios::ate);
   if (file.is_open())
   {
      delete fileData;// delete any previous data
      fileSize = file.tellg();
      fileData = new U8 [fileSize];
      file.seekg (0, ios::beg);
      file.read ( reinterpret_cast< char* >( fileData ), fileSize);
      file.close();
      isLoaded = true;

      fileModificationTime = GetFileModificationTime( path );
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
      if( path.size() == 0 )
         return false;
   }
   else //if( isLayout == true )
   {
      if( path.size() == 0 || platform == 0 )
         return false;
   }
   return true;
}

bool  AssetDefinition:: IsFileChanged() const 
{
   struct stat fileInfo;
   stat( path.c_str(), &fileInfo);
   if( fileModificationTime != fileInfo.st_mtime )
   {
      return true;
   }
   return false;
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


//////////////////////////////////////////////////////////////////////////

bool  FillInAsset( string& line, AssetDefinition& asset )
{
   vector< string > listOfStuff;

   splitOnFirstFound( listOfStuff, line );

   if( listOfStuff.size() != 0 )
   {
      const string& potentionalKey = ConvertStringToLower( listOfStuff[ 0 ] );
      const string& value = ConvertStringToLower( listOfStuff[ 1 ] );
      const string undecoratedValue = RemoveEnds( listOfStuff[ 1 ] );

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
            asset.priority = boost::lexical_cast< int >( value );
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
            asset.path = undecoratedValue;
            if( DoesFileExist( asset.path ) == true )
            {
               return true;
            }
            else
            {
               cout << "Invalid file :  " << asset.path << endl;
            }
         }
         else if( potentionalKey == "begintime" ) 
         {
            asset.beginTime = ConvertStringToStandardTimeFormat( undecoratedValue );
            return true;
         }
         else if( potentionalKey == "endtime" ) 
         {
            asset.endTime = ConvertStringToStandardTimeFormat( undecoratedValue );
            return true;
         }
         else if( potentionalKey == "optional" ) 
         {
            if( value == "true" )
               asset.isOptional = true;
            else if( value == "false" )
               asset.isOptional = false;
            else
            {
               asset.isOptional = boost::lexical_cast< int >( value ) ? true:false;
            }
            return true;
         }
         else if( potentionalKey == "payload" )
         {
            asset.payload = value;
            return true;
         }
         else if( potentionalKey == "name" || potentionalKey == "id" )
         {
            asset.name = undecoratedValue;
            return true;
         }
         else if( potentionalKey == "filters" )
         {
            return ParseListOfItems( asset.filters, value, ",", "[]{}" );
         }
         else if( potentionalKey == "gates" )
         {
            return ParseListOfItems( asset.gates, value, ",", "[]{}" );
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
   asset.category = m_category;
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
            // the file will self update if the time has changed, so don't do this.
            //time_t recentMileModificationTime = GetFileModificationTime( asset.path );
            //asset.fileModificationTime

            if( testAsset.version < asset.version ) // only replace older versions. 
            {
               *it = asset;// simple replacement
               return true;
            }

            // same asset but not replaced..  we don't want dups.
            return true;
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


   asset.category = m_category;
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

bool  AssetOrganizer::LoadAssetManifest()
{
   m_isInitializedProperly = false;

   ifstream infile( m_dictionaryPath.c_str() );
   string line;
   if (!infile) 
   { 
      std::cerr << "Error opening file!" << endl; 
      std::cout << "file not found: " << m_dictionaryPath << endl; 
      return 1; 
   }

   m_assetFileModificationTime = GetFileModificationTime( m_dictionaryPath );
   if( m_assetFileModificationTime == 0 )
   {
      return false;
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
            // 1) first match by gate and add automagically
            bool wasAdded = false;
            vector< string >::const_iterator gateIt = it->gates.begin();
            while( gateIt != it->gates.end() )
            {
               if( listOfFilters.find( *gateIt ) != listOfFilters.end() )
               {
                  listOfAssetsByHash.push_back( it->hash );
                  wasAdded = true;
                  break;
               }
               gateIt++;
            }
            if( wasAdded == false )
            {
               // 2)  now try to remove
               bool filterFound = false;
               // this solution found here: 
               // http://stackoverflow.com/questions/14284444/how-to-find-matching-string-in-a-list-of-strings
               vector< string >::const_iterator filterIt = it->filters.begin();
               while( filterIt != it->filters.end() )
               {
                  if( listOfFilters.find( *filterIt ) != listOfFilters.end() )
                  {
                     filterFound = true;
                     break;
                  }
                  filterIt++;
               }
               if( filterFound == false )// it's not there
               {
                  listOfAssetsByHash.push_back( it->hash );
               }
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
   time_t currentTime;
   time( &currentTime );

   if( m_allFilesAreNowLoaded == true && 
      difftime( currentTime, m_checkFileChangesTime ) >= FileChangesTimeout ) 
   {
      m_checkFileChangesTime = currentTime;

      vector< AssetDefinition >::iterator it = m_assets.begin();
      while( it != m_assets.end() )
      {
         if( it->IsFileChanged() == true )
         {
            //m_allFilesAreNowLoaded = false;
            it->LoadFile();
         }
         it++;
      }
      time_t fileTime = GetFileModificationTime( m_dictionaryPath );
      if( m_assetFileModificationTime != fileTime )
      {
         LoadAssetManifest();
      }
   }

   if( m_assets.size() == 0 || m_allFilesAreNowLoaded == true || m_isInitializedProperly == false )
      return;

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
