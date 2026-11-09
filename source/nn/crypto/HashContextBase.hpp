#ifndef LEGACY_CONSOLE_WIIU_NN_CRYPTO_HASHCONTEXTBASE_HPP
#define LEGACY_CONSOLE_WIIU_NN_CRYPTO_HASHCONTEXTBASE_HPP

namespace nn
{
namespace crypto
{

class HashContextBase
{
public:
    virtual void Initialize() {}
    virtual void Finalize() {}
    virtual void Update(const void* pData, unsigned int uiSize) {}
    virtual unsigned int GetHashSize() { return 0; }
    virtual void GetHash(void* pHash) {}
    virtual ~HashContextBase() {}
    virtual void ProcessBlock() {}
};

}
}

#endif
