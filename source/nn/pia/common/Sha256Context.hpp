#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_SHA256_CONTEXT_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_SHA256_CONTEXT_HPP

#include "nn/pia/common/ShaBlock512BitContext.hpp"

namespace nn
{
namespace pia
{
namespace common
{

class Sha256Context : public ShaBlock512BitContext
{
public:
    virtual void Initialize();
    virtual void Update(const void* data, unsigned int size);
    virtual unsigned int GetHashSize() const;
    virtual void GetHash(void* hash);
    virtual void ProcessBlock();

private:
    unsigned int m_state[8];
};

}
}
}

#endif
