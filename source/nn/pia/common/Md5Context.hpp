#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_MD5_CONTEXT_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_MD5_CONTEXT_HPP

#include "nn/pia/common/Hmac.hpp"

namespace nn
{
namespace pia
{
namespace common
{

class Md5Context : public HashContextBase
{
public:
    virtual void Initialize();
    virtual void Update(const void* data, unsigned int size);
    virtual unsigned int GetHashSize() const;
    virtual unsigned int GetBlockSize() const;
    virtual void GetHash(void* hash);
    virtual void ProcessBlock();

private:
    enum
    {
        cBlockSize = 64,
        cBlockWordNum = cBlockSize / 4,
        cStateWordNum = 4,
        cHashSize = cStateWordNum * 4,
        cLengthSize = 8
    };

    unsigned int m_state[cStateWordNum];
    unsigned int m_length;
    union
    {
        unsigned char bytes[cBlockSize];
        unsigned int words[cBlockWordNum];
    } m_block;
};

}
}
}

#endif
