#include "nn/pia/inet/MonitoringDataSendJob.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

MonitoringDataSendJob::MonitoringDataSendJob()
{
    m_callContext.Reset();
}

MonitoringDataSendJob::~MonitoringDataSendJob()
{
}

void MonitoringDataSendJob::Cleanup()
{
    if (m_callContext.m_state == common::CallContext::cStateInProgress)
    {
        m_callContext.SignalCancel();
    }
    m_callContext.Reset();
    m_sender = 0;
}

void MonitoringDataSendJob::Trace(unsigned long long) const
{
}

}
}
}
