#include "nn/nex/NATTraversalReportInternalProtocolClient.hpp"

#include "nn/nex/Core.hpp"
#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/Message.hpp"
#include "nn/nex/ProtocolCallContext.hpp"
#include "nn/nex/ProtocolRegistry.hpp"
#include "nn/nex/ProtocolRequestBrokerInterface.hpp"
#include "nn/nex/RVClientCore.hpp"
#include "nn/nex/Scheduler.hpp"

namespace nn
{
namespace nex
{

namespace
{

typedef void (*ProtoReturnFunction)(Message* pMessage, ProtocolCallContext* pCallContext);
const unsigned int METHOD_ID_MASK = 0x7FFF;
enum
{
    RESULT_NOT_IMPLEMENTED = static_cast<int>(0x80010002u)
};
const unsigned int METHOD_ID_COUNT = 4;
const unsigned int PROTOCOL_FLAG_UNLOCK_BEFORE_SEND = 0x10;
enum MethodId
{
    METHOD_REPORT_NAT_TRAVERSAL_RESULT = 1,
    METHOD_REPORT_NAT_PROPERTIES = 2,
    METHOD_REPORT_NAT_TRAVERSAL_RESULT_DETAIL = 3
};
ProtoReturnFunction s_ProtoReturn[METHOD_ID_COUNT];

}

NATTraversalReportInternalProtocolClient::NATTraversalReportInternalProtocolClient(unsigned short uiProtocolID)
    : ClientProtocol(1)
{
    if (uiProtocolID == 0)
        return;
    SetProtocolID(uiProtocolID);
    RVClientCore::GetInstance()->GetProtocolRegistry()->RegisterProtocol(
        uiProtocolID, new NATTraversalReportInternalProtocolClient());
}

void NATTraversalReportInternalProtocolClient::ExtractCallSpecificResults(Message* pMessage,
                                                                          ProtocolCallContext* pCallContext)
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

Protocol* NATTraversalReportInternalProtocolClient::CreateResponder() const
{
    return new NATTraversalReportInternalProtocolClient();
}

bool NATTraversalReportInternalProtocolClient::CallReportNATTraversalResult(ProtocolCallContext* pCallContext,
                                                                            const unsigned int& rCID,
                                                                            const bool& rResult,
                                                                            const unsigned int& rRTT)
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
    AddMethodID(&message, METHOD_REPORT_NAT_TRAVERSAL_RESULT);
    _Type_uint32::Add(&message, rCID);
    message << rResult;
    _Type_uint32::Add(&message, rRTT);
    s_ProtoReturn[METHOD_REPORT_NAT_TRAVERSAL_RESULT] = ProtoReturn_ReportNATTraversalResult;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool NATTraversalReportInternalProtocolClient::ReportNATTraversalResult_PrepareRequest(
    ProtocolCallContext* pCallContext, Message* pMessage, const unsigned int& rCID, const bool& rResult,
    const unsigned int& rRTT)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_REPORT_NAT_TRAVERSAL_RESULT);
    _Type_uint32::Add(pMessage, rCID);
    *pMessage << rResult;
    _Type_uint32::Add(pMessage, rRTT);
    s_ProtoReturn[METHOD_REPORT_NAT_TRAVERSAL_RESULT] = ProtoReturn_ReportNATTraversalResult;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool NATTraversalReportInternalProtocolClient::CallReportNATProperties(ProtocolCallContext* pCallContext,
                                                                       const unsigned int& rNATMapping,
                                                                       const unsigned int& rNATFiltering,
                                                                       const unsigned int& rRTT)
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
    AddMethodID(&message, METHOD_REPORT_NAT_PROPERTIES);
    _Type_uint32::Add(&message, rNATMapping);
    _Type_uint32::Add(&message, rNATFiltering);
    _Type_uint32::Add(&message, rRTT);
    s_ProtoReturn[METHOD_REPORT_NAT_PROPERTIES] = ProtoReturn_ReportNATProperties;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool NATTraversalReportInternalProtocolClient::ReportNATProperties_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                                  Message* pMessage,
                                                                                  const unsigned int& rNATMapping,
                                                                                  const unsigned int& rNATFiltering,
                                                                                  const unsigned int& rRTT)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_REPORT_NAT_PROPERTIES);
    _Type_uint32::Add(pMessage, rNATMapping);
    _Type_uint32::Add(pMessage, rNATFiltering);
    _Type_uint32::Add(pMessage, rRTT);
    s_ProtoReturn[METHOD_REPORT_NAT_PROPERTIES] = ProtoReturn_ReportNATProperties;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool NATTraversalReportInternalProtocolClient::CallReportNATTraversalResultDetail(ProtocolCallContext* pCallContext,
                                                                                  const unsigned int& rCID,
                                                                                  const bool& rResult,
                                                                                  const int& rConnectionType,
                                                                                  const unsigned int& rRTT)
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
    AddMethodID(&message, METHOD_REPORT_NAT_TRAVERSAL_RESULT_DETAIL);
    _Type_uint32::Add(&message, rCID);
    message << rResult;
    message.Append(reinterpret_cast<const unsigned char*>(&rConnectionType), sizeof(rConnectionType), 1);
    _Type_uint32::Add(&message, rRTT);
    s_ProtoReturn[METHOD_REPORT_NAT_TRAVERSAL_RESULT_DETAIL] = ProtoReturn_ReportNATTraversalResultDetail;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool NATTraversalReportInternalProtocolClient::ReportNATTraversalResultDetail_PrepareRequest(
    ProtocolCallContext* pCallContext, Message* pMessage, const unsigned int& rCID, const bool& rResult,
    const int& rConnectionType, const unsigned int& rRTT)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_REPORT_NAT_TRAVERSAL_RESULT_DETAIL);
    _Type_uint32::Add(pMessage, rCID);
    *pMessage << rResult;
    pMessage->Append(reinterpret_cast<const unsigned char*>(&rConnectionType), sizeof(rConnectionType), 1);
    _Type_uint32::Add(pMessage, rRTT);
    s_ProtoReturn[METHOD_REPORT_NAT_TRAVERSAL_RESULT_DETAIL] = ProtoReturn_ReportNATTraversalResultDetail;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

void NATTraversalReportInternalProtocolClient::ProtoReturn_ReportNATTraversalResult(Message*, ProtocolCallContext*)
{
}

void NATTraversalReportInternalProtocolClient::ProtoReturn_ReportNATProperties(Message*, ProtocolCallContext*)
{
}

void NATTraversalReportInternalProtocolClient::ProtoReturn_ReportNATTraversalResultDetail(Message*,
                                                                                          ProtocolCallContext*)
{
}

}
}
