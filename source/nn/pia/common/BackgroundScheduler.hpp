#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_BACKGROUND_SCHEDULER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_BACKGROUND_SCHEDULER_HPP

#include "nn/pia.h"
#include "nn/pia/common/PooledList.hpp"
#include "platform/wiiu/pead.hpp"

namespace nn
{
namespace pia
{
namespace common
{

class Job;

class BackgroundScheduler
{
public:
    BackgroundScheduler(int threadPriority, CriticalSection* criticalSection);
    ~BackgroundScheduler();
    void EntryJob(Job* job);
    void ResetJob(Job* job);
    void Dispatch(pead::Thread* thread, int message);
    void Trace(unsigned long long value) const;

    pead::Delegate2<BackgroundScheduler, pead::Thread*, int> m_delegate;
    pead::DelegateThread m_thread;
    CriticalSection* m_criticalSection;
    pead::Event m_event;
    bool m_finalize;
    OffsetList m_jobList;
};

}
}
}

#endif
