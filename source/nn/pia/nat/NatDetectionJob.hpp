#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_NAT_NAT_DETECTION_JOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_NAT_NAT_DETECTION_JOB_HPP

#include "nn/pia/Common.hpp"
#include "nn/pia/common/StepSequenceJob.hpp"
#include "nn/pia/common/Time.hpp"

namespace nn
{
namespace pia
{
namespace common
{
class CallContext;
}
namespace nat
{

class NatDetecter;
class NatTraverser;

class NatDetectionJob : public common::StepSequenceJob
{
public:
    NatDetectionJob();
    virtual ~NatDetectionJob();

    Result Startup(common::CallContext* context, NatDetecter* detecter, NatTraverser* traverser);
    void Cleanup();
    virtual void CancelCleanup();
    ExecuteResult StepPreTest();
    ExecuteResult StepStart();
    void StopReceivingMessage();
    ExecuteResult StepSend();
    ExecuteResult StepWait();
    ExecuteResult StepEnd();
    ExecuteResult StepComplete();

private:
    common::CallContext* m_context;
    NatDetecter* m_detecter;
    NatTraverser* m_traverser;
    common::Time m_endTime;
    common::Time m_waitTime;
    bool m_sendStarted;
    bool m_receivedFirstReply;
    common::Time m_firstSendTime;
    bool m_closeSocketFailed;
};

}
}
}

#endif
