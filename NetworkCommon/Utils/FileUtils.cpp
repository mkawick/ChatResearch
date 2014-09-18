// FileUtils.cpp

#include "FileUtils.h"

#include "../Platform.h"
#include <sys/stat.h>
#include <time.h>

#include <assert.h>

#if PLATFORM == PLATFORM_UNIX
#include <ftw.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h> 

//typedef int (*DirectoryCallbackFunction) (const char *fpath, const struct stat *sb, int typeflag );
vector <string> globalDirectoryList;
#include <vector>
#include <string>
using namespace std;

#elif PLATFORM == PLATFORM_WINDOWS
//typedef stat _stat;

#endif

//////////////////////////////////////////////////////////////////////////

bool  LoadFile( const string& path, U8*& fileData, int& fileSize )
{
   ifstream file ( path.c_str(), ios::in|ios::binary|ios::ate);
   if (file.is_open())
   {
      delete [] fileData;// delete any previous data
      fileSize = file.tellg();
      fileData = new U8 [fileSize];
      file.seekg (0, ios::beg);
      file.read ( reinterpret_cast< char* >( fileData ), fileSize);
      file.close();
   }
   else
   {
      fileSize = 0;
      return false;
   }
   
   return true;
}

#if PLATFORM == PLATFORM_UNIX

int DirectoryCallbackFunction (const char *fpath, const struct stat *sb, int typeflag )
{
   if( typeflag == FTW_D )
   {
      globalDirectoryList.push_back( fpath );
   }
   return 0;
}

#endif

//////////////////////////////////////////////////////////////////////////

void    FindAllSubDirectories ( string baseDir, vector <string>& directoryList )
{
   //const int LONGEST_PATH = 256;

   assert( baseDir.size() != 0 );
   directoryList.clear();
   directoryList.push_back( baseDir );

   std::replace( baseDir.begin(), baseDir.end(), '\\', '/' );
   
   if( *baseDir.rbegin() != '/' )
      baseDir += "/";

   string searchString = baseDir + "*.";

   ///int ret = 
      chdir ( baseDir.c_str() );

 #if PLATFORM == PLATFORM_WINDOWS
   WIN32_FIND_DATAA FindFileData;
   HANDLE hFind;

   hFind = FindFirstFileA( searchString.c_str(), &FindFileData );
   if (hFind == INVALID_HANDLE_VALUE) 
   {
      printf ("FindFirstFile failed (%d)\n", GetLastError());
      return;
   } 
   else 
   {
      do
      {
         if (FindFileData.cFileName[0] != '.')
         {
            if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
               string path = baseDir + FindFileData.cFileName;
               directoryList.push_back( path );
               //printf("  %s   <DIR>\n", FindFileData.cFileName);
            }
         }
        if( FindNextFileA( hFind, &FindFileData ) == false )
            break;
      }
      while(1);
      FindClose(hFind);
   }
#else
   /*   DIR *dir;
    class dirent *ent;
    class stat st;

    dir = opendir(directory);
    while ((ent = readdir(dir)) != NULL) {
        const string file_name = ent->d_name;
        const string full_file_name = directory + "/" + file_name;

        if (file_name[0] == '.')
            continue;

        if (stat(full_file_name.c_str(), &st) == -1)
            continue;

        const bool is_directory = (st.st_mode & S_IFDIR) != 0;

        if (is_directory)
            continue;

        out.push_back(full_file_name);
    }
    closedir(dir);*/
   
   globalDirectoryList.clear();
   if (ftw(baseDir.c_str(), DirectoryCallbackFunction, 20 ) == -1) 
   {
      perror("nftw");
      return ;
   }

   vector <string> ::iterator it = globalDirectoryList.begin();
   while( it != globalDirectoryList.end() )
   {
      directoryList.push_back( *it++ );
   }
   globalDirectoryList.clear();
   
#endif
}

//////////////////////////////////////////////////////////////////////////

bool  DoesFileExist ( const std::string& name, time_t& lastModifiedTime ) 
{
  struct stat buffer;
  bool result = ( stat( name.c_str(), &buffer ) == 0 ); 
  if( result == true )
     lastModifiedTime = buffer.st_mtime;
  else
     lastModifiedTime = 0;
  return result;
}

//////////////////////////////////////////////////////////////////////////

void  FindFilesInSubdirectories( const vector< string >& directoriesToSearch, const string& filename, vector <FileVersion>& fileList )
{

   assert( directoriesToSearch.size() != 0 );
   assert( filename.size() != 0 );

   vector< string >::const_iterator it = directoriesToSearch.begin();
   while( it != directoriesToSearch.end() )
   {
      string baseDir = *it++;
      std::replace( baseDir.begin(), baseDir.end(), '\\', '/' );

      unsigned found = baseDir.find_last_of("/");
      assert( found != baseDir.size() - 1 ); // work to be done if this ever happens

      string versionStr = baseDir.substr( found+1, baseDir.size()-found );
      int ver = atoi( versionStr.c_str() );
      if( *baseDir.rbegin() != '/' )
         baseDir += "/";

      time_t lastModifiedTime;
      string fullName = baseDir + filename;
      if( DoesFileExist ( fullName, lastModifiedTime ) == true )
      {
         FileVersion filer;
         filer.filePath = fullName;
         filer.version = ver;
         filer.lastModifiedTime = lastModifiedTime;
         fileList.push_back( filer );
      }
   }
}

//////////////////////////////////////////////////////////////////////////

void    FindFilesInSubdirectories( const vector< string >& subdirectories, const string& filename )
{
   char nowBuff[100];
   cout << "*Searched file:" << filename << endl;
   vector< FileVersion > fullPaths;
   FindFilesInSubdirectories ( subdirectories, filename, fullPaths );
   vector< FileVersion >::iterator it = fullPaths.begin();
   while( it != fullPaths.end() )
   {
      const FileVersion fileInfo = *it++;
      struct tm *nowtm;
      nowtm = gmtime(&fileInfo.lastModifiedTime);
      strftime(nowBuff, sizeof(nowBuff), "%Y-%m-%d %H:%M:%S", nowtm);
      cout << "  - " << fileInfo.filePath << ":" << fileInfo.version << "... time: " << nowBuff<< endl;
   }
}

//////////////////////////////////////////////////////////////////////////

void  FindFilesInSubdirectories( const string& filePath, vector< FileVersion >& fullyQualifiedPaths )
{
   fullyQualifiedPaths.clear();
   //string filePath = "c:/gwshare/avatar/JakeSully.png";
   string path = filePath.substr(0, filePath.find_last_of("\\/"));
   string file = filePath.substr(filePath.find_last_of("\\/")+1, filePath.size());
   assert( path.size() != 0 );
   if( path == file )
   {
      cout << "Improper path" << endl;
      return;
   }

   vector< string > subdirectories;
   FindAllSubDirectories ( path, subdirectories );

   FindFilesInSubdirectories ( subdirectories, file, fullyQualifiedPaths );
}

//////////////////////////////////////////////////////////////////////////

void  PrintFileDetails( const string& fileName, vector< FileVersion >& fileDetails )
{
   cout << "*Searched file:" << fileName << endl;
   char nowBuff[100];
   //FindAllFiles ( subdirectories, filename, fullPaths );
   vector< FileVersion >::iterator it = fileDetails.begin();
   while( it != fileDetails.end() )
   {
      const FileVersion fileInfo = *it++;
      struct tm *nowtm;
      nowtm = gmtime(&fileInfo.lastModifiedTime);
      strftime(nowBuff, sizeof(nowBuff), "%Y-%m-%d %H:%M:%S", nowtm);
      cout << "  - " << fileInfo.filePath << ":" << fileInfo.version << "... time: " << nowBuff<< endl;
   }
}


//////////////////////////////////////////////////////////////////////////