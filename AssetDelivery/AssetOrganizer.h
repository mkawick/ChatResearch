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
   string         version;
   string         path;
   string         beginTime;
   string         endTime;
   string         payload;
   string         name;
   string         hash;

   vector< string > filters;
   vector< string > gates;

   U8*            fileData;
   int            fileSize;
   U8             compressionType;

   AssetDefinition();
   ~AssetDefinition();

   void  SetupHash();
   bool  LoadFile();

   bool  IsDefinitionComplete();

   bool  IsLoaded() const { return isLoaded; }
   void  Print();
};

//////////////////////////////////////////////////////////////////////////

class AssetOrganizer
{
public:
   AssetOrganizer();
   ~AssetOrganizer();

   bool  Init( const string& assetManifestFile );
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

   AssetVector       m_assets;
};

//////////////////////////////////////////////////////////////////////////
