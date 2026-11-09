#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_STREAMBUNDLING_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_STREAMBUNDLING_HPP

#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/Time.hpp"

namespace nn
{
namespace nex
{

class StreamBundling : public RootObject
{
public:
    StreamBundling();
    virtual ~StreamBundling();

    StreamBundling& operator=(const StreamBundling& rOther)
    {
        m_bEnabled = rOther.m_bEnabled;
        m_iMaxFlushDelay = rOther.m_iMaxFlushDelay;
        m_lastFlushTime = rOther.m_lastFlushTime;
        m_unknown10 = rOther.m_unknown10;
        m_unknown12 = rOther.m_unknown12;
        return *this;
    }

    void Enable(int iMaxFlushDelay);
    void Flush();
    void SetMaxFlushDelay(int iMaxFlushDelay);
    void UpdateFlushTime();
    Time GetLastFlushTime();

    bool IsEnabled() const { return m_bEnabled; }

    unsigned short GetPlainBundleLimit() const { return m_unknown10; }

    unsigned short GetReliableBundleLimit() const { return m_unknown12; }

private:
    bool m_bEnabled;
    int m_iMaxFlushDelay;
    Time m_lastFlushTime;
    unsigned short m_unknown10;
    unsigned short m_unknown12;
    CriticalSection m_criticalSection;
};

}
}

#endif
