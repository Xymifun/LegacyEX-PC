#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_STATIONPROBELIST_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_STATIONPROBELIST_HPP

#include "nn/nex/RootObject.hpp"
#include "nn/nex/StationContactInfo.hpp"
#include "nn/nex/StationURL.hpp"
#include "nn/nex/qList.hpp"

namespace nn
{
namespace nex
{

class StationProbe : public RootObject
{
public:
    ~StationProbe();

    void Update();
    void Reset();

    unsigned int m_uiReserved0;
    StationContactInfo m_contactInfo;
    StationURL m_bestURL;
    int m_iPingTime;
    unsigned int m_uiReserved1;
    bool m_bFailed;
};

class StationProbeList : public qList<StationProbe>
{
public:
    struct PingStats
    {
        unsigned int m_uiMinimum;
        unsigned int m_uiMaximum;
        unsigned int m_uiAverage;
        unsigned int m_uiMedian;
    };

    ~StationProbeList();

    void Update();
    PingStats GetPingStats() const;
    bool EveryoneReached() const;
    bool AnyoneFailed() const;
    void Reset();
};

}
}

#endif
