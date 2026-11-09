#include "nn/pia/Common.hpp"
#include "nn/pia/common/Md5Context.hpp"

namespace nn
{
namespace pia
{
namespace common
{

unsigned int hashWithMd5(unsigned int value)
{
    unsigned char input[4];
    unsigned char digest[16];
    serializeU32(input, value);
    Md5Context context;
    context.Initialize();
    context.Update(input, sizeof(input));
    context.GetHash(digest);
    return deserializeU32(digest);
}

unsigned int hashWithMd5(unsigned long long value)
{
    unsigned char input[8];
    unsigned char digest[16];
    serializeU32(input, static_cast<unsigned int>(value >> 32));
    serializeU32(input + 4, static_cast<unsigned int>(value));
    Md5Context context;
    context.Initialize();
    context.Update(input, sizeof(input));
    context.GetHash(digest);
    return deserializeU32(digest);
}

}
}
}
