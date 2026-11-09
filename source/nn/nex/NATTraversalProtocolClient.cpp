#include "nn/nex/NATTraversalProtocolClient.hpp"

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
const unsigned int METHOD_ID_COUNT = 8;
const unsigned int PROTOCOL_FLAG_UNLOCK_BEFORE_SEND = 0x10;
enum MethodId
{
    METHOD_REQUEST_PROBE_INITIATION = 1,
    METHOD_INITIATE_PROBE = 2,
    METHOD_REQUEST_PROBE_INITIATION_EXT = 3,
    METHOD_REPORT_NAT_TRAVERSAL_RESULT = 4,
    METHOD_REPORT_NAT_PROPERTIES = 5,
    METHOD_GET_RELAY_SIGNATURE_KEY = 6,
    METHOD_REPORT_NAT_TRAVERSAL_RESULT_DETAIL = 7
};
ProtoReturnFunction s_ProtoReturn[METHOD_ID_COUNT];

}

NATTraversalProtocolClient::NATTraversalProtocolClient(unsigned short uiProtocolID) : ClientProtocol(1)
{
    if (uiProtocolID == 0)
        return;
    SetProtocolID(uiProtocolID);
    RVClientCore::GetInstance()->GetProtocolRegistry()->RegisterProtocol(uiProtocolID,
                                                                         new NATTraversalProtocolClient());
}

void NATTraversalProtocolClient::ExtractCallSpecificResults(Message* pMessage, ProtocolCallContext* pCallContext)
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

Protocol* NATTraversalProtocolClient::CreateResponder() const
{
    return new NATTraversalProtocolClient();
}

bool NATTraversalProtocolClient::CallRequestProbeInitiation(ProtocolCallContext* pCallContext,
                                                            const qList<StationURL>& rTargetList)
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
    AddMethodID(&message, METHOD_REQUEST_PROBE_INITIATION);
    _Type_std_list<_Type_stationurl>::Add(&message, rTargetList);
    s_ProtoReturn[METHOD_REQUEST_PROBE_INITIATION] = ProtoReturn_RequestProbeInitiation;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool NATTraversalProtocolClient::RequestProbeInitiation_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                       Message* pMessage,
                                                                       const qList<StationURL>& rTargetList)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_REQUEST_PROBE_INITIATION);
    _Type_std_list<_Type_stationurl>::Add(pMessage, rTargetList);
    s_ProtoReturn[METHOD_REQUEST_PROBE_INITIATION] = ProtoReturn_RequestProbeInitiation;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool NATTraversalProtocolClient::CallInitiateProbe(ProtocolCallContext* pCallContext, const StationURL& rStationToProbe)
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
    AddMethodID(&message, METHOD_INITIATE_PROBE);
    _Type_stationurl::Add(&message, rStationToProbe);
    s_ProtoReturn[METHOD_INITIATE_PROBE] = ProtoReturn_InitiateProbe;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool NATTraversalProtocolClient::InitiateProbe_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                              const StationURL& rStationToProbe)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_INITIATE_PROBE);
    _Type_stationurl::Add(pMessage, rStationToProbe);
    s_ProtoReturn[METHOD_INITIATE_PROBE] = ProtoReturn_InitiateProbe;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool NATTraversalProtocolClient::CallRequestProbeInitiationExt(ProtocolCallContext* pCallContext,
                                                               const qList<StationURL>& rTargetList,
                                                               const StationURL& rStationToProbe)
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
    AddMethodID(&message, METHOD_REQUEST_PROBE_INITIATION_EXT);
    _Type_std_list<_Type_stationurl>::Add(&message, rTargetList);
    _Type_stationurl::Add(&message, rStationToProbe);
    s_ProtoReturn[METHOD_REQUEST_PROBE_INITIATION_EXT] = ProtoReturn_RequestProbeInitiationExt;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool NATTraversalProtocolClient::RequestProbeInitiationExt_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                          Message* pMessage,
                                                                          const qList<StationURL>& rTargetList,
                                                                          const StationURL& rStationToProbe)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_REQUEST_PROBE_INITIATION_EXT);
    _Type_std_list<_Type_stationurl>::Add(pMessage, rTargetList);
    _Type_stationurl::Add(pMessage, rStationToProbe);
    s_ProtoReturn[METHOD_REQUEST_PROBE_INITIATION_EXT] = ProtoReturn_RequestProbeInitiationExt;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool NATTraversalProtocolClient::CallReportNATTraversalResult(ProtocolCallContext* pCallContext,
                                                              const unsigned int& rCID, const bool& rResult,
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

bool NATTraversalProtocolClient::ReportNATTraversalResult_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                         Message* pMessage, const unsigned int& rCID,
                                                                         const bool& rResult, const unsigned int& rRTT)
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

bool NATTraversalProtocolClient::CallReportNATProperties(ProtocolCallContext* pCallContext,
                                                         const unsigned int& rNATMapping,
                                                         const unsigned int& rNATFiltering, const unsigned int& rRTT)
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

bool NATTraversalProtocolClient::ReportNATProperties_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                    Message* pMessage, const unsigned int& rNATMapping,
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

bool NATTraversalProtocolClient::CallGetRelaySignatureKey(ProtocolCallContext* pCallContext, int* pRelayMode,
                                                          DateTime* pCurrentUTCTime, String* pRelayAddress,
                                                          unsigned short* pRelayPort, int* pRelayAddressType,
                                                          unsigned int* pGameServerID)
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
    AddMethodID(&message, METHOD_GET_RELAY_SIGNATURE_KEY);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pRelayMode);
        pCallContext->AddReturnValuePtr(pCurrentUTCTime);
        pCallContext->AddReturnValuePtr(pRelayAddress);
        pCallContext->AddReturnValuePtr(pRelayPort);
        pCallContext->AddReturnValuePtr(pRelayAddressType);
        pCallContext->AddReturnValuePtr(pGameServerID);
    }
    s_ProtoReturn[METHOD_GET_RELAY_SIGNATURE_KEY] = ProtoReturn_GetRelaySignatureKey;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool NATTraversalProtocolClient::GetRelaySignatureKey_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                     Message* pMessage, int* pRelayMode,
                                                                     DateTime* pCurrentUTCTime, String* pRelayAddress,
                                                                     unsigned short* pRelayPort, int* pRelayAddressType,
                                                                     unsigned int* pGameServerID)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_GET_RELAY_SIGNATURE_KEY);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pRelayMode);
        pCallContext->AddReturnValuePtr(pCurrentUTCTime);
        pCallContext->AddReturnValuePtr(pRelayAddress);
        pCallContext->AddReturnValuePtr(pRelayPort);
        pCallContext->AddReturnValuePtr(pRelayAddressType);
        pCallContext->AddReturnValuePtr(pGameServerID);
    }
    s_ProtoReturn[METHOD_GET_RELAY_SIGNATURE_KEY] = ProtoReturn_GetRelaySignatureKey;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool NATTraversalProtocolClient::CallReportNATTraversalResultDetail(ProtocolCallContext* pCallContext,
                                                                    const unsigned int& rCID, const bool& rResult,
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

bool NATTraversalProtocolClient::ReportNATTraversalResultDetail_PrepareRequest(
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

void NATTraversalProtocolClient::ProtoReturn_RequestProbeInitiation(Message*, ProtocolCallContext*)
{
}

void NATTraversalProtocolClient::ProtoReturn_InitiateProbe(Message*, ProtocolCallContext*)
{
}

void NATTraversalProtocolClient::ProtoReturn_RequestProbeInitiationExt(Message*, ProtocolCallContext*)
{
}

void NATTraversalProtocolClient::ProtoReturn_ReportNATTraversalResult(Message*, ProtocolCallContext*)
{
}

void NATTraversalProtocolClient::ProtoReturn_ReportNATProperties(Message*, ProtocolCallContext*)
{
}

void NATTraversalProtocolClient::ProtoReturn_ReportNATTraversalResultDetail(Message*, ProtocolCallContext*)
{
}

void NATTraversalProtocolClient::ProtoReturn_GetRelaySignatureKey(Message* pMessage, ProtocolCallContext* pCallContext)
{
    int iDiscardedRelayMode;
    int* pRelayMode = static_cast<int*>(pCallContext->GetReturnValuePtr(0));
    int* pRelayModeDestination = &iDiscardedRelayMode;
    if (pRelayMode != 0)
        pRelayModeDestination = pRelayMode;
    pMessage->ExtractPrimitive(pRelayModeDestination);
    if (pMessage->HasError())
        return;
    DateTime* pCurrentUTCTime = static_cast<DateTime*>(pCallContext->GetReturnValuePtr(1));
    if (pCurrentUTCTime != 0)
    {
        uint64_t uiCurrentUTCTime;
        pMessage->ExtractPrimitive(&uiCurrentUTCTime);
        *pCurrentUTCTime = DateTime(uiCurrentUTCTime);
    }
    else
    {
        DateTime discardedCurrentUTCTime[1];
        uint64_t uiCurrentUTCTime;
        pMessage->ExtractPrimitive(&uiCurrentUTCTime);
        DateTime currentUTCTime(uiCurrentUTCTime);
        discardedCurrentUTCTime[0] = currentUTCTime;
    }
    if (pMessage->HasError())
        return;
    String* pRelayAddress = static_cast<String*>(pCallContext->GetReturnValuePtr(2));
    if (pRelayAddress != 0)
    {
        _Type_string::Extract(pMessage, pRelayAddress);
        if (pMessage->HasError())
            return;
    }
    else
    {
        String discardedRelayAddress[1];
        _Type_string::Extract(pMessage, &discardedRelayAddress[0]);
        if (pMessage->HasError())
            return;
    }
    unsigned short usDiscardedRelayPort;
    unsigned short* pRelayPort = static_cast<unsigned short*>(pCallContext->GetReturnValuePtr(3));
    unsigned short* pRelayPortDestination = &usDiscardedRelayPort;
    if (pRelayPort != 0)
        pRelayPortDestination = pRelayPort;
    pMessage->ExtractPrimitive(pRelayPortDestination);
    if (pMessage->HasError())
        return;
    int iDiscardedRelayAddressType;
    int* pRelayAddressType = static_cast<int*>(pCallContext->GetReturnValuePtr(4));
    int* pRelayAddressTypeDestination = &iDiscardedRelayAddressType;
    if (pRelayAddressType != 0)
        pRelayAddressTypeDestination = pRelayAddressType;
    pMessage->ExtractPrimitive(pRelayAddressTypeDestination);
    if (pMessage->HasError())
        return;
    unsigned int* pGameServerID = static_cast<unsigned int*>(pCallContext->GetReturnValuePtr(5));
    if (pGameServerID != 0)
    {
        pMessage->ExtractPrimitive(pGameServerID);
        return;
    }
    unsigned int uiDiscardedGameServerID;
    pMessage->ExtractPrimitive(&uiDiscardedGameServerID);
}

}
}
