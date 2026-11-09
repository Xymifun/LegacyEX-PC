#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_SCHEDULER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_SCHEDULER_HPP

#include "nn/pia.h"
#include "nn/pia/common/PooledList.hpp"
#include "nn/pia/common/Time.hpp"

namespace nn
{
namespace pia
{
namespace common
{

class Job;
class BackgroundScheduler;

class Scheduler
{
public:
    Scheduler(int threadPriority);
    ~Scheduler();
    static Result CreateInstance(int threadPriority);
    static void DestroyInstance();
    void Dispatch(unsigned int limitMsec);
    void EntryJob(Job* job, bool isBackground);
    void EntryJobNext(Job* job);
    void ResetJob(Job* job);
    void SetMonitoringData();
    unsigned int GetJobNum() const;
    void Trace(unsigned long long value) const;

    OffsetList m_jobList;
    OffsetList m_nextJobList;
    Time m_dispatchTime;
    unsigned int m_dispatchCount;
    CriticalSection m_criticalSection;
    BackgroundScheduler* m_backgroundScheduler;
};

extern Scheduler* g_piaJobScheduler;

}
}
}

#endif
