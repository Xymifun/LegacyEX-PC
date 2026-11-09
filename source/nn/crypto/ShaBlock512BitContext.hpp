#ifndef LEGACY_CONSOLE_WIIU_NN_CRYPTO_SHABLOCK512BITCONTEXT_HPP
#define LEGACY_CONSOLE_WIIU_NN_CRYPTO_SHABLOCK512BITCONTEXT_HPP

#include "nn/crypto/HashContextBase.hpp"

namespace nn
{
namespace crypto
{

class ShaBlock512BitContext : public HashContextBase
{
public:
    virtual void Update(const void* pData, unsigned int uiSize);
    virtual ~ShaBlock512BitContext() {}
    virtual void ProcessBlock() {}
    virtual void InitializeWithContext(const void* pContext, unsigned long long uiSize) {}

protected:
    void AddPadding();

    enum
    {
        BlockSize = 64,
        BlockWordCount = BlockSize / 4
    };

    union
    {
        unsigned char bytes[BlockSize];
        unsigned int words[BlockWordCount];
    } m_block;
    unsigned int m_blockSize;
    unsigned int m_blockCount;
    unsigned int m_blockCountHigh;
};

}
}

#endif
