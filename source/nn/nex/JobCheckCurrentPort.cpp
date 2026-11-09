#include "nn/nex/JobCheckCurrentPort.hpp"

#include "nn/nex/CallContextRegister.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/NATProperties.hpp"
#include "nn/nex/RootTransport.hpp"
#include "nn/nex/StationURL.hpp"
#include "nn/nex/StringStream.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

namespace
{

enum CheckResult
{
    NAT_CHECK_FAILED_RESULT = static_cast<int>(0x80010001u),
    NAT_CHECK_ABORTED_RESULT = static_cast<int>(0x80010004u),
    NAT_CHECK_ERROR_RESULT = static_cast<int>(0x80050021u),
    NAT_CHECK_SUCCEEDED_RESULT = 0x10001
};

enum MessageType
{
    MESSAGE_TYPE_ECHO = 0,
    MESSAGE_TYPE_TEST1_REQUEST = 0x65
};

const int SEND_REPEAT_COUNT = 5;
const unsigned int CHECK_TIMEOUT = 2500;
const unsigned short NAT_CHECK_ECHO_PORT = 33335;

unsigned int s_uiPreviousPortWord = 0;

}

JobCheckCurrentPort::JobCheckCurrentPort(CallContext* pCallContext, const InetAddress& rAddress, bool bRelayProbe)
    : JobNATCheckBase(pCallContext, rAddress),
      m_bRelayProbe(bRelayProbe)
{
    SetStep(Step(static_cast<Step::Function>(&JobCheckCurrentPort::StepInitialize), L""));
}

void JobCheckCurrentPort::StepInitializeImpl()
{
    SetStep(Step(static_cast<Step::Function>(&JobCheckCurrentPort::StepPrepareSocket), L""));
}

void JobCheckCurrentPort::StepPrepareSocket()
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        SetStep(Step(static_cast<Step::Function>(&JobCheckCurrentPort::StepFinish), L""));
        return;
    }
    if (!InitializeSocket())
    {
        pCallContext->SetStateImpl(CallContext::CallFailed, qResult(NAT_CHECK_FAILED_RESULT), false);
        SetToComplete();
        return;
    }
    SetStep(Step(static_cast<Step::Function>(&JobCheckCurrentPort::StepSend), L""));
}

void JobCheckCurrentPort::StepSend()
{
    if (RootTransport::GetInstance() == 0 || !RootTransport::GetInstance()->m_bNATCheckServerAddressesSet)
    {
        SetStep(Step(static_cast<Step::Function>(&JobCheckCurrentPort::StepFinish), L""));
        return;
    }
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        SetStep(Step(static_cast<Step::Function>(&JobCheckCurrentPort::StepFinish), L""));
        return;
    }
    const qVector<StringStream*>& rServerAddresses = RootTransport::GetInstance()->GetNATCheckServerAddresses();
    for (int i = 0; i < SEND_REPEAT_COUNT; ++i)
    {
        InetAddress echoAddress(StationURL(rServerAddresses[0]->GetBuffer()).GetInetAddress()->GetAddress(),
                                NAT_CHECK_ECHO_PORT);
        QueueMessage(echoAddress, MESSAGE_TYPE_ECHO);
    }
    for (int i = 0; i < SEND_REPEAT_COUNT; ++i)
    {
        if (RootTransport::GetInstance()->GetNATProperties().GetNATMapping()
                == NATProperties::MAPPING_ENDPOINT_DEPENDENT
            && rServerAddresses.size() > 1)
        {
            StationURL url(rServerAddresses[1]->GetBuffer());
            QueueMessage(*url.GetInetAddress(), MESSAGE_TYPE_TEST1_REQUEST);
        }
        else
        {
            StationURL url(rServerAddresses[0]->GetBuffer());
            QueueMessage(*url.GetInetAddress(), MESSAGE_TYPE_TEST1_REQUEST);
        }
    }
    ExecuteNATCheck(CHECK_TIMEOUT);
}

qResult JobCheckCurrentPort::HandleResult()
{
    if (RootTransport::GetInstance() == 0)
        return qResult(NAT_CHECK_ABORTED_RESULT);
    NATCheckMessage& rMessage = m_aReceived[0].m_message;
    unsigned int uiPortWord = rMessage.m_uiPortWord;
    if (uiPortWord == 0)
        return qResult(NAT_CHECK_ERROR_RESULT);
    if (m_bRelayProbe && uiPortWord < s_uiPreviousPortWord && CanRetry())
        return qResult(NAT_CHECK_ERROR_RESULT);
    s_uiPreviousPortWord = uiPortWord;
    {
        NATProperties properties = RootTransport::GetInstance()->GetNATProperties();
        properties.SetPublicPort(rMessage.GetPortNumber());
        properties.SetInterfacePort(m_localAddress.GetPortNumber());
        RootTransport::GetInstance()->SetNATProperties(properties);
        return qResult(NAT_CHECK_SUCCEEDED_RESULT);
    }
}

bool JobCheckCurrentPort::Finished()
{
    return m_aReceived[0].m_message.m_uiPortWord != 0;
}

}
}
