#pragma once


//////////////////////////////////////////////////////////////////////////

struct AssetDefinition
{
   unsigned char  productId;
   int            platform;
   int            priorityType;
   string         version;
   string         path;
   string         beginTime;
   string         endTime;
   string         payload;
   string         name;
   string         hash;

   U8*            fileData;
   int            fileSize;
   U8             compressionType;

   AssetDefinition();
   ~AssetDefinition();

   void  SetupHash();
   bool  LoadFile();

   bool  IsDefinitionComplete();
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

   bool  Find( const string& name, AssetDefinition*& asset );
   bool  FindByHash( const string& hash, AssetDefinition const *& asset ) const; // returns a reference const... you cannot modify the result.

   bool  GetListOfAssets( U8 productId, int platformType, vector< string >& listOfAssetsByHash ) const;
   

private:

   void  AddAssetDefinition( AssetDefinition& asset );
   bool  ParseNextAsset( ifstream& infile, int& lineCount );
   bool  LoadAllFiles();

   typedef vector< AssetDefinition >   AssetVector;
   typedef AssetVector::iterator       AssetVectorIterator;

   //----------------------------------------
   bool              m_isInitializedProperly;

   AssetVector       m_assets;
};

//////////////////////////////////////////////////////////////////////////
