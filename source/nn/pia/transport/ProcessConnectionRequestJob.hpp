#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_PROCESSCONNECTIONREQUESTJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_PROCESSCONNECTIONREQUESTJOB_HPP

#include "nn/pia.h"
#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/common/StepSequenceJob.hpp"
#include "nn/pia/common/Time.hpp"

namespace nn
{
namespace pia
{
namespace transport
{

class ProcessConnectionRequestJob : public common::StepSequenceJob
{
public:
    enum
    {
        cResponseMessageNum = 2,
        cResponseOverheadSize = 0x40,
        cResponseDataSize = 0xfb,
        cDividedResponseDataSize = 0x1be
    };

    ProcessConnectionRequestJob();
    virtual ~ProcessConnectionRequestJob();
    bool Startup(Station* station, int timeoutMsec, bool secure);
    bool StartupRelayConnection(Station* station, int timeoutMsec, bool secure);
    ExecuteResult ConnectToRequesterStation();
    ExecuteResult WaitInverseConnection();
    bool TimeOutCheck();
    ExecuteResult SendConnectionResponse();
    ExecuteResult WaitResponseAck();
    void Cleanup();
    virtual void Trace(unsigned long long value) const;

private:
    common::Time m_deadline;
    int m_timeoutMsec;
    unsigned int m_responseAckIds[cResponseMessageNum];
    Station* m_station;
    common::CallContext* m_callContext;
    bool m_isRelayConnection;
};

}
}
}

#endif
