#include "nn/pia/common/Crypto.hpp"

#include "nn/crypto/Random.hpp"

namespace nn
{
namespace pia
{
namespace common
{
namespace crypto
{

void ClearMemory(void* destination, unsigned int size)
{
    unsigned char* bytes = static_cast<unsigned char*>(destination);
    for (int index = 0; index < static_cast<int>(size); ++index)
        bytes[index] = 0;
}

unsigned long long GenerateRandomSeed()
{
    unsigned long long seed = 0;
    nn::crypto::GenerateRandomBytes(&seed, sizeof(seed));
    return seed;
}

void GenerateRandomParameter(void* destination, unsigned int size)
{
    nn::crypto::GenerateRandomBytes(destination, size);
}

}
}
}
}
