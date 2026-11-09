#include "nn/nex/StationProbeList.hpp"

#include <algorithm>
#include <functional>
#include <vector>

#include "nn/nex/MemAllocator.hpp"
#include "nn/nex/NATTraversalEngine.hpp"
#include "nn/nex/Network.hpp"

namespace
{

const unsigned int UNREACHED_PING_TIME = static_cast<unsigned int>(-1);

struct AscendingPingSort
{
    bool operator()(unsigned int uiLeft, unsigned int uiRight) const { return uiLeft < uiRight; }
};

}

namespace nn
{
namespace nex
{

StationProbe::~StationProbe()
{
}

void StationProbe::Update()
{
    NATTraversalEngine* pEngine = Network::GetInstance()->GetNATTraversalEngine();
    bool bFailed = true;
    for (qList<StationURL>::const_iterator url = m_contactInfo.GetURLs().begin(); url != m_contactInfo.GetURLs().end();
         ++url)
    {
        unsigned int uiPingTime = pEngine->GetURLPingTime(*url);
        if (uiPingTime != UNREACHED_PING_TIME && uiPingTime < static_cast<unsigned int>(m_iPingTime))
        {
            m_iPingTime = uiPingTime;
            if (!pEngine->GetUpdatedURL(*url, &m_bestURL))
                m_bestURL = *url;
        }

        if (!pEngine->GetProbeFailed(*url))
            bFailed = false;
    }

    if (bFailed)
        m_bFailed = true;
}

void StationProbe::Reset()
{
    m_bestURL = StationURL(L"");
    m_iPingTime = -1;
}

StationProbeList::~StationProbeList()
{
}

void StationProbeList::Update()
{
    std::for_each(begin(), end(), std::mem_fun_ref(&StationProbe::Update));
}

StationProbeList::PingStats StationProbeList::GetPingStats() const
{
    std::vector<unsigned int, MemAllocator<unsigned int> > pingTimes;
    unsigned int uiTotal = 0;
    unsigned int uiCount = 0;
    for (const_iterator probe = begin(); probe != end(); ++probe)
    {
        pingTimes.push_back(probe->m_iPingTime);
        uiTotal += probe->m_iPingTime;
        ++uiCount;
    }

    PingStats stats;
    stats.m_uiMinimum = 0;
    stats.m_uiMaximum = 0;
    stats.m_uiAverage = 0;
    stats.m_uiMedian = 0;
    if (uiCount == 0)
        return stats;

    std::sort(pingTimes.begin(), pingTimes.end(), AscendingPingSort());

    stats.m_uiMinimum = pingTimes[0];
    stats.m_uiMaximum = pingTimes[uiCount - 1];
    stats.m_uiAverage = stats.m_uiMaximum;
    if (stats.m_uiMaximum != UNREACHED_PING_TIME)
        stats.m_uiAverage = uiTotal / uiCount;

    if (uiCount % 2 != 0)
    {
        stats.m_uiMedian = pingTimes[uiCount / 2];
    }
    else
    {
        stats.m_uiMedian = pingTimes[uiCount / 2];
        if (stats.m_uiMedian != UNREACHED_PING_TIME)
            stats.m_uiMedian = (pingTimes[uiCount / 2 - 1] + pingTimes[uiCount / 2]) / 2;
    }

    return stats;
}

bool StationProbeList::EveryoneReached() const
{
    return GetPingStats().m_uiMaximum != UNREACHED_PING_TIME;
}

bool StationProbeList::AnyoneFailed() const
{
    for (const_iterator probe = begin(); probe != end(); ++probe)
    {
        if (probe->m_bFailed)
            return true;
    }

    return false;
}

void StationProbeList::Reset()
{
    std::for_each(begin(), end(), std::mem_fun_ref(&StationProbe::Reset));
}

}
}
