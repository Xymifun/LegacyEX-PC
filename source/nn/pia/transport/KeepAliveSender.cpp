#include "nn/pia/transport/KeepAliveSender.hpp"

#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/transport/StationManager.hpp"

namespace nn
{
namespace pia
{
namespace transport
{

KeepAliveSender::KeepAliveSender() : m_interval(1000), m_enabled(true)
{
}

KeepAliveSender::~KeepAliveSender()
{
    if (this == 0)
        return;
}

Result KeepAliveSender::SetInterval(int interval)
{
    if (interval < 0)
    {
        return static_cast<Result>(0x10c07);
    }
    m_interval = static_cast<unsigned int>(interval);
    return 0;
}

unsigned int KeepAliveSender::Update(unsigned int stationBitmap, const common::Time& time)
{
    if (!m_enabled)
        return 0;
    StationManager* manager = StationManager::s_pInstance;
    const Station* localStation = manager->m_localStation;
    if (localStation == 0)
        return 0;

    const int intervalMsec = static_cast<int>(m_interval);
    const long long intervalTicks = common::TimeSpan::GetTicksPerMSec().m_tick * intervalMsec;
    const unsigned long long threshold = time.m_tick - intervalTicks;
    unsigned int expired = 0;
    for (Station** it = manager->m_stations.data; it != manager->m_stations.data + manager->m_stations.size; ++it)
    {
        Station* station = *it;
        if (station == localStation || station->m_field20 != 5 || station->m_field10 >= StationManager::cStationNumMax)
            continue;
        const unsigned int bit = 1u << station->m_field10;
        if ((stationBitmap & bit) != 0)
            station->m_lastSendTime = time;
        else if (station->m_lastSendTime.m_tick < threshold)
            expired |= bit;
    }
    return expired;
}

}
}
}
