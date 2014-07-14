#pragma once

#include <map>
#include <string>
#include <vector>
using namespace std;

////////////////////////////////////////////////////////////////////////////////

class NotificationSender
{
public:
   typedef map< string, string>  KeyValueMap;
   typedef pair< string, string>  KeyValuePair;

public:
   NotificationSender() : m_timeLastAuthKeyRequested( 0 ), m_numSecondsUntilExpires( 60 ),  m_errorState( 0 ), m_authKey(), m_clientId(), m_clientKey(), m_authRequestUrl(){}

   void  SetAuthKey( const string& authKey ) { m_authKey = authKey; }
   void  SetClientId( const string& clientId ) { m_clientId = clientId; }
   void  SetClientKey( const string& clientKey ) { m_clientKey = clientKey; }
   void  SetApplicationId( const string& applicationId ) { m_applicationId = applicationId; }
   void  SetAuthRequestUrl( const string& authUrl ) { m_authRequestUrl = authUrl; }
   void  SetNotificationUrl( const string& notificationUrl ) { m_notificationUrl = notificationUrl; }

   virtual bool  SendNotification( const string& deviceId, const KeyValueMap& dataMap ) = 0; // always custom

public: // utils
   std::string    hex( unsigned int c );
   std::string    url_encode( const std::string& str );
   
protected:
   time_t         m_timeLastAuthKeyRequested;
   unsigned int   m_numSecondsUntilExpires;
   int            m_errorState;
   string         m_authKey; 
   string         m_clientId;
   string         m_clientKey;
   string         m_applicationId;
   string         m_authRequestUrl;
   string         m_notificationUrl;

   bool  ParseListOfItems( std::vector< std::string >& listOfStuff, std::string text, const char* delimiter = "=:", const char* charsToRemove = NULL );
   void  ParseIntoKeyValue( map< string, string >& result, string& text );
   virtual bool  Notify( const string& deviceId, const string& message ) = 0;// always custom
   virtual string   PrepJsonStrings( const string& deviceId, const NotificationSender::KeyValueMap& keyValues ) = 0;
};

////////////////////////////////////////////////////////////////////////////////

class GoogleAndroidNotificationSender : public NotificationSender
{
public:
   GoogleAndroidNotificationSender();
   bool  SendNotification( const string& deviceId, const KeyValueMap& dataMap );

private:
   bool  Notify( const string& deviceId, const string& message );
   string   PrepJsonStrings( const string& deviceId, const NotificationSender::KeyValueMap& keyValues );
};

////////////////////////////////////////////////////////////////////////////////

class AmazonAndroidNotificationSender : public NotificationSender
{
public:
   AmazonAndroidNotificationSender();
   bool  SendNotification( const string& deviceId, const KeyValueMap& message );

private:
   bool  RequestAuthorizationKey( const string& url, const string& clientId, const string& clientSecret, string& resultWithKey );
   bool  Notify( const string& deviceId, const string& message );
   string   PrepJsonStrings( const string& deviceId, const NotificationSender::KeyValueMap& keyValues );

};

////////////////////////////////////////////////////////////////////////////////

