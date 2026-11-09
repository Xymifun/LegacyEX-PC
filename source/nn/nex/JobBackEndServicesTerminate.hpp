#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_JOBBACKENDSERVICESTERMINATE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_JOBBACKENDSERVICESTERMINATE_HPP

#include "nn/nex/CallContext.hpp"
#include "nn/nex/StepSequenceJob.hpp"
#include "nn/nex/qResult.hpp"

namespace nn
{
namespace nex
{

class BackEndServices;

class JobBackEndServicesTerminate : public StepSequenceJob
{
public:
    JobBackEndServicesTerminate(unsigned int uiCallId, BackEndServices* pBackEndServices);
    virtual ~JobBackEndServicesTerminate();

    void StepWaitOnPendingJobs();
    void StepWaitOnLoginCancel();
    void SetFinalState(CallContext::State state, const qResult& rResult);
    void StepInitLogoutIfRequired();
    void StepLogoutComplete();

private:
    BackEndServices* m_pBackEndServices;
    unsigned int m_uiCallId;
    CallContext m_callContext;
};

}
}

#endif
