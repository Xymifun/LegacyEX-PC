#include "nn/nex/ProtocolRequestBrokerInterface.hpp"

#include "nn/nex/Buffer.hpp"
#include "nn/nex/DebugString.hpp"
#include "nn/nex/EndPoint.hpp"
#include "nn/nex/HighResolutionClock.hpp"
#include "nn/nex/JobProcessProtocolMessage.hpp"
#include "nn/nex/SecurityContextManager.hpp"
#include "nn/nex/Scheduler.hpp"

namespace nn
{
namespace nex
{

unsigned int (*g_protocolCurrentPID)() = 0;
unsigned int (*g_protocolCurrentCID)() = 0;

bool g_jobProcessProtocolEventCloseGuard = false;

namespace
{
bool g_protocolIdentityCallbacksInitialized = false;
}

ProtocolRequestBrokerInterface::ProtocolRequestBrokerInterface()
    : m_bandwidthCounters(String(L"Protocol")),
      m_operationManager()
{
    if (!g_protocolIdentityCallbacksInitialized)
    {
        g_protocolCurrentCID = SecurityContextManager::StaticGetCurrentCID;
        g_protocolCurrentPID = SecurityContextManager::StaticGetCurrentPID;
        g_protocolIdentityCallbacksInitialized = true;
    }
}

ProtocolRequestBrokerInterface::~ProtocolRequestBrokerInterface()
{
}

void ProtocolRequestBrokerInterface::InitMessage(Message* message, unsigned short protocolId,
                                                 Protocol::_Command command)
{
    Protocol::AddProtocolKey(message, command, protocolId);
}

JobProcessProtocolEvent::JobProcessProtocolEvent(ProtocolRequestBrokerInterface* pBroker, EndPoint* pEndPoint,
                                                 const wchar_t* pName)
    : Job(JOB_TYPE_2, DebugString(pName)),
      m_pBroker(pBroker),
      m_pEndPoint(pEndPoint)
{
    m_processingTime = 0;
    m_processingStart = 0;
    if (!g_jobProcessProtocolEventCloseGuard)
    {
        m_pEndPoint->Open();
        m_lifeTime = HighResolutionClock::GetHighResolutionTime();
    }
}

void JobProcessProtocolEvent::SetBroker(ProtocolRequestBrokerInterface* pBroker)
{
    m_pBroker = pBroker;
}

JobProcessProtocolMessage::JobProcessProtocolMessage(ProtocolRequestBrokerInterface* pBroker, EndPoint* pEndPoint,
                                                     Buffer* pBuffer)
    : JobProcessProtocolEvent(pBroker, pEndPoint, L"ProcessProtocoMessage"),
      m_operation(),
      m_pBuffer(pBuffer)
{
    pBuffer->AddRef();
}

void ProtocolRequestBrokerInterface::Receive(EndPoint* endPoint, Buffer* buffer)
{
    Scheduler::GetInstance()->Queue(new JobProcessProtocolMessage(this, endPoint, buffer), false);
}

JobProcessProtocolEvent::~JobProcessProtocolEvent()
{
    if (!g_jobProcessProtocolEventCloseGuard)
        m_pEndPoint->Close();
}

JobProcessProtocolMessage::~JobProcessProtocolMessage()
{
    m_pBuffer->ReleaseRef();
}

void JobProcessProtocolMessage::Execute()
{
    m_pBroker->ProcessMessage(&m_operation, m_pEndPoint, m_pBuffer);

    if (m_operation.m_postponed)
        SetToWaiting(m_operation.m_postponementDelay);
    else
        SetToComplete();
}

}
}
