#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_JOBPROCESSPROTOCOLMESSAGE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_JOBPROCESSPROTOCOLMESSAGE_HPP

#include <stdint.h>

#include "nn/nex/CallProtocolMethodOperation.hpp"
#include "nn/nex/JobProcessProtocolEvent.hpp"

namespace nn
{
namespace nex
{

class Buffer;

class JobProcessProtocolMessage : public JobProcessProtocolEvent
{
public:
    JobProcessProtocolMessage(ProtocolRequestBrokerInterface*, EndPoint*, Buffer*);
    virtual ~JobProcessProtocolMessage();
    virtual void Execute();
    virtual void Trace(uint64_t) {}

private:
    CallProtocolMethodOperation m_operation;
    Buffer* m_pBuffer;
};

}
}

#endif
