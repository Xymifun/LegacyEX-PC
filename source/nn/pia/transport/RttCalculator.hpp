#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_RTT_CALCULATOR_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_RTT_CALCULATOR_HPP
#include "nn/pia.h"
#include "nn/pia/common/Time.hpp"
#include "nn/pia/common/LatestMedian.hpp"
namespace nn
{
namespace pia
{
namespace transport
{
class RttCalculator
{
public:
    RttCalculator();
    ~RttCalculator();
    int GetRtt() const;
    int GetRtt(unsigned int count) const;
    void ResetRttMinMax();
    int GetRttMin() const;
    int GetRttMax() const;
    void Startup();
    void Cleanup();
    bool IsTimeOut() const;
    void SetLastRequestedTime(const common::Time& time);
    void Update(int rtt);
    static Result SetInitialPulseInterval(int interval);
    static Result SetPulseInterval(int interval);
    void Trace(unsigned long long value) const;

private:
    unsigned char m_started;
    int m_lastRtt;
    int m_rttMin;
    int m_rttMax;
    common::Time m_lastRequestedTime;
    common::LatestMedian<int, 9> m_latestMedian;
    static int s_pulseInterval;
    static int s_initialPulseInterval;
};
}
}
}
#endif
