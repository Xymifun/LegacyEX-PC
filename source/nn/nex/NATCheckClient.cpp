#include "nn/nex/NATCheckClient.hpp"

#include "nn/nex/CallContext.hpp"
#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/JobCheckCurrentPort.hpp"
#include "nn/nex/JobCheckNATProperties.hpp"
#include "nn/nex/JobPerformNATCheck.hpp"
#include "nn/nex/Scheduler.hpp"

namespace nn
{
namespace nex
{

NATCheckClient::NATCheckClient(const InetAddress& rAddress) : m_address()
{
    m_address = rAddress;
}

NATCheckClient::~NATCheckClient()
{
}

bool NATCheckClient::DetectFullNATProperties(CallContext* pCallContext)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!pCallContext->InitiateCall())
        return false;

    JobPerformNATCheck* pJob = new JobPerformNATCheck(pCallContext, m_address);
    pJob->DetectFullNATProperties();
    Scheduler::GetInstance()->Queue(pJob, false);
    return true;
}

bool NATCheckClient::DetectSimplifiedNATProperties(CallContext* pCallContext)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!pCallContext->InitiateCall())
        return false;

    JobCheckNATProperties* pJob = new JobCheckNATProperties(pCallContext, m_address);
    Scheduler::GetInstance()->Queue(pJob, false);
    return true;
}

bool NATCheckClient::DetectNATCurrentPortMapping(CallContext* pCallContext, bool bRelayProbe)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!pCallContext->InitiateCall())
        return false;

    JobCheckCurrentPort* pJob = new JobCheckCurrentPort(pCallContext, m_address, bRelayProbe);
    Scheduler::GetInstance()->Queue(pJob, false);
    return true;
}

}
}
