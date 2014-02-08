#pragma once

#include <set>

//////////////////////////////////////////////////////////////////////////

struct AssetDefinition
{
   bool           isLoaded;
   bool           isLayout;
   bool           isOptional;
   unsigned char  productId;
   int            platform;
   int            priority;
   time_t         fileModificationTime;
   string         version;
   string         path;
   string         beginTime;
   string         endTime;
   string         payload;
   string         name;
   string         hash;
   string         category;

   vector< string > filters;
   vector< string > gates;

   U8*            fileData;
   int            fileSize;
   U8             compressionType;

   AssetDefinition();
   AssetDefinition( const AssetDefinition& assetDefn );
   ~AssetDefinition();
   AssetDefinition& operator = ( const AssetDefinition& rhs );

   void  SetupHash();
   bool  LoadFile();
   //bool  ReloadFile()

   bool  IsDefinitionComplete();
   bool  IsFileChanged() const ;

   bool  IsLoaded() const { return isLoaded; }
   void  Print();
};

//////////////////////////////////////////////////////////////////////////

class AssetOrganizer
{
public:
   AssetOrganizer();
   ~AssetOrganizer();

   bool  LoadAssetManifest();
   void  SetCategory( const string& name ) { m_category = name; }
   const string& GetCategory() const { return m_category; }
   void  SetPath( const string& path ) { m_dictionaryPath = path; }
   const string& GetPath() const { return m_dictionaryPath; }

   bool  IsInitialized() const { return m_isInitializedProperly; }
   bool  IsFullyLoaded() const { return m_allFilesAreNowLoaded; }

   bool  Find( const string& name, AssetDefinition*& asset );
   bool  FindByHash( const string& hash, AssetDefinition const *& asset ) const; // returns a reference const... you cannot modify the result.

   bool  GetListOfAssets( U8 productId, int platformType, vector< string >& listOfAssetsByHash ) const;
   bool  GetListOfAssets( int platformId, const set< string >& listOfFilters, vector< string >& listOfAssetsByHash ) const;

   void  Update();

private:

   void  AddAssetDefinition( AssetDefinition& asset );
   bool  ReplaceExistingAssetBasedOnHash( AssetDefinition& asset );
   bool  ParseNextAsset( ifstream& infile, int& lineCount );
   bool  ParseNextLayout( ifstream& infile, int& lineCount );
   bool  LoadAllFiles();

   typedef vector< AssetDefinition >   AssetVector;
   typedef AssetVector::iterator       AssetVectorIterator;

   //----------------------------------------
   bool              m_isInitializedProperly;
   bool              m_allFilesAreNowLoaded;

   time_t            m_lastFileLoadedTime;
   time_t            m_checkFileChangesTime;
   time_t            m_assetFileModificationTime;
   static const int  FileChangesTimeout = 60 * 2; // two minutes

   AssetVector       m_assets;

   string   m_category;
   string   m_dictionaryPath;
};

//////////////////////////////////////////////////////////////////////////
// prototypes

std::istream& safeGetline(std::istream& is, std::string& t);
bool IsBracketedTag( const string& line, const char* bracketPairs = "[]{}<>()" );
bool IsStringBracketed( const std::string& s, const char* bracketPairs = "[]{}<>()" );

//////////////////////////////////////////////////////////////////////////
/*
struct AssetDictionaryInfo
{
   string   name;
   string   dictionaryPath;

   vector< AssetDefinition > assets;

   bool VerifyPath();
};*/

//////////////////////////////////////////////////////////////////////////
