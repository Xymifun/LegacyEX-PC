#include "nn/nex/RTT.hpp"

namespace nn
{
namespace nex
{

RTT::RTT(unsigned int uiInitialRTT) : m_uiLastRTT(uiInitialRTT), m_bFirstSample(true)
{
    m_uiSmoothedRTT = uiInitialRTT << 3;
    m_uiRTTVariation = 0;
}

RTT::~RTT()
{
}

void RTT::Adjust(unsigned int uiRTT)
{
    if (m_bFirstSample)
    {
        m_uiLastRTT = uiRTT;
        m_uiSmoothedRTT = uiRTT << 3;
        m_bFirstSample = false;
        m_uiRTTVariation = uiRTT << 1;
        return;
    }
    int iError = uiRTT - (m_uiSmoothedRTT >> 3);
    m_uiSmoothedRTT += iError;
    if (iError < 0)
        iError = -iError;
    m_uiLastRTT = uiRTT;
    m_uiRTTVariation += iError - (m_uiRTTVariation >> 2);
}

void RTT::Trace(unsigned long long)
{
}

}
}
