#include "nn/nex/JobCheckAndRelay.hpp"

#include "nn/nex/CallContextRegister.hpp"
#include "nn/nex/ConnectionManager.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/DebugString.hpp"
#include "nn/nex/InetAddress.hpp"
#include "nn/nex/NATProperties.hpp"
#include "nn/nex/NATRelayInterface.hpp"
#include "nn/nex/NATTraversalEngine.hpp"
#include "nn/nex/Network.hpp"
#include "nn/nex/RootTransport.hpp"
#include "nn/nex/qResult.hpp"

namespace nn
{
namespace nex
{

namespace
{

enum
{
    WAIT_CONTEXT_CHECK_AND_RELAY = 50
};

enum CheckAndRelayResult
{
    CHECK_AND_RELAY_FAILED_RESULT = static_cast<int>(0x80010001u),
    CHECK_AND_RELAY_SUCCEEDED_RESULT = 0x10001
};

const int CHECK_AND_RELAY_CHECK_TIMEOUT = 5000;

}

JobCheckAndRelay::JobCheckAndRelay(CallContext* pCallContext, const qList<StationURL>& rLocalURLs,
                                   const StationURL& rStationURL)
    : StepSequenceJob(JOB_TYPE_2, DebugString(L"")),
      m_deadline(),
      m_localURLs(rLocalURLs),
      m_stationURL(rStationURL)
{
    pCallContext->InitiateCall();
    m_uiCallId = pCallContext->GetCallID();
    SetStep(Step(static_cast<Step::Function>(&JobCheckAndRelay::StepCheck), L""));
}

JobCheckAndRelay::~JobCheckAndRelay()
{
}

void JobCheckAndRelay::StepCheck()
{
    if (Network::GetInstance() == 0)
    {
        SetStep(Step(static_cast<Step::Function>(&JobCheckAndRelay::StepFailure), L""));
        return;
    }

    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        SetStep(Step(static_cast<Step::Function>(&JobCheckAndRelay::StepFailure), L""));
        return;
    }

    NATTraversalEngine* pEngine = Network::GetInstance()->GetNATTraversalEngine();
    NATRelayInterface* pRelay = pEngine->m_pRelay;
    if (pRelay == 0)
    {
        SetStep(Step(static_cast<Step::Function>(&JobCheckAndRelay::StepFailure), L""));
        return;
    }

    pRelay->CheckCurrentPublicPort(&m_callContext, InetAddress(), true);
    Core::GetInstance()->GetCallContextRegister()->AssociateCallContext(m_uiCallId, m_callContext.GetCallID());
    SetStep(Step(static_cast<Step::Function>(&JobCheckAndRelay::StepWaitForCheck), L""));
    m_deadline = Time::GetTime() + CHECK_AND_RELAY_CHECK_TIMEOUT;
}

void JobCheckAndRelay::StepWaitForCheck()
{
    if (RootTransport::GetInstance() == 0)
    {
        SetStep(Step(static_cast<Step::Function>(&JobCheckAndRelay::StepFailure), L""));
        return;
    }

    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        SetStep(Step(static_cast<Step::Function>(&JobCheckAndRelay::StepFailure), L""));
        return;
    }

    Time currentTime = Time::GetTime();
    if (currentTime > m_deadline)
    {
        SetStep(Step(static_cast<Step::Function>(&JobCheckAndRelay::StepFailure), L""));
        return;
    }

    if (m_callContext.GetState() == CallContext::CallSucceeded)
    {
        NATProperties properties = RootTransport::GetInstance()->GetNATProperties();
        SetStep(Step(static_cast<Step::Function>(&JobCheckAndRelay::StepRelay), L""));
        return;
    }

    if (m_callContext.GetState() == CallContext::CallFailed)
        SetStep(Step(static_cast<Step::Function>(&JobCheckAndRelay::StepFailure), L""));

    SetToWaiting(WAIT_CONTEXT_CHECK_AND_RELAY);
}

void JobCheckAndRelay::StepRelay()
{
    if (Network::GetInstance() == 0 || RootTransport::GetInstance() == 0)
    {
        SetStep(Step(static_cast<Step::Function>(&JobCheckAndRelay::StepFailure), L""));
        return;
    }

    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        SetStep(Step(static_cast<Step::Function>(&JobCheckAndRelay::StepFailure), L""));
        return;
    }

    NATTraversalEngine* pEngine = Network::GetInstance()->GetNATTraversalEngine();
    NATRelayInterface* pRelay = pEngine->m_pRelay;
    if (pRelay == 0)
    {
        SetStep(Step(static_cast<Step::Function>(&JobCheckAndRelay::StepFailure), L""));
        return;
    }

    NATProperties properties = RootTransport::GetInstance()->GetNATProperties();
    m_stationURL.SetPortNumber(properties.GetPublicPort() + properties.GetNATMappingPortIncrement());
    pRelay->RelayProbeRequest(m_localURLs, m_stationURL);
    if (!m_stationURL.GetProbeRequestInitiation())
        pEngine->StartSendProbe(m_localURLs.front());

    SetStep(Step(static_cast<Step::Function>(&JobCheckAndRelay::StepWaitForProbe), L""));
    m_deadline = Time::GetTime() + ConnectionManager::s_iNATTraversalTimeout;
}

void JobCheckAndRelay::StepWaitForProbe()
{
    if (Network::GetInstance() == 0)
    {
        SetStep(Step(static_cast<Step::Function>(&JobCheckAndRelay::StepFailure), L""));
        return;
    }

    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        SetStep(Step(static_cast<Step::Function>(&JobCheckAndRelay::StepFailure), L""));
        return;
    }

    Time currentTime = Time::GetTime();
    if (currentTime > m_deadline)
    {
        SetStep(Step(static_cast<Step::Function>(&JobCheckAndRelay::StepFailure), L""));
        return;
    }

    NATTraversalEngine* pEngine = Network::GetInstance()->GetNATTraversalEngine();
    bool bReached = false;
    bool bAllFailed = true;
    for (qList<StationURL>::iterator it = m_localURLs.begin(); it != m_localURLs.end(); ++it)
    {
        if (pEngine->GetCIDPingTime(it->GetRVConnectionID()) != -1)
            bReached = true;

        if (!pEngine->GetProbeFailed(*it))
            bAllFailed = false;
    }

    if (bReached)
    {
        pCallContext->SetStateImpl(CallContext::CallSucceeded, qResult(CHECK_AND_RELAY_SUCCEEDED_RESULT), true);
        SetToComplete();
        return;
    }

    if (bAllFailed)
    {
        SetStep(Step(static_cast<Step::Function>(&JobCheckAndRelay::StepFailure), L""));
        return;
    }

    SetToWaiting(WAIT_CONTEXT_CHECK_AND_RELAY);
}

void JobCheckAndRelay::StepFailure()
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext != 0)
        pCallContext->SetStateImpl(CallContext::CallFailed, qResult(CHECK_AND_RELAY_FAILED_RESULT), false);

    SetToComplete();
}

}
}
