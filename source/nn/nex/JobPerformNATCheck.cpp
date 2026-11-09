#include "nn/nex/JobPerformNATCheck.hpp"

#include <cwchar>

#include "nn/nex/CallContext.hpp"
#include "nn/nex/CallContextRegister.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/DebugString.hpp"
#include "nn/nex/NATRelayInterface.hpp"
#include "nn/nex/RootTransport.hpp"
#include "nn/nex/StringStream.hpp"
#include "nn/nex/Time.hpp"
#include "nn/nex/qList.hpp"
#include "nn/nex/qResult.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

namespace
{

enum CheckMode
{
    CHECK_MODE_NONE = 0,
    CHECK_MODE_SIMPLIFIED = 1,
    CHECK_MODE_FULL = 2,
    CHECK_MODE_PORT_MAPPING = 3
};

enum CheckResult
{
    NAT_CHECK_FAILED_RESULT = static_cast<int>(0x80010001u),
    NAT_CHECK_SUCCEEDED_RESULT = 0x10001
};

enum MessageType
{
    MESSAGE_TYPE_ECHO = 0,
    MESSAGE_TYPE_TEST1_REQUEST = 0x65,
    MESSAGE_TYPE_TEST2_REQUEST = 0x66,
    MESSAGE_TYPE_TEST3_REQUEST = 0x67
};

enum TestNumber
{
    TEST_NONE = 0,
    TEST_1 = 1,
    TEST_2 = 2,
    TEST_3 = 3,
    TEST_4 = 4,
    TEST_5 = 5
};

const unsigned int SEND_REPEAT_COUNT = 5;
const unsigned int BIND_ATTEMPT_COUNT = 5;
const unsigned int MAX_TIMEOUT_COUNT = 3;
const unsigned int MAX_ATTEMPT_COUNT = 3;
const unsigned int PORT_MAPPING_ATTEMPT_LIMIT = 4;
const unsigned int INITIAL_TIMEOUT = 2500;
const unsigned int MINIMUM_TIMEOUT = 625;
const unsigned int RECEIVE_WAIT = 16;
const unsigned int RUN_WAIT = 100;
const unsigned short NAT_CHECK_ECHO_PORT = 33334;
const unsigned short MAXIMUM_PORT_NUMBER = 0xFFFF;

const wchar_t* s_pUnknownName = L"Unknown (u)";
const wchar_t* s_pEndpointIndependentName = L"Endpoint-Independent (ei)";
const wchar_t* s_pAddressDependentName = L"Address-Dependent (ad)";
const wchar_t* s_pAddressAndPortDependentName = L"Address and Port-Dependent (apd)";
unsigned int s_uiPreviousPortWord = 0;

}

JobPerformNATCheck::JobPerformNATCheck(CallContext* pCallContext, const InetAddress& rAddress)
    : StepSequenceJob(JOB_TYPE_2, DebugString(L"JobPerformNATCheck")),
      m_ioNotifierFirst(),
      m_ioNotifierSecond(),
      m_servers(),
      m_localAddress(rAddress),
      m_publicAddress(),
      m_receivedFrom(),
      m_mode(CHECK_MODE_NONE),
      m_recvMessage(),
      m_socket(0, 1000),
      m_pRelay(0),
      m_targetURL(),
      m_sourceURL(),
      m_uiTimeoutCount(0),
      m_uiRoundTripTime(0),
      m_uiTest(TEST_NONE),
      m_pMapping(s_pUnknownName),
      m_pFiltering(s_pUnknownName),
      m_deadline(0),
      m_sendTime(0),
      m_destination(),
      m_sendMessage(),
      m_bPortPreserved(false),
      m_uiFirstPassPort(0),
      m_uiFirstPassFinalPort(0),
      m_uiSecondPassPort(0),
      m_uiSecondPassFinalPort(0),
      m_uiAttempt(0),
      m_bResponsesReceived(false),
      m_unknown2C8(0),
      m_unknown2CC(0),
      m_queue(),
      m_natProperties(),
      m_aReceived(),
      m_bRelayProbe(false)
{
    m_uiCallId = pCallContext->m_uiCallId;
    SetStep(Step(static_cast<Step::Function>(&JobPerformNATCheck::StepPreTest), L""));
}

JobPerformNATCheck::~JobPerformNATCheck()
{
}

void JobPerformNATCheck::DetectFullNATProperties()
{
    m_mode = CHECK_MODE_FULL;
    m_bFirstPass = true;
}

void JobPerformNATCheck::DetectSimplifiedNATProperties()
{
    m_mode = CHECK_MODE_SIMPLIFIED;
}

bool JobPerformNATCheck::BindRandomPort()
{
    unsigned int uiAttempt = 0;
    while (uiAttempt < BIND_ATTEMPT_COUNT)
    {
        m_localAddress.SetPortNumber(RootTransport::GetNextPortNumber());
        if (m_socket.Bind(&m_localAddress, false))
            return true;
        ++uiAttempt;
    }
    return false;
}

bool JobPerformNATCheck::HandleSimplifiedResults()
{
    if (RootTransport::GetInstance() == 0 || m_aReceived[0].m_message.m_uiPortWord == 0
        || m_aReceived[2].m_message.m_uiPortWord == 0)
        return false;
    m_natProperties.SetInterfaceAddress(m_localAddress.GetAddressStr());
    m_natProperties.SetInterfacePort(m_localAddress.GetPortNumber());
    m_publicAddress.SetAddress(m_aReceived[2].m_message.m_uiAddress);
    m_publicAddress.SetPortNumber(m_aReceived[2].m_message.GetPortNumber());
    m_natProperties.SetPublicAddress(m_publicAddress.GetAddressStr());
    m_natProperties.SetPublicPort(m_publicAddress.GetPortNumber());
    m_natProperties.SetNATFiltering(m_aReceived[1].m_message.m_uiPortWord == 0
                                        ? NATProperties::FILTERING_ENDPOINT_DEPENDENT
                                        : NATProperties::FILTERING_ENDPOINT_INDEPENDENT);
    if (m_aReceived[0].m_message.m_uiPortWord == m_aReceived[2].m_message.m_uiPortWord)
    {
        m_natProperties.SetNATMapping(NATProperties::MAPPING_ENDPOINT_INDEPENDENT);
        RootTransport::GetInstance()->SetNATProperties(m_natProperties);
    }
    else
    {
        m_natProperties.SetNATMapping(NATProperties::MAPPING_ENDPOINT_DEPENDENT);
        unsigned int uiFirstWord = m_aReceived[0].m_message.m_uiPortWord;
        unsigned int uiThirdWord = m_aReceived[2].m_message.m_uiPortWord;
        if (uiThirdWord < uiFirstWord)
            m_natProperties.SetNATMappingPortIncrement(uiFirstWord - uiThirdWord);
        else
            m_natProperties.SetNATMappingPortIncrement(uiThirdWord - uiFirstWord);
        if (m_aReceived[2].m_message.m_uiPortWord == m_localAddress.GetPortNumber())
        {
            m_natProperties.m_bMappingPortIncrementValid = true;
            m_natProperties.SetNATMappingPortIncrement(1);
        }
        RootTransport::GetInstance()->SetNATProperties(m_natProperties);
    }
    return true;
}

bool JobPerformNATCheck::HandleFullResults()
{
    if (std::wcscmp(m_pMapping, s_pEndpointIndependentName) == 0)
    {
    }
    else if (std::wcscmp(m_pMapping, s_pAddressDependentName) == 0)
    {
    }
    else if (std::wcscmp(m_pMapping, s_pAddressAndPortDependentName) == 0)
    {
    }
    if (std::wcscmp(m_pFiltering, s_pEndpointIndependentName) == 0)
    {
    }
    else if (std::wcscmp(m_pFiltering, s_pAddressDependentName) == 0)
    {
    }
    else if (std::wcscmp(m_pFiltering, s_pAddressAndPortDependentName) == 0)
    {
    }
    if (std::wcscmp(m_pMapping, s_pAddressDependentName) == 0)
    {
    }
    else if (std::wcscmp(m_pMapping, s_pAddressAndPortDependentName) == 0)
    {
    }
    return true;
}

void JobPerformNATCheck::StepPreTest()
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        SetToComplete();
        return;
    }
    if (RootTransport::GetInstance() == 0 || !RootTransport::GetInstance()->m_bNATCheckServerAddressesSet)
    {
        pCallContext->SetStateImpl(CallContext::CallFailed, qResult(NAT_CHECK_FAILED_RESULT), false);
        SetToComplete();
        return;
    }
    if (m_mode == CHECK_MODE_NONE)
    {
        pCallContext->SetStateImpl(CallContext::CallFailed, qResult(NAT_CHECK_FAILED_RESULT), false);
        SetToComplete();
        return;
    }
    const qVector<StringStream*>& rServerAddresses = RootTransport::GetInstance()->GetNATCheckServerAddresses();
    for (qVector<StringStream*>::const_iterator it = rServerAddresses.begin(); it < rServerAddresses.end(); ++it)
    {
        if ((*it)->GetLength() != 0)
        {
            StationURL url((*it)->GetBuffer());
            m_servers.push_back(*url.GetInetAddress());
        }
    }
    if (m_servers.empty())
    {
        pCallContext->SetStateImpl(CallContext::CallFailed, qResult(NAT_CHECK_FAILED_RESULT), false);
        SetToComplete();
        return;
    }
    m_socket.Open(true);
    if ((m_localAddress.GetPortNumber() == 0 || !m_socket.Bind(&m_localAddress, false)) && !BindRandomPort())
    {
        m_socket.Close();
        pCallContext->SetStateImpl(CallContext::CallFailed, qResult(NAT_CHECK_FAILED_RESULT), false);
        SetToComplete();
        return;
    }
    if (m_mode == CHECK_MODE_PORT_MAPPING)
        SetStep(Step(static_cast<Step::Function>(&JobPerformNATCheck::StepStart), L""));
    else if (m_mode == CHECK_MODE_SIMPLIFIED)
        SetStep(Step(static_cast<Step::Function>(&JobPerformNATCheck::StepStart), L""));
    else if (m_mode == CHECK_MODE_FULL)
        SetStep(Step(static_cast<Step::Function>(&JobPerformNATCheck::StepTest1), L""));
}

void JobPerformNATCheck::StepStart()
{
    if (RootTransport::GetInstance() == 0)
    {
        CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
        if (pCallContext != 0)
            pCallContext->SetStateImpl(CallContext::CallFailed, qResult(NAT_CHECK_FAILED_RESULT), false);
        SetToComplete();
        return;
    }
    unsigned int uiIndex = 0;
    while (uiIndex < RECEIVED_MESSAGE_COUNT)
    {
        m_aReceived[uiIndex] = RecvNATCheckMessage();
        ++uiIndex;
    }
    m_deadline = Time::ConvertTimeoutToDeadline(INITIAL_TIMEOUT);
    unsigned int uiSent = 0;
    while (uiSent < SEND_REPEAT_COUNT)
    {
        QueueMessage(InetAddress(m_servers.front().GetAddress(), NAT_CHECK_ECHO_PORT), MESSAGE_TYPE_ECHO);
        ++uiSent;
    }
    ServerList::iterator second = m_servers.begin();
    bool bUseSecondServer = second != m_servers.end() && ++second != m_servers.end()
                            && m_mode == CHECK_MODE_PORT_MAPPING
                            && RootTransport::GetInstance()->GetNATProperties().GetNATMapping()
                                   == NATProperties::MAPPING_ENDPOINT_DEPENDENT;
    if (bUseSecondServer)
    {
        unsigned int uiSent = 0;
        while (uiSent < SEND_REPEAT_COUNT)
        {
            QueueMessage(*second, MESSAGE_TYPE_TEST1_REQUEST);
            ++uiSent;
        }
    }
    else
    {
        unsigned int uiSent = 0;
        while (uiSent < SEND_REPEAT_COUNT)
        {
            QueueMessage(m_servers.front(), MESSAGE_TYPE_TEST1_REQUEST);
            ++uiSent;
        }
    }
    if (m_mode == CHECK_MODE_SIMPLIFIED)
    {
        ServerList::iterator it = m_servers.begin();
        unsigned int uiSent = 0;
        while (uiSent < SEND_REPEAT_COUNT)
        {
            QueueMessage(*it, MESSAGE_TYPE_TEST2_REQUEST);
            ++uiSent;
        }
        ++it;
        while (it != m_servers.end())
        {
            if (it->GetAddress() != m_servers.front().GetAddress()
                && it->GetPortNumber() == m_servers.front().GetPortNumber())
            {
                unsigned int uiSent = 0;
                while (uiSent < SEND_REPEAT_COUNT)
                {
                    QueueMessage(*it, MESSAGE_TYPE_TEST3_REQUEST);
                    ++uiSent;
                }
                SetStep(Step(static_cast<Step::Function>(&JobPerformNATCheck::StepRun), L""));
                return;
            }
            ++it;
        }
    }
    SetStep(Step(static_cast<Step::Function>(&JobPerformNATCheck::StepRun), L""));
}

void JobPerformNATCheck::StepTest1()
{
    if (m_servers.empty())
    {
        SetStep(Step(static_cast<Step::Function>(&JobPerformNATCheck::StepEnd), L""));
        return;
    }
    m_destination = m_servers.front();
    m_uiTest = TEST_1;
    m_uiRoundTripTime = INITIAL_TIMEOUT;
    SetStep(Step(static_cast<Step::Function>(&JobPerformNATCheck::StepSend), L""));
}

void JobPerformNATCheck::StepTest2()
{
    m_uiTest = TEST_2;
    SetStep(Step(static_cast<Step::Function>(&JobPerformNATCheck::StepSend), L""));
}

void JobPerformNATCheck::StepTest3()
{
    m_uiTest = TEST_3;
    SetStep(Step(static_cast<Step::Function>(&JobPerformNATCheck::StepSend), L""));
}

void JobPerformNATCheck::StepTest4()
{
    m_servers.remove(m_destination);
    for (ServerList::iterator it = m_servers.begin(); it != m_servers.end(); ++it)
    {
        if (it->GetAddress() != m_destination.GetAddress() && it->GetPortNumber() == m_destination.GetPortNumber())
        {
            m_destination = *it;
            m_uiTest = TEST_4;
            SetStep(Step(static_cast<Step::Function>(&JobPerformNATCheck::StepSend), L""));
            return;
        }
    }
    SetStep(Step(static_cast<Step::Function>(&JobPerformNATCheck::StepEnd), L""));
}

void JobPerformNATCheck::StepTest5()
{
    m_servers.remove(m_destination);
    for (ServerList::iterator it = m_servers.begin(); it != m_servers.end(); ++it)
    {
        if (it->GetAddress() == m_destination.GetAddress() && it->GetPortNumber() != m_destination.GetPortNumber())
        {
            m_destination = *it;
            m_uiTest = TEST_5;
            SetStep(Step(static_cast<Step::Function>(&JobPerformNATCheck::StepSend), L""));
            return;
        }
    }
    SetStep(Step(static_cast<Step::Function>(&JobPerformNATCheck::StepEnd), L""));
}

void JobPerformNATCheck::StepSend()
{
    m_sendMessage.m_uiType = m_uiTest;
    m_sendMessage.m_uiTargetAddress = m_publicAddress.GetAddress();
    m_sendMessage.ToNetworkByteOrder();
    m_socket.Send(reinterpret_cast<const unsigned char*>(&m_sendMessage), sizeof(NATCheckMessage), m_destination);
    m_sendTime = Time::GetTime();
    unsigned int uiTimeout = m_uiRoundTripTime + (m_uiRoundTripTime >> 2);
    if (uiTimeout < MINIMUM_TIMEOUT)
        uiTimeout = MINIMUM_TIMEOUT;
    m_deadline = Time::ConvertTimeoutToDeadline(uiTimeout);
    SetStep(Step(static_cast<Step::Function>(&JobPerformNATCheck::StepRecv), L""));
}

void JobPerformNATCheck::StepRecv()
{
    unsigned int uiReceived = 0;
    if (m_socket.RecvNonBlock(reinterpret_cast<unsigned char*>(&m_recvMessage), sizeof(NATCheckMessage), &uiReceived,
                              &m_receivedFrom)
        == SOCKET_SUCCESS)
    {
        m_uiTimeoutCount = 0;
        m_recvMessage.FromNetworkByteOrder();
        MessageReceived(m_recvMessage);
        return;
    }
    Time now(Time::GetTime());
    if (now > m_deadline)
    {
        ++m_uiTimeoutCount;
        TimeoutReceived();
        return;
    }
    SetToWaiting(RECEIVE_WAIT);
}

void JobPerformNATCheck::MessageReceived(const NATCheckMessage& rMessage)
{
    if (rMessage.m_uiType != m_uiTest)
        return;
    InetAddress reportedAddress;
    reportedAddress.SetAddress(rMessage.m_uiAddress);
    reportedAddress.SetPortNumber(rMessage.GetPortNumber());
    switch (rMessage.m_uiType)
    {
    case TEST_1:
    {
        m_uiRoundTripTime = static_cast<unsigned int>(Time::GetTime() - m_sendTime);
        m_publicAddress = reportedAddress;
        if (m_publicAddress.GetPortNumber() == m_localAddress.GetPortNumber())
            m_bPortPreserved = true;
        unsigned short uiPort = reportedAddress.GetPortNumber();
        if (m_bFirstPass)
            m_uiFirstPassPort = uiPort;
        else
            m_uiSecondPassPort = uiPort;
        if (m_publicAddress == m_localAddress)
        {
            m_pMapping = s_pEndpointIndependentName;
            m_pFiltering = s_pEndpointIndependentName;
            SetStep(Step(static_cast<Step::Function>(&JobPerformNATCheck::StepEnd), L""));
            break;
        }
        SetStep(Step(static_cast<Step::Function>(&JobPerformNATCheck::StepTest2), L""));
        break;
    }
    case TEST_2:
        m_pFiltering = s_pEndpointIndependentName;
        SetStep(Step(static_cast<Step::Function>(&JobPerformNATCheck::StepTest4), L""));
        break;
    case TEST_3:
        m_pFiltering = s_pAddressDependentName;
        SetStep(Step(static_cast<Step::Function>(&JobPerformNATCheck::StepTest4), L""));
        break;
    case TEST_4:
        if (reportedAddress.GetPortNumber() == m_publicAddress.GetPortNumber())
        {
            unsigned short uiPort = reportedAddress.GetPortNumber();
            if (m_bFirstPass)
                m_uiFirstPassFinalPort = uiPort;
            else
                m_uiSecondPassFinalPort = uiPort;
            m_pMapping = s_pEndpointIndependentName;
            SetStep(Step(static_cast<Step::Function>(&JobPerformNATCheck::StepEnd), L""));
        }
        else
        {
            m_publicAddress = reportedAddress;
            SetStep(Step(static_cast<Step::Function>(&JobPerformNATCheck::StepTest5), L""));
        }
        break;
    case TEST_5:
    {
        unsigned short uiPort = reportedAddress.GetPortNumber();
        if (m_bFirstPass)
            m_uiFirstPassFinalPort = uiPort;
        else
            m_uiSecondPassFinalPort = uiPort;
        if (reportedAddress.GetPortNumber() == m_publicAddress.GetPortNumber())
            m_pMapping = s_pAddressDependentName;
        else
            m_pMapping = s_pAddressAndPortDependentName;
        m_publicAddress = reportedAddress;
        SetStep(Step(static_cast<Step::Function>(&JobPerformNATCheck::StepEnd), L""));
        break;
    }
    default:
        break;
    }
}

void JobPerformNATCheck::TimeoutReceived()
{
    switch (m_uiTest)
    {
    case TEST_1:
    {
        m_servers.erase(m_servers.begin());
        ServerList::iterator it = m_servers.begin();
        while (it != m_servers.end())
        {
            if (it->GetPortNumber() == m_destination.GetPortNumber())
                break;
            ++it;
            m_servers.erase(m_servers.begin());
        }
        m_uiTimeoutCount = 0;
        SetStep(Step(static_cast<Step::Function>(&JobPerformNATCheck::StepTest1), L""));
        break;
    }
    case TEST_2:
        if (m_uiTimeoutCount < MAX_TIMEOUT_COUNT)
        {
            SetStep(Step(static_cast<Step::Function>(&JobPerformNATCheck::StepTest2), L""));
        }
        else
        {
            m_uiTimeoutCount = 0;
            SetStep(Step(static_cast<Step::Function>(&JobPerformNATCheck::StepTest3), L""));
        }
        break;
    case TEST_3:
        if (m_uiTimeoutCount < MAX_TIMEOUT_COUNT)
        {
            SetStep(Step(static_cast<Step::Function>(&JobPerformNATCheck::StepTest3), L""));
        }
        else
        {
            m_uiTimeoutCount = 0;
            m_pFiltering = s_pAddressAndPortDependentName;
            SetStep(Step(static_cast<Step::Function>(&JobPerformNATCheck::StepTest4), L""));
        }
        break;
    case TEST_4:
        if (m_uiTimeoutCount < MAX_TIMEOUT_COUNT)
        {
            SetStep(Step(static_cast<Step::Function>(&JobPerformNATCheck::StepTest4), L""));
        }
        else
        {
            m_uiTimeoutCount = 0;
            SetStep(Step(static_cast<Step::Function>(&JobPerformNATCheck::StepEnd), L""));
        }
        break;
    case TEST_5:
        if (m_uiTimeoutCount < MAX_TIMEOUT_COUNT)
        {
            SetStep(Step(static_cast<Step::Function>(&JobPerformNATCheck::StepTest5), L""));
        }
        else
        {
            m_uiTimeoutCount = 0;
            SetStep(Step(static_cast<Step::Function>(&JobPerformNATCheck::StepEnd), L""));
        }
        break;
    default:
        break;
    }
}

bool JobPerformNATCheck::SendNextQueuedMessage()
{
    if (m_queue.empty())
        return false;
    bool bSent = true;
    if (m_socket.Send(reinterpret_cast<const unsigned char*>(&m_queue.begin()->second), sizeof(NATCheckMessage),
                      m_queue.begin()->first)
        != SOCKET_SUCCESS)
        bSent = false;
    m_queue.pop_front();
    return bSent;
}

void JobPerformNATCheck::StepRun()
{
    uint64_t now = Time::GetTime();
    if (now < m_deadline && !m_bResponsesReceived)
    {
        bool bSent;
        do
        {
            bSent = SendNextQueuedMessage();
        } while (bSent);
        unsigned int uiReceived = 0;
        if (m_socket.RecvNonBlock(reinterpret_cast<unsigned char*>(&m_recvMessage), sizeof(NATCheckMessage),
                                  &uiReceived, &m_receivedFrom)
                == SOCKET_SUCCESS
            && uiReceived == sizeof(NATCheckMessage))
        {
            m_recvMessage.FromNetworkByteOrder();
            unsigned int uiIndex = m_recvMessage.m_uiType - MESSAGE_TYPE_TEST1_REQUEST;
            if (uiIndex < RECEIVED_MESSAGE_COUNT)
            {
                m_aReceived[uiIndex].m_message = m_recvMessage;
                ++m_aReceived[uiIndex].m_uiCount;
            }
            if (m_mode == CHECK_MODE_PORT_MAPPING)
            {
                if (m_aReceived[0].m_uiCount != 0)
                    m_bResponsesReceived = true;
            }
            else if (m_mode == CHECK_MODE_SIMPLIFIED)
            {
                if (m_aReceived[0].m_uiCount != 0 && m_aReceived[1].m_uiCount != 0 && m_aReceived[2].m_uiCount != 0)
                    m_bResponsesReceived = true;
            }
        }
        SetToWaiting(RUN_WAIT);
        return;
    }
    unsigned int uiFirstWord = m_aReceived[0].m_message.m_uiPortWord;
    if (m_mode != CHECK_MODE_PORT_MAPPING)
    {
        if ((uiFirstWord == 0 || m_aReceived[2].m_message.m_uiPortWord == 0) && m_uiAttempt < MAX_ATTEMPT_COUNT)
        {
            m_bResponsesReceived = false;
            ++m_uiAttempt;
            SetStep(Step(static_cast<Step::Function>(&JobPerformNATCheck::StepStart), L""));
            return;
        }
        SetStep(Step(static_cast<Step::Function>(&JobPerformNATCheck::StepEnd), L""));
        return;
    }
    if (uiFirstWord != 0 || m_uiAttempt >= PORT_MAPPING_ATTEMPT_LIMIT)
    {
        if (uiFirstWord >= s_uiPreviousPortWord || !m_bRelayProbe || m_uiAttempt >= PORT_MAPPING_ATTEMPT_LIMIT)
        {
            s_uiPreviousPortWord = uiFirstWord;
            SetStep(Step(static_cast<Step::Function>(&JobPerformNATCheck::StepSendProbeRequest), L""));
            return;
        }
        m_socket.Close();
        m_socket.Open(true);
        BindRandomPort();
    }
    m_bResponsesReceived = false;
    ++m_uiAttempt;
    SetStep(Step(static_cast<Step::Function>(&JobPerformNATCheck::StepStart), L""));
}

void JobPerformNATCheck::StepSendProbeRequest()
{
    InetAddress probeAddress(m_aReceived[0].m_message.m_uiAddress, m_aReceived[0].m_message.GetPortNumber());
    m_sourceURL.SetInetAddress(&probeAddress);
    short iPortIncrement = RootTransport::GetInstance()->GetNATProperties().GetNATMappingPortIncrement();
    unsigned short uiPort = m_sourceURL.GetPortNumber() + iPortIncrement;
    if (uiPort >= MAXIMUM_PORT_NUMBER)
        uiPort = MAXIMUM_PORT_NUMBER;
    m_sourceURL.SetPortNumber(uiPort);
    if (m_bRelayProbe)
    {
        qList<StationURL> targetURLs;
        targetURLs.push_back(m_targetURL);
        m_pRelay->RelayProbeRequest(targetURLs, m_sourceURL);
    }
    SetStep(Step(static_cast<Step::Function>(&JobPerformNATCheck::StepEnd), L""));
}

void JobPerformNATCheck::StepEnd()
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        SetToComplete();
        return;
    }
    if (RootTransport::GetInstance() == 0)
    {
        pCallContext->SetStateImpl(CallContext::CallFailed, qResult(NAT_CHECK_FAILED_RESULT), false);
        SetToComplete();
        return;
    }
    m_socket.Close();
    bool bSucceeded = true;
    if (m_mode == CHECK_MODE_SIMPLIFIED)
    {
        bSucceeded = HandleSimplifiedResults();
    }
    else if (m_mode == CHECK_MODE_FULL)
    {
        if (m_bFirstPass)
        {
            m_servers.clear();
            m_localAddress.SetAddress(L"");
            m_localAddress.SetPortNumber(0);
            m_uiTest = TEST_NONE;
            m_sendTime = 0;
            m_bFirstPass = false;
            m_deadline = 0;
            m_pMapping = s_pUnknownName;
            m_uiRoundTripTime = 0;
            m_uiTimeoutCount = 0;
            m_pFiltering = s_pUnknownName;
            SetStep(Step(static_cast<Step::Function>(&JobPerformNATCheck::StepPreTest), L""));
            return;
        }
        bSucceeded = HandleFullResults();
    }
    else if (m_mode == CHECK_MODE_PORT_MAPPING)
    {
        if (!m_bRelayProbe)
        {
            NATProperties properties = RootTransport::GetInstance()->GetNATProperties();
            properties.SetPublicPort(m_aReceived[0].m_message.GetPortNumber());
            properties.SetInterfacePort(m_localAddress.GetPortNumber());
            RootTransport::GetInstance()->SetNATProperties(properties);
        }
    }
    else
    {
        bSucceeded = false;
    }
    if (bSucceeded)
        pCallContext->SetStateImpl(CallContext::CallSucceeded, qResult(NAT_CHECK_SUCCEEDED_RESULT), true);
    else
        pCallContext->SetStateImpl(CallContext::CallFailed, qResult(NAT_CHECK_FAILED_RESULT), false);
    SetToComplete();
}

}
}
