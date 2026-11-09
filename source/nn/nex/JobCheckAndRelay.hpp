#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_JOBCHECKANDRELAY_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_JOBCHECKANDRELAY_HPP

#include <list>

#include "nn/nex/CallContext.hpp"
#include "nn/nex/MemAllocator.hpp"
#include "nn/nex/StationURL.hpp"
#include "nn/nex/StepSequenceJob.hpp"
#include "nn/nex/Time.hpp"
#include "nn/nex/qList.hpp"

namespace nn
{
namespace nex
{

class JobCheckAndRelay : public StepSequenceJob
{
public:
    JobCheckAndRelay(CallContext* pCallContext, const qList<StationURL>& rLocalURLs, const StationURL& rStationURL);
    virtual ~JobCheckAndRelay();

    void StepCheck();
    void StepWaitForCheck();
    void StepRelay();
    void StepWaitForProbe();
    void StepFailure();

private:
    unsigned int m_uiCallId;
    CallContext m_callContext;
    Time m_deadline;
    qList<StationURL> m_localURLs;
    StationURL m_stationURL;
};

}
}

#endif
