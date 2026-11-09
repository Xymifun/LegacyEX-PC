#ifndef LEGACY_CONSOLE_WIIU_NN_CRYPTO_SHA1CONTEXT_HPP
#define LEGACY_CONSOLE_WIIU_NN_CRYPTO_SHA1CONTEXT_HPP

#include "nn/crypto/ShaBlock512BitContext.hpp"

namespace nn
{
namespace crypto
{

class Sha1Context : public ShaBlock512BitContext
{
public:
    enum
    {
        HashSize = 20
    };

    virtual void Initialize();
    virtual void Finalize() {}
    virtual void Update(const void* pData, unsigned int uiSize);
    virtual unsigned int GetHashSize() { return HashSize; }
    virtual void GetHash(void* pHash);
    virtual ~Sha1Context() {}
    virtual void ProcessBlock();
    virtual void InitializeWithContext(const void* pContext, unsigned long long uiSize);

private:
    enum
    {
        StateWordCount = 5
    };

    unsigned int m_state[StateWordCount];
};

void CalculateSha1(void* pHash, const void* pData, unsigned int uiSize);

}
}

#endif
