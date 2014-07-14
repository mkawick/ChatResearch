#include <ctype.h>
#include <sstream>
#include <iostream>

#if PLATFORM == PLATFORM_WINDOWS
#pragma warning( disable:4996 )// stupid api rewrite warnings
#endif


#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <boost/range/algorithm/remove_if.hpp>
#include <boost/lexical_cast.hpp>
using boost::format;
#include <curl/curl.h>

#include "HTTPSender.h"

#include "../General/server_log.h"

static string   printResponseData; //will hold the response text

// libs to include
// ../Debug/PacketLibrary.lib ws2_32.lib Secur32.lib ssleay32MTd.lib libeay32MTd.lib curllib.lib

/////////////////////////////////////////////////////////////////////////////////////


static size_t   WriteCallback(char* buf, size_t size, size_t nmemb, void* up)
{ 
   //callback must have this declaration
   //buf is a pointer to the data that curl has for us
   //size*nmemb is the size of the buffer

   size_t num =  size*nmemb;
   int numItems = static_cast<int> ( num );

   for (int c = 0; c<numItems; c++)
   {
     printResponseData.push_back( buf[c] );
   }
   return num; //tell curl how many bytes we handled
}


/////////////////////////////////////////////////////////////////////////////////////

std::string    NotificationSender::hex( unsigned int c )
{
    std::ostringstream stm ;
    stm << '%' << std::hex << std::uppercase << c ;
    return stm.str() ;
}


std::string    NotificationSender::url_encode( const std::string& str )
{
    static const std::string unreserved = "0123456789"
                                            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                            "abcdefghijklmnopqrstuvwxyz"
                                            "-_.~" ;
    std::string result ;

    for( std::string::const_iterator it = str.begin(); it != str.end(); ++ it )
    {
       char c = *it;
        if( unreserved.find(c) != std::string::npos ) result += c ;
        else result += hex(c) ;
    }

    return result ;
}

/////////////////////////////////////////////////////////////////////////////////////


void  NotificationSender::ParseIntoKeyValue( map< string, string >& result, string& text )
{
   std::string word;
   //std::istream   stream = inString;
   for(size_t i = 0; i < text.size(); ++i) 
   {
      char c = text[i];

      if(c == ',' || c == '{' || c == '}' || c == '\n') 
      {
        if(!word.empty()) 
        {
            //std::cout << word << std::endl;
           std::vector< std::string > listOfStuff;
           ParseListOfItems( listOfStuff, word, ":" );

           word.clear();
           cout << "start of line: " << endl;
           std::vector< std::string >::iterator it = listOfStuff.begin();
           while( it != listOfStuff.end() )
           {
              cout << *it << " ... " << endl;
              it++;
           }
           if( listOfStuff.size() > 1 )
           {
            result.insert( KeyValuePair ( *listOfStuff.begin(), *(listOfStuff.begin()+1 ) ) );
           }
           
        }
      } 
      else 
      {
         // Append the current character to the end of the string
         word += c; // or word.push_back(c)
      }
   }
}

/////////////////////////////////////////////////////////////////////////////////////

/*void  NotificationSender::StandardMessageSetup( NotificationSender::KeyValueMap& dataMap, const string& deviceId, unsigned int userId, int gameType, unsigned int gameId, int badge, const char *opponent, const char* messageTypeString, const char* extraText );
//void StandardMessageSetup( KeyValueMap& dataMap, const string& deviceId, unsigned int userId, int gameType, unsigned int gameId, int badge, const char *opponent, const char* messageTypeString, const char* extraText )
{
   if (!opponent || !*opponent)
   {
      opponent = "Someone";
   }

   static string  NOTIFY_ALERT_SOUND   = "PlaydekNotification.ogg";

   dataMap.insert( KeyValuePair( "loc-key", messageTypeString ) );
   dataMap.insert( KeyValuePair( "loc-args", opponent ) );
   dataMap.insert( KeyValuePair( "badge", (format( "%1%" ) % badge ).str()) );
   dataMap.insert( KeyValuePair( "sound", NOTIFY_ALERT_SOUND ) );
   dataMap.insert( KeyValuePair( "game", (format( "%1%" ) % gameId  ).str()) );
   dataMap.insert( KeyValuePair( "user", (format( "%1%" ) % userId ).str()) );
   dataMap.insert( KeyValuePair( "text", extraText ) );
}*/

/////////////////////////////////////////////////////////////////////////////////////

bool  NotificationSender::ParseListOfItems( vector< string >& listOfStuff, string text, const char* delimiter, const char* charsToRemove )
{
   //text.erase( boost::remove_if( text.begin(), text.end(), "[]{}"), text.end() );
   if( charsToRemove )
   {
      text.erase( boost::remove_if( text, boost::is_any_of( charsToRemove )), text.end() );
   }

   string separator1( "" );//dont let quoted arguments escape themselves
   string separator2( delimiter );//split on = and :
   string separator3( "\"\'" );//let it have quoted arguments


   boost::escaped_list_separator<char> els( separator1, separator2, separator3 );
   boost::tokenizer<boost::escaped_list_separator<char> > tokens( text, els );

   for (boost::tokenizer<boost::escaped_list_separator<char> >::iterator i(tokens.begin());
      i!=tokens.end(); ++i) 
   {
      listOfStuff.push_back(*i);
   }

   if( listOfStuff.size() > 0 )
      return true;

   return false;
}

/////////////////////////////////////////////////////////////////////////////////////

GoogleAndroidNotificationSender::GoogleAndroidNotificationSender()
{
   string  PlaydekGoogleAndroidAuthKey = "AIzaSyD-KJoLqlrgnGipT5lqFUeQu4YojgqWgTA";
   m_notificationUrl = "https://android.googleapis.com/gcm/send";
   SetAuthKey( PlaydekGoogleAndroidAuthKey );
}

bool  GoogleAndroidNotificationSender::SendNotification( const string& deviceId, const KeyValueMap& dataMap )
{
   return Notify( deviceId, PrepJsonStrings( deviceId, dataMap ) );
}

bool  GoogleAndroidNotificationSender::Notify( const string& deviceId, const string& json )
{
   CURL *curl;
   CURLcode res = CURL_LAST;

   //curl_global_init( CURL_GLOBAL_DEFAULT );
   curl_global_init( CURL_GLOBAL_NOTHING );

   bool returnCode = false;
   curl = curl_easy_init();
   if( curl == NULL )
   {
      LogMessage(LOG_PRIO_ERR, "curl_easy_init() failed: %s\n", curl_easy_strerror(res) );
   }
   else
   {
      string website = m_notificationUrl; 

      vector< string > headers;
      headers.push_back( "Content-Type: application/json" );
	   string authorization = "Authorization:key=";
		authorization += m_authKey;
      headers.push_back( authorization );

      struct curl_slist* curlStringList=NULL;
      int numHeaders = headers.size();
      
      //cout << "Headers: {";
      //LogMessage(LOG_PRIO_DEBUG, "Headers:\n" );
      for( int i=0; i<numHeaders; i++ )
      {
         curlStringList = curl_slist_append( curlStringList, headers[i].c_str() );
         //cout << headers[i] << "," << endl;
         //LogMessage(LOG_PRIO_DEBUG, "    %s\n", headers[i].c_str() );
      }

      //cout << "}" << endl;
      //cout << "json: " << json << endl;
      //LogMessage(LOG_PRIO_DEBUG, "json:  \'%s\'\n", json.c_str() );
      //cout << "json length: " << json.size() << endl;
      //LogMessage(LOG_PRIO_DEBUG, "json length:  %d\n", json.size() );

      //------------------------------------------------

      curl_easy_setopt( curl, CURLOPT_URL, website.c_str() );
      curl_easy_setopt( curl, CURLOPT_HTTPHEADER, curlStringList );
      curl_easy_setopt( curl, CURLOPT_SSL_VERIFYPEER, false ); // --insecure or -k
      curl_easy_setopt( curl, CURLOPT_POST, true );
      
      curl_easy_setopt( curl, CURLOPT_POSTFIELDS, json.c_str() );
      curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, &WriteCallback);

      res = curl_easy_perform( curl );  // << Main functionality is here
      if( res == CURLE_OK )
      {
         returnCode = true; // success
      }
      else
      {
         //fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
         LogMessage(LOG_PRIO_ERR, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res) );
         returnCode = false;
      }

      long code = 0;
      curl_easy_getinfo ( curl, CURLINFO_RESPONSE_CODE, &code );

      //cout << "http return code was:" << code << endl;
      //LogMessage(LOG_PRIO_DEBUG, "http return code was:  %d\n", code );

      //cout << endl << printResponseData << endl;
      //LogMessage(LOG_PRIO_DEBUG, "response:  %s\n", printResponseData.c_str() );

      curl_easy_cleanup( curl );
   }

   curl_global_cleanup();

   printResponseData.clear();

   return returnCode;
}

/////////////////////////////////////////////////////////////////////////////////////

string   GoogleAndroidNotificationSender::PrepJsonStrings( const string& deviceId, const KeyValueMap& keyValues )
{
   string returnString ( (format("{\"registration_ids\":[\"%1%\"], \"data\":{") % deviceId).str() );   
   string commaString = ",";

   KeyValueMap::const_iterator it = keyValues.begin();
   while( it != keyValues.end() )
   {
      const string& key = it->first;
      const string& value = it->second;
      it++;
      
      if( it == keyValues.end() )
      {
         commaString = "";
      }

     returnString += (format("\"%1%\":\"%2%\"%3%") % key % value % commaString).str();
   }
   returnString += "} }";
   return returnString;
}


/////////////////////////////////////////////////////////////////////////////////////

AmazonAndroidNotificationSender::AmazonAndroidNotificationSender() : NotificationSender() 
{
   // current keys
   string applicationId = "eyJhbGciOiJSU0EtU0hBMjU2IiwidmVyIjoiMSJ9.eyJhcHBGYW1pbHlJZCI6ImFtem4xLmFwcGxpY2F0aW9uLjMyNTY2MTRkMzUzOTQ1NWI4ZmY1NTU0ZmY5NDY3ZDNlIiwiaWQiOiJiZmMzNmFiZi1hNTkyLTExZTMtOGY4Ni04ZDhkYzBjMWFjODIiLCJhcHBJZCI6ImFtem4xLmFwcGxpY2F0aW9uLWNsaWVudC41ZmQwMzc2ZWUyMTI0YTYzYTk3OWM5YmYwNTkxMDcwMCIsImlzcyI6IkFtYXpvbiIsInBrZyI6ImNvbS5wbGF5ZGVrZ2FtZXMuYW5kcm9pZC5TdW1tb25lcldhcnNfQmV0YSIsInZlciI6IjIiLCJhcHBWYXJpYW50SWQiOiJhbXpuMS5hcHBsaWNhdGlvbi1jbGllbnQuNWZkMDM3NmVlMjEyNGE2M2E5NzljOWJmMDU5MTA3MDAiLCJ0eXBlIjoiQVBJS2V5IiwiaWF0IjoiMTM5NDE1MzU0NTI1MyIsImNsaWVudElkIjoiYW16bjEuYXBwbGljYXRpb24tb2EyLWNsaWVudC5mN2U3ZmIwODNkNzQ0YzNjYjNhYjNjZDVhZTMxODU4ZCIsImFwcHNpZyI6IjExOjhFOjhEOjQ4OjYyOjI0OkI1OkREOkIxOjVGOkU2OkMyOjY4OjQ5OkQyOjI0In0=.We2BMqj/nVbtDW0xM4GSigDBEll1wS72/uZUAImvUF3mI5foe6Tt4G3fOI54t/FKAINWGMEw8b3dgHnbJnWjyQQxHOTZUXcZkdtdpj21xdVGK32cCTWhh3tNSl92FE1t7ditWupDYLzk1zYZS/ySpX3VcbsLycZ0kG/YP2ZPrIgj2RGYe/jT8zzPiOIhFQ30TbjtxSkBz4WHD3lM9Nu+F3S4MIn5ofFIJseJbNdlnCJSluWEwUHwSx+9JSJP9/EHkcD0NG/1c6VWiYdhUx18Riw+VQii1yfeb/Jj1GsRyOUzAtlAEeEogNBtGSRzEmjDj5GuHIzI8KtizVheoDI5rQ==";
   string authRequestUrl = "https://api.amazon.com/auth/O2/token";
   string clientId = "amzn1.application-oa2-client.d2987bb626394c2cb1da997c47b83619";
   string clientKey = "a30ac53af44c37036beeb1740956bdbd31eca0aad9f0c9fe84ab012d08d19ba0";
   string notificationUrl = "https://api.amazon.com/messaging/registrations/%s/messages";

   SetAuthRequestUrl( authRequestUrl );
   SetNotificationUrl( notificationUrl );
   SetClientId( clientId );
   SetClientKey( clientKey );
   SetApplicationId( applicationId );
}

bool     AmazonAndroidNotificationSender::RequestAuthorizationKey( const string& url, const string& clientId, const string& clientSecret, string& resultWithKey )
{
   CURL *curl;
   CURLcode res = CURL_LAST;

   //curl_global_init( CURL_GLOBAL_DEFAULT );
   curl_global_init( CURL_GLOBAL_NOTHING );

   resultWithKey.clear();
   bool returnCode = false;
   curl = curl_easy_init();
   if( curl == NULL )
   {
      LogMessage(LOG_PRIO_ERR, "curl_easy_init() failed: %s\n", curl_easy_strerror(res) );
   }
   else
   {
      vector< string > headers;
      headers.push_back( "Host: api.amazon.com" );
      headers.push_back( "Content-Type: application/x-www-form-urlencoded;charset=UTF-8" );

      string body = "grant_type="    + url_encode( "client_credentials" ) + "&" +
                   "scope="         + url_encode( "messaging:push" )     + "&" +
                   "client_id="     + url_encode( clientId )             + "&" +
                   "client_secret=" + url_encode( clientSecret );

      struct curl_slist* curlStringList=NULL;
      int numHeaders = headers.size();
      
      //cout << "Headers: {";
      //LogMessage(LOG_PRIO_DEBUG, "Headers:\n" );
      for( int i=0; i<numHeaders; i++ )
      {
         curlStringList = curl_slist_append( curlStringList, headers[i].c_str() );
         //cout << headers[i] << "," << endl;
         //LogMessage(LOG_PRIO_DEBUG, "    %s\n", headers[i].c_str() );
      }

      //cout << "}" << endl;
      //cout << "body: " << body << endl;
      //LogMessage(LOG_PRIO_DEBUG, "body:  \'%s\'\n", body.c_str() );
      //cout << "body length: " << body.size() << endl;
      //LogMessage(LOG_PRIO_DEBUG, "body length: %d\n", body.size() );

      //------------------------------------------------

      curl_easy_setopt( curl, CURLOPT_URL, url.c_str() );
      curl_easy_setopt( curl, CURLOPT_HTTPHEADER, curlStringList );
      curl_easy_setopt( curl, CURLOPT_SSL_VERIFYPEER, false ); // --insecure or -k
      curl_easy_setopt( curl, CURLOPT_POST, true );
      
      curl_easy_setopt( curl, CURLOPT_POSTFIELDS, body.c_str() );
      curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, &WriteCallback);


      res = curl_easy_perform( curl );  
      /* Check for errors */ 
      if( res == CURLE_OK )
      {
         returnCode = true; // success
      }
      else
      {
         //fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
         LogMessage(LOG_PRIO_ERR, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res) );
         returnCode = false;
      }

      long code = 0;
      curl_easy_getinfo ( curl, CURLINFO_RESPONSE_CODE, &code );

      //cout << "http return code was:" << code << endl;
      //LogMessage(LOG_PRIO_DEBUG, "http return code was:  %d\n", code );

      //cout << endl << printResponseData << endl;
      //LogMessage(LOG_PRIO_DEBUG, "response:  %s\n", printResponseData.c_str() );

      curl_easy_cleanup( curl );
   }

   curl_global_cleanup();

   if( returnCode == true )
   {
      resultWithKey = printResponseData;
   }
   printResponseData.clear();

   return returnCode;
}

/////////////////////////////////////////////////////////////////////////////////////

// deviceId is called app-instance-id in Amazon's parlance
bool  AmazonAndroidNotificationSender::Notify( const string& deviceId, const string& message )
{
   CURL *curl;
   CURLcode res = CURL_LAST;

   //curl_global_init( CURL_GLOBAL_DEFAULT );
   curl_global_init( CURL_GLOBAL_NOTHING );

   bool returnCode = false;
   curl = curl_easy_init();
   if( curl == NULL )
   {
      LogMessage(LOG_PRIO_ERR, "curl_easy_init() failed: %s\n", curl_easy_strerror(res) );
   }
   else
   {
      string   finalUrl = m_notificationUrl;
      boost::replace_first( finalUrl, "%s", deviceId );

      vector< string > headers;
      headers.push_back( "Host: api.amazon.com" );
      
      headers.push_back( (format("Authorization: Bearer %1%") % m_authKey).str() );
      headers.push_back( "Content-Type: application/json" );
      headers.push_back( "X-Amzn-Type-Version: com.amazon.device.messaging.ADMMessage@1.0" );
      headers.push_back( "Accept: application/json" );
      headers.push_back( "X-Amzn-Accept-Type: com.amazon.device.messaging.ADMSendResult@1.0" );

      //--------------------------------------------------------


      string body = 
         "{" + 
               message + "," + 
               "\"consolidationKey\":\"text\",\"expiresAfter\":86400" +
         "}";

      struct curl_slist* curlStringList=NULL;
      int numHeaders = headers.size();
      
      //cout << "Headers: {";
      //LogMessage(LOG_PRIO_DEBUG, "Headers:\n" );
      for( int i=0; i<numHeaders; i++ )
      {
         curlStringList = curl_slist_append( curlStringList, headers[i].c_str() );
         //cout << headers[i] << "," << endl;
         //LogMessage(LOG_PRIO_DEBUG, "    %s\n", headers[i].c_str() );
      }

      //--------------------------------------------------------

      //cout << "Destination: " << finalUrl << endl;
      //LogMessage(LOG_PRIO_DEBUG, "Destination:  \'%s\'\n", finalUrl.c_str() );
      //cout << "}" << endl;
      //cout << "body: " << body << endl;
      //LogMessage(LOG_PRIO_DEBUG, "body:  \'%s\'\n", body.c_str() );
      //cout << "body length: " << body.size() << endl;
      //LogMessage(LOG_PRIO_DEBUG, "body length: %d\n", body.size() );

      //------------------------------------------------

      curl_easy_setopt( curl, CURLOPT_URL, finalUrl.c_str() );
      curl_easy_setopt( curl, CURLOPT_HTTPHEADER, curlStringList );
      curl_easy_setopt( curl, CURLOPT_SSL_VERIFYPEER, false ); // --insecure or -k
      curl_easy_setopt( curl, CURLOPT_POST, true );
      
      curl_easy_setopt( curl, CURLOPT_POSTFIELDS, body.c_str() );
      curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, &WriteCallback);


      res = curl_easy_perform( curl );  
      /* Check for errors */ 
      if( res == CURLE_OK )
      {
         returnCode = true; // success
      }
      else
      {
         //fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
         LogMessage(LOG_PRIO_ERR, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res) );
         returnCode = false;
      }

      long code = 0;
      curl_easy_getinfo ( curl, CURLINFO_RESPONSE_CODE, &code );

      //cout << "http return code was:" << code << endl;
      //LogMessage(LOG_PRIO_DEBUG, "http return code was:  %d\n", code );

      //cout << endl << printResponseData << endl;
      //LogMessage(LOG_PRIO_DEBUG, "response:  %s\n", printResponseData.c_str() );

      curl_easy_cleanup( curl );
   }

   curl_global_cleanup();

   printResponseData.clear();

   return returnCode;
}

/////////////////////////////////////////////////////////////////////////////////////

bool  AmazonAndroidNotificationSender::SendNotification( const string& deviceId, const KeyValueMap& dataMap )
{
   time_t currentTime;
   time( &currentTime );
   bool timeExpired = false;
   if( m_timeLastAuthKeyRequested == 0 )
   {
      timeExpired = true;
   }
   else
   {
      //double timeSinceLastAuthKey = difftime( currentTime, m_timeLastAuthKeyRequested );
      //LogMessage(LOG_PRIO_DEBUG, "    timeSinceLastAuthKey:      %.2f\n", (float)timeSinceLastAuthKey );
      //LogMessage(LOG_PRIO_DEBUG, "    m_numSecondsUntilExpires:  %d\n", m_numSecondsUntilExpires );

      unsigned int elapsedSeconds = (unsigned int)(currentTime - m_timeLastAuthKeyRequested);
      //LogMessage(LOG_PRIO_DEBUG, "    timeSinceLastAuthKey:      %d\n", elapsedSeconds );
      //LogMessage(LOG_PRIO_DEBUG, "    m_numSecondsUntilExpires:  %d\n", m_numSecondsUntilExpires );
      if( elapsedSeconds >= m_numSecondsUntilExpires )
      {
         timeExpired = true;
      }
   }

   if( timeExpired || m_errorState == 1 )
   {
      m_timeLastAuthKeyRequested = currentTime;
      string returnFromAmazon;
      bool result = RequestAuthorizationKey( m_authRequestUrl, m_clientId, m_clientKey, returnFromAmazon );

      if( result == true )
      { 
         m_errorState = 0;
         KeyValueMap mapOfStrings;
         ParseIntoKeyValue( mapOfStrings, returnFromAmazon );
         KeyValueMap :: iterator expireIt = mapOfStrings.find( "expires_in" );         
         KeyValueMap :: iterator accessIt = mapOfStrings.find( "access_token" );
         if( expireIt != mapOfStrings.end() && accessIt != mapOfStrings.end() )
         {
            //LogMessage(LOG_PRIO_DEBUG, "    expires_in: %s\n", expireIt->second.c_str() );
            m_numSecondsUntilExpires = boost::lexical_cast< int >( expireIt->second ) - 15;// fudge factor, because this can take a while to complete
            //LogMessage(LOG_PRIO_DEBUG, "    m_numSecondsUntilExpires:  %d\n", m_numSecondsUntilExpires );
            m_authKey = accessIt->second;
         }
      }
      else
      {
         m_errorState = 1;// I don't know what to do about this. We should retry really.
         m_numSecondsUntilExpires = 0;
         m_timeLastAuthKeyRequested = 0;
         return false;
      }
   }
   if( m_errorState == 0 )
   {
      Notify( deviceId, PrepJsonStrings( deviceId, dataMap ) );
   }
   return m_errorState == 0;
}


/////////////////////////////////////////////////////////////////////////////////////

string   AmazonAndroidNotificationSender::PrepJsonStrings( const string& deviceId, const NotificationSender::KeyValueMap& keyValues )
{
   string returnString( "\"data\":{" );
   string commaString = ",";

   NotificationSender::KeyValueMap::const_iterator it = keyValues.begin();
   while( it != keyValues.end() )
   {
      const string& key = it->first;
      const string& value = it->second;
      it++;
      
      if( it == keyValues.end() )
      {
         commaString = "";
      }

      returnString += (format("\"%1%\":\"%2%\"%3%") % key % value % commaString).str();
   }
   returnString += "}";
   return returnString;
}

/////////////////////////////////////////////////////////////////////////////////////
