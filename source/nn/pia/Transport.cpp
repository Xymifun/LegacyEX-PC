#include "nn/pia/Transport.hpp"
#include "nn/pia/common/SessionBeginMonitoringContent.hpp"
#include "nn/pia/common/SessionStateMonitoringContent.hpp"
#include "nn/pia/transport/NetworkRttManager.hpp"
#include "nn/pia/transport/StationManager.hpp"
#include "nn/pia/transport/StationConnectionInfoTable.hpp"
#include "nn/pia/common/IPacketOutput.hpp"
#include "nn/pia/PayloadSizeManager.hpp"
#include "nn/pia/common/Job.hpp"
#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/transport/TransportAnalysisData.hpp"
#include "nn/pia/transport/RelayStationPacketHandler.hpp"
#include "nn/pia/transport/ResendingMessageManager.hpp"
#include "nn/pia/transport/ThreadStreamManager.hpp"
#include "nn/pia/transport/TransportAnalyzer.hpp"
#include "nn/pia/transport/TransportSupport.hpp"
#include "nn/pia/transport/IdentificationInfoTable.hpp"
#include "nn/pia/transport/DispatchJob.hpp"
#include "nn/pia/transport/StationPacketHandler.hpp"
#include "nn/pia/transport/StationProtocolManager.hpp"
#include "nn/pia/common/Memory.hpp"
#include "nn/pia/HeapManager.hpp"
#include "platform/wiiu/pead.hpp"
#include <new>
namespace nn
{
namespace pia
{
namespace transport
{

Transport* g_piaTransport = 0;

namespace
{

enum
{
    cInvalidRtt = 65535
};
const unsigned int cInvalidHash = 4294967295u;

}

namespace
{

unsigned char* g_piaTransportStorage = 0;
unsigned int g_sendLatencyEntryNum = 0;
unsigned int g_receiveLatencyEntryNum = 0;
bool g_isPacketLossEnabled = false;
unsigned short g_stationNum = 0;
unsigned int g_sendPacketNum = 0;
unsigned int g_receivePacketNum = 0;

}

common::IPacketInput* Transport::GetInputStream()
{
    return g_piaThreadStreamManager ? g_piaThreadStreamManager->m_packetInput : 0;
}

common::IPacketOutput* Transport::GetOutputStream()
{
    return g_piaThreadStreamManager ? g_piaThreadStreamManager->m_packetOutput : 0;
}

void Transport::OutputStreamUpdateEvent()
{
    common::IPacketOutput* output = GetOutputStream();
    if (output)
        output->OnStationConnectionEvent();
}

void Transport::SetMonitoringNetworkRtt(bool isSessionBegin)
{
    NetworkRttManager* rttManager = NetworkRttManager::sInstance;
    StationManager* stationManager = StationManager::s_pInstance;
    if (!rttManager || !stationManager)
        return;

    unsigned int minimum = cInvalidRtt;
    unsigned int maximum = 0;
    Station* minimumStation = 0;
    Station* maximumStation = 0;
    for (unsigned int index = 0; index < stationManager->m_stations.size; ++index)
    {
        Station* station = stationManager->m_stations.data[index];
        if (station->m_field20 != 5 || !station->m_stationAddress.IsValid())
            continue;
        unsigned int average = rttManager->GetAverage(station->m_stationAddress);
        if (average == 0)
            continue;
        if (average < minimum)
        {
            minimum = average;
            minimumStation = station;
        }
        if (maximum < average)
        {
            maximum = average;
            maximumStation = station;
        }
    }

    StationConnectionInfoTable* table = g_stationConnectionInfoTable;
    unsigned long long principal = table->GetPrincipalIdByStation(minimumStation);
    common::SessionStateMonitoringContent& state = common::g_sessionStateMonitoringContent;
    common::SessionBeginMonitoringContent& begin = common::g_sessionBeginMonitoringContent;
    unsigned int minimumHash = minimum < cInvalidRtt && principal != 0 ? common::hashWithMd5(principal) : cInvalidHash;
    unsigned int previousHash = state.m_metric163;
    state.m_metric163 = minimumHash;
    unsigned short minimumRtt = minimum < cInvalidRtt ? static_cast<unsigned short>(minimum) : cInvalidRtt;
    if (isSessionBegin)
    {
        begin.m_metric37 = minimumRtt;
        begin.m_metric38 = state.m_metric163;
        state.m_metric163 = previousHash;
    }
    else
        state.m_metric162 = minimumRtt;

    principal = table->GetPrincipalIdByStation(maximumStation);
    unsigned int maximumHash = maximum != 0 && principal != 0 ? common::hashWithMd5(principal) : cInvalidHash;
    unsigned short maximumRtt = maximum != 0 ? static_cast<unsigned short>(maximum) : cInvalidRtt;
    if (isSessionBegin)
    {
        begin.m_metric40 = maximumHash;
        begin.m_metric39 = maximumRtt;
    }
    else
    {
        state.m_metric165 = maximumHash;
        state.m_metric164 = maximumRtt;
    }
}

Transport::Transport()
    : m_networkFactory(0),
      m_packetHandler(0),
      m_stationNum(0),
      m_startedUp(false),
      m_connectionRouteRelay(false),
      m_connectionError(0),
      m_dispatchJob(0),
      m_connectionState(0),
      m_relayRouteManager(0),
      m_analysisInterval(0),
      m_dispatchCount(0),
      m_disconnectionEventListener(0),
      m_stationIdTable(0),
      m_transportStatus(static_cast<Result>(cResultInvalidState))
{
    m_time.SetNow();
    StationManager::CreateInstance();
    StationProtocolManager::CreateInstance();
    StationConnectionInfoTable::CreateInstance();
    ResendingMessageManager::CreateInstance();
    IdentificationInfoTable::CreateInstance();
    NetworkRttManager::CreateInstance();
    ProtocolMessageFilteringManager::CreateInstance();
    TransportAnalyzer::CreateInstance();
    m_stationIdTable = CreateObject<StationIdTable>();
    m_dispatchJob = CreateObject<DispatchJob>();
}

NetworkFactory* Transport::GetNetworkFactory()
{
    return m_networkFactory;
}

unsigned int Transport::GenerateConnectionId() const
{
    return m_packetHandler->GenerateConnectionId();
}

Result Transport::UpdateSetting(const common::CryptoSetting* cryptoSetting)
{
    return m_packetHandler->UpdateSetting(cryptoSetting);
}

Result Transport::SetKeepAliveInterval(int interval)
{
    if (m_networkFactory->IsStarConnection())
        return ResultInvalidState();
    return static_cast<StationPacketHandler*>(m_packetHandler)->m_keepAliveSender.SetInterval(interval);
}

int Transport::GetKeepAliveInterval() const
{
    if (m_networkFactory->IsStarConnection())
        return 0;
    const StationPacketHandler* handler = static_cast<const StationPacketHandler*>(m_packetHandler);
    return handler->m_keepAliveSender.m_interval;
}

bool Transport::IsKeepAliveEnabled() const
{
    if (m_networkFactory->IsStarConnection())
        return false;
    StationPacketHandler* handler = static_cast<StationPacketHandler*>(m_packetHandler);
    return handler->m_keepAliveSender.m_enabled && handler->m_keepAliveReceiver.m_enabled;
}

void Transport::EnableKeepAlive()
{
    if (!m_networkFactory->IsStarConnection())
    {
        StationPacketHandler* handler = static_cast<StationPacketHandler*>(m_packetHandler);
        handler->m_keepAliveSender.m_enabled = true;
        handler->m_keepAliveReceiver.m_enabled = true;
    }
}

void Transport::DisableKeepAlive()
{
    if (!m_networkFactory->IsStarConnection())
    {
        StationPacketHandler* handler = static_cast<StationPacketHandler*>(m_packetHandler);
        handler->m_keepAliveSender.m_enabled = false;
        handler->m_keepAliveReceiver.m_enabled = false;
    }
}

Result Transport::GetLocalStationConnectionInfo(StationConnectionInfo* stationConnectionInfo)
{
    if (stationConnectionInfo == 0)
        return static_cast<Result>(cResultInvalidArgument);
    *stationConnectionInfo = g_stationConnectionInfoTable->m_localStationConnectionInfo;
    return 0;
}

Result Transport::SetTransportStatus(Result transportStatus)
{
    if (transportStatus != 0 && transportStatus != static_cast<Result>(cResultInvalidState)
        && transportStatus != static_cast<Result>(cResultTransportUnavailable))
        return static_cast<Result>(cResultInvalidArgument);
    m_transportStatus = transportStatus;
    return 0;
}

Transport::~Transport()
{
    if (m_dispatchJob != 0)
    {
        m_dispatchJob->Reset(false);
        DestroyObject(m_dispatchJob);
        m_dispatchJob = 0;
    }
    if (m_stationIdTable != 0)
    {
        DestroyObject(m_stationIdTable);
        m_stationIdTable = 0;
    }
    TransportAnalyzer::DestroyInstance();
    ProtocolMessageFilteringManager::DestroyInstance();
    NetworkRttManager::DestroyInstance();
    IdentificationInfoTable::DestroyInstance();
    ResendingMessageManager::DestroyInstance();
    StationConnectionInfoTable::DestroyInstance();
    StationProtocolManager::DestroyInstance();
    StationManager::DestroyInstance();
}

Result Transport::CreateInstance(const Setting& setting)
{
    if (IsInitialized() == 0)
        return cResultNotInitialized;
    if (IsInSetupMode() == 0)
        return cResultInvalidState;
    if (g_piaTransport != 0)
        return cResultAlreadyExists;
    g_piaTransportStorage = static_cast<unsigned char*>(
        pead::allocateFromSeadHeap(sizeof(Transport), common::HeapManager::GetHeap(), 8));
    if (g_piaTransportStorage == 0)
        return cResultOutOfMemory;
    for (unsigned int index = 0; index < sizeof(Transport); ++index)
        g_piaTransportStorage[index] = 0;
    g_piaTransport = new (g_piaTransportStorage) Transport();
    const Result result = g_piaTransport->initialize(setting);
    if (result == 0)
        return 0;
    g_piaTransport->~Transport();
    g_piaTransport = 0;
    common::DeleteArray(g_piaTransportStorage);
    g_piaTransportStorage = 0;
    return result;
}

void Transport::DestroyInstance()
{
    if (g_piaTransport != 0)
    {
        g_piaTransport->finalize();
        g_piaTransport->~Transport();
        g_piaTransport = 0;
    }
    if (g_piaTransportStorage != 0)
    {
        common::DeleteArray(g_piaTransportStorage);
        g_piaTransportStorage = 0;
    }
}

Result Transport::initialize(const Setting& setting)
{
    NetworkFactory* factory = setting.m_networkFactory;
    const unsigned short stationNum = setting.m_stationNum;
    if (factory == 0)
        return cResultInvalidArgument;
    if (stationNum < cStationNumMin || stationNum > cStationNumMax)
        return cResultInvalidArgument;
    if (factory->GetMaxStationNum() < stationNum)
        return cResultInvalidArgument;
    if (stationNum > cStationNumLimit)
        return cResultInvalidArgument;

    m_networkFactory = factory->CreateSelf();
    common::PayloadSizeManager* payloadSizeManager = common::g_payloadSizeManager;
    payloadSizeManager->m_minMtuSize = factory->GetMtuSizeMin() - factory->GetHeaderSize();
    payloadSizeManager = common::g_payloadSizeManager;
    payloadSizeManager->m_maxMtuSize = factory->GetMtuSizeMax() - factory->GetHeaderSize();
    payloadSizeManager = common::g_payloadSizeManager;
    payloadSizeManager->m_payloadSizeLimit = factory->GetHeaderSize();

    Result result = NetworkRttManager::sInstance->InitializeImpl(stationNum + 8, stationNum);
    m_stationNum = stationNum;
    m_protocolManager.Initialize(m_networkFactory);
    g_piaStationProtocolManager->Initialize(stationNum, factory->GetDefaultRttInitialPulseInterval());
    result = StationManager::s_pInstance->Initialize(factory, stationNum,
                                                     g_piaStationProtocolManager->GetStationProtocol(),
                                                     g_piaStationProtocolManager->GetRttProtocol());
    if (result != 0)
        return result;

    result = g_stationConnectionInfoTable->Initialize(stationNum);
    result = g_piaResendingMessageManager->Initialize(stationNum << 1);
    result = IdentificationInfoTable::GetInstance()->Initialize(stationNum);
    result = ThreadStreamManager::CreateInstance(factory, setting.m_sendPacketNum, setting.m_receivePacketNum,
                                                 g_sendLatencyEntryNum, g_receiveLatencyEntryNum,
                                                 g_isPacketLossEnabled);
    ThreadStreamManager* threadStreamManager = g_piaThreadStreamManager;
    m_packetHandler = factory->CreatePacketHandler();
    const bool isStationHandler = m_packetHandler != 0
                                  && m_packetHandler->checkDerivedRuntimeTypeInfo(
                                      StationPacketHandler::getRuntimeTypeInfoStatic());
    if (isStationHandler == m_networkFactory->IsStarConnection())
    {
        factory->DeletePacketHandler(m_packetHandler);
        m_packetHandler = 0;
        return cResultInvalidArgument;
    }
    const bool isRelayHandler = m_packetHandler != 0
                                && m_packetHandler->checkDerivedRuntimeTypeInfo(
                                    RelayStationPacketHandler::getRuntimeTypeInfoStatic());
    if (isRelayHandler != m_networkFactory->IsClientRelayConnectionPermitted())
    {
        factory->DeletePacketHandler(m_packetHandler);
        m_packetHandler = 0;
        return cResultInvalidArgument;
    }

    if (!m_networkFactory->IsStarConnection())
        result = static_cast<StationPacketHandler*>(m_packetHandler)
                     ->Initialize(threadStreamManager->m_packetOutput->GetMulticastDestinationNum(),
                                  threadStreamManager->m_packetOutput->IsBroadcastSupported(),
                                  factory->GetHeaderSize());
    else
        result = m_packetHandler->Initialize(factory->GetHeaderSize());
    if (result != 0)
        return result;

    result = factory->CreateProtocols();
    if (result != 0)
        return result;
    result = g_piaTransportAnalyzer->Initialize();
    if (result != 0)
        return result;

    m_analysisInterval = setting.m_analysisInterval;
    m_keepAliveEnabled = setting.m_keepAliveEnabled;
    bool monitoringEnabled = true;
    if (threadStreamManager->m_packetOutput->IsBroadcastSupported())
        monitoringEnabled = setting.m_monitoringEnabled;
    m_monitoringEnabled = monitoringEnabled;
    result = m_stationIdTable->Initialize(stationNum);
    if (result != 0)
        return result;

    common::Time now;
    now.SetNow();
    pead::Random random;
    random.init(static_cast<unsigned int>(now.m_tick));
    m_localStationId = random.getU64();
    g_stationNum = setting.m_stationNum;
    g_sendPacketNum = setting.m_sendPacketNum;
    g_receivePacketNum = setting.m_receivePacketNum;
    return 0;
}

void Transport::finalize()
{
    if (m_stationIdTable != 0)
        m_stationIdTable->Finalize();
    if (g_piaTransportAnalyzer != 0)
        g_piaTransportAnalyzer->Finalize();
    if (m_packetHandler != 0)
    {
        m_packetHandler->Finalize();
        m_networkFactory->DeletePacketHandler(m_packetHandler);
        m_packetHandler = 0;
    }
    ThreadStreamManager::DestroyInstance();
    g_piaStationProtocolManager->Finalize();
    m_protocolManager.Finalize();
    if (m_packetHandler != 0)
    {
        m_networkFactory->DeletePacketHandler(m_packetHandler);
        m_packetHandler = 0;
    }
    StationManager::s_pInstance->Finalize();
    NetworkRttManager::sInstance->Finalize();
    if (m_networkFactory != 0)
    {
        m_networkFactory->DeleteSelf(m_networkFactory);
        m_networkFactory = 0;
    }
}

void Transport::Cleanup()
{
    if (!m_startedUp)
        return;
    m_protocolManager.CleanupProtocols();
    m_connectionError = 0;
    m_startedUp = false;
    m_connectionRouteRelay = false;
    if (m_dispatchJob != 0)
        m_dispatchJob->Reset(false);
    if (g_piaTransportAnalyzer != 0)
        g_piaTransportAnalyzer->Cleanup();
    if (g_piaResendingMessageManager != 0)
        g_piaResendingMessageManager->Cleanup();
    m_protocolManager.Cleanup();
    if (m_packetHandler != 0)
        m_packetHandler->Cleanup();
    if (g_piaThreadStreamManager != 0)
        g_piaThreadStreamManager->Cleanup();
}

Result Transport::Restartup(const common::CryptoSetting* cryptoSetting)
{
    if (!m_connectionRouteRelay)
        return ResultInvalidState();
    const unsigned int payloadSize = common::g_payloadSizeManager->m_mtuSize
                                     + common::g_payloadSizeManager->m_payloadSizeLimit;
    Cleanup();
    const Result result = Startup(payloadSize, cryptoSetting);
    return result;
}

Result Transport::Startup(unsigned int payloadSize, const common::CryptoSetting* cryptoSetting)
{
    if (IsInSetupMode() != 0)
        return static_cast<Result>(cResultInvalidState);
    if (m_startedUp)
        return static_cast<Result>(cResultInvalidState);

    Result result = common::g_payloadSizeManager->SetMtuSize(payloadSize
                                                             - common::g_payloadSizeManager->m_payloadSizeLimit);
    if (result != 0)
        return result;

    result = g_piaThreadStreamManager->Startup();

    PacketStream::Writer* const writer = &g_piaThreadStreamManager->m_sendThreadStream->m_writer;
    PacketStream::Reader* const reader = &g_piaThreadStreamManager->m_receiveThreadStream->m_reader;
    common::IPacketInput* const packetInput = g_piaThreadStreamManager->m_packetInput;
    common::IPacketOutput* const packetOutput = g_piaThreadStreamManager->m_packetOutput;

    if (m_networkFactory->IsStarConnection())
    {
        m_packetHandler->Startup(writer, reader, packetInput, packetOutput, &m_localStationId, cryptoSetting);
    }
    else if (m_networkFactory->IsClientRelayConnectionPermitted())
    {
        static_cast<RelayStationPacketHandler*>(m_packetHandler)
            ->Startup(writer, reader, packetInput, packetOutput, &m_localStationId, m_relayRouteManager, cryptoSetting);
    }
    else
    {
        static_cast<StationPacketHandler*>(m_packetHandler)
            ->Startup(writer, reader, packetInput, packetOutput, &m_localStationId, cryptoSetting);
    }

    result = m_protocolManager.Startup(m_packetHandler);
    result = g_piaResendingMessageManager->Startup(m_packetHandler, m_networkFactory->IsStarConnection());
    result = g_piaTransportAnalyzer->Startup();
    m_dispatchJob->Ready(false);

    m_connectionError = 0;
    m_startedUp = true;
    m_lastAnalysisTime = m_time;
    m_dispatchCount = 0;
    m_connectionRouteRelay = false;

    const Result protocolResult = m_protocolManager.StartupProtocols();
    if (protocolResult != 0)
        return protocolResult;
    return 0;
}

Result Transport::dispatch()
{
    common::Time now;
    now.SetNow();
    m_time = now;
    ++m_dispatchCount;
    if (g_piaThreadStreamManager == 0)
        return ResultInvalidState();

    Result result;
    result = g_piaThreadStreamManager->m_sendThreadStream->GetLastResult();
    if (result != 0)
        m_connectionError = result;
    result = g_piaThreadStreamManager->m_receiveThreadStream->GetLastResult();
    if (result != 0)
        m_connectionError = result;

    m_packetHandler->BeginDispatch(m_time);
    result = m_protocolManager.Dispatch();
    if (result != 0)
    {
        m_packetHandler->EndDispatch(m_time);
        return result;
    }
    result = g_piaResendingMessageManager->Dispatch();
    m_packetHandler->EndDispatch(m_time);
    if (result != 0)
        return result;
    updateTransportAnalyzer();
    return 0;
}

void Transport::updateTransportAnalyzer()
{
    if (!m_startedUp || m_analysisInterval == 0)
        return;
    TransportAnalyzer* const analyzer = g_piaTransportAnalyzer;
    if (analyzer == 0)
        return;
    const int elapsedSeconds = static_cast<int>(static_cast<long long>(m_time.m_tick - m_lastAnalysisTime.m_tick)
                                                / common::TimeSpan::GetTicksPerSec().m_tick);
    if (elapsedSeconds < m_analysisInterval || analyzer->Update() != 0)
        return;

    m_lastAnalysisTime = m_time;
    TransportAnalysisData data = analyzer->m_analysisData;
    data.Print(m_keepAliveEnabled, m_monitoringEnabled);
}

Result Transport::ConvertToStationIndex(StationIndex* index, uint64_t stationId) const
{
    if (index == 0)
        return static_cast<Result>(cResultInvalidArgument);
    if (m_transportStatus == static_cast<Result>(cResultInvalidState))
        return static_cast<Result>(cResultInvalidState);
    if (m_transportStatus == static_cast<Result>(cResultTransportUnavailable))
        return static_cast<Result>(cResultTransportUnavailable);
    if (stationId == 0)
    {
        *index = static_cast<StationIndex>(cStationIndexInvalid);
        return 0;
    }
    StationIdTable::Entry entry;
    const Result result = m_stationIdTable->FindByStationId(&entry, stationId);
    if (result == 0)
    {
        *index = static_cast<StationIndex>(entry.stationIndex);
        return 0;
    }
    return result;
}
Result Transport::ConvertToStationId(uint64_t* stationId, StationIndex stationIndex) const
{
    if (stationId == 0)
        return static_cast<Result>(cResultInvalidArgument);
    if (m_transportStatus == static_cast<Result>(cResultInvalidState))
        return static_cast<Result>(cResultInvalidState);
    if (m_transportStatus == static_cast<Result>(cResultTransportUnavailable))
        return static_cast<Result>(cResultTransportUnavailable);
    if (stationIndex == static_cast<StationIndex>(cStationIndexInvalid))
    {
        *stationId = 0;
        return 0;
    }
    if (stationIndex == static_cast<StationIndex>(cStationIndexReserved))
    {
        *stationId = 0;
        return 0;
    }
    if (stationIndex == static_cast<StationIndex>(cStationIndexBroadcast))
    {
        *stationId = 0;
        return 0;
    }
    StationIdTable::Entry entry;
    const Result result = m_stationIdTable->FindByStationIndex(&entry, stationIndex);
    if (result == 0)
    {
        *stationId = entry.stationId;
        return 0;
    }
    return result;
}

StationIndex Transport::ConvertToStationIndexUnsafe(uint64_t stationId) const
{
    return m_stationIdTable->FindUnsafe(stationId);
}

StationIndex Conv2StationIndex(uint64_t stationId)
{
    if (g_piaTransport == 0)
        return static_cast<StationIndex>(cStationIndexInvalid);
    StationIndex index;
    const Result result = g_piaTransport->ConvertToStationIndex(&index, stationId);
    if (result != 0)
        return static_cast<StationIndex>(cStationIndexInvalid);
    return index;
}
}
}
}
