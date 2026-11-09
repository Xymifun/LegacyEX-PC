#include "nn/nex/SecureConnectionProtocolClient.hpp"

#include "nn/nex/ConnectionData.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/DDLClassMetaType.hpp"
#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/Message.hpp"
#include "nn/nex/ProtocolCallContext.hpp"
#include "nn/nex/ProtocolRegistry.hpp"
#include "nn/nex/ProtocolRequestBrokerInterface.hpp"
#include "nn/nex/RVClientCore.hpp"
#include "nn/nex/Scheduler.hpp"
#include "nn/nex/StationURL.hpp"
#include "nn/nex/_Type_qresult.hpp"
#include "nn/nex/qBuffer.hpp"
#include "nn/nex/qResult.hpp"

namespace nn
{
namespace nex
{

namespace
{

typedef void (*ProtoReturnFunction)(Message* pMessage, ProtocolCallContext* pCallContext);
typedef _Type_any<DDLClassMetaType<Data>, _Type_string> CustomDataAny;

const unsigned int METHOD_ID_MASK = 0x7FFF;
enum
{
    RESULT_NOT_IMPLEMENTED = static_cast<int>(0x80010002u)
};
const unsigned int METHOD_ID_COUNT = 9;
const unsigned int PROTOCOL_FLAG_UNLOCK_BEFORE_SEND = 0x10;
enum MethodId
{
    METHOD_REGISTER = 1,
    METHOD_REQUEST_CONNECTION_DATA = 2,
    METHOD_REQUEST_URLS = 3,
    METHOD_REGISTER_EX = 4,
    METHOD_TEST_CONNECTIVITY = 5,
    METHOD_UPDATE_URLS = 6,
    METHOD_REPLACE_URL = 7,
    METHOD_SEND_REPORT = 8
};
ProtoReturnFunction s_ProtoReturn[METHOD_ID_COUNT];

}

SecureConnectionProtocolClient::SecureConnectionProtocolClient(unsigned short uiProtocolID) : ClientProtocol(1)
{
    if (uiProtocolID == 0)
        return;
    SetProtocolID(uiProtocolID);
    RVClientCore::GetInstance()->GetProtocolRegistry()->RegisterProtocol(uiProtocolID,
                                                                         new SecureConnectionProtocolClient());
}

void SecureConnectionProtocolClient::ExtractCallSpecificResults(Message* pMessage, ProtocolCallContext* pCallContext)
{
    const unsigned int uiMethodID = Protocol::ExtractMethodID(pMessage) & METHOD_ID_MASK;
    if (uiMethodID - 1 >= METHOD_ID_COUNT - 1)
    {
        ClientProtocol::SetCallError(qResult(RESULT_NOT_IMPLEMENTED));
        return;
    }
    if (s_ProtoReturn[uiMethodID] != 0)
        s_ProtoReturn[uiMethodID](pMessage, pCallContext);
}

Protocol* SecureConnectionProtocolClient::CreateResponder() const
{
    return new SecureConnectionProtocolClient();
}

bool SecureConnectionProtocolClient::CallRegister(ProtocolCallContext* pCallContext, qResult* pResult,
                                                  const qList<StationURL>& rURLs, unsigned int* pConnectionID,
                                                  StationURL* pPublicURL)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_REGISTER);
    _Type_std_list<_Type_stationurl>::Add(&message, rURLs);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pResult);
        pCallContext->AddReturnValuePtr(pConnectionID);
        pCallContext->AddReturnValuePtr(pPublicURL);
    }
    s_ProtoReturn[METHOD_REGISTER] = ProtoReturn_Register;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool SecureConnectionProtocolClient::Register_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                             qResult* pResult, const qList<StationURL>& rURLs,
                                                             unsigned int* pConnectionID, StationURL* pPublicURL)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_REGISTER);
    _Type_std_list<_Type_stationurl>::Add(pMessage, rURLs);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pResult);
        pCallContext->AddReturnValuePtr(pConnectionID);
        pCallContext->AddReturnValuePtr(pPublicURL);
    }
    s_ProtoReturn[METHOD_REGISTER] = ProtoReturn_Register;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool SecureConnectionProtocolClient::CallRequestConnectionData(ProtocolCallContext* pCallContext, bool* pSuccess,
                                                               const unsigned int& rConnectionID,
                                                               const unsigned int& rPrincipalID,
                                                               qList<ConnectionData>* pConnectionData)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_REQUEST_CONNECTION_DATA);
    _Type_uint32::Add(&message, rConnectionID);
    _Type_uint32::Add(&message, rPrincipalID);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
        pCallContext->AddReturnValuePtr(pConnectionData);
    }
    s_ProtoReturn[METHOD_REQUEST_CONNECTION_DATA] = ProtoReturn_RequestConnectionData;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool SecureConnectionProtocolClient::RequestConnectionData_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                          Message* pMessage, bool* pSuccess,
                                                                          const unsigned int& rConnectionID,
                                                                          const unsigned int& rPrincipalID,
                                                                          qList<ConnectionData>* pConnectionData)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_REQUEST_CONNECTION_DATA);
    _Type_uint32::Add(pMessage, rConnectionID);
    _Type_uint32::Add(pMessage, rPrincipalID);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
        pCallContext->AddReturnValuePtr(pConnectionData);
    }
    s_ProtoReturn[METHOD_REQUEST_CONNECTION_DATA] = ProtoReturn_RequestConnectionData;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool SecureConnectionProtocolClient::CallRequestURLs(ProtocolCallContext* pCallContext, bool* pSuccess,
                                                     const unsigned int& rConnectionID,
                                                     const unsigned int& rPrincipalID, qList<StationURL>* pURLs)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_REQUEST_URLS);
    _Type_uint32::Add(&message, rConnectionID);
    _Type_uint32::Add(&message, rPrincipalID);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
        pCallContext->AddReturnValuePtr(pURLs);
    }
    s_ProtoReturn[METHOD_REQUEST_URLS] = ProtoReturn_RequestURLs;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool SecureConnectionProtocolClient::RequestURLs_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                                bool* pSuccess, const unsigned int& rConnectionID,
                                                                const unsigned int& rPrincipalID,
                                                                qList<StationURL>* pURLs)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_REQUEST_URLS);
    _Type_uint32::Add(pMessage, rConnectionID);
    _Type_uint32::Add(pMessage, rPrincipalID);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
        pCallContext->AddReturnValuePtr(pURLs);
    }
    s_ProtoReturn[METHOD_REQUEST_URLS] = ProtoReturn_RequestURLs;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool SecureConnectionProtocolClient::CallRegisterEx(ProtocolCallContext* pCallContext, qResult* pResult,
                                                    const qList<StationURL>& rURLs, unsigned int* pConnectionID,
                                                    StationURL* pPublicURL,
                                                    const AnyObjectHolder<Data, String>& rCustomData)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_REGISTER_EX);
    _Type_std_list<_Type_stationurl>::Add(&message, rURLs);
    CustomDataAny::Add(&message, rCustomData);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pResult);
        pCallContext->AddReturnValuePtr(pConnectionID);
        pCallContext->AddReturnValuePtr(pPublicURL);
    }
    s_ProtoReturn[METHOD_REGISTER_EX] = ProtoReturn_RegisterEx;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool SecureConnectionProtocolClient::RegisterEx_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                               qResult* pResult, const qList<StationURL>& rURLs,
                                                               unsigned int* pConnectionID, StationURL* pPublicURL,
                                                               const AnyObjectHolder<Data, String>& rCustomData)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_REGISTER_EX);
    _Type_std_list<_Type_stationurl>::Add(pMessage, rURLs);
    CustomDataAny::Add(pMessage, rCustomData);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pResult);
        pCallContext->AddReturnValuePtr(pConnectionID);
        pCallContext->AddReturnValuePtr(pPublicURL);
    }
    s_ProtoReturn[METHOD_REGISTER_EX] = ProtoReturn_RegisterEx;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool SecureConnectionProtocolClient::CallTestConnectivity(ProtocolCallContext* pCallContext)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_TEST_CONNECTIVITY);
    s_ProtoReturn[METHOD_TEST_CONNECTIVITY] = ProtoReturn_TestConnectivity;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool SecureConnectionProtocolClient::TestConnectivity_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                     Message* pMessage)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_TEST_CONNECTIVITY);
    s_ProtoReturn[METHOD_TEST_CONNECTIVITY] = ProtoReturn_TestConnectivity;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool SecureConnectionProtocolClient::CallUpdateURLs(ProtocolCallContext* pCallContext, const qList<StationURL>& rURLs)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_UPDATE_URLS);
    _Type_std_list<_Type_stationurl>::Add(&message, rURLs);
    s_ProtoReturn[METHOD_UPDATE_URLS] = ProtoReturn_UpdateURLs;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool SecureConnectionProtocolClient::UpdateURLs_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                               const qList<StationURL>& rURLs)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_UPDATE_URLS);
    _Type_std_list<_Type_stationurl>::Add(pMessage, rURLs);
    s_ProtoReturn[METHOD_UPDATE_URLS] = ProtoReturn_UpdateURLs;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool SecureConnectionProtocolClient::CallReplaceURL(ProtocolCallContext* pCallContext, const StationURL& rTarget,
                                                    const StationURL& rReplacement)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_REPLACE_URL);
    _Type_stationurl::Add(&message, rTarget);
    _Type_stationurl::Add(&message, rReplacement);
    s_ProtoReturn[METHOD_REPLACE_URL] = ProtoReturn_ReplaceURL;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool SecureConnectionProtocolClient::ReplaceURL_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                               const StationURL& rTarget,
                                                               const StationURL& rReplacement)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_REPLACE_URL);
    _Type_stationurl::Add(pMessage, rTarget);
    _Type_stationurl::Add(pMessage, rReplacement);
    s_ProtoReturn[METHOD_REPLACE_URL] = ProtoReturn_ReplaceURL;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool SecureConnectionProtocolClient::CallSendReport(ProtocolCallContext* pCallContext, const unsigned int& rReportID,
                                                    const qBuffer& rReport)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_SEND_REPORT);
    _Type_uint32::Add(&message, rReportID);
    _Type_qBuffer::Add(&message, rReport);
    s_ProtoReturn[METHOD_SEND_REPORT] = ProtoReturn_SendReport;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool SecureConnectionProtocolClient::SendReport_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                               const unsigned int& rReportID, const qBuffer& rReport)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_SEND_REPORT);
    _Type_uint32::Add(pMessage, rReportID);
    _Type_qBuffer::Add(pMessage, rReport);
    s_ProtoReturn[METHOD_SEND_REPORT] = ProtoReturn_SendReport;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

void SecureConnectionProtocolClient::ProtoReturn_Register(Message* pMessage, ProtocolCallContext* pCallContext)
{
    qResult* pResult = static_cast<qResult*>(pCallContext->GetReturnValuePtr(0));
    if (pResult == 0)
    {
        qResult discardedResult[1];
        pResult = &discardedResult[0];
    }
    _Type_qresult::Extract(pMessage, pResult);
    if (pMessage->HasError())
        return;
    unsigned int uiDiscardedConnectionID;
    unsigned int* pConnectionID = static_cast<unsigned int*>(pCallContext->GetReturnValuePtr(1));
    unsigned int* pDestination = &uiDiscardedConnectionID;
    if (pConnectionID != 0)
        pDestination = pConnectionID;
    pMessage->ExtractPrimitive(pDestination);
    if (pMessage->HasError())
        return;
    StationURL* pPublicURL = static_cast<StationURL*>(pCallContext->GetReturnValuePtr(2));
    if (pPublicURL != 0)
    {
        _Type_stationurl::Extract(pMessage, pPublicURL);
    }
    else
    {
        StationURL discardedPublicURL[1];
        _Type_stationurl::Extract(pMessage, &discardedPublicURL[0]);
    }
}

void SecureConnectionProtocolClient::ProtoReturn_RegisterEx(Message* pMessage, ProtocolCallContext* pCallContext)
{
    qResult* pResult = static_cast<qResult*>(pCallContext->GetReturnValuePtr(0));
    if (pResult == 0)
    {
        qResult discardedResult[1];
        pResult = &discardedResult[0];
    }
    _Type_qresult::Extract(pMessage, pResult);
    if (pMessage->HasError())
        return;
    unsigned int uiDiscardedConnectionID;
    unsigned int* pConnectionID = static_cast<unsigned int*>(pCallContext->GetReturnValuePtr(1));
    unsigned int* pDestination = &uiDiscardedConnectionID;
    if (pConnectionID != 0)
        pDestination = pConnectionID;
    pMessage->ExtractPrimitive(pDestination);
    if (pMessage->HasError())
        return;
    StationURL* pPublicURL = static_cast<StationURL*>(pCallContext->GetReturnValuePtr(2));
    if (pPublicURL != 0)
    {
        _Type_stationurl::Extract(pMessage, pPublicURL);
    }
    else
    {
        StationURL discardedPublicURL[1];
        _Type_stationurl::Extract(pMessage, &discardedPublicURL[0]);
    }
}

void SecureConnectionProtocolClient::ProtoReturn_RequestConnectionData(Message* pMessage,
                                                                       ProtocolCallContext* pCallContext)
{
    bool bDiscardedSuccess;
    bool* pSuccess = static_cast<bool*>(pCallContext->GetReturnValuePtr(0));
    bool* pDestination = &bDiscardedSuccess;
    if (pSuccess != 0)
        pDestination = pSuccess;
    *pMessage >> *pDestination;
    if (pMessage->HasError())
        return;
    qList<ConnectionData>* pConnectionData = static_cast<qList<ConnectionData>*>(pCallContext->GetReturnValuePtr(1));
    if (pConnectionData != 0)
    {
        _Type_std_list<DDLClassMetaType<ConnectionData> >::Extract(pMessage, pConnectionData);
        return;
    }
    qList<ConnectionData> discardedConnectionData[1];
    _Type_std_list<DDLClassMetaType<ConnectionData> >::Extract(pMessage, &discardedConnectionData[0]);
}

void SecureConnectionProtocolClient::ProtoReturn_RequestURLs(Message* pMessage, ProtocolCallContext* pCallContext)
{
    bool bDiscardedSuccess;
    bool* pSuccess = static_cast<bool*>(pCallContext->GetReturnValuePtr(0));
    bool* pDestination = &bDiscardedSuccess;
    if (pSuccess != 0)
        pDestination = pSuccess;
    *pMessage >> *pDestination;
    if (pMessage->HasError())
        return;
    qList<StationURL>* pURLs = static_cast<qList<StationURL>*>(pCallContext->GetReturnValuePtr(1));
    if (pURLs != 0)
    {
        _Type_std_list<_Type_stationurl>::Extract(pMessage, pURLs);
        return;
    }
    qList<StationURL> discardedURLs[1];
    _Type_std_list<_Type_stationurl>::Extract(pMessage, &discardedURLs[0]);
}

void SecureConnectionProtocolClient::ProtoReturn_TestConnectivity(Message* pMessage, ProtocolCallContext* pCallContext)
{
}

void SecureConnectionProtocolClient::ProtoReturn_UpdateURLs(Message* pMessage, ProtocolCallContext* pCallContext)
{
}

void SecureConnectionProtocolClient::ProtoReturn_ReplaceURL(Message* pMessage, ProtocolCallContext* pCallContext)
{
}

void SecureConnectionProtocolClient::ProtoReturn_SendReport(Message* pMessage, ProtocolCallContext* pCallContext)
{
}

}
}
