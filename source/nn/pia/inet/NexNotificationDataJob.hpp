#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXNOTIFICATIONDATAJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXNOTIFICATIONDATAJOB_HPP

#include "nn/nex/ProtocolCallContext.hpp"
#include "nn/pia/Common.hpp"
#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/common/StepSequenceJob.hpp"

namespace nn
{
namespace nex
{
template <typename T> class qList;
class NotificationEvent;
}
}
namespace nn
{
namespace pia
{
namespace common
{
class String;
}
}
}

namespace nn
{
namespace pia
{
namespace inet
{

class NexNotificationDataJob : public common::StepSequenceJob
{
public:
    NexNotificationDataJob();
    virtual ~NexNotificationDataJob();
    virtual Result Startup(common::CallContext* callContext, unsigned int type, unsigned int param1,
                           unsigned int param2, const common::String& text);
    virtual Result Startup(common::CallContext* callContext, unsigned char typeMask,
                           nex::qList<nex::NotificationEvent>* events);
    virtual void Cleanup();
    ExecuteResult WaitUpdateNotificationData();
    ExecuteResult WaitRetrieveNotificationData();

private:
    common::CallContext* m_callContext;
    nex::ProtocolCallContext m_protocolCallContext;
};

}
}
}

#endif
