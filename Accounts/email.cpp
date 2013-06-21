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
using namespace std;

#include "../NetworkCommon/Platform.h"
#include "../NetworkCommon/Serialize.h"
#include "../NetworkCommon/Utils/Utils.h"

#if PLATFORM == PLATFORM_WINDOWS

#undef UNICODE
#define WIN32_LEAN_AND_MEAN

#include <ws2tcpip.h>
#pragma warning (disable:4996)

#elif PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX

#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>

#define SOCKET_ERROR -1

#endif

#define LogToFile 1
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
   dumpFile.open( fileName,ios::app );
#endif
}


void LogTextToFile( const char* text ) 
{
#if defined( LogToFile )
   dumpFile << text;
#endif
}

void  CloseFile()
{
#if defined( LogToFile )
   dumpFile.close();
#endif
}

//////////////////////////////////////////////////////////////////////////////

int  SendConfirmationEmail( const char* toAddr, const char* fromAddr, const char* emailServerName, const char* bodyText, const char* subject, const char* linkText, const char* linkAddr )
{
   // Lookup email server's IP address.

  char        buffer[4096]       = "";
  //char        messageLine[255]       = "";
  int maxLen = strlen( bodyText ) + 64;
  auto_ptr< char > messageLine( new char[ maxLen ] );

  OpenLogFile( "emailDump.log" );
  hostent*   hostEntry = gethostbyname(emailServerName);
  if(!hostEntry)
  {
    cout << "Cannot find SMTP mail server " << emailServerName << endl;

    return 1;
  }

  // Create a TCP/IP socket, no specific protocol
  int      socketId = socket(PF_INET, SOCK_STREAM, 0);
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

  sockaddr_in SockAddr;
  // Setup a Socket Address structure
  SockAddr.sin_family = AF_INET;
  SockAddr.sin_port   = iProtocolPort;
  memcpy( &( SockAddr.sin_addr ), hostEntry->h_addr_list[0], hostEntry->h_length );

  // Connect the Socket
  if(connect(socketId, (sockaddr*) &SockAddr, sizeof(SockAddr)))
  {
    cout << "Error connecting to Server socket" << endl;

    return 1;
  }

  // Receive initial response from SMTP server
  Check(recv(socketId, buffer, sizeof(buffer), 0), "recv() Reply");

  // Send HELO server.com
  sprintf( messageLine.get(), "HELO %s%s", emailServerName, CRLF);
  Check(send(socketId, messageLine.get(), strlen( messageLine.get() ), 0), "send() HELO");
  LogTextToFile( messageLine.get() );
  Check(recv(socketId, buffer, sizeof(buffer), 0), "recv() HELO");

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

  sprintf( messageLine.get(), "Content-Type: text/html%s", CRLF);
  Check(send(socketId, messageLine.get(), strlen( messageLine.get() ), 0), "send() content type");
  LogTextToFile( messageLine.get() );



  sprintf( messageLine.get(), "%s%s", bodyText, CRLF);
  Check(send(socketId, messageLine.get(), strlen( messageLine.get() ), 0), "send() message-line");
  LogTextToFile( messageLine.get() );
  
  
 /* if( linkText && strlen( linkText ) > 0 && 
     linkAddr && strlen( linkAddr ) > 0 )
  {
     const char* linkString = "<html><body><a href=\"%s\">%s</a>%s</body></html>";
     sprintf(messageLine, linkString, linkAddr, linkText, CRLF);
     Check(send(socketId, messageLine, strlen( messageLine.get() ), 0), "send() link text");
     LogTextToFile( messageLine );
  }*/

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