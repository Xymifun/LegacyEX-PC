#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_SHA_BLOCK_512_BIT_CONTEXT_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_SHA_BLOCK_512_BIT_CONTEXT_HPP

#include <stddef.h>

#include "nn/pia/common/Hmac.hpp"

namespace nn
{
namespace pia
{
namespace common
{

class ShaBlock512BitContext : public HashContextBase
{
public:
    virtual unsigned int GetBlockSize() const;
    virtual void Update(const void* data, unsigned int size);
    void AddPadding();
    virtual void ProcessBlock();
    void Trace(unsigned long long value) const;

protected:
    unsigned char m_block[64];
    unsigned int m_blockSize;
    unsigned int m_blockCount;
    unsigned int m_blockCountHigh;
};

}
}
}

#endif
