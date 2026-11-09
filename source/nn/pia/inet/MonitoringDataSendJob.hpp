#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_MONITORINGDATASENDJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_MONITORINGDATASENDJOB_HPP

#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/common/StepSequenceJob.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

class MonitoringDataSender;

class MonitoringDataSendJob : public common::StepSequenceJob
{
public:
    MonitoringDataSendJob();
    virtual ~MonitoringDataSendJob();
    void Cleanup();
    void Trace(unsigned long long) const;
    ExecuteResult StepSendReport();
    Result Startup(MonitoringDataSender* sender);

private:
    common::CallContext m_callContext;
    MonitoringDataSender* m_sender;
};

}
}
}

#endif
