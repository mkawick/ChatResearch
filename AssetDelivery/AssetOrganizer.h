#pragma once

#include <set>

#include "../NetworkCommon/ServerConstants.h"
#include "../NetworkCommon/DataTypes.h"

#include "../NetworkCommon/Utils/FileUtils.h"

//////////////////////////////////////////////////////////////////////////

struct LoadedFile : public FileVersion
{
   LoadedFile();
   LoadedFile( const FileVersion& );
   LoadedFile( const LoadedFile& );

   ~LoadedFile();
   const LoadedFile& operator = ( const LoadedFile& );
   void  operator = ( const FileVersion& ver );
   U8*            fileData;
   int            fileSize;
   U8             compressionType;
   
   
};

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
   string         compressionType;


   bool           FindFile( int version, LoadedFile& file ) const;

   vector< string > filters;
   vector< string > gates;
   vector< LoadedFile > listOfVersionedFiles;

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

   bool  GetListOfAssets( U8 productId, int platformType, const string& compressionType, vector< string >& listOfAssetsByHash ) const;
   bool  GetListOfAssets( int platformId, const set< string >& listOfFilters, const string& compressionType, vector< string >& listOfAssetsByHash, int maxNum = 250 ) const;

   void  Update();

private:

   void  AddAssetDefinition( AssetDefinition& asset );
   bool  ReplaceExistingAssetBasedOnHash( AssetDefinition& asset );
   bool  ParseNextAsset( ifstream& infile, int& lineCount );
   bool  ParseNextLayout( ifstream& infile, int& lineCount );
   void  LoadAllAssets( time_t& currentTime );

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
