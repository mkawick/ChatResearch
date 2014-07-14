// Base64.cpp

#include "../DataTypes.h"
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <sstream>
#include <string>
using namespace std;
using namespace boost;
using namespace boost::archive::iterators;

#include "../ServerConstants.h"
#if PLATFORM == PLATFORM_WINDOWS
#pragma warning (disable: 4996)
#endif

U32 base64_encode( char* dest, const char* src, U32 len )
{
    char tail[3] = {0,0,0};
    typedef base64_from_binary<transform_width<const char *, 6, 8> > base64_enc;

    U32 one_third_len = len/3;
    U32 len_rounded_down = one_third_len*3;
    U32 j = len_rounded_down + one_third_len;

    std::copy(base64_enc(src), base64_enc(src + len_rounded_down), dest);

    if (len_rounded_down != len)
    {
        U32 i=0;
        for(; i < len - len_rounded_down; ++i)
        {
            tail[i] = src[len_rounded_down+i];
        }

        std::copy(base64_enc(tail), base64_enc(tail + 3), dest + j);

        for(i=len + one_third_len + 1; i < j+4; ++i)
        {
            dest[i] = '=';
        }

        return i;
    }

    return j;
}


const char* base64_decode( char* dest, const char* src, U32* len )
{
    U32 output_len = *len;

    typedef transform_width<binary_from_base64<const char*>, 8, 6> base64_dec;

    U32 i=0;
    try
    {
        base64_dec src_it(src);
        for(; i < output_len; ++i)
        {
            *dest++ = *src_it;
            ++src_it;
        }
    }
    catch(dataflow_exception&)
    {
    }

    *len = i;
    return src + (i+2)/3*4; // bytes in = bytes out / 3 rounded up * 4
}