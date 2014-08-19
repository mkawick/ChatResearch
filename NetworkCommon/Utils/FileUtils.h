// FileUtils.h

#pragma once

#include <iostream>       // std::cout, std::ios
#include <sstream>        // std::istringstream
#include <ctime>          // std::tm
#include <locale>         // std::locale, std::time_get, std::use_facet

#include <fstream> 
#include <string>
#include <algorithm>
using namespace std;

#include "../ServerConstants.h"
#include "../DataTypes.h"

#include "../Utils/Utils.h"

#if PLATFORM == PLATFORM_WINDOWS
#pragma warning (disable: 4996)
   #include <windows.h>
   #include <mmsystem.h>
   #include <sys/stat.h>
   #include <direct.h>
   #include <io.h>
#endif


//////////////////////////////////////////////////////////////////////////

struct FileVersion
{
   int      version;
   string   filePath;
   time_t   lastModifiedTime;
};

//////////////////////////////////////////////////////////////////////////

bool  DoesFileExist ( const std::string& name, time_t& lastModifiedTime );
bool  LoadFile( const string& path, U8*& fileData, int& fileSize );
void  FindAllSubDirectories( string baseDir, vector <string>& directoryList );


void  FindFilesInSubdirectories( const vector< string >& directoriesToSearch, const string& filename, vector <FileVersion>& fileList );
void  FindFilesInSubdirectories( const vector< string >& subdirectories, const string& filename );
void  FindFilesInSubdirectories( const string& fullPath, vector< FileVersion >& fullyQualifiedPaths );

void  PrintFileDetails( const string& filePath, vector< FileVersion >& fileDetails );
