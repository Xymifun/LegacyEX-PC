#include "nn/crypto/RandomImpl.hpp"

#include "nn/crypto/Sha1Context.hpp"
#include "nn/crypto/iSeed.hpp"

#include <cstring>

namespace nn
{
namespace crypto
{
namespace detail
{

namespace
{

bool s_IsInitialized = false;
unsigned int s_State[5];

void InitializeState()
{
    unsigned char seed[4096];
    GetSeed(seed, sizeof(seed));
    std::memset(s_State, 0, sizeof(s_State));
    CalculateSha1(s_State, seed, sizeof(seed));
    s_IsInitialized = true;
}

void UpdateState()
{
    for (int i = 4; i >= 0; --i)
    {
        if (s_State[i] == 0xFFFFFFFF)
        {
            s_State[i] = 0;
        }
        else
        {
            ++s_State[i];
        }
    }
}

void GetRandom(void* pBuffer, unsigned int uiSize)
{
    unsigned char* pOut = static_cast<unsigned char*>(pBuffer);
    while (uiSize != 0)
    {
        unsigned int uiCopySize = uiSize;
        if (uiCopySize > sizeof(s_State))
        {
            uiCopySize = sizeof(s_State);
        }
        unsigned int hash[5];
        std::memset(hash, 0, sizeof(hash));
        CalculateSha1(hash, s_State, sizeof(s_State));
        UpdateState();
        std::memcpy(pOut, hash, uiCopySize);
        uiSize -= uiCopySize;
        pOut += uiCopySize;
    }
}

}

void GenerateRandomBytesImpl(void* pBuffer, unsigned int uiSize)
{
    if (!s_IsInitialized)
    {
        InitializeState();
    }
    GetRandom(pBuffer, uiSize);
}

}
}
}
