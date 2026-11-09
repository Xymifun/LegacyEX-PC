#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_MD5_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_MD5_HPP

#include "nn/crypto/Md5Context.hpp"

namespace nn
{
namespace nex
{

class MD5 : public nn::crypto::Md5Context
{
public:
    enum
    {
        DigestSize = 16
    };

    MD5();

    void init();
    void update(const unsigned char* pInput, unsigned int uiLength);
    void finalize();
    void raw_digest(unsigned char* pDigest);

    const unsigned char* GetDigest() const { return m_digest; }

private:
    unsigned char m_digest[DigestSize];
    bool m_finalized;
};

}
}

#endif
