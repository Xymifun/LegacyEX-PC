#ifndef LEGACY_CONSOLE_WIIU_NN_CRYPTO_MD5CONTEXT_HPP
#define LEGACY_CONSOLE_WIIU_NN_CRYPTO_MD5CONTEXT_HPP

#include "nn/crypto/HashContextBase.hpp"

namespace nn
{
namespace crypto
{

class Md5Context : public HashContextBase
{
public:
    virtual void Initialize();
    virtual void Finalize();
    virtual void Update(const void* pData, unsigned int uiSize);
    virtual unsigned int GetHashSize();
    virtual void GetHash(void* pHash);
    virtual ~Md5Context() {}
    virtual void ProcessBlock();
    virtual unsigned int GetHashLength();

protected:
    enum
    {
        BlockSize = 64,
        BlockWordCount = BlockSize / 4,
        StateWordCount = 4
    };

    unsigned int m_state[StateWordCount];
    unsigned int m_length;
    union
    {
        unsigned char bytes[BlockSize];
        unsigned int words[BlockWordCount];
    } m_block;
};

}
}

#endif
