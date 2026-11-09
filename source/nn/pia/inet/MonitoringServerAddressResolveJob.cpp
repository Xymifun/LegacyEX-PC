#include "nn/pia/inet/MonitoringServerAddressResolveJob.hpp"
#include "nn/pia/common/StepSequenceJob.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

MonitoringServerAddressResolveJob::MonitoringServerAddressResolveJob()
    : common::StepSequenceJob(),
      m_callContext(),
      m_monitoringServerId(0),
      m_address(0),
      m_sender(0)
{
    m_callContext.Reset();
}

MonitoringServerAddressResolveJob::~MonitoringServerAddressResolveJob()
{
}

void MonitoringServerAddressResolveJob::Cleanup()
{
    while (m_callContext.m_state == common::CallContext::cStateInProgress)
    {
        m_cancelRequested = true;
        WaitForCompletion(5);
    }
    m_callContext.Reset();
    m_monitoringServerId = 0;
    m_sender = 0;
    m_address = 0;
}

}
}
}
