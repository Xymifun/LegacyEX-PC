#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_JOBTERMINATE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_JOBTERMINATE_HPP

#include "nn/nex/CallContext.hpp"
#include "nn/nex/StepSequenceJob.hpp"

namespace nn
{
namespace nex
{

class NgsFacade;

class JobTerminate : public StepSequenceJob
{
public:
    JobTerminate(unsigned int uiCallId, NgsFacade* pNgsFacade);
    virtual ~JobTerminate();

    void StepWaitOnPendingJobs();

private:
    NgsFacade* m_pNgsFacade;
    unsigned int m_uiCallId;
    CallContext m_callContext;
};

}
}

#endif
