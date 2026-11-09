#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_RTT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_RTT_HPP

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class RTT : public RootObject
{
public:
    explicit RTT(unsigned int uiInitialRTT);
    virtual ~RTT();

    void Adjust(unsigned int uiRTT);
    void Trace(unsigned long long uiFilter);

    unsigned int GetAverage() const { return m_uiSmoothedRTT >> 3; }

    unsigned int GetVariation() const { return m_uiRTTVariation; }

    unsigned int GetLast() const { return m_uiLastRTT; }

private:
    unsigned int m_uiSmoothedRTT;
    unsigned int m_uiRTTVariation;
    unsigned int m_uiLastRTT;
    bool m_bFirstSample;
};

}
}

#endif
