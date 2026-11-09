#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_JOBNAMERESOLVE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_JOBNAMERESOLVE_HPP

#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/ObjectThread.hpp"
#include "nn/nex/StepSequenceJob.hpp"
#include "nn/nex/qResult.hpp"

namespace nn
{
namespace nex
{

class RootTransport;

extern unsigned int g_nameResolveThreadPriority;
extern int g_nameResolveThreadAffinity;

class JobNameResolve : public StepSequenceJob
{
public:
    typedef bool (RootTransport::*ResolveMethod)();
    typedef ObjectThread<JobNameResolve, int> Thread;

    JobNameResolve(unsigned int uiThreadPriority, int iThreadAffinity, ResolveMethod method);
    virtual ~JobNameResolve();

    void Initialize(unsigned int uiCallId);
    void CompleteJob(const qResult& rResult);
    void StepRunThread();
    void StepCheck();
    void StepCancelWait();
    void CheckThread(int iUnused);

private:
    unsigned int m_uiCallId;
    qResult m_result;
    Thread* m_pThread;
    ForcedCriticalSection m_criticalSection;
    unsigned int m_uiThreadPriority;
    int m_iThreadAffinity;
    ResolveMethod m_method;
};

}
}

#endif
