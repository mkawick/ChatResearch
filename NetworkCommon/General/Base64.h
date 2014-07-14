
#include "../DataTypes.h"

///
/// Convert up to len bytes of binary data in src to base64 and store it in dest
///
/// \param dest Destination buffer to hold the base64 data.
/// \param src Source binary data.
/// \param len The number of bytes of src to convert.
///
/// \return The number of characters written to dest.
/// \remarks Does not store a terminating null in dest.
///
U32         base64_encode( char* dest, const char* src, U32 len );

///
/// Convert null-terminated string src from base64 to binary and store it in dest.
///
/// \param dest Destination buffer
/// \param src Source base64 string
/// \param len Pointer to unsigned int representing size of dest buffer. After function returns this is set to the number of character written to dest.
///
/// \return Pointer to first character in source that could not be converted (the terminating null on success)
///

const char* base64_decode( char* dest, const char* src, U32* len );

/*

use case:

void  TestEncode( char*& outputBuffer, const string& source )
{
   
   //char* 
   outputBuffer = new char[ source.size() *4 /3 + 4 ];
   uint length = base64_encode(outputBuffer, source.c_str(), source.size() );

   outputBuffer[length] = 0;
   cout << outputBuffer << endl; 
   //delete outputBuffer;
}

void  TestDecode( char*& outputBuffer, const string& source )
{
   uint newLen = source.size() *3 / 4 + 4;
   outputBuffer = new char[ newLen ];
   
   const char* buffer = base64_decode( outputBuffer, source.c_str(), &newLen );

   outputBuffer[ newLen ] = 0;
   cout << outputBuffer << endl; 
}

void  TestBase64()
{
   std::string test = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Fusce ornare ullamcorper ipsum ac gravida.";

   char* outputBuffer = NULL;
   TestEncode( outputBuffer, test );

   string decoder( outputBuffer );
   char* output2 = NULL;
   TestDecode( output2, decoder);

   delete [] outputBuffer;
   delete [] output2;
}
*/