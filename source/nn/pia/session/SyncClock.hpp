#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_SYNC_CLOCK_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_SYNC_CLOCK_HPP

#include "nn/pia/common/Time.hpp"

namespace nn
{
namespace pia
{
namespace session
{

class SyncClock
{
public:
    SyncClock();
    ~SyncClock();

    void SetBaseTime(const common::Time& time);
    void ClearBaseTime();
    unsigned long long GetTime() const;
    void Trace(unsigned long long value) const;

private:
    unsigned long long m_baseTime;
    bool m_hasBaseTime;
};

}
}
}

#endif
