#pragma once

#include <map>
#include <string>
#include <vector>
using namespace std;

namespace HTTP
{
   typedef map< string, string>  KeyValueMap;
   typedef pair< string, string>  KeyValuePair;

   std::string    hex( unsigned int c );
   std::string    url_encode( const std::string& str );
   void           ParseIntoKeyValue( KeyValueMap& result, string& text );
   bool           ParseListOfItems( std::vector< std::string >& listOfStuff, std::string text, const char* delimiter = "=:", const char* charsToRemove = NULL );

   ////////////////////////////////////////////////////////////////////////////////

   class NotificationSender
   {

   public:
      NotificationSender() : m_timeLastAuthKeyRequested( 0 ), m_numSecondsUntilExpires( 60 ),  m_errorState( 0 ), m_authKey(), m_clientId(), m_clientKey(), m_authRequestUrl(){}

      void  SetAuthKey( const string& authKey ) { m_authKey = authKey; }
      void  SetClientId( const string& clientId ) { m_clientId = clientId; }
      void  SetClientKey( const string& clientKey ) { m_clientKey = clientKey; }
      void  SetApplicationId( const string& applicationId ) { m_applicationId = applicationId; }
      void  SetAuthRequestUrl( const string& authUrl ) { m_authRequestUrl = authUrl; }
      void  SetNotificationUrl( const string& notificationUrl ) { m_notificationUrl = notificationUrl; }

      virtual bool  SendNotification( const string& deviceId, const KeyValueMap& dataMap ) = 0; // always custom

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
      string   PrepJsonStrings( const string& deviceId, const KeyValueMap& keyValues );
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
      string   PrepJsonStrings( const string& deviceId, const KeyValueMap& keyValues );

   };

   ////////////////////////////////////////////////////////////////////////////////

   
   ///////////////////////////////////////////////////////////////

   class ReceiptValidator
   {
   public:
      enum ReceiptState
      {
         ReceiptState_unknown,
         ReceiptState_valid,
         ReceiptState_invalid,
         ReceiptState_cannot_be_verified
      };

   public:
      ReceiptValidator() : m_isBusy ( false ), m_isFinished( false ), m_state( ReceiptState_unknown ) {}

      void     SetUrl( const string& url ) { m_validationUrl = url; }

      virtual bool     ValidateReceipt( const char* ) = 0;

      bool     IsBusy() { return m_isBusy; }
      bool     IsFinished() { return m_isFinished; }
      int      GetErrorCode() { return m_state; }

   protected:

      virtual void   ParseResult() = 0;

      string         m_validationUrl;
      string         m_receipt;
      bool           m_isBusy;
      bool           m_isFinished;

      ReceiptState   m_state;
   };

   ///////////////////////////////////////////////////////////////

   class AppleReceiptValidator : public ReceiptValidator
   {
   public:
      AppleReceiptValidator() : ReceiptValidator() {}
      bool     ValidateReceipt( const char* receipt );
   protected:
      void     ParseResult() ;
   };

   ///////////////////////////////////////////////////////////////
}
