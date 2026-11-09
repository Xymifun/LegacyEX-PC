#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_JOBSTARTNATSESSION_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_JOBSTARTNATSESSION_HPP

#include "nn/nex/CallContext.hpp"
#include "nn/nex/StepSequenceJob.hpp"
#include "nn/nex/qResult.hpp"

namespace nn
{
namespace nex
{

class JobResolveRelayInfo;

class JobStartNATSession : public StepSequenceJob
{
public:
    explicit JobStartNATSession(CallContext* pCallContext);
    virtual ~JobStartNATSession();

    bool StartJob();
    void CompleteJob(const qResult& rResult);
    void StepReady();
    void StepCallCompletion();
    void StepDNSCallCompletion();
    void StepFinish();

private:
    CallContext* m_pCallContext;
    CallContext m_sessionContext;
    unsigned int m_uiCallId;
    CallContext m_relayInfoContext;
    JobResolveRelayInfo* m_pRelayInfoJob;
    CallContext m_nameResolveContext;
};

}
}

#endif
