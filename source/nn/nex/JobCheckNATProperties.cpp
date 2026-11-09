#include "nn/nex/JobCheckNATProperties.hpp"

#include "nn/nex/CallContextRegister.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/NATProperties.hpp"
#include "nn/nex/NATTraversalEngine.hpp"
#include "nn/nex/Network.hpp"
#include "nn/nex/RootTransport.hpp"
#include "nn/nex/StationURL.hpp"
#include "nn/nex/StringStream.hpp"
#include "nn/nex/Time.hpp"
#include "nn/nex/qList.hpp"
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
    MESSAGE_TYPE_TEST1_REQUEST = 0x65,
    MESSAGE_TYPE_TEST2_REQUEST = 0x66,
    MESSAGE_TYPE_TEST3_REQUEST = 0x67
};

enum StationType
{
    STATION_TYPE_BEHIND_NAT = 1,
    STATION_TYPE_PUBLIC = 2
};

const int SEND_REPEAT_COUNT = 5;
const unsigned int CHECK_TIMEOUT = 2500;
const unsigned short NAT_CHECK_ECHO_PORT = 33335;
const unsigned int UNKNOWN_ROUND_TRIP_TIME = 0x7FFFFFFF;
const unsigned long long NAT_TRACE_FILTER = 0x40000;

}

JobCheckNATProperties::JobCheckNATProperties(CallContext* pCallContext, const InetAddress& rAddress)
    : JobNATCheckBase(pCallContext, rAddress),
      m_sendTime(0),
      m_uiRoundTripTime(UNKNOWN_ROUND_TRIP_TIME)
{
    SetStep(Step(static_cast<Step::Function>(&JobCheckNATProperties::StepInitialize), L""));
}

void JobCheckNATProperties::StepInitializeImpl()
{
    SetStep(Step(static_cast<Step::Function>(&JobCheckNATProperties::StepPrepareSocket), L""));
}

void JobCheckNATProperties::StepPrepareSocket()
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        SetStep(Step(static_cast<Step::Function>(&JobCheckNATProperties::StepFinish), L""));
        return;
    }
    if (!InitializeSocket())
    {
        pCallContext->SetStateImpl(CallContext::CallFailed, qResult(NAT_CHECK_FAILED_RESULT), false);
        SetToComplete();
        return;
    }
    SetStep(Step(static_cast<Step::Function>(&JobCheckNATProperties::StepSend), L""));
}

void JobCheckNATProperties::StepSend()
{
    if (RootTransport::GetInstance() == 0 || !RootTransport::GetInstance()->m_bNATCheckServerAddressesSet)
    {
        SetStep(Step(static_cast<Step::Function>(&JobCheckNATProperties::StepFinish), L""));
        return;
    }
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext != 0)
    {
        const qVector<StringStream*>& rServerAddresses = RootTransport::GetInstance()->GetNATCheckServerAddresses();
        for (int i = 0; i < SEND_REPEAT_COUNT; ++i)
        {
            InetAddress echoAddress(StationURL(rServerAddresses[0]->GetBuffer()).GetInetAddress()->GetAddress(),
                                    NAT_CHECK_ECHO_PORT);
            QueueMessage(echoAddress, MESSAGE_TYPE_ECHO);
        }
        for (int i = 0; i < SEND_REPEAT_COUNT; ++i)
        {
            StationURL url(rServerAddresses[0]->GetBuffer());
            QueueMessage(*url.GetInetAddress(), MESSAGE_TYPE_TEST1_REQUEST);
        }
        for (int i = 0; i < SEND_REPEAT_COUNT; ++i)
        {
            StationURL url(rServerAddresses[0]->GetBuffer());
            QueueMessage(*url.GetInetAddress(), MESSAGE_TYPE_TEST2_REQUEST);
        }
        for (int i = 0; i < SEND_REPEAT_COUNT; ++i)
        {
            StationURL url(rServerAddresses[2]->GetBuffer());
            QueueMessage(*url.GetInetAddress(), MESSAGE_TYPE_TEST3_REQUEST);
        }
        ExecuteNATCheck(CHECK_TIMEOUT);
        m_sendTime = Time::GetTime();
        return;
    }
    SetStep(Step(static_cast<Step::Function>(&JobCheckNATProperties::StepFinish), L""));
}

qResult JobCheckNATProperties::HandleResult()
{
    if (Network::GetInstance() == 0 || RootTransport::GetInstance() == 0)
        return qResult(NAT_CHECK_ABORTED_RESULT);

    NATProperties properties;
    properties.SetInterfaceAddress(m_localAddress.GetAddressStr());
    properties.SetInterfacePort(m_localAddress.GetPortNumber());
    InetAddress publicAddress;
    publicAddress.SetAddress(m_aReceived[2].m_message.m_uiAddress);
    publicAddress.SetPortNumber(m_aReceived[2].m_message.GetPortNumber());
    properties.SetPublicAddress(publicAddress.GetAddressStr());
    properties.SetPublicPort(publicAddress.GetPortNumber());
    properties.SetNATFiltering(m_aReceived[1].m_message.m_uiPortWord == 0
                                   ? NATProperties::FILTERING_ENDPOINT_DEPENDENT
                                   : NATProperties::FILTERING_ENDPOINT_INDEPENDENT);

    qResult result;
    if (m_aReceived[0].m_message.m_uiPortWord == 0 || m_aReceived[2].m_message.m_uiPortWord == 0)
    {
        properties.SetNATMapping(NATProperties::MAPPING_UNKNOWN);
        result = qResult(NAT_CHECK_ERROR_RESULT);
    }
    else if (m_aReceived[0].m_message.m_uiPortWord == m_aReceived[2].m_message.m_uiPortWord)
    {
        properties.SetNATMapping(NATProperties::MAPPING_ENDPOINT_INDEPENDENT);
        RootTransport::GetInstance()->SetNATProperties(properties);
        result = qResult(NAT_CHECK_SUCCEEDED_RESULT);
    }
    else
    {
        properties.SetNATMapping(NATProperties::MAPPING_ENDPOINT_DEPENDENT);
        unsigned int uiFirstWord = m_aReceived[0].m_message.m_uiPortWord;
        unsigned int uiThirdWord = m_aReceived[2].m_message.m_uiPortWord;
        if (uiThirdWord < uiFirstWord)
            properties.SetNATMappingPortIncrement(static_cast<unsigned short>(uiFirstWord - uiThirdWord));
        else
            properties.SetNATMappingPortIncrement(static_cast<unsigned short>(uiThirdWord - uiFirstWord));
        if (m_aReceived[2].m_message.m_uiPortWord == properties.GetInterfacePort())
        {
            properties.m_bMappingPortIncrementValid = true;
            properties.SetNATMappingPortIncrement(1);
        }
        RootTransport::GetInstance()->SetNATProperties(properties);
        result = qResult(NAT_CHECK_SUCCEEDED_RESULT);
    }

    if (result)
    {
        qList<StationURL> localURLs;
        Network::GetInstance()->GetLocalURLs(&localURLs);
        unsigned int uiRVConnectionID = 0;
        unsigned char uiStreamID = 0;
        bool bPublicURLKnown = false;
        for (qList<StationURL>::iterator it = localURLs.begin(); it != localURLs.end(); ++it)
        {
            if (it->GetRVConnectionID() != 0)
            {
                uiRVConnectionID = it->GetRVConnectionID();
                uiStreamID = it->GetStreamID();
            }
            if ((it->GetType() & STATION_TYPE_PUBLIC) != 0)
            {
                bPublicURLKnown = true;
                break;
            }
        }
        if (!bPublicURLKnown)
        {
            InetAddress reportedAddress(properties.GetPublicAddress(), properties.GetPublicPort());
            StationURL publicURL;
            publicURL.SetInetAddress(&reportedAddress);
            publicURL.SetNATFiltering(properties.GetNATFiltering());
            publicURL.SetNATMapping(properties.GetNATMapping());
            publicURL.SetType(STATION_TYPE_BEHIND_NAT | STATION_TYPE_PUBLIC);
            publicURL.SetStreamID(uiStreamID);
            publicURL.SetRVConnectionID(uiRVConnectionID);
            publicURL.Trace(NAT_TRACE_FILTER);
            Network::GetInstance()->AddLocalURL(publicURL);
        }
    }

    NATTraversalEngine* pEngine = Network::GetInstance()->GetNATTraversalEngine();
    if (pEngine != 0)
        pEngine->ReportNATProperties(properties.GetNATMapping(), properties.GetNATFiltering(), m_uiRoundTripTime);
    return result;
}

bool JobCheckNATProperties::Finished()
{
    if (m_aReceived[0].m_message.m_uiPortWord != 0 && m_uiRoundTripTime == UNKNOWN_ROUND_TRIP_TIME)
        m_uiRoundTripTime = static_cast<unsigned int>(Time::GetTime() - m_sendTime);
    if (m_aReceived[0].m_message.m_uiPortWord != 0 && m_aReceived[2].m_message.m_uiPortWord != 0)
        return true;
    return false;
}

}
}
