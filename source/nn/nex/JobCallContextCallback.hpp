#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_JOBCALLCONTEXTCALLBACK_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_JOBCALLCONTEXTCALLBACK_HPP

#include "nn/nex/StepSequenceJob.hpp"

namespace nn
{
namespace nex
{

class CallContext;
class qResult;

class JobCallContextCallback : public StepSequenceJob
{
public:
    JobCallContextCallback(JobType jobType, const wchar_t* pName, CallContext* pCallContext);
    virtual ~JobCallContextCallback();

    virtual CallContext* CreateCallContext();
    virtual void StepCallCompletionImp();
    virtual void ProccessSuccess(CallContext* pCallContext);
    virtual void ProccessFailure(CallContext* pCallContext);
    virtual void ProccessCancel();

    CallContext* GetCallContext()
    {
        if (m_pJobCallContext == 0)
            m_pJobCallContext = CreateCallContext();

        return m_pJobCallContext;
    }

    bool StartJob();
    void CompleteJob(const qResult& rResult);
    void StepCallStart();
    void StepCallCompletion();
    void StepFinish();

protected:
    unsigned int m_uiCallId;
    CallContext* m_pCallContext;
    CallContext* m_pJobCallContext;
};

}
}

#endif
