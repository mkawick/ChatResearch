// email.cpp


#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <memory>

#include "../NetworkCommon/Platform.h"
#include "../NetworkCommon/Packets/Serialize.h"
#include "../NetworkCommon/Utils/Utils.h"
#include "../NetworkCommon/General/Base64.h"

#if PLATFORM == PLATFORM_WINDOWS

#undef UNICODE
#define WIN32_LEAN_AND_MEAN

#include <ws2tcpip.h>
#pragma warning (disable:4996)

#elif PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX

#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>

#define SOCKET_ERROR -1

#endif

using namespace std;


#define LogToFile 0
ofstream dumpFile;

static const char* CRLF = "\r\n";

//////////////////////////////////////////////////////////////////////////////

void Check(int iStatus, const char* functionName )
{
  if((iStatus != SOCKET_ERROR) && (iStatus))
    return;

  int errorCode = 0;
#if PLATFORM == PLATFORM_WINDOWS
  errorCode = GetLastError() ;
#else
  errorCode = errno;
#endif

  cerr << "Error during call to " << functionName << ": " << iStatus << " - " << errorCode << endl;
}

void   OpenLogFile( const char* fileName )
{
#if defined( LogToFile )
   dumpFile.open( fileName, ios::out | ios::app | ios::binary );
#endif
}


void LogTextToFile( const char* text ) 
{
#if defined( LogToFile )
   //dumpFile << text;
   dumpFile.write( text, strlen( text ) );
#endif
}

void LogTextToFile( const char* text, int length ) 
{
#if defined( LogToFile )

   //dumpFile << str;
   dumpFile.write( text, length );
#endif
}

void  CloseFile()
{
#if defined( LogToFile )
   dumpFile.close();
#endif
}


//////////////////////////////////////////////////////////////////////////////

void HtmlEntitySafeSplit( char* buffer, const char* source, int splitSize, const char* delimiter )
{
   int   position = 0;
   bool  unsafeSplit = false;
   int   sourceLength = strlen( source );

   for(int i=0; i<sourceLength; i++)
   {
      if(position >= splitSize && unsafeSplit == false )
      {
         int len = strlen( delimiter );
         memcpy( buffer, delimiter, len );
         buffer += len;

         unsafeSplit = false;
         position = 0;
      }

      char c = *source++;

      if( c == '&' )
         unsafeSplit = true;
      else if( c == ';' )
         unsafeSplit = false;
      else if( c == ' ' )
         unsafeSplit = false;

      *buffer = c;
      buffer++;
      position++;
   }
   *buffer = 0;
}

//////////////////////////////////////////////////////////////////////////////
/*
bool IsValidEmailLine( const char* source, int linePosition )
{
   int countOfCRLF = 0;
   const char* counter = source;
   while( counter-source <  linePosition )
   {
      if( *counter == '\r' && *(counter+1) == '\n' )
         countOfCRLF ++;
      counter++;
   }
   if( linePosition < 3 )
   {
      if( *source == '\r' && *(source+1) == '\n' )
         return false;
   }

   return true;
}*/

void  SendEmailTextLine( int socketId, const char* source, int linePosition )
{
   const char* counter = source;
   const char* offset = source;
   
   const char* test = "https://itunes.apple.com/us/app/agricola/id561521557?mt=8";
   const char* pos = strstr( source, test );
   int position = pos-source;
   if (position > 0 && position < 100 )
   {
      position = position;
   }
   while( counter-source <  linePosition )
   {
      if( *counter == '\r' && *(counter+1) == '\n' )
      {
         int num = (counter-offset)+2;// include the crlf
         send( socketId, offset, num, 0 );// do not send ending cr lfs
         LogTextToFile( offset, num );

         if( *(counter+2) == '\r' && *(counter+3) == '\n' )// too many cr/lf
         {
            counter += 4;// move past the cr/lfs
         }
         else
         {
            counter += 2;// move past the cr/lfs
            
         }
         offset = counter;
      }
      else
      {
         counter++;
      }
   }
   int len = linePosition-(offset-source);
   if( len > 0 )// do not write a terminating 0
   {
      send( socketId, offset, len, 0);
      LogTextToFile( offset, len );
   }
}

//////////////////////////////////////////////////////////////////////////////

int   SendEveryFewCharcters( const char* source, int len, int minCharacters, int maxCharacters, int socketId )
{
   int longest = 0;
   int position = 0;
   //const char* html = strstr( source, "</html>" );
   while( position < len )
   {
      const char* text = source;
      int linePosition = 0;
      while( linePosition < minCharacters )
      {
         linePosition ++;
         text ++;
      }
      // now we look for crlf
      do
      {
         while( ( position + linePosition < len ) && 
            *text != '\r' )
         {
            linePosition ++;
            text++;
         }
      } while( ( position + linePosition < len ) && *(text+1) != '\n' );

      if( *text == '\r' && *(text+1) == '\n' )
      {
         linePosition += 2;
      }
      if( position + linePosition > len )
      {
         linePosition = len - position;
      }

      /// quick test for validity... no bare cr/lf
      /*if( IsValidEmailLine( source, linePosition ) )
      {
         send( socketId, source, linePosition, 0);
         LogTextToFile( source, linePosition );
      }*/
      SendEmailTextLine( socketId, source, linePosition );
      if( linePosition > longest )
          longest = linePosition;

      // if we can, move past the cr/lf
      if( position < len && *text ) 
      {
         // now we are ready for a break
         linePosition += 2;// point to the thing after the line feed
         text += 2;
      }

      source = text;
      position += linePosition;
   }

   return longest;
}


//////////////////////////////////////////////////////////////////////////////

int   SendEmailAuthPiece( int socketId, const char* authenticationUsername, const char* authenticationPassword )
{
   if( authenticationUsername == NULL || strlen( authenticationUsername ) < 5 )
      return 1;
   if( authenticationPassword == NULL || strlen( authenticationPassword ) < 5 )
      return 1;

   const int maxLen = 512;
   auto_ptr< char > outputBuffer( new char[ maxLen ] );
   U32 length = base64_encode( outputBuffer.get(), authenticationUsername, strlen( authenticationUsername ) );

   outputBuffer.get()[ length ] = 0;
   string username = outputBuffer.get();
   cout << "Orig: " << authenticationUsername << endl;
   cout << "Enc : " << username << endl;
   length = base64_encode( outputBuffer.get(), authenticationPassword, strlen( authenticationPassword ) );

   outputBuffer.get()[ length ] = 0;
   string password = outputBuffer.get();
   cout << "Orig: " << authenticationPassword << endl;
   cout << "Enc : " << password << endl;

   sprintf( outputBuffer.get(), "auth login:%s", CRLF);
   Check( send(socketId, outputBuffer.get(), strlen( outputBuffer.get() ), 0), outputBuffer.get());
   LogTextToFile( outputBuffer.get() );

   sprintf( outputBuffer.get(), "%s%s", username.c_str(), CRLF);
   Check( send(socketId, outputBuffer.get(), strlen( outputBuffer.get() ), 0), username.c_str());
   LogTextToFile( outputBuffer.get() );

   sprintf( outputBuffer.get(), "%s%s", password.c_str(), CRLF);
   Check( send(socketId, outputBuffer.get(), strlen( outputBuffer.get() ), 0), username.c_str());
   LogTextToFile( outputBuffer.get() );

   const char* successString = "253 ... authentication succeded ::";
   Check(recv(socketId, outputBuffer.get(), maxLen, 0), successString );

   cout << "Response: " << outputBuffer.get() << endl;
   /*if( strcmp( successString, outputBuffer.get() ) != 0 )
      return 1;*/

   // the response is nonsense

   return 0;
}

//////////////////////////////////////////////////////////////////////////////

//int  sendConfirmationEmail( const char* toAddr, const char* fromAddr, const char* emailServer, const char* bodyText, const char* linkText, const char* linkAddr )

int  SendConfirmationEmail( const char* toAddr, const char* fromAddr, 
                           const char* emailServerName, 
                           const char* bodyText, const char* subject, 
                           const char* linkText, const char* linkAddr,
                           unsigned short portOverride, 
                           const char* authenticationUsername, const char* authenticationPassword )
{
   // Lookup email server's IP address.
  char        buffer[4096]       = "";
  int bodyLen = strlen( bodyText );
  int maxLen = bodyLen + 1024;// we're going to be inserting a lot of CRLFs
  auto_ptr< char > messageLine( new char[ maxLen ] );

  OpenLogFile( "emailDump.log" );
  hostent*   hostEntry = gethostbyname(emailServerName);
  if(!hostEntry)
  {
    cout << "Cannot find SMTP mail server " << emailServerName << endl;

    return 1;
  }

  // Create a TCP/IP socket, no specific protocol
  int      socketId = //socket(PF_INET, SOCK_RAW, 0);
                        socket(PF_INET, SOCK_STREAM, 0);
  if(socketId == SOCKET_ERROR)
  {
    cout << "Cannot open mail server socket" << endl;

    return 1;
  }

  // Get the mail service port
  servent* emailServer = getservbyname("mail", 0);

  // Use the SMTP default port if no other port is specified
  int         iProtocolPort        = 0;
  if(!emailServer)
    iProtocolPort = htons(IPPORT_SMTP);
  else
    iProtocolPort = emailServer->s_port;
  if( portOverride )
  {
     iProtocolPort = htons( portOverride );
  }

  sockaddr_in SockAddr;
  // Setup a Socket Address structure
  SockAddr.sin_family = AF_INET;
  SockAddr.sin_port   = iProtocolPort;
  memcpy( &( SockAddr.sin_addr ), hostEntry->h_addr_list[0], hostEntry->h_length );

  // Connect the Socket
  if(connect(socketId, (sockaddr*) &SockAddr, sizeof(SockAddr)))
  {
    cout << "Error connecting to Server socket" << endl;
    closesocket( socketId );

    return 1;
  }

  // Receive initial response from SMTP server
  Check(recv(socketId, buffer, sizeof(buffer), 0), "recv() Reply");

  string intro = "Starting email: ";
  intro += GetDateInUTC( 0, 0, 0 );
  LogTextToFile( intro.c_str() );

  // Send HELO server.com
  sprintf( messageLine.get(), "HELO %s%s", emailServerName, CRLF);
  Check(send(socketId, messageLine.get(), strlen( messageLine.get() ), 0), "send() HELO");
  LogTextToFile( messageLine.get() );

   if( authenticationUsername == NULL )
   {
      Check(recv(socketId, buffer, sizeof(buffer), 0), "recv() HELO");
   }
   else
   {
      // this is specific to godaddy who don't return a "recv() HELO"
      if( SendEmailAuthPiece( socketId, authenticationUsername, authenticationPassword ) != 0 )
      {
         cout << "The authentication failed:" << endl;
         closesocket(socketId);
         CloseFile();
         return 1;
      }
   }

  // Send MAIL FROM: <sender@mydomain.com>
  sprintf( messageLine.get(), "MAIL FROM:<%s>%s", fromAddr, CRLF);
  Check(send(socketId, messageLine.get(), strlen( messageLine.get() ), 0), "send() MAIL FROM");
  LogTextToFile( messageLine.get() );
  Check(recv(socketId, buffer, sizeof(buffer), 0), "recv() MAIL FROM");

  // Send RCPT TO: <receiver@domain.com>
  sprintf( messageLine.get(), "RCPT TO:<%s>%s", toAddr, CRLF);
  Check(send(socketId, messageLine.get(), strlen( messageLine.get() ), 0), "send() RCPT TO");
  LogTextToFile( messageLine.get() );
  Check(recv(socketId, buffer, sizeof(buffer), 0), "recv() RCPT TO");

  // Send DATA
  sprintf( messageLine.get(), "DATA%s", CRLF);
  Check(send(socketId, messageLine.get(), strlen( messageLine.get() ), 0), "send() DATA");
  LogTextToFile( messageLine.get() );
  Check(recv(socketId, buffer, sizeof(buffer), 0), "recv() DATA");

  sprintf( messageLine.get(), "From:%s%s", fromAddr, CRLF);
  Check(send(socketId, messageLine.get(), strlen( messageLine.get() ), 0), "send() From TO");
  LogTextToFile( messageLine.get() );

  sprintf( messageLine.get(), "To:%s%s", toAddr, CRLF);
  Check(send(socketId, messageLine.get(), strlen( messageLine.get() ), 0), "send() From TO");
  LogTextToFile( messageLine.get() );

  sprintf( messageLine.get(), "Subject:%s%s", subject, CRLF);
  Check(send(socketId, messageLine.get(), strlen( messageLine.get() ), 0), "send() Subject TO");
  LogTextToFile( messageLine.get() );

  sprintf( messageLine.get(), "Content-Type: text/html; charset=\"utf-8\"%s", CRLF);
  Check(send(socketId, messageLine.get(), strlen( messageLine.get() ), 0), "send() content type");
  LogTextToFile( messageLine.get() );

  SendEveryFewCharcters( bodyText, bodyLen, 34, 76, socketId );

  // Send blank line and a period
  sprintf( messageLine.get(), "%s.%s", CRLF, CRLF);
  Check(send(socketId, messageLine.get(), strlen( messageLine.get() ), 0), "send() end-message");
  LogTextToFile( messageLine.get() );
  Check(recv(socketId, buffer, sizeof(buffer), 0), "recv() end-message");

  // Send QUIT
  sprintf( messageLine.get(), "QUIT%s", CRLF);
  Check(send(socketId, messageLine.get(), strlen( messageLine.get() ), 0), "send() QUIT");
  LogTextToFile( messageLine.get() );
  Check(recv(socketId, buffer, sizeof(buffer), 0), "recv() QUIT");

  // Close server socket and prepare to exit.
  closesocket(socketId);
  CloseFile();

  return 0;
}

//////////////////////////////////////////////////////////////////////////////


std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}


std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

static inline bool is_alnum(char c)
{
    return (isalpha(c) || isdigit(c));
}

static inline bool is_not_alpha(char c)
{
    return ! isalpha(c) ;
}

static inline bool is_alnum_space(char c)
{
    return ( is_alnum(c) || (c == ' '));
}

static inline bool filter_disallowed_characters( char c )
{
   if( c == ' ' || c == '{' || c == '}' || c == '+' || c == '*' || c =='#' )
      return true;

   return false;
}

// look at http://en.wikipedia.org/wiki/Email_address

bool  IsValidEmailAddress( const string& test )
{
   if( test.size() < 3 )
      return false;

   std::vector<std::string> splits = split( test, '@' );

   if( splits.size() != 2 )
      return false;
   string firstPart = splits[0];

   //RFC 832 allows almost any precursor for the first part... move along
   if( firstPart.size() < 1 )
      return false;

   string secondPart = splits[1];
   std::vector<std::string> splits2 = split( secondPart, '.' );
   int num = splits2.size();
   if( num < 2 || num > 4 )// just strange, fail me because too few periods or too many
      return false;

  /* if( find( secondPart.begin(), secondPart.end(), '.') == secondPart.end() )// must have a period
      return false;*/

   if( is_alnum( *secondPart.rbegin() ) == false ) // no ending with bad charaters
      return false;

   if( find_if( secondPart.begin(), secondPart.end(), filter_disallowed_characters) != secondPart.end() ) // filter out some email addresses by special characters
      return false;

   if( find_if( secondPart.begin(), secondPart.end(), is_alnum) == secondPart.end() )// some of this string must be alpha numeric
      return false;

   const string& domain = *splits2.rbegin();
   if( domain.size() < 2 ) // domains must be at least 2 characters
      return false;

   if( find_if( domain.begin(), domain.end(), is_not_alpha) != domain.end() )// all of the domain must be alpha numeric
      return false;

   return true;
}


bool  allowed_text_string( char c )
{
   if( isalpha(c) || isdigit(c) || c==' ' || c=='_' || c =='.' || c =='-' )
      return true;

   return false;
}

vector<string> CreateDictionary( const string& textString, char searchChar )
{
   vector<string> dictionary;
   string::const_iterator start = textString.begin();
   while( start != textString.end() )
   {
      char c = *start++;
      if( c == searchChar )// begin of replacement
      {
         string::const_iterator end = start;
         while( end != textString.end() )
         {
            char test = *end;
            if( test == searchChar )
            {
               string subString;
               subString.assign( start, end );
               start = end+1;// move past the marker

               if( subString.size() > 0 )
               {
                  dictionary.push_back( subString );
               }
               break;
            }
            if( allowed_text_string( test ) == false ) // must be an invalid set of 
               break;
            end ++;
         }
      } 
   }

   return dictionary;
}