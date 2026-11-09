#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_URLPROBE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_URLPROBE_HPP

#include "nn/nex/RootObject.hpp"
#include "nn/nex/StationURL.hpp"
#include "nn/nex/Time.hpp"

namespace nn
{
namespace nex
{

class URLProbe : public RootObject
{
public:
    static const unsigned int UNREACHED_PING_TIME = static_cast<unsigned int>(-1);

    URLProbe(const StationURL& rStationURL, int iTimeout, int iInterval, unsigned int uiMaxProbes);
    virtual ~URLProbe() {}

    bool Update(const StationURL& rStationURL, Time pingSendTime);
    bool UpdateIsNeeded(Time currentTime) const;
    void Trace(unsigned long long uiFilter);

    StationURL m_stationURL;
    Time m_expirationTime;
    unsigned int m_uiPingTime;
    Time m_lastUpdateTime;
    int m_iInterval;
    unsigned int m_uiProbeCount;
    unsigned int m_uiMaxProbes;
};

}
}

#endif
