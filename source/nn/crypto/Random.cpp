#include "nn/crypto/Random.hpp"

#include "nn/crypto/RandomImpl.hpp"

namespace nn
{
namespace crypto
{

void GenerateRandomBytes(void* pBuffer, unsigned int uiSize)
{
    detail::GenerateRandomBytesImpl(pBuffer, uiSize);
}

}
}
