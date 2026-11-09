#include "nn/pia/inet/NexFacade.hpp"

#include <nn/ndm.h>

#include "nn/nex/Core.hpp"
#include "nn/nex/Credentials.hpp"
#include "nn/nex/InetAddress.hpp"
#include "nn/nex/NATProperties.hpp"
#include "nn/nex/NATTraversalRelayClient.hpp"
#include "nn/nex/Network.hpp"
#include "nn/nex/StationURL.hpp"
#include "nn/nex/qList.hpp"
#include "nn/pia/common/InetAddress.hpp"
#include "nn/nex/NgsBridgeInterface.hpp"
#include "nn/nex/RVClientCore.hpp"
#include "nn/nex/Reconstruction.hpp"
#include "nn/nex/RootTransport.hpp"
#include "nn/pia/common/Memory.hpp"
#include "nn/pia/common/SessionBeginMonitoringContent.hpp"
#include "nn/pia/common/Socket.hpp"
#include "nn/pia/common/SocketAddress.hpp"
#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/common/SocketStreamBase.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/inet/Api.hpp"
#include "nn/pia/inet/InetInputStream.hpp"
#include "nn/pia/inet/InetOutputStream.hpp"
#include "nn/pia/inet/NexNatRelay.hpp"
#include "nn/pia/inet/NexNatServerAddressResolveJob.hpp"
#include "nn/pia/inet/NexNotificationDataJob.hpp"
#include "nn/pia/nat/NatTraverser.hpp"
#include "nn/pia/session/Mesh.hpp"
#include "nn/pia/transport/StationConnectionInfo.hpp"
#include "nn/pia/transport/StationConnectionInfoTable.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

namespace
{
enum
{
    cRandomPortRangeMin = 0xc000,
    cRandomPortRangeMax = 0xffff,
    cResultOutOfMemory = 0x10c01,
    cResultInvalidArgument = 0x10c07,
    cResultInvalidState = 0x10c08,
    cResultNotInitialized = 0x10c0c,
    cResultAlreadyCreated = 0x10c0f,
    cResultDisconnected = 0xc406,
    cResultPortDetectionFailed = 0x6434,
    cTraceError = 1,
    cTraceNatSession = 0x100,
    cTraceStationLocation = 0x1000
};

enum
{
    cStationUrlPublicTypeBit = 1,
    cNatTypeFlagNatPropertyFlag = 0x80,
    cReplaceUrlTimeout = 30000
};

enum
{
    cInstanceAlignment = 8,
    cDefaultNatTraversalTimeout = 5000,
    cDefaultNatTraversalTimeoutEdmExtension = 1500
};

enum
{
    cBossDaemons = 3,
    cNatServerNum = 4,
    cNatServerPort = 10025,
    cNatServerAlternatePort = 10125,
    cTraceSocketAddress = 0x1000,
    cResultNatServerAddressUnavailable = 0x6436
};

common::InetAddress s_noRelayServerAddress;
}

NexFacade* NexFacade::s_instance = 0;
unsigned char* NexFacade::s_instanceMemory = 0;

namespace
{
const char* s_natServerHostName1 = "nncs1.app.nintendowifi.net";
const char* s_natServerHostName2 = "nncs2.app.nintendowifi.net";
common::SocketAddress s_natServerSocketAddress1;
common::SocketAddress s_natServerSocketAddress2;
}

NexFacade::NexFacade()
{
    m_started = false;
    m_natSessionActive = false;
    m_relayServiceEnabled = false;
    m_outputStream = 0;
    m_inputStream = 0;
    m_socket = 0;
    m_gameServerId = 0;
    m_accessKey.Clear();
    m_ngsBridge = 0;
    m_gatheringId = 0;
    m_natRelayClientMemory = 0;
    m_natRelayClient = 0;
    m_matchmakeExtensionMemory = 0;
    m_matchmakeExtensionClient = 0;
    m_natTraverser = 0;
    m_natTraversalTimeout = cDefaultNatTraversalTimeout;
    m_natTraversalTimeoutEdmExtension = cDefaultNatTraversalTimeoutEdmExtension;
    m_bossDaemonSuspendEnabled = true;
    m_bossDaemonSuspendCount = 0;
    m_notificationEventHandler = 0;
    m_notificationDataJob = 0;
    m_asyncProcess = 0;
    m_natServerAddressResolveJob = 0;
    m_localStationUrl = 0;
    m_publicStationUrl = 0;
    m_replaceUrlContext = 0;
    m_ignoreNatProbeRequest = true;
    m_localServiceConnectionId = 0;
}

Result NexFacade::Initialize()
{
    void* socketMemory = pead::allocateFromSeadHeap(sizeof(common::Socket), common::HeapManager::GetHeap());
    m_socket = socketMemory != 0 ? ::new (socketMemory) common::Socket() : 0;
    m_natRelayClientMemory = common::NewArray<unsigned char>(sizeof(nex::NATTraversalRelayClient), cInstanceAlignment);
    m_natRelayClient = 0;
    m_matchmakeExtensionMemory = common::NewArray<unsigned char>(sizeof(nex::MatchmakeExtensionClient),
                                                                 cInstanceAlignment);
    m_matchmakeExtensionClient = 0;
    void* natTraverserMemory = pead::allocateFromSeadHeap(sizeof(nat::NatTraverser), common::HeapManager::GetHeap());
    m_natTraverser = natTraverserMemory != 0 ? ::new (natTraverserMemory) nat::NatTraverser() : 0;
    void* natRelayMemory = pead::allocateFromSeadHeap(sizeof(NexNatRelay), common::HeapManager::GetHeap());
    m_natRelay = natRelayMemory != 0 ? ::new (natRelayMemory) NexNatRelay() : 0;
    void* notificationDataJobMemory = pead::allocateFromSeadHeap(sizeof(NexNotificationDataJob),
                                                                 common::HeapManager::GetHeap());
    m_notificationDataJob = notificationDataJobMemory != 0 ? ::new (notificationDataJobMemory) NexNotificationDataJob()
                                                           : 0;
    void* resolveJobMemory = pead::allocateFromSeadHeap(sizeof(NexNatServerAddressResolveJob),
                                                        common::HeapManager::GetHeap());
    m_natServerAddressResolveJob = resolveJobMemory != 0 ? ::new (resolveJobMemory) NexNatServerAddressResolveJob() : 0;
    void* matchmakeSessionMemory = pead::allocateFromSeadHeap(sizeof(nex::MatchmakeSession),
                                                              common::HeapManager::GetHeap());
    m_matchmakeSession = matchmakeSessionMemory != 0 ? ::new (matchmakeSessionMemory) nex::MatchmakeSession() : 0;
    m_localStationUrl = common::NewObj<nex::StationURL>();
    m_publicStationUrl = common::NewObj<nex::StationURL>();
    void* replaceUrlContextMemory = pead::allocateFromSeadHeap(sizeof(nex::ProtocolCallContext),
                                                               common::HeapManager::GetHeap());
    m_replaceUrlContext = replaceUrlContextMemory != 0 ? ::new (replaceUrlContextMemory) nex::ProtocolCallContext() : 0;
    return 0;
}

Result NexFacade::CreateInstance()
{
    if (!IsInitialized())
        return ResultNotInitialized();
    if (!IsInSetupMode())
        return cResultInvalidState;
    if (s_instance != 0)
        return cResultAlreadyCreated;
    s_instanceMemory = common::NewArray<unsigned char>(sizeof(NexFacade), cInstanceAlignment);
    if (s_instanceMemory == 0)
        return cResultOutOfMemory;
    s_instance = new (s_instanceMemory) NexFacade();
    s_instance->Initialize();
    return 0;
}

void NexFacade::Finalize()
{
    if (m_natTraverser != 0)
    {
        common::DeleteObj(m_natTraverser);
        m_natTraverser = 0;
    }
    if (m_natRelay != 0)
    {
        common::DeleteObj(m_natRelay);
        m_natRelay = 0;
    }
    if (m_notificationDataJob != 0)
    {
        m_notificationDataJob->Cleanup();
        common::DeleteObj(m_notificationDataJob);
        m_notificationDataJob = 0;
    }
    if (m_natServerAddressResolveJob != 0)
    {
        CancelCleanupNatServerAddressResolution();
        common::DeleteObj(m_natServerAddressResolveJob);
        m_natServerAddressResolveJob = 0;
    }
    if (m_matchmakeSession != 0)
    {
        m_matchmakeSession->Reset();
        common::DeleteObj(m_matchmakeSession);
        m_matchmakeSession = 0;
    }
    if (m_localStationUrl != 0)
    {
        common::DeleteObj(m_localStationUrl);
        m_localStationUrl = 0;
    }
    if (m_publicStationUrl != 0)
    {
        common::DeleteObj(m_publicStationUrl);
        m_publicStationUrl = 0;
    }
    if (m_replaceUrlContext != 0)
    {
        if (m_replaceUrlContext->GetState() == nex::CallContext::CallInProgress)
            m_replaceUrlContext->Cancel(nex::CallContext::CallCanceled);
        common::DeleteObj(m_replaceUrlContext);
        m_replaceUrlContext = 0;
    }
    if (m_natRelayClient != 0)
    {
        m_natRelayClient->Unbind();
        m_natRelayClient->UnregisterRelayInterface();
        m_natRelayClient->~NATTraversalRelayClient();
        m_natRelayClient = 0;
    }
    if (m_natRelayClientMemory != 0)
    {
        common::DeleteArray<unsigned char>(m_natRelayClientMemory);
        m_natRelayClientMemory = 0;
    }
    if (m_matchmakeExtensionClient != 0)
    {
        m_matchmakeExtensionClient->Unbind();
        m_matchmakeExtensionClient->~MatchmakeExtensionClient();
        m_matchmakeExtensionClient = 0;
    }
    if (m_matchmakeExtensionMemory != 0)
    {
        common::DeleteArray<unsigned char>(m_matchmakeExtensionMemory);
        m_matchmakeExtensionMemory = 0;
    }
    if (m_natTraverserStartupContext.m_state == common::CallContext::cStateInProgress)
        m_natTraverserStartupContext.SignalCancel();
    m_natTraverserStartupContext.Reset();
    if (m_natServerAddressResolveContext.m_state == common::CallContext::cStateInProgress)
        m_natServerAddressResolveContext.SignalCancel();
    m_natServerAddressResolveContext.Reset();
    if (m_natSessionContext.m_state == common::CallContext::cStateInProgress)
        m_natSessionContext.SignalCancel();
    m_natSessionContext.Reset();
    if (m_socket != 0)
    {
        common::DeleteObj(m_socket);
        m_socket = 0;
    }
}

void NexFacade::DestroyInstance()
{
    if (s_instance != 0)
    {
        s_instance->Finalize();
        s_instance->~NexFacade();
        s_instance = 0;
    }
    if (s_instanceMemory != 0)
    {
        common::DeleteArray<unsigned char>(s_instanceMemory);
        s_instanceMemory = 0;
    }
}

Result NexFacade::CreateProtocols()
{
    if (m_natTraverser == 0)
        return cResultInvalidState;
    const Result result = m_natTraverser->CreateProtocols();
    if (result != 0)
        return result;
    return 0;
}

void NexFacade::StopNatSession()
{
    m_ignoreNatProbeRequest = true;
    m_natSessionActive = false;
    if (m_outputStream != 0)
        m_outputStream->Cleanup();
    if (m_inputStream != 0)
        m_inputStream->Cleanup();
    if (m_socket->m_openCount > 0)
        m_socket->Close();
    m_outputStream = 0;
    m_inputStream = 0;
    if (m_natTraverser != 0)
        m_natTraverser->Cleanup();
    if (m_natServerAddressResolveJob != 0)
        CancelCleanupNatServerAddressResolution();
    if (m_natTraverserStartupContext.m_state == common::CallContext::cStateInProgress)
        m_natTraverserStartupContext.SignalCancel();
    m_natTraverserStartupContext.Reset();
    if (m_natServerAddressResolveContext.m_state == common::CallContext::cStateInProgress)
        m_natServerAddressResolveContext.SignalCancel();
    m_natServerAddressResolveContext.Reset();
    if (m_natSessionContext.m_state == common::CallContext::cStateInProgress)
        m_natSessionContext.SignalCancel();
    m_natSessionContext.Reset();
    resumeBossDaemon();
}

unsigned int NexFacade::GetNatPropertyMapping() const
{
    if (m_natTraverser == 0)
        return 0;
    const nat::NatTraverser::NatProperty& natProperty = m_natTraverser->m_natProperty;
    return natProperty.m_mappingType;
}

unsigned short NexFacade::GetReservedPort() const
{
    return nex::RootTransport::GetWellKnownPortNumber();
}

void NexFacade::NotifyLeave(const transport::StationLocation& location, bool clearMonitoringData, bool relayLeave)
{
    if (m_natTraverser != 0)
        m_natTraverser->ClearNatTraversal(location, clearMonitoringData);
    if (clearMonitoringData && IsBeginMonitoringDataAvailable())
        m_natTraverser->ClearNatMonitoringData(location);
    if (relayLeave && m_relayServiceEnabled && m_natRelay != 0 && m_natRelay->IsRelayServerAvailable())
        m_natRelay->RemoveRelayEntry(location);
}

unsigned int NexFacade::GetRandomPortRangeMin() const
{
    return cRandomPortRangeMin;
}

unsigned int NexFacade::GetRandomPortRangeMax() const
{
    return cRandomPortRangeMax;
}

void NexFacade::RelayProbeRequest(const transport::StationLocation& source,
                                  const transport::StationLocation& destination)
{
    if (m_natRelay != 0)
        m_natRelay->RelayProbeRequest(source, destination);
}

void NexFacade::UpdateRelayTable()
{
    if (m_natRelay != 0)
        m_natRelay->UpdateRelayTable();
}

unsigned int NexFacade::GetLocalServiceConnectionId() const
{
    return m_localServiceConnectionId;
}

bool NexFacade::IsBeginMonitoringDataAvailable() const
{
    return session::g_piaMesh == 0
           || (!session::g_piaMesh->IsProcessedAtMeshBegin() && !session::g_piaMesh->IsProcessingLeaveMesh());
}

const common::InetAddress& NexFacade::GetRelayServerAddress() const
{
    if (m_natRelay != 0)
        return m_natRelay->GetRelayServerAddress();
    return s_noRelayServerAddress;
}

bool NexFacade::IsStartNatSessionCompleted() const
{
    if (m_asyncProcess != 1)
        return false;
    return m_natSessionContext.m_state == common::CallContext::cStateSuccess
           || m_natSessionContext.m_state == common::CallContext::cStateFailure
           || m_natSessionContext.m_state == common::CallContext::cStateCancelled;
}

Result NexFacade::GetStartNatSessionResult() const
{
    if (static_cast<unsigned int>(m_natSessionContext.m_state) - 2 > 2 || m_asyncProcess != 1)
        return cResultInvalidState;
    return m_natSessionContext.m_result;
}

Result NexFacade::CancelStartNatSessionAsync()
{
    if (m_natSessionContext.m_state != common::CallContext::cStateInProgress || m_asyncProcess != 1)
        return cResultInvalidState;
    m_natSessionContext.Cancel();
    if (m_natTraverserStartupContext.m_state == common::CallContext::cStateInProgress)
        m_natTraverserStartupContext.Cancel();
    if (m_natServerAddressResolveContext.m_state == common::CallContext::cStateInProgress)
        m_natServerAddressResolveContext.Cancel();
    return 0;
}

Result NexFacade::Bind(const LoginInfo* loginInfo)
{
    if (loginInfo == 0)
        return cResultInvalidArgument;
    if (loginInfo->gameServerId == 0)
        return cResultInvalidArgument;
    if (loginInfo->accessKey == 0)
        return cResultInvalidArgument;
    if (loginInfo->ngsFacade == 0)
        return cResultInvalidArgument;
    if (loginInfo->ngsFacade->GetCredentials() == 0)
        return cResultInvalidArgument;
    m_ngsBridge = loginInfo->ngsFacade->GetNgsBridge();
    if (m_matchmakeExtensionClient == 0)
    {
        m_matchmakeExtensionClient = ::new (m_matchmakeExtensionMemory) nex::MatchmakeExtensionClient();
        m_matchmakeExtensionClient->Bind(m_ngsBridge->GetCredentials());
    }
    m_gameServerId = loginInfo->gameServerId;
    m_accessKey.Copy(*loginInfo->accessKey);
    return 0;
}

void NexFacade::Unbind()
{
    m_gameServerId = 0;
    m_accessKey.Clear();
    if (m_matchmakeExtensionClient != 0)
    {
        m_matchmakeExtensionClient->Unbind();
        m_matchmakeExtensionClient->~MatchmakeExtensionClient();
        m_matchmakeExtensionClient = 0;
    }
    m_ngsBridge = 0;
}

unsigned long long NexFacade::GetLocalPrincipalId() const
{
    unsigned long long localPrincipalId = 0;
    if (m_ngsBridge != 0)
    {
        const unsigned int principalId = m_ngsBridge->GetPrincipalID();
        localPrincipalId = principalId;
    }
    return localPrincipalId;
}

bool NexFacade::RegisterNexNotificationEventHandler4Pia(nex::NotificationEventHandler* handler)
{
    if (m_ngsBridge != 0 && m_ngsBridge->RegisterNotificationEventHandler(handler))
    {
        m_notificationEventHandler = handler;
        return true;
    }
    return false;
}

bool NexFacade::UnregisterNexNotificationEventHandler4Pia(nex::NotificationEventHandler* handler)
{
    if (m_ngsBridge != 0 && m_ngsBridge->UnregisterNotificationEventHandler(handler))
    {
        m_notificationEventHandler = 0;
        return true;
    }
    return false;
}

void NexFacade::SetPersistentGatheringCodeKey()
{
    if (m_accessKey.GetEncodingType() == common::String::EncodingType_Utf16)
    {
        nex::MatchmakeExtensionClient::SetPersistentGatheringCodeKey(
            nex::String(static_cast<const wchar_t*>(m_accessKey.GetBuffer())));
    }
    else
    {
        nex::MatchmakeExtensionClient::SetPersistentGatheringCodeKey(
            nex::String(static_cast<const char*>(m_accessKey.GetBuffer())));
    }
}

void NexFacade::ClearPersistentGatheringCodeKey()
{
    nex::MatchmakeExtensionClient::ClearPersistentGatheringCodeKey();
}

Result NexFacade::UpdateNotificationDataAsync(nex::NotificationEvents::NotificationEvents type, unsigned int param1,
                                              unsigned int param2, const common::String& text)
{
    if (m_ngsBridge == 0)
        return cResultInvalidState;
    if (m_natSessionContext.m_state == common::CallContext::cStateInProgress)
        return cResultInvalidState;
    m_natSessionContext.Reset();
    const Result result = m_notificationDataJob->Startup(&m_natSessionContext, type, param1, param2, text);
    if (result != 0)
        return result;
    m_asyncProcess = 2;
    m_notificationDataJob->Ready(false);
    return 0;
}

Result NexFacade::RetrieveNotificationDataAsync(unsigned char typeMask, nex::qList<nex::NotificationEvent>* events)
{
    if (m_ngsBridge == 0)
        return cResultInvalidState;
    if (events == 0)
        return cResultInvalidArgument;
    if (typeMask == 0)
        return cResultInvalidArgument;
    if (m_natSessionContext.m_state == common::CallContext::cStateInProgress)
        return cResultInvalidState;
    m_natSessionContext.Reset();
    const Result result = m_notificationDataJob->Startup(&m_natSessionContext, typeMask, events);
    if (result != 0)
        return result;
    m_asyncProcess = 3;
    m_notificationDataJob->Ready(false);
    return 0;
}

Result NexFacade::Startup()
{
    nex::NgsBridgeInterface* ngsBridge = m_ngsBridge;
    if (ngsBridge == 0 || m_gameServerId == 0)
        return cResultInvalidState;
    if (nex::g_threadMode != nex::Core::THREAD_MODE_3 && nex::g_threadMode != nex::Core::THREAD_MODE_2)
        return cResultInvalidState;
    m_gatheringId = 0;
    if (m_natRelayClientMemory == 0 || m_matchmakeExtensionMemory == 0)
        return cResultInvalidState;
    if (nex::RVClientCore::GetInstance() == 0)
        return cResultInvalidState;
    if (nex::RVClientCore::GetInstance()->GetProtocolRegistry() == 0)
        return cResultInvalidState;
    if (m_natRelayClient == 0)
    {
        m_natRelayClient = ::new (m_natRelayClientMemory) nex::NATTraversalRelayClient();
        m_natRelayClient->Init();
        m_natRelayClient->RegisterRelayInterface(&m_natRelay->m_relayInterface);
        m_natRelayClient->Bind(ngsBridge->GetCredentials());
    }
    m_started = true;
    m_ignoreNatProbeRequest = true;
    return 0;
}

void NexFacade::Cleanup()
{
    if (!m_started)
        return;
    m_started = false;
    if (m_natRelayClient != 0)
    {
        m_natRelayClient->Unbind();
        m_natRelayClient->UnregisterRelayInterface();
        m_natRelayClient->~NATTraversalRelayClient();
        m_natRelayClient = 0;
    }
    if (m_natRelay != 0)
        m_natRelay->Cleanup();
    if (m_ngsBridge != 0 && m_notificationEventHandler != 0)
        m_ngsBridge->UnregisterNotificationEventHandler(m_notificationEventHandler);
    m_notificationEventHandler = 0;
    if (m_notificationDataJob != 0)
        m_notificationDataJob->Cleanup();
    if (m_natServerAddressResolveJob != 0)
        CancelCleanupNatServerAddressResolution();
    m_gatheringId = 0;
    if (m_natTraverserStartupContext.m_state == common::CallContext::cStateInProgress)
        m_natTraverserStartupContext.SignalCancel();
    m_natTraverserStartupContext.Reset();
    if (m_natServerAddressResolveContext.m_state == common::CallContext::cStateInProgress)
        m_natServerAddressResolveContext.SignalCancel();
    m_natServerAddressResolveContext.Reset();
    if (m_natSessionContext.m_state == common::CallContext::cStateInProgress)
        m_natSessionContext.SignalCancel();
    m_natSessionContext.Reset();
    m_asyncProcess = 0;
}

void ReplaceURLCompletionCallback(nex::CallContext*, const nex::UserContext*)
{
}

void NexFacade::CompleteStartNatSession()
{
    if (m_natTraverserStartupContext.m_state == common::CallContext::cStateCancelled)
    {
        if (m_natSessionContext.m_state == common::CallContext::cStateInProgress)
            m_natSessionContext.SignalCancel();
        return;
    }
    const Result startupResult = m_natTraverserStartupContext.m_result;
    if (startupResult != 0)
    {
        if (m_natSessionContext.m_state == common::CallContext::cStateInProgress)
            m_natSessionContext.SignalFailure(startupResult);
        return;
    }
    const unsigned short port = m_natTraverser->m_publicPort;
    Result result = m_socket->Open();
    if (result != 0)
    {
        m_natSessionContext.SignalFailure(result == cResultDisconnected ? result : Result(cResultPortDetectionFailed));
        return;
    }
    result = m_socket->Bind(port, false);
    if (result != 0)
    {
        m_natSessionContext.SignalFailure(result == cResultDisconnected ? result : Result(cResultPortDetectionFailed));
        return;
    }
    result = m_outputStream->ImportSocket(m_socket);
    if (result != 0)
    {
        m_natSessionContext.SignalFailure(result);
        return;
    }
    result = m_inputStream->ImportSocket(m_socket);
    if (result != 0)
    {
        m_natSessionContext.SignalFailure(result);
        return;
    }
    result = m_outputStream->Startup();
    if (result != 0)
    {
        m_natSessionContext.SignalFailure(result);
        return;
    }
    result = m_inputStream->Startup();
    if (result != 0)
    {
        m_natSessionContext.SignalFailure(result);
        return;
    }
    const nat::NatTraverser::NatProperty& natProperty = m_natTraverser->m_natProperty;
    nex::qProtectedList<nex::StationURL>& localUrls = nex::Network::GetInstance()->GetLocalURLs();
    bool publicUrlReplaced = false;
    transport::StationConnectionInfo info;
    transport::StationLocation* publicLocation = &info.m_stationLocation;
    transport::StationLocation* localLocation = &info.m_localStationLocation;
    for (nex::qList<nex::StationURL>::iterator url = localUrls.GetList().begin(); url != localUrls.GetList().end();
         ++url)
    {
        if ((url->GetType() >> cStationUrlPublicTypeBit) & 1)
        {
            ConvertNexStationUrlToStationLocation(*url, publicLocation);
            if (!publicLocation->m_stationAddress.m_inetAddress.IsValid())
            {
                transport::StationLocation location;
                ConvertNexStationUrlToStationLocation(*url, &location);
                location.Trace(cTraceError);
                url->Trace(cTraceError);
                m_natSessionContext.SignalFailure(cResultPortDetectionFailed);
                return;
            }
            publicLocation->m_field23 = static_cast<unsigned char>(natProperty.m_mappingType);
            publicLocation->m_field24 = static_cast<unsigned char>(natProperty.m_filteringType);
            if (IsGlobal(*publicLocation))
            {
                localLocation->m_stationAddress.m_inetAddress.m_port = port;
                localLocation->Trace(cTraceNatSession);
            }
            publicLocation->Trace(cTraceNatSession);
        }
        else
        {
            *m_localStationUrl = *url;
            *m_publicStationUrl = *url;
            nex::StationURL* publicUrl = m_publicStationUrl;
            publicUrl->SetNATMapping(static_cast<nex::NATProperties::MappingProperties>(natProperty.m_mappingType));
            publicUrl->SetNATFiltering(
                static_cast<nex::NATProperties::FilteringProperties>(natProperty.m_filteringType));
            if (natProperty.m_mappingType == nex::NATProperties::MAPPING_ENDPOINT_INDEPENDENT)
                publicUrl->SetPortNumber(natProperty.m_publicPort);
            else
                publicUrl->SetPortNumber(port);
            ConvertNexStationUrlToStationLocation(*publicUrl, localLocation);
            if (!localLocation->m_stationAddress.m_inetAddress.IsValid())
            {
                transport::StationLocation location;
                ConvertNexStationUrlToStationLocation(*publicUrl, &location);
                location.Trace(cTraceError);
                publicUrl->Trace(cTraceError);
                m_natSessionContext.SignalFailure(cResultPortDetectionFailed);
                return;
            }
            if (natProperty.m_mappingType == nex::NATProperties::MAPPING_ENDPOINT_INDEPENDENT)
                localLocation->m_stationAddress.m_inetAddress.m_port = natProperty.m_publicPort;
            else
                localLocation->m_stationAddress.m_inetAddress.m_port = port;
            localLocation->Trace(cTraceNatSession);
            publicUrlReplaced = true;
        }
    }
    nex::NgsBridgeInterface* ngsBridge = m_ngsBridge;
    if (ngsBridge != 0 && m_replaceUrlContext->GetState() != nex::CallContext::CallInProgress && publicUrlReplaced)
    {
        m_replaceUrlContext->Reset();
        m_replaceUrlContext->SetTimeout(cReplaceUrlTimeout);
        m_replaceUrlContext->RegisterCompletionCallback(ReplaceURLCompletionCallback, nex::UserContext(), true);
        ngsBridge->ReplaceURL(m_replaceUrlContext, *m_localStationUrl, *m_publicStationUrl);
    }
    info.m_stationLocation.m_principalId = s_instance->m_matchmakeExtensionClient->GetCredentials()->GetPrincipalID();
    info.m_localStationLocation.m_principalId =
        s_instance->m_matchmakeExtensionClient->GetCredentials()->GetPrincipalID();
    const unsigned int connectionId = transport::g_piaTransport->GenerateConnectionId();
    info.m_stationLocation.m_field18 = connectionId;
    info.m_localStationLocation.m_field18 = connectionId;
    transport::g_stationConnectionInfoTable->GetLocalStationConnectionInfo() = info;
    m_natTraverser->SetLocalPublicLocation(info.m_stationLocation);
    unsigned char natTypeFlags = info.m_stationLocation.m_field25;
    if (natProperty.m_flag != 0)
        natTypeFlags |= cNatTypeFlagNatPropertyFlag;
    common::SessionBeginMonitoringContent& content = common::g_sessionBeginMonitoringContent;
    content.m_natMapping = static_cast<unsigned char>(natProperty.m_mappingType);
    content.m_natFiltering = static_cast<unsigned char>(natProperty.m_filteringType);
    content.m_natPortDelta = static_cast<unsigned char>(natProperty.m_portDelta);
    content.m_publicPort = natProperty.m_publicPort;
    content.m_publicAddress = info.m_stationLocation.m_stationAddress.m_inetAddress.m_address;
    content.m_privatePort = natProperty.m_privatePort;
    content.m_localAddress = info.m_localStationLocation.m_stationAddress.m_inetAddress.m_address;
    content.m_natTypeFlags = natTypeFlags;
    content.m_principalIdHash = common::hashWithMd5(info.m_stationLocation.m_principalId);
    content.m_gameServerId = s_instance->m_gameServerId;
    content.m_threadMode = static_cast<unsigned char>(nex::g_threadMode);
    m_ignoreNatProbeRequest = false;
    m_natSessionActive = true;
    m_natSessionContext.SignalSuccess(0);
}

void NexFacade::suspendBossDaemon()
{
    if (!m_bossDaemonSuspendEnabled)
        return;
    nn::Result result = ndm::Initialize();
    if (result.IsFailure())
        return;
    ++m_bossDaemonSuspendCount;
    ndm::SuspendDaemons(cBossDaemons);
}

Result NexFacade::StartNatSessionAsync()
{
    if (m_natSessionContext.m_state == common::CallContext::cStateInProgress)
        return cResultInvalidState;
    if (!m_started)
        return cResultInvalidState;
    m_outputStream = static_cast<InetOutputStream*>(transport::g_piaTransport->GetOutputStream());
    if (m_outputStream == 0)
        return cResultInvalidState;
    m_inputStream = static_cast<InetInputStream*>(transport::g_piaTransport->GetInputStream());
    if (m_inputStream == 0)
        return cResultInvalidState;
    m_natSessionContext.Reset();
    m_natTraverserStartupContext.Reset();
    m_natServerAddressResolveContext.Reset();
    m_asyncProcess = 1;
    m_natServerAddressResolveContext.RegisterCompletionCallback(NatServerAddressResolveCompletionCallback, 0);
    const Result result = m_natServerAddressResolveJob->Startup(&m_natServerAddressResolveContext);
    if (result != 0)
        return result;
    m_natSessionContext.InitiateCall();
    m_natServerAddressResolveJob->Ready(true);
    suspendBossDaemon();
    return 0;
}

void NexFacade::resumeBossDaemon()
{
    if (m_bossDaemonSuspendCount > 0)
    {
        ndm::ResumeDaemons(cBossDaemons);
        ndm::Finalize();
        --m_bossDaemonSuspendCount;
    }
}

void NexFacade::CancelCleanupNatServerAddressResolution()
{
    if (m_natServerAddressResolveJob->IsRunning())
    {
        if (m_natServerAddressResolveJob->IsBackground())
        {
            m_natServerAddressResolveJob->m_cancelRequested = true;
            do
            {
                m_natServerAddressResolveJob->WaitForCompletion(5);
                if (!m_natServerAddressResolveJob->IsRunning())
                    break;
            } while (m_natServerAddressResolveJob->IsBackground());
        }
        m_natServerAddressResolveJob->CancelCleanup();
        m_natServerAddressResolveJob->Reset(false);
    }
}

void NexFacade::CompletedNatServerAddressResolution()
{
    if (m_natServerAddressResolveContext.m_state == common::CallContext::cStateCancelled)
    {
        if (m_natSessionContext.m_state == common::CallContext::cStateInProgress)
            m_natSessionContext.SignalCancel();
        return;
    }
    const Result resolveResult = m_natServerAddressResolveContext.m_result;
    if (resolveResult != 0)
    {
        if (m_natSessionContext.m_state == common::CallContext::cStateInProgress)
            m_natSessionContext.SignalFailure(resolveResult);
        return;
    }
    m_natTraverserStartupContext.RegisterCompletionCallback(NatTraverserStartupCompletionCallback, 0);
    const Result startupResult = m_natTraverser->Startup(this, &m_natTraverserStartupContext);
    if (startupResult != 0)
    {
        m_natSessionContext.SignalFailure(startupResult);
        return;
    }
    m_natTraverserStartupContext.InitiateCall();
}

void NexFacade::UpdateGatheringID(unsigned int gatheringId)
{
    m_gatheringId = gatheringId;
}

bool NexFacade::IsRelayServiceEnabled() const
{
    return m_relayServiceEnabled;
}

Result NexFacade::UpdateNatServerAddress()
{
    common::Time startTime;
    startTime.SetNow();
    if (m_natTraverser->m_servers.size != cNatServerNum)
    {
        common::InetAddress primaryAddress;
        common::InetAddress secondaryAddress;
        m_natTraverser->m_servers.size = 0;
        s_natServerSocketAddress1.GetInetAddress(&primaryAddress);
        if (!primaryAddress.IsValidAddress())
        {
            if (!s_natServerSocketAddress1.GetAddressInfo(s_natServerHostName1, 0, 0))
                return cResultNatServerAddressUnavailable;
            s_natServerSocketAddress1.Trace(cTraceSocketAddress);
        }
        s_natServerSocketAddress2.GetInetAddress(&secondaryAddress);
        if (!secondaryAddress.IsValidAddress())
        {
            if (!s_natServerSocketAddress2.GetAddressInfo(s_natServerHostName2, 0, 0))
                return cResultNatServerAddressUnavailable;
            s_natServerSocketAddress2.Trace(cTraceSocketAddress);
        }
        s_natServerSocketAddress1.GetInetAddress(&primaryAddress);
        primaryAddress.m_port = cNatServerPort;
        if (m_natTraverser->m_servers.size == cNatServerNum)
            return cResultNatServerAddressUnavailable;
        m_natTraverser->m_servers.data[m_natTraverser->m_servers.size++] = primaryAddress;
        primaryAddress.m_port = cNatServerAlternatePort;
        if (m_natTraverser->m_servers.size == cNatServerNum)
            return cResultNatServerAddressUnavailable;
        m_natTraverser->m_servers.data[m_natTraverser->m_servers.size++] = primaryAddress;
        s_natServerSocketAddress2.GetInetAddress(&secondaryAddress);
        secondaryAddress.m_port = cNatServerPort;
        if (m_natTraverser->m_servers.size == cNatServerNum)
            return cResultNatServerAddressUnavailable;
        m_natTraverser->m_servers.data[m_natTraverser->m_servers.size++] = secondaryAddress;
        secondaryAddress.m_port = cNatServerAlternatePort;
        if (m_natTraverser->m_servers.size == cNatServerNum)
            return cResultNatServerAddressUnavailable;
        m_natTraverser->m_servers.data[m_natTraverser->m_servers.size++] = secondaryAddress;
        InetInputStream::AddIgnoreAddress(primaryAddress, secondaryAddress);
    }
    common::Time endTime;
    endTime.SetNow();
    common::g_sessionBeginMonitoringContent.m_metric24 = static_cast<unsigned short>(
        static_cast<long long>(endTime.m_tick - startTime.m_tick) / common::TimeSpan::GetTicksPerMSec().m_tick);
    return 0;
}

void NexFacade::SetIgnoreNatProbeRequest(bool ignore)
{
    m_ignoreNatProbeRequest = ignore;
}

void NexFacade::NotifyLocalServiceConnectionId(unsigned int connectionId)
{
    m_localServiceConnectionId = connectionId;
}

void NexFacade::NotifyProbeRequest(const transport::StationLocation& location)
{
    if (m_ignoreNatProbeRequest)
        return;
    if (m_natTraverser == 0)
        return;
    m_natTraverser->ProcessProbeRequest(location);
}

bool NexFacade::IsTraversed(const transport::StationLocation& location) const
{
    if (m_natTraverser == 0)
        return false;
    return m_natTraverser->IsTraversed(location);
}

bool NexFacade::IsTraversed(unsigned int index) const
{
    if (m_natTraverser == 0)
        return false;
    return m_natTraverser->IsTraversed(index);
}

bool NexFacade::PrepareNatTraversal(const transport::StationLocation& location,
                                    const transport::StationConnectionInfo& info, common::CallContext* context,
                                    bool relay, bool passive, StationIndex stationIndex)
{
    if (m_natTraverser == 0)
        return false;
    return m_natTraverser->PrepareNatTraversal(location, info, context, relay, passive,
                                               stationIndex == cStationIndexSentinelFE);
}

bool NexFacade::CheckLatestStationLocation(transport::StationLocation& location)
{
    if (m_natTraverser == 0)
        return 0;
    return m_natTraverser->CheckLatestStationLocation(location);
}

void NexFacade::ResetProbeStatus(const transport::StationLocation& location, bool relay)
{
    if (m_natTraverser != 0)
        m_natTraverser->ResetProbeStatus(location, relay);
}

void NexFacade::ReportNatTraversalResult(const transport::StationLocation& location, Result result, bool relay)
{
    if (m_natTraverser != 0)
        m_natTraverser->ReportNatTraversalResult(location, result, relay);
}

void NexFacade::NatServerAddressResolveCompletionCallback(Result, void*)
{
    s_instance->CompletedNatServerAddressResolution();
}

void NexFacade::NatTraverserStartupCompletionCallback(Result, void*)
{
    s_instance->CompleteStartNatSession();
}

void NexFacade::ConvertNexInetAddressToInetAddress(const nex::InetAddress& source, common::InetAddress* destination)
{
    destination->m_address = source.GetAddress();
    destination->m_port = static_cast<unsigned short>(source.GetPortNumber());
}

void NexFacade::ConvertInetAddressToNexInetAddress(const common::InetAddress& source, nex::InetAddress* destination)
{
    destination->SetAddress(source.m_address);
    destination->SetPortNumber(source.m_port);
}

void NexFacade::ConvertNexStationUrlToStationLocation(const nex::StationURL& url, transport::StationLocation* location)
{
    common::InetAddress address;
    ConvertNexInetAddressToInetAddress(*url.GetInetAddress(), &address);
    location->m_stationAddress.SetInetAddress(address);
    location->m_principalId = url.GetPrincipalID();
    location->m_field18 = url.GetConnectionID();
    location->m_field1c = url.GetRVConnectionID();
    location->m_field20 = url.GetURLType();
    location->m_field21 = url.GetStreamID();
    location->m_field22 = url.GetStreamType();
    location->m_field23 = url.GetNATMapping();
    location->m_field24 = url.GetNATFiltering();
    location->m_field25 = url.GetType();
    location->m_field26 = url.GetProbeRequestInitiation();
    if (url.GetUseRelayServer())
    {
        nex::InetAddress relayAddress;
        relayAddress.SetAddress(static_cast<const wchar_t*>(url.GetRelayServerAddress()));
        ConvertNexInetAddressToInetAddress(relayAddress, &address);
        if (address.m_address != 0)
        {
            address.m_port = url.GetRelayServerPort();
            location->m_inetAddress = address;
        }
        else
        {
            address.m_address = 0;
            address.m_port = 0;
            location->m_inetAddress = address;
        }
    }
}

Result NexFacade::ConvertNexStationUrlToStationConnectionInfo(const nex::qList<nex::StationURL>& urls,
                                                              transport::StationConnectionInfo* info)
{
    if (urls.size() != 1 && urls.size() != 2)
        return cResultInvalidArgument;
    nex::qList<nex::StationURL>::const_iterator url = urls.begin();
    transport::StationLocation* location = &info->m_stationLocation;
    ConvertNexStationUrlToStationLocation(*url, location);
    location->Trace(cTraceStationLocation);
    ++url;
    if (url == urls.end())
        return 0;
    location = &info->m_localStationLocation;
    ConvertNexStationUrlToStationLocation(*url, location);
    location->Trace(cTraceStationLocation);
    return 0;
}

}
}
}
