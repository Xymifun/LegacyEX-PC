#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_PERIODICJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_PERIODICJOB_HPP

#include <stdint.h>

#include "nn/nex/Job.hpp"

namespace nn
{
namespace nex
{

class DebugString;

class PeriodicJob : public Job
{
public:
    PeriodicJob(JobType jobType, const DebugString& rDebugString);
    virtual ~PeriodicJob() {}

    virtual void SetDefaultPostExecutionState();
    virtual uint32_t SkipWaitDelayAtTermination();

    void SetPeriod(int iPeriod) { m_iPeriod = iPeriod; }

private:
    int m_iPeriod;
};

}
}

#endif
