
// AssetOrganizer.cpp 
//

#include <iostream>       // std::cout, std::ios
#include <sstream>        // std::istringstream
#include <ctime>          // std::tm
#include <locale>         // std::locale, std::time_get, std::use_facet

#include <fstream> 
#include <string>
using namespace std;

#include <time.h>


#include "../NetworkCommon/ServerConstants.h"
#include "../NetworkCommon/DataTypes.h"

#include "../NetworkCommon/Utils/Utils.h"

#if PLATFORM == PLATFORM_WINDOWS
#pragma warning (disable: 4996)
#endif

#include <boost/format.hpp>
//#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
//#include <boost/date_time.hpp>

//using namespace boost::posix_time;
//namespace bt = boost::posix_time;


#include "AssetOrganizer.h"



// found here:
//http://stackoverflow.com/questions/3786201/parsing-of-date-time-from-string-boost



AssetOrganizer::AssetOrganizer() : m_isInitializedProperly( false ), m_allFilesAreNowLoaded( false ), m_lastFileLoadedTime( 0 )
{
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
   /*const char* timeFormats [] = 
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
   for( size_t i=0; i<timeFormatsCount; ++i )
   {
      std::istringstream is( timeString );
      is.imbue( std::locale( std::locale::classic(), new bt::time_input_facet( timeFormats[i] ) ) );
      is >> pt;
      if(pt != bt::ptime()) 
      {
         success = true;
         break;
      }
   }

   if( success )
   {
      return bt::to_simple_string( pt );
   }

   return string();*/
   // because of all of the boost problems, this was my only real solution
 /*  int year, month, day, hour, minute, second = 0;
   int r = 0;

   string returnString;

   tm brokenTime;
   strftime( timeString.c_str(), "%Y-%m-%d %T", &brokenTime);
   r = scanf ("%d-%d-%d %d:%d:%d", &year, &month, &day,
              &hour, &minute, &second);
   if (r == 6) 
   {
      char buffer[100];
     sprintf ( buffer, "%d-%d-%d %d:%d:%d\n", year, month, day, hour, minute,
             second);
     returnString = buffer;
   }
   return returnString;*/

   /*std::wstring input = L"2011-Februar-18 23:12:34";
    std::tm t;
    std::wistringstream ss(input);
    ss.imbue(std::locale("de_DE"));
    ss >> std::get_time(&t, L"%Y-%b-%d %H:%M:%S"); // uses std::time_get<wchar_t>
    std::cout << std::asctime(&t);*/

  /* ios_base::iostate;

   std::locale loc;        // classic "C" locale

   // get time_get facet:
   const std::time_get<char>& tmget = std::use_facet <std::time_get<char> > (loc);

   std::ios::iostate state;
   std::istringstream iss ( timeString );
   std::tm when;

   tmget.get_date (iss, std::time_get<char>::iter_type(), iss, state, &when);

   std::cout << "year: " << when.tm_year << '\n';
   std::cout << "month: " << when.tm_mon << '\n';
   std::cout << "day: " << when.tm_mday << '\n';*/

/*
  int year, mon, day, hour, minute, second;

   year = stoa<int>(timeString.substr(0, 4));
   mon = stoa<int>(timeString.substr(5, 2));
   day = stoa<int>(timeString.substr(8, 2));
   hour = stoa<int>(timeString.substr(11, 2));
   minute = stoa<int>(timeString.substr(14, 2));
   second = stoa<int>(timeString.substr(17, 2));

   if (timeString.substr(20, 2) == "PM")
   {
      hour += 12;
   }

   std::locale loc;        // classic "C" locale

   // get time_get facet:
   const std::time_get<char>& tmget = std::use_facet <std::time_get<char> > (loc);

   std::ios::iostate state;
   std::istringstream iss ( timeString );
   std::tm when;

   tmget.get_date (iss, std::time_get<char>::iter_type(), iss, state, &when);

   char buffer[100];
   sprintf ( buffer, "%d-%d-%d %d:%d:%d", year, month, day, hour, minute,
             second);
   returnString = buffer;

   return returnString;*/

   typedef std::istreambuf_iterator<char, std::char_traits<char> > Iter;

    // time struct to parse date into
    std::tm datetime;  // zero initialized
    memset( &datetime, 0, sizeof( datetime ) );

    // Unused, required by time_get
    std::ios_base::iostate state;

    // Stream object to read from
    std::istringstream ins ("");

    // Iterators into the stream object
    Iter begin (ins);
    Iter end;

    const std::locale loc ("C");

    // Get a reference to the time_get facet in locale loc.
    const std::time_get<char, Iter> &tg =
        std::use_facet<std::time_get<char, Iter> >(loc);

    // Display time_base::dateorder value.
    std::cout << "time_base::dateorder == " << tg.date_order () << ".\n";
  
    // Insert date string into stream.
    ins.str ( timeString );

    // get_date from the stream and output tm contents.
    /*tg.get_date (begin, end, ins, state, &timeb);
    std::cout << "Date: " << timeb.tm_year << "-" << timeb.tm_mon << "-" << timeb.tm_mday << std::endl;*/

  /*  // Insert weekday string into stream.
    ins.str ("Monday");

    // get_weekday from the stream and output tm contents.
    tg.get_weekday (begin, end, ins, state, &timeb);
    std::cout << "Weekday: Monday\n" << timeb << std::endl;
  
    // Insert time string into stream.
    ins.str ("06:47:32");

    // get_time from the stream and output tm contents.
    tg.get_time (begin, end, ins, state, &timeb);
    std::cout << "Time: 06:47:32\n" << timeb << std::endl;*/


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

AssetDefinition:: AssetDefinition() :  isLoaded( false ), 
                                       isLayout( false ), 
                                       isOptional( false ), 
                                       productId( 0 ), 
                                       platform( 0 ), 
                                       priority( 0 ),
                                       version( "0.7" ),
                                       fileData( NULL ), 
                                       fileSize( 0 ),
                                       compressionType( 0 )
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
      ConvertToString( GenerateUniqueHash( name ), hash );
   }
   else
   {
      ConvertToString( GenerateUniqueHash( path ), hash );
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
      std::cerr << "Error opening file!" << endl; 
      std::cout << "file not found: " << assetManifestFile << endl; 
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
