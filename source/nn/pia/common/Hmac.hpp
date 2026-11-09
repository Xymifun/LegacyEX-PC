#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_HMAC_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_HMAC_HPP

namespace nn
{
namespace pia
{
namespace common
{

class HashContextBase
{
public:
    virtual void Initialize() = 0;
    virtual void Update(const void* data, unsigned int size) = 0;
    virtual unsigned int GetHashSize() const = 0;
    virtual unsigned int GetBlockSize() const = 0;
    virtual void GetHash(void* hash) = 0;
};

class Hmac
{
public:
    Hmac();
    void Initialize(HashContextBase* context, const void* key, unsigned int keySize);
    void Calc(void* hash, const void* data, unsigned int size);
    void Trace(unsigned long long value) const;

private:
    HashContextBase* m_context;
    unsigned int m_innerPad[16];
    unsigned int m_outerPad[16];
};

}
}
}

#endif
