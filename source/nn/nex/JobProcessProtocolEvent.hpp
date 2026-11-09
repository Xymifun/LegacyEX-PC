#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_JOBPROCESSPROTOCOLEVENT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_JOBPROCESSPROTOCOLEVENT_HPP

#include <stdint.h>

#include "nn/nex/Job.hpp"

namespace nn
{
namespace nex
{

extern bool g_jobProcessProtocolEventCloseGuard;

class EndPoint;
class ProtocolRequestBrokerInterface;

class JobProcessProtocolEvent : public Job
{
public:
    JobProcessProtocolEvent(ProtocolRequestBrokerInterface*, EndPoint*, const wchar_t*);
    virtual ~JobProcessProtocolEvent();
    virtual void Trace(uint64_t) {}

    void SetBroker(ProtocolRequestBrokerInterface*);
    uint64_t GetLifeTime() const;
    uint32_t GetProcessingTime() const;
    void RecordProcessingStart();

protected:
    ProtocolRequestBrokerInterface* m_pBroker;
    EndPoint* m_pEndPoint;
    uint64_t m_lifeTime;
    uint32_t m_processingStart;
    uint32_t m_processingTime;
};

}
}

#endif
