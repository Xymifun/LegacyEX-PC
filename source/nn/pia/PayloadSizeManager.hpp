#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_PAYLOADSIZEMANAGER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_PAYLOADSIZEMANAGER_HPP

#include "nn/pia/Common.hpp"

namespace nn

{
namespace pia
{
namespace common
{

class PayloadSizeManager
{
public:
    PayloadSizeManager();
    ~PayloadSizeManager();
    static Result CreateInstance();
    static void DestroyInstance();
    Result SetMtuSize(unsigned int mtuSize);
    Result SetSignatureSize(unsigned int signatureSize);
    void Trace(unsigned long long value) const;

    unsigned int m_mtuSize;
    unsigned int m_minMtuSize;
    unsigned int m_maxMtuSize;
    unsigned int m_payloadSizeLimit;
    unsigned int m_signatureSize;
};

extern PayloadSizeManager* g_payloadSizeManager;

}
}
}

#endif
