

int  SendConfirmationEmail( const char* toAddr, const char* fromAddr, 
                           const char* emailServerName, 
                           const char* bodyText, const char* subject, 
                           const char* linkText, const char* linkAddr,
                           unsigned short portOverride, 
                           const char* authenticationUsername, const char* authenticationPassword );
bool  IsValidEmailAddress( const string& test );

vector<string> CreateDictionary( const string& textString, char searchChar = '%' );