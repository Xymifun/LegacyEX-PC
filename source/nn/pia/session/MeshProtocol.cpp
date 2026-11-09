#include "nn/pia/session/MeshProtocol.hpp"

#include <limits>

#include <cstring>
#include <new>

#include "nn/pia/HeapManager.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/common/Memory.hpp"
#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/common/WatermarkManager.hpp"
#include "nn/pia/session/DestroyMeshJob.hpp"
#include "nn/pia/session/JoinMeshJob.hpp"
#include "nn/pia/session/KickoutManageJob.hpp"
#include "nn/pia/session/LeaveMeshJob.hpp"
#include "nn/pia/session/LeaveWithHostMigrationJob.hpp"
#include "nn/pia/session/Mesh.hpp"
#include "nn/pia/session/MeshEventListener.hpp"
#include "nn/pia/session/ProcessDestroyMeshJob.hpp"
#include "nn/pia/session/ProcessHostMigrationJob.hpp"
#include "nn/pia/session/ProcessJoinRequestJob.hpp"
#include "nn/pia/session/ProcessUpdateMeshJob.hpp"
#include "nn/pia/session/RelayRouteManageJob.hpp"
#include "nn/pia/session/Session.hpp"
#include "nn/pia/transport/IdentificationInfoTable.hpp"
#include "nn/pia/transport/NetworkRttManager.hpp"
#include "nn/pia/transport/PacketHandler.hpp"
#include "nn/pia/transport/ProtocolEvent.hpp"
#include "nn/pia/transport/ProtocolMessageWriter.hpp"
#include "nn/pia/transport/RelayRouteManager.hpp"
#include "nn/pia/transport/ReliableSlidingWindow.hpp"
#include "nn/pia/transport/ResendingMessageManager.hpp"
#include "nn/pia/transport/Station.hpp"
#include "nn/pia/transport/StationConnectionInfo.hpp"
#include "nn/pia/transport/StationConnectionInfoTable.hpp"
#include "nn/pia/transport/StationManager.hpp"
#include "nn/pia/transport/StationPacketHandler.hpp"
#include "nn/pia/transport/StationProtocol.hpp"
#include "nn/pia/transport/ThreadStreamManager.hpp"
#include "platform/wiiu/pead.hpp"

namespace nn
{
namespace pia
{
namespace session
{

namespace
{
enum MeshProtocolResult
{
    cResultInvalidArgument = 0x10c07,
    cResultInvalidState = 0x10c08
};

enum
{
    cStationIndexNone = 0xfd,
    cStationNumMax = transport::StationManager::cStationNumMax,
    cStationIndexMax = cStationNumMax - 1,
    cWatermarkSendBuffer = 7,
    cWatermarkReceiveBuffer = 8,
    cUpdateMeshHeaderSize = 12,
    cSystemMessageHeaderSize = 4,
    cJoinResponseHeaderSize = 16,
    cJoinResponsePartHeaderSize = 8,
    cMultiMigrationMessageSize = 44,
    cRankDecisionMessageBufferSize = 16,
    cRankDecisionMessageSize = 13,
    cAckIdSize = 4,
    cRankDecisionReceivedSize = cRankDecisionMessageSize + cAckIdSize,
    cStationEntryAlignMask = 3,
    cDefaultUpdateInterval = 5000,
    cBroadcastUpdateInterval = 2000,
    cDefaultUnknown24 = 10000,
    cTraceStationLevel = 0x20000000,
    cTraceStationJoinLevel = 0x10000000,
    cTraceStationUpdateLevel = 0x8000000,
    cTraceStationMigrationLevel = 0x40000000,
    cTraceRelayRouteLevel = 0x1000000,
    cTraceTableLevel = 0x4000000,
    cTraceMessageLevel = 0x4000000,
    cStationIndexBroadcast = 0xff,
    cSlidingWindowSendCapacity = 4,
    cSlidingWindowReceiveCapacity = 4,
    cStationReconnectState = 6,
    cStationConnectedState = 5,
    cConnectionFailureNoticeSize = 9,
    cConnectionReportHeaderSize = 0x10,
    cKickoutReasonRelayFailureBase = 3,
    cDisconnectReason6 = 6,
    cDisconnectReason8 = 8,
    cDisconnectReason9 = 9,
    cResultJoinMeshCancelled = 0x6439,
    cSecondaryProtocolIdSuffix = 1,
    cHostMigrationModeMulti = 2
};

const unsigned int cUnlimitedDispatchSize = std::numeric_limits<unsigned int>::max();

inline void updateReceiveWatermark(const transport::ReliableSlidingWindow& window)
{
    if (transport::g_piaWatermarkManager != 0)
    {
        unsigned int receiveCount = window.m_receiveBuffer.m_count;
        transport::g_piaWatermarkManager->GetWatermark(cWatermarkReceiveBuffer)->Update(receiveCount);
    }
}

inline void updateSendWatermark(const transport::ReliableSlidingWindow& window)
{
    if (transport::g_piaWatermarkManager != 0)
    {
        unsigned int sendCount = window.m_sendBuffer.m_count;
        transport::g_piaWatermarkManager->GetWatermark(cWatermarkSendBuffer)->Update(sendCount);
    }
}
}

MeshProtocol::MeshProtocol()
    : transport::Protocol(),
      m_waitingJoinRequestJob(0),
      m_waitingJoinResponseJob(0),
      m_waitingLeaveRequestJob(0),
      m_waitingLeaveResponseJob(0),
      m_unknown24(cDefaultUnknown24),
      m_unknown28(cDefaultUpdateInterval),
      m_unknown30(),
      m_windowNum(0),
      m_windows(0),
      m_unknown40(0),
      m_bufferSize(0),
      m_buffer(0),
      m_updateMeshBufferSize(0),
      m_updateMeshBuffer(0),
      m_updateMeshBufferValid(0),
      m_nextTimeNum(0),
      m_nextTimes(0),
      m_broadcastSupported(0)
{
    if (transport::g_piaWatermarkManager != 0)
    {
        transport::g_piaWatermarkManager->GetWatermark(cWatermarkSendBuffer)
            ->SetName("MeshProtocolReliable send buffer num");
        if (transport::g_piaWatermarkManager != 0)
        {
            transport::g_piaWatermarkManager->GetWatermark(cWatermarkReceiveBuffer)
                ->SetName("MeshProtocolReliable receive buffer num");
        }
    }
}

MeshProtocol::~MeshProtocol()
{
}

void MeshProtocol::Trace(unsigned long long value) const
{
}

unsigned short MeshProtocol::GetProtocolType() const
{
    return cProtocolType;
}

Result MeshProtocol::StartupWithStationAddress(const common::StationAddress& address)
{
    return transport::Protocol::StartupWithStationAddress(address);
}

Result MeshProtocol::StartupNative()
{
    m_waitingLeaveResponseJob = 0;
    m_waitingJoinResponseJob = 0;
    m_waitingJoinRequestJob = 0;
    m_waitingLeaveRequestJob = 0;
    m_unknown38 = 0;
    unsigned int broadcastSupported = transport::g_piaThreadStreamManager->m_packetOutput->IsBroadcastSupported();
    m_unknown28 = broadcastSupported != 0 ? cBroadcastUpdateInterval : cDefaultUpdateInterval;
    m_broadcastSupported = broadcastSupported;
    return 0;
}

common::StepSequenceJob* MeshProtocol::GetWaitingJoinRequestJob()
{
    return m_waitingJoinRequestJob;
}

common::StepSequenceJob* MeshProtocol::GetWaitingJoinResponseJob()
{
    return m_waitingJoinResponseJob;
}

common::StepSequenceJob* MeshProtocol::GetWaitingLeaveResponseJob()
{
    return m_waitingLeaveResponseJob;
}

void MeshProtocol::SetWaitingJoinRequestJob(common::StepSequenceJob* job)
{
    m_waitingJoinRequestJob = job;
}

void MeshProtocol::SetWaitingJoinResponseJob(common::StepSequenceJob* job)
{
    m_waitingJoinResponseJob = job;
}

void MeshProtocol::SetWaitingLeaveResponseJob(common::StepSequenceJob* job)
{
    m_waitingLeaveResponseJob = job;
}

unsigned int MeshProtocol::GetJoinRequestDataSize() const
{
    return transport::StationManager::s_pInstance->m_localStation->m_stationAddress.GetSerializedSize()
           + cSystemMessageHeaderSize;
}

void MeshProtocol::MakeJoinRequestData(unsigned char* buffer)
{
    transport::Station* localStation = transport::StationManager::s_pInstance->m_localStation;
    buffer[0] = cMessageTypeJoinRequest;
    buffer[1] = static_cast<unsigned char>(localStation->GetStationIndex());
    buffer[3] = 0;
    buffer[2] = 0;
    unsigned int size = 0;
    localStation->m_stationAddress.Serialize(buffer + cSystemMessageHeaderSize, &size,
                                             GetJoinRequestDataSize() - cSystemMessageHeaderSize);
}

void MeshProtocol::Finalize()
{
    if (m_buffer != 0)
    {
        common::DeleteArray<unsigned char>(m_buffer);
        m_buffer = 0;
    }
    m_bufferSize = 0;
    if (m_updateMeshBuffer != 0)
    {
        common::DeleteArray<unsigned char>(m_updateMeshBuffer);
        m_updateMeshBuffer = 0;
    }
    transport::ReliableSlidingWindow* windows = m_windows;
    m_updateMeshBufferSize = 0;
    if (windows != 0)
    {
        unsigned int count = pead::getArraySize(windows) / sizeof(transport::ReliableSlidingWindow);
        transport::ReliableSlidingWindow* window = windows;
        while (count != 0)
        {
            window->~ReliableSlidingWindow();
            ++window;
            --count;
        }
        pead::freeToSeadHeap(windows);
        m_windows = 0;
    }
    m_windowNum = 0;
    if (m_nextTimes != 0)
    {
        pead::getArraySize(m_nextTimes);
        pead::freeToSeadHeap(m_nextTimes);
        m_nextTimes = 0;
    }
    m_nextTimeNum = 0;
}

Result MeshProtocol::Initialize()
{
    unsigned int windowNum = static_cast<unsigned short>(transport::g_piaTransport->m_stationNum - 1);
    m_windowNum = windowNum;
    pead::Heap* heap = common::HeapManager::GetHeap();
    transport::ReliableSlidingWindow* windows = static_cast<transport::ReliableSlidingWindow*>(
        pead::allocateFromSeadHeap(windowNum * sizeof(transport::ReliableSlidingWindow), heap));
    if (windows == 0)
    {
        m_windows = 0;
    }
    else
    {
        transport::ReliableSlidingWindow* window = windows;
        for (; windowNum != 0; --windowNum)
        {
            new (window) transport::ReliableSlidingWindow();
            ++window;
        }
        m_windows = windows;
    }

    for (unsigned short index = 0; index < m_windowNum; ++index)
    {
        Result result = m_windows[index].Initialize(cSlidingWindowSendCapacity, cSlidingWindowReceiveCapacity);
        if (result != 0)
        {
            Finalize();
            return result;
        }
    }

    unsigned int timeNum = static_cast<unsigned short>(transport::g_piaTransport->m_stationNum - 1);
    m_nextTimeNum = timeNum;
    heap = common::HeapManager::GetHeap();
    common::Time* times = static_cast<common::Time*>(pead::allocateFromSeadHeap(timeNum * sizeof(common::Time), heap));
    if (times == 0)
    {
        m_nextTimes = 0;
    }
    else
    {
        common::Time* time = times;
        for (; timeNum != 0; --timeNum)
        {
            new (time) common::Time();
            ++time;
        }
        m_nextTimes = times;
    }

    for (unsigned short index = 0; index < m_nextTimeNum; ++index)
    {
        common::Time now;
        now.SetNow();
        m_nextTimes[index] = now;
    }

    unsigned int infoSize = transport::StationConnectionInfo().GetSerializedSize();
    unsigned int bufferSize = (infoSize + (~infoSize & cStationEntryAlignMask) + 1)
                                  * transport::g_piaTransport->m_stationNum
                              + cJoinResponseHeaderSize;
    m_bufferSize = bufferSize;
    m_updateMeshBufferSize = bufferSize;
    m_buffer = common::NewArray<unsigned char>(bufferSize);
    m_updateMeshBuffer = common::NewArray<unsigned char>(m_updateMeshBufferSize);
    return 0;
}

Result MeshProtocol::UpdateProtocolEvent(const transport::ProtocolEvent& event)
{
    transport::Station* eventStation = transport::StationManager::s_pInstance->GetStation(event.m_stationAddress);
    int eventIndex = eventStation->GetStationIndex();
    StationIndex localIndex = g_piaMesh->GetLocalStationIndex();
    transport::Station* localStation = transport::StationManager::s_pInstance->GetStation(localIndex);
    unsigned long long localStationId = localStation->m_stationId;
    if (static_cast<int>(localIndex) > cStationIndexMax)
        return cResultInvalidState;
    if (eventIndex < 0 || eventIndex >= static_cast<int>(m_windowNum) + 1)
        return cResultInvalidArgument;

    transport::ReliableSlidingWindow* window;
    if (eventIndex < static_cast<int>(localIndex))
        window = m_windows + eventIndex;
    else if (static_cast<int>(localIndex) < eventIndex)
        window = m_windows + (eventIndex - 1);
    else
        return cResultInvalidArgument;

    if (event.m_type == 0)
    {
        unsigned int protocolType = GetProtocolType();
        Result result = window->Startup(static_cast<transport::StationPacketHandler*>(GetStationPacketHandler()),
                                        ((protocolType & 0xff) << 8) | 1, localStationId, eventStation->m_stationId);
        return result;
    }
    if (event.m_type == 1)
    {
        window->Cleanup();
        return 0;
    }
    return cResultInvalidArgument;
}

unsigned char MeshProtocol::GetSystemMessagePartitionNum(unsigned char messageType, unsigned char stationCount)
{
    unsigned int infoSize = transport::StationConnectionInfo().GetSerializedSize();
    unsigned int entrySize = infoSize + (~infoSize & cStationEntryAlignMask) + 1;
    unsigned int payloadLimit = mStationPacketHandler->GetPayloadSizeLimit();
    unsigned int messageSize = 0;
    if (stationCount > 0 && stationCount <= cStationNumMax)
    {
        for (int perMessage = stationCount; perMessage > 0; --perMessage)
        {
            if (messageType == cMessageTypeJoinResponse)
                messageSize = entrySize * perMessage + cJoinResponseHeaderSize;
            else if (messageType == cMessageTypeUpdateMesh)
                messageSize = entrySize * perMessage + cUpdateMeshHeaderSize;
            if (messageSize <= payloadLimit)
            {
                return stationCount / perMessage + (stationCount % perMessage != 0);
            }
        }
    }
    return 0;
}

void MeshProtocol::SendLeaveRequest()
{
    transport::Station* hostStation = transport::StationManager::s_pInstance->GetStation(
        g_piaMesh->GetHostStationIndex());
    if (hostStation == 0)
        return;
    transport::Station* localStation = transport::StationManager::s_pInstance->m_localStation;
    if (localStation == 0)
        return;

    int hostIndex = hostStation->GetStationIndex();
    int localIndex = localStation->GetStationIndex();
    if (hostIndex <= cStationIndexMax && localIndex <= cStationIndexMax)
    {
        transport::ReliableSlidingWindow* window;
        bool inCommunication;
        if (hostIndex == localIndex)
            return;
        if (hostIndex < localIndex)
        {
            window = m_windows + hostIndex;
            inCommunication = window->IsInCommunication();
        }
        else
        {
            window = m_windows + (hostIndex - 1);
            inCommunication = window->IsInCommunication();
        }

        if (inCommunication)
        {
            m_buffer[0] = cMessageTypeLeaveRequest;
            m_buffer[1] = static_cast<unsigned char>(localIndex);
            m_buffer[3] = 0;
            m_buffer[2] = 0;
            unsigned int size = 0;
            localStation->m_stationAddress.Serialize(m_buffer + cSystemMessageHeaderSize, &size,
                                                     m_bufferSize - cSystemMessageHeaderSize);
            Result result = window->PushData(m_buffer, size + cSystemMessageHeaderSize);
            if (result == 0)
                updateSendWatermark(*window);
        }
    }
}

void MeshProtocol::SendLeaveResponse(transport::Station* station)
{
    transport::Station* localStation = transport::StationManager::s_pInstance->m_localStation;
    if (localStation == 0)
        return;

    unsigned int messageSize = localStation->m_stationAddress.GetSerializedSize() + cSystemMessageHeaderSize;
    transport::ProtocolId protocolId(mProtocolId);
    transport::ProtocolMessageWriter* writer = mStationPacketHandler->AssignByStationAddress(
        protocolId, station->m_stationAddress, messageSize, false, false);
    if (writer == 0)
    {
        station->Trace(cTraceStationJoinLevel);
        return;
    }

    m_buffer[0] = cMessageTypeLeaveResponse;
    m_buffer[1] = static_cast<unsigned char>(localStation->GetStationIndex());
    m_buffer[3] = 0;
    m_buffer[2] = 0;
    unsigned int size;
    localStation->m_stationAddress.Serialize(m_buffer + cSystemMessageHeaderSize, &size,
                                             m_bufferSize - cSystemMessageHeaderSize);
    writer->SetPayload(m_buffer);
    Result result = mStationPacketHandler->Commit();
    if (result == 0)
    {
        transport::ProtocolId reliableProtocolId(mProtocolId);
        writer = mStationPacketHandler->AssignByStationAddress(reliableProtocolId, station->m_stationAddress,
                                                               messageSize, true, false);
        if (writer != 0)
        {
            writer->SetPayload(m_buffer);
            mStationPacketHandler->Commit();
        }
    }
}

void MeshProtocol::SendDenyingJoinResponse(const common::StationAddress& address, unsigned char reason)
{
    if (transport::StationManager::s_pInstance->m_localStation == 0)
        return;

    transport::ProtocolId protocolId(mProtocolId);
    transport::ProtocolMessageWriter* writer = mStationPacketHandler->AssignByStationAddress(protocolId, address, 5,
                                                                                             false, false);
    if (writer == 0)
    {
        address.Trace(cTraceStationUpdateLevel);
        return;
    }

    m_buffer[0] = cMessageTypeJoinResponse;
    m_buffer[1] = 0;
    m_buffer[2] = 0xff;
    m_buffer[3] = 0xff;
    m_buffer[4] = reason;
    writer->SetPayload(m_buffer);
    Result result = mStationPacketHandler->Commit();
    if (result == 0)
    {
        transport::ProtocolId reliableProtocolId(mProtocolId);
        writer = mStationPacketHandler->AssignByStationAddress(reliableProtocolId, address, 5, true, false);
        if (writer != 0)
        {
            writer->SetPayload(m_buffer);
            mStationPacketHandler->Commit();
        }
    }
}

bool MeshProtocol::SendDestroyMesh(StationIndex stationIndex)
{
    transport::Station* localStation = transport::StationManager::s_pInstance->m_localStation;
    if (localStation == 0)
        return false;
    int localIndex = static_cast<int>(g_piaMesh->GetLocalStationIndex());
    if (localIndex > cStationIndexMax)
        return false;
    if (localIndex != localStation->GetStationIndex())
        return false;
    if (transport::StationManager::s_pInstance->GetStation(stationIndex) == 0)
        return false;
    int windowIndex = static_cast<int>(stationIndex);
    if (windowIndex >= localIndex)
        --windowIndex;
    transport::ReliableSlidingWindow* window = m_windows + windowIndex;
    if (!window->IsInCommunication())
        return false;
    localStation->m_stationAddress.GetSerializedSize();
    m_buffer[0] = cMessageTypeDestroyMesh;
    m_buffer[1] = static_cast<unsigned char>(localIndex);
    m_buffer[3] = 0;
    m_buffer[2] = 0;
    unsigned int size;
    localStation->m_stationAddress.Serialize(m_buffer + cSystemMessageHeaderSize, &size,
                                             m_bufferSize - cSystemMessageHeaderSize);
    Result result = window->PushData(m_buffer, size + cSystemMessageHeaderSize);
    if (result == 0)
        updateSendWatermark(*window);
    return result == 0;
}

bool MeshProtocol::SendDestroyResponse(StationIndex stationIndex)
{
    transport::Station* localStation = transport::StationManager::s_pInstance->m_localStation;
    if (localStation == 0)
        return false;
    int localIndex = static_cast<int>(g_piaMesh->GetLocalStationIndex());
    if (localIndex > cStationIndexMax)
        return false;
    if (localIndex != localStation->GetStationIndex())
        return false;
    transport::Station* station = transport::StationManager::s_pInstance->GetStation(stationIndex);
    if (station == 0)
        return false;
    transport::ProtocolId protocolId(mProtocolId);
    transport::ProtocolMessageWriter* writer = mStationPacketHandler->AssignByStationAddress(
        protocolId, station->m_stationAddress, 2, false, false);
    if (writer == 0)
    {
        station->Trace(cTraceStationUpdateLevel);
        return false;
    }

    m_buffer[0] = cMessageTypeDestroyResponse;
    m_buffer[1] = static_cast<unsigned char>(localIndex);
    writer->SetPayload(m_buffer);
    Result result = mStationPacketHandler->Commit();
    if (result == 0)
    {
        transport::ProtocolId reliableProtocolId(mProtocolId);
        writer = mStationPacketHandler->AssignByStationAddress(reliableProtocolId, station->m_stationAddress, 2, true,
                                                               false);
        if (writer != 0)
        {
            writer->SetPayload(m_buffer);
            mStationPacketHandler->Commit();
        }
    }
    return true;
}

bool MeshProtocol::SendGreeting(StationIndex stationIndex)
{
    transport::Station* localStation = transport::StationManager::s_pInstance->m_localStation;
    if (localStation == 0)
        return false;
    unsigned int localIndex = g_piaMesh->GetLocalStationIndex();
    if (static_cast<int>(localIndex) > cStationIndexMax)
        return false;
    if (localIndex != localStation->GetStationIndex())
        return false;
    if (transport::StationManager::s_pInstance->GetStation(stationIndex) == 0)
    {
        transport::StationManager::s_pInstance->Trace(cTraceStationUpdateLevel);
        return false;
    }
    transport::ProtocolId protocolId(mProtocolId);
    transport::ProtocolMessageWriter* writer = static_cast<transport::StationPacketHandler*>(mStationPacketHandler)
                                                   ->AssignByStationIndex(protocolId, stationIndex, 2, false, false,
                                                                          true);
    if (writer == 0)
        return false;
    writer->WriteU8(cMessageTypeGreeting, 0);
    writer->WriteU8(localIndex & 0xff, 1);
    Result result = mStationPacketHandler->Commit();
    return result == 0;
}

bool MeshProtocol::SendMigrationFinish(bool succeeded)
{
    transport::Station* localStation = transport::StationManager::s_pInstance->m_localStation;
    if (localStation == 0)
        return false;
    int localIndex = static_cast<int>(g_piaMesh->GetLocalStationIndex());
    if (localIndex > cStationIndexMax)
        return false;
    if (localIndex != localStation->GetStationIndex())
        return false;
    m_buffer[0] = cMessageTypeMigrationFinish;
    m_buffer[1] = static_cast<unsigned char>(localIndex);
    m_buffer[2] = succeeded;
    for (unsigned short index = 0; index < m_windowNum; ++index)
    {
        if (m_windows[index].IsInCommunication())
        {
            Result result = m_windows[index].PushData(m_buffer, 3);
            if (result == 0)
            {
                if (transport::g_piaWatermarkManager != 0)
                {
                    unsigned int sendCount = m_windows[index].m_sendBuffer.m_count;
                    transport::g_piaWatermarkManager->GetWatermark(cWatermarkSendBuffer)->Update(sendCount);
                }
            }
        }
    }
    return true;
}

bool MeshProtocol::SendGreetingResponse(StationIndex stationIndex)
{
    transport::Station* localStation = transport::StationManager::s_pInstance->m_localStation;
    if (localStation == 0)
        return false;
    int localIndex = static_cast<int>(g_piaMesh->GetLocalStationIndex());
    if (localIndex > cStationIndexMax)
        return false;
    if (localIndex != localStation->GetStationIndex())
        return false;
    if (transport::StationManager::s_pInstance->GetStation(stationIndex) == 0)
        return false;
    int windowIndex = static_cast<int>(stationIndex);
    if (windowIndex >= localIndex)
        --windowIndex;
    transport::ReliableSlidingWindow* window = m_windows + windowIndex;
    if (!window->IsInCommunication())
        return false;
    m_buffer[0] = cMessageTypeGreetingResponse;
    m_buffer[1] = static_cast<unsigned char>(localIndex);
    Result result = window->PushData(m_buffer, 2);
    if (result == 0)
        updateSendWatermark(*window);
    return result == 0;
}

bool MeshProtocol::SendMigrationStart(StationIndex stationIndex, StationIndex newHostIndex)
{
    transport::Station* localStation = transport::StationManager::s_pInstance->m_localStation;
    if (localStation == 0)
        return false;
    int localIndex = static_cast<int>(g_piaMesh->GetLocalStationIndex());
    if (localIndex > cStationIndexMax)
        return false;
    if (localIndex != localStation->GetStationIndex())
        return false;
    if (transport::StationManager::s_pInstance->GetStation(stationIndex) == 0)
        return false;
    int windowIndex = static_cast<int>(stationIndex);
    if (windowIndex >= localIndex)
        --windowIndex;
    transport::ReliableSlidingWindow* window = m_windows + windowIndex;
    if (!window->IsInCommunication())
        return false;
    m_buffer[0] = cMessageTypeMigrationStart;
    m_buffer[1] = static_cast<unsigned char>(localIndex);
    m_buffer[2] = static_cast<unsigned char>(newHostIndex);
    Result result = window->PushData(m_buffer, 3);
    if (result == 0)
        updateSendWatermark(*window);
    return result == 0;
}

bool MeshProtocol::SendMigrationResponse(StationIndex stationIndex)
{
    transport::Station* localStation = transport::StationManager::s_pInstance->m_localStation;
    if (localStation == 0)
        return false;
    int localIndex = static_cast<int>(g_piaMesh->GetLocalStationIndex());
    if (localIndex > cStationIndexMax)
        return false;
    if (localIndex != localStation->GetStationIndex())
        return false;
    transport::Station* station = transport::StationManager::s_pInstance->GetStation(stationIndex);
    if (station == 0)
        return false;
    transport::ProtocolId protocolId(mProtocolId);
    transport::ProtocolMessageWriter* writer = mStationPacketHandler->AssignByStationAddress(
        protocolId, station->m_stationAddress, 2, false, false);
    if (writer == 0)
    {
        station->Trace(cTraceStationMigrationLevel);
        return false;
    }

    m_buffer[0] = cMessageTypeMigrationResponse;
    m_buffer[1] = static_cast<unsigned char>(localIndex);
    writer->SetPayload(m_buffer);
    Result result = mStationPacketHandler->Commit();
    if (result == 0)
    {
        transport::ProtocolId reliableProtocolId(mProtocolId);
        writer = mStationPacketHandler->AssignByStationAddress(reliableProtocolId, station->m_stationAddress, 2, true,
                                                               false);
        if (writer != 0)
        {
            writer->SetPayload(m_buffer);
            mStationPacketHandler->Commit();
        }
    }
    return true;
}

bool MeshProtocol::SendRelayRouteDirections()
{
    transport::Station* localStation = transport::StationManager::s_pInstance->m_localStation;
    if (localStation == 0)
        return false;
    int localIndex = static_cast<int>(g_piaMesh->GetLocalStationIndex());
    if (localIndex > cStationIndexMax)
        return false;
    if (localIndex != localStation->GetStationIndex())
        return false;
    transport::RelayRouteManager* relayRouteManager = transport::g_piaTransport->m_relayRouteManager;
    if (relayRouteManager == 0)
        return false;
    unsigned int directionsSize = relayRouteManager->GetRelayRouteDirectionsSize();
    unsigned char* buffer = m_buffer;
    buffer[0] = cMessageTypeRelayRouteDirections;
    buffer[1] = static_cast<unsigned char>(localIndex);
    buffer[2] = g_piaMesh->GetStationNum();
    buffer[3] = 0;
    common::serializeU32(buffer + 4, relayRouteManager->m_unknown20);
    common::serializeU32(buffer + 8, relayRouteManager->m_unknown1c);
    unsigned int writtenSize = 0;
    Result result = relayRouteManager->WriteRelayRouteDirectionsToBuffer(
        buffer + cUpdateMeshHeaderSize, m_bufferSize - cUpdateMeshHeaderSize, &writtenSize);
    if (result != 0)
        return false;
    for (unsigned short index = 0; index < m_windowNum; ++index)
    {
        if (m_windows[index].IsInCommunication())
        {
            result = m_windows[index].PushData(m_buffer, directionsSize + cUpdateMeshHeaderSize);
            if (result == 0)
            {
                if (transport::g_piaWatermarkManager != 0)
                {
                    unsigned int sendCount = m_windows[index].m_sendBuffer.m_count;
                    transport::g_piaWatermarkManager->GetWatermark(cWatermarkSendBuffer)->Update(sendCount);
                }
            }
        }
    }
    return true;
}

bool MeshProtocol::SendKickoutNotice(StationIndex stationIndex, unsigned char reason)
{
    int localIndex = static_cast<int>(g_piaMesh->GetLocalStationIndex());
    if (localIndex > cStationIndexMax)
        return false;
    int windowIndex = static_cast<int>(stationIndex);
    if (windowIndex >= localIndex)
        --windowIndex;
    transport::ReliableSlidingWindow* window = m_windows + windowIndex;
    if (!window->IsInCommunication())
        return false;
    m_buffer[0] = cMessageTypeKickoutNotice;
    m_buffer[1] = static_cast<unsigned char>(localIndex);
    m_buffer[2] = reason;
    Result result = window->PushData(m_buffer, 3);
    if (result == 0)
        updateSendWatermark(*window);
    return result == 0;
}

bool MeshProtocol::SendDummyMessage(StationIndex stationIndex)
{
    unsigned int localIndex = g_piaMesh->GetLocalStationIndex();
    if (static_cast<int>(localIndex) > cStationIndexMax)
        return false;
    transport::ProtocolId protocolId(mProtocolId);
    transport::ProtocolMessageWriter* writer = static_cast<transport::StationPacketHandler*>(mStationPacketHandler)
                                                   ->AssignByStationIndex(protocolId, stationIndex, 2, false, false,
                                                                          true);
    if (writer == 0)
        return false;
    writer->WriteU8(cMessageTypeDummyMessage, 0);
    writer->WriteU8(localIndex & 0xff, 1);
    Result result = mStationPacketHandler->Commit();
    return result == 0;
}

bool MeshProtocol::SendDummyAck(StationIndex stationIndex)
{
    unsigned int localIndex = g_piaMesh->GetLocalStationIndex();
    if (static_cast<int>(localIndex) > cStationIndexMax)
        return false;
    transport::ProtocolId protocolId(mProtocolId);
    transport::ProtocolMessageWriter* writer = static_cast<transport::StationPacketHandler*>(mStationPacketHandler)
                                                   ->AssignByStationIndex(protocolId, stationIndex, 2, false, false,
                                                                          true);
    if (writer == 0)
        return false;
    writer->WriteU8(cMessageTypeDummyAck, 0);
    writer->WriteU8(localIndex & 0xff, 1);
    Result result = mStationPacketHandler->Commit();
    return result == 0;
}

void MeshProtocol::SendConnectionFailureNotice(StationIndex destination, StationIndex reporter, StationIndex failed,
                                               unsigned char reason, unsigned int version)
{
    int localIndex = static_cast<int>(g_piaMesh->GetLocalStationIndex());
    if (localIndex <= cStationIndexMax)
    {
        transport::ProtocolId protocolId(mProtocolId);
        transport::ProtocolMessageWriter* writer = static_cast<transport::StationPacketHandler*>(mStationPacketHandler)
                                                       ->AssignByStationIndex(protocolId, destination, 9, false, false,
                                                                              true);
        if (writer != 0)
        {
            unsigned char payload[9];
            payload[0] = cMessageTypeConnectionFailureNotice;
            payload[1] = static_cast<unsigned char>(localIndex);
            payload[2] = static_cast<unsigned char>(reporter);
            payload[3] = static_cast<unsigned char>(failed);
            payload[4] = reason;
            common::serializeU32(payload + 5, version);
            writer->SetPayload(payload, 0, 9);
            mStationPacketHandler->Commit();
        }
    }
}

void MeshProtocol::SendUpdateMesh(bool update, unsigned int versionIncrement)
{
    bool leavingMesh = g_piaMesh->IsProcessingLeaveMesh();
    transport::StationConnectionInfo info;
    unsigned int infoSize = info.GetSerializedSize();
    if (transport::StationManager::s_pInstance->m_localStation == 0)
        return;

    unsigned int hostIndex = g_piaMesh->GetHostStationIndex();
    unsigned int bufferSize = (infoSize + (~infoSize & cStationEntryAlignMask) + 1) * g_piaMesh->m_unknown54
                              + cUpdateMeshHeaderSize;
    if (!update)
    {
        if (!m_updateMeshBufferValid)
            return;
    }
    else
    {
        g_piaMesh->m_unknown80 += versionIncrement;
        std::memset(m_updateMeshBuffer, 0, m_updateMeshBufferSize);
        transport::StationConnectionInfoTable* table = transport::g_stationConnectionInfoTable;
        unsigned char stationCount = 0;
        unsigned char hostSlot = 0xff;
        int offset = cUpdateMeshHeaderSize;
        unsigned char* buffer = m_updateMeshBuffer;
        for (transport::Station** it = transport::StationManager::s_pInstance->m_stations.data;
             it
             != transport::StationManager::s_pInstance->m_stations.data
                    + transport::StationManager::s_pInstance->m_stations.size;
             ++it)
        {
            transport::Station* station = *it;
            unsigned char stationIndex = station->GetStationIndex();
            if (stationIndex >= cStationNumMax)
            {
                station->Trace(cTraceStationLevel);
                continue;
            }
            if (!g_piaMesh->IsValidStationIndex(static_cast<StationIndex>(stationIndex)))
            {
                station->Trace(cTraceStationLevel);
                continue;
            }
            if (stationIndex == hostIndex)
                hostSlot = stationCount;
            unsigned int size = 0;
            table->GetStationConnectionInfo(station, &info);
            info.Serialize(buffer + offset, &size, infoSize);
            buffer[offset + size] = stationIndex;
            stationCount = (stationCount + 1) & 0xff;
            offset += size + 1 + (~infoSize & cStationEntryAlignMask);
        }
        if (g_piaMesh->m_unknown54 < stationCount)
        {
            transport::StationManager::s_pInstance->Trace(cTraceTableLevel);
            table->Trace(cTraceTableLevel);
        }
        buffer[2] = hostSlot;
        buffer[1] = stationCount;
        buffer[0] = cMessageTypeUpdateMesh;
        common::serializeU32(buffer + 4, g_piaMesh->m_unknown80);
        buffer[10] = stationCount;
        buffer[8] = 1;
        buffer[11] = 0;
        buffer[9] = 0;
        m_updateMeshBufferValid = 1;
    }

    if (!leavingMesh)
    {
        if (!m_broadcastSupported)
        {
            int sentCount = 0;
            if (update)
            {
                for (unsigned short index = 0; index < m_windowNum; ++index)
                {
                    if (m_windows[index].IsInCommunication())
                    {
                        transport::Station* station =
                            transport::StationManager::s_pInstance->GetStationByStationIdWithoutConvert(
                                m_windows[index].m_remoteStationId);
                        if (station != 0 && g_piaMesh->IsValidStationIndex(station->GetStationIndex()))
                        {
                            int delay = (m_unknown28 / transport::g_piaTransport->m_stationNum) * sentCount;
                            Result result = m_windows[index].PushData(m_updateMeshBuffer, bufferSize);
                            if (result == 0)
                            {
                                updateSendWatermark(m_windows[index]);
                                delay += m_unknown28;
                            }
                            m_nextTimes[index].m_tick = transport::g_piaTransport->m_time.m_tick
                                                        + common::TimeSpan::GetTicksPerMSec().m_tick * delay;
                            ++sentCount;
                        }
                    }
                }
            }
            else
            {
                for (unsigned short index = 0; index < m_windowNum; ++index)
                {
                    if (m_windows[index].IsInCommunication())
                    {
                        transport::Station* station =
                            transport::StationManager::s_pInstance->GetStationByStationIdWithoutConvert(
                                m_windows[index].m_remoteStationId);
                        if (station != 0 && g_piaMesh->IsValidStationIndex(station->GetStationIndex()))
                        {
                            int delay = (m_unknown28 / transport::g_piaTransport->m_stationNum) * sentCount;
                            m_nextTimes[index].m_tick = transport::g_piaTransport->m_time.m_tick
                                                        + common::TimeSpan::GetTicksPerMSec().m_tick * delay;
                            ++sentCount;
                        }
                    }
                }
            }
        }
        else if (mStationPacketHandler != 0)
        {
            transport::ProtocolId protocolId(mProtocolId);
            transport::ProtocolMessageWriter* writer =
                static_cast<transport::StationPacketHandler*>(mStationPacketHandler)
                    ->AssignByStationIndex(protocolId, static_cast<StationIndex>(cStationIndexBroadcast), bufferSize,
                                           false, false, true);
            if (writer != 0)
            {
                writer->SetPayload(m_updateMeshBuffer);
                Result result = mStationPacketHandler->Commit();
                if (result == 0)
                    m_unknown30 = transport::g_piaTransport->m_time;
            }
        }
    }
}

void MeshProtocol::ParseJoinResponse(const transport::ReceivedMessageAccessor& message)
{
    if (transport::StationManager::s_pInstance->m_localStation == 0)
        return;

    const unsigned char* data = message.m_message;
    unsigned int ackId = transport::g_piaResendingMessageManager->ExtractAckIdFromMessage(data, message.m_messageSize);
    JoinMeshJob* job = static_cast<JoinMeshJob*>(GetWaitingJoinResponseJob());
    if (job == 0)
    {
        static_cast<transport::StationProtocol*>(g_piaMesh->GetStationProtocol())
            ->SendAck(ackId, message.m_stationAddress);
    }
    else if (job->m_waitingJoinResponse)
    {
        transport::Station* hostStation = transport::StationManager::s_pInstance->GetStation(
            g_piaMesh->GetHostStationIndex());
        if (hostStation != 0)
        {
            if (hostStation->m_stationAddress == message.m_stationAddress
                && job->SetStationDataList(data, message.m_messageSize) != 0)
            {
                static_cast<transport::StationProtocol*>(g_piaMesh->GetStationProtocol())
                    ->SendAck(ackId, hostStation->m_stationAddress);
            }
        }
    }
}

void MeshProtocol::ParseLeaveRequest(const transport::ReceivedMessageAccessor& message)
{
    if (transport::StationManager::s_pInstance->m_localStation == 0)
        return;

    const unsigned char* data = message.m_message;
    common::StationAddress address;
    address.Deserialize(data + cSystemMessageHeaderSize);
    int stationIndex = data[1];
    transport::Station* station = transport::StationManager::s_pInstance->GetStation(
        static_cast<StationIndex>(stationIndex));
    if (station != 0)
    {
        SendLeaveResponse(station);
        if (g_piaMesh->IsValidStationIndex(static_cast<StationIndex>(stationIndex)))
            g_piaMesh->UnfixDisconnectedId(static_cast<StationIndex>(stationIndex));
        transport::RelayRouteManager* relayRouteManager = transport::g_piaTransport->m_relayRouteManager;
        if (relayRouteManager != 0 && stationIndex <= cStationIndexMax)
            relayRouteManager->SearchRefugeRelayRoute(static_cast<StationIndex>(stationIndex));
        station->m_field20 = cStationReconnectState;
        station->SetStationIdAndIndex(0, static_cast<StationIndex>(cStationIndexNone));
        SendUpdateMesh(true, 1);
    }
}

void MeshProtocol::ParseLeaveResponse(const transport::ReceivedMessageAccessor& message)
{
    if (transport::StationManager::s_pInstance->m_localStation == 0)
        return;

    const unsigned char* data = message.m_message;
    LeaveMeshJob* job = static_cast<LeaveMeshJob*>(GetWaitingLeaveResponseJob());
    if (job != 0 && job->m_disconnectStarted)
    {
        common::StationAddress address;
        address.Deserialize(data + cSystemMessageHeaderSize);
        unsigned char stationIndex = data[1];
        transport::Station* hostStation = transport::StationManager::s_pInstance->GetStation(
            g_piaMesh->GetHostStationIndex());
        if (hostStation != 0 && hostStation->GetStationIndex() == stationIndex)
            job->m_disconnectStarted = false;
    }
}

void MeshProtocol::ParseDestroyResponse(const transport::ReceivedMessageAccessor& message)
{
    transport::Station* localStation = transport::StationManager::s_pInstance->m_localStation;
    if (localStation == 0)
        return;
    DestroyMeshJob* job = g_piaMesh->m_destroyMeshJob;
    StationIndex stationIndex = static_cast<StationIndex>(message.m_message[1]);
    if (!job->m_waitingResponses)
        return;
    job->ReceiveDestroyResponse(stationIndex);
}

void MeshProtocol::ParseKickoutNotice(const transport::ReceivedMessageAccessor& message)
{
    if (!g_piaMesh->IsHost() && message.m_messageSize == 3)
    {
        const unsigned char* data = message.m_message;
        int stationIndex = data[1];
        if (stationIndex <= cStationIndexMax && stationIndex == g_piaMesh->GetHostStationIndex())
        {
            KickoutManageJob* job = g_piaMesh->m_kickoutManageJob;
            if (job->StartupClient(static_cast<KickoutManageJob::KickoutReason>(data[2])))
                job->Ready(false);
        }
    }
}

void MeshProtocol::ParseDummyMessage(const transport::ReceivedMessageAccessor& message)
{
    if (message.m_sourceStationId == 0)
        return;
    const unsigned char* data = message.m_message;
    if (message.m_messageSize != 2)
        return;
    SendDummyAck(static_cast<StationIndex>(data[1]));
}

void MeshProtocol::ParseRelayRouteDirections(const transport::ReceivedMessageAccessor& message)
{
    transport::RelayRouteManager* relayRouteManager = 0;
    if (transport::StationManager::s_pInstance->m_localStation != 0 && !g_piaMesh->IsHost()
        && (relayRouteManager = transport::g_piaTransport->m_relayRouteManager) != 0)
    {
        const unsigned char* data = message.m_message;
        unsigned int messageSize = message.m_messageSize;
        unsigned char senderIndex = data[1];
        if (senderIndex == g_piaMesh->GetHostStationIndex()
            && static_cast<unsigned short>(data[2]) <= g_piaMesh->m_unknown54 && data[3] == 0)
        {
            unsigned int versionHigh = common::deserializeU32(data + 4);
            unsigned int versionLow = common::deserializeU32(data + 8);
            if (relayRouteManager->m_unknown20 <= versionHigh
                && (relayRouteManager->m_unknown20 < versionHigh || versionLow > relayRouteManager->m_unknown1c))
            {
                relayRouteManager->Trace(cTraceRelayRouteLevel);
                relayRouteManager->ReadRelayRouteDirectionsFromBuffer(data + cUpdateMeshHeaderSize,
                                                                      messageSize - cUpdateMeshHeaderSize);
                relayRouteManager->m_unknown1c = versionLow;
                relayRouteManager->m_unknown20 = versionHigh;
            }
        }
    }
}

void MeshProtocol::ParseJoinRequest(const transport::ReceivedMessageAccessor& message)
{
    if (transport::StationManager::s_pInstance->m_localStation == 0)
        return;

    const unsigned char* data = message.m_message;
    unsigned int ackId = transport::g_piaResendingMessageManager->ExtractAckIdFromMessage(data, message.m_messageSize);
    ProcessJoinRequestJob* job = static_cast<ProcessJoinRequestJob*>(GetWaitingJoinRequestJob());
    if (job == 0)
        return;

    if (job->m_unknown78 != 1)
    {
        if (job->GetState() == common::Job::cStateSuspended)
        {
            common::StationAddress address(message.m_stationAddress);
            transport::Station* station = transport::StationManager::s_pInstance->GetStation(address);
            if (station != 0 && station->GetStationIndex() == cStationIndexNone && data[1] == cStationIndexNone)
            {
                station->m_lastReceiveTime = transport::g_piaTransport->m_time;
                job->SetJoiningStationData(address);
                job->m_unknown4c = ackId;
                static_cast<transport::StationProtocol*>(g_piaMesh->GetStationProtocol())->SendAck(ackId, address);
                job->Resume(false);
            }
            return;
        }
        if (job->m_unknown78 != 1)
            return;
    }

    if (job->m_unknown4c == ackId)
    {
        static_cast<transport::StationProtocol*>(g_piaMesh->GetStationProtocol())
            ->SendAck(ackId, message.m_stationAddress);
        transport::Station* station = transport::StationManager::s_pInstance->GetStation(message.m_stationAddress);
        if (station != 0)
        {
            station->m_lastReceiveTime = transport::g_piaTransport->m_time;
        }
    }
}

void MeshProtocol::ParseDestroyMesh(const transport::ReceivedMessageAccessor& message)
{
    if (transport::StationManager::s_pInstance->m_localStation == 0)
        return;

    const unsigned char* data = message.m_message;
    common::StationAddress address;
    address.Deserialize(data + cSystemMessageHeaderSize);
    unsigned char senderIndex = data[1];
    transport::Station* hostStation = transport::StationManager::s_pInstance->GetStation(
        g_piaMesh->GetHostStationIndex());
    if (hostStation == 0)
    {
        if (g_piaMesh->GetProcessHostMigrationJobPtr()->m_started)
        {
            ProcessDestroyMeshJob* job = g_piaMesh->m_processDestroyMeshJob;
            if (job->Startup())
                job->Ready(false);
        }
    }
    else if (hostStation->GetStationIndex() == senderIndex)
    {
        ProcessDestroyMeshJob* job = g_piaMesh->m_processDestroyMeshJob;
        if (job->Startup())
            job->Ready(false);
    }
}

void MeshProtocol::ParseGreeting(const transport::ReceivedMessageAccessor& message)
{
    if (g_piaMesh->IsHostMigrationEnabled() && transport::StationManager::s_pInstance->m_localStation != 0)
    {
        StationIndex stationIndex = static_cast<StationIndex>(message.m_message[1]);
        if (!g_piaMesh->m_processHostMigrationJob->m_unknownA5)
        {
            if (!g_piaMesh->m_processHostMigrationJob->m_started && g_piaMesh->m_meshEventListener != 0)
            {
                MeshEventListener::MeshEvent event;
                event.stationIndex = stationIndex;
                event.stationId = 0;
                event.type = Session::cEventTypeUnknown15;
                g_piaMesh->m_meshEventListener->MeshEventListener::NoticeEvent(event);
            }
        }
        else
        {
            g_piaMesh->m_processHostMigrationJob->ReceiveGreeting(stationIndex);
        }
    }
}

void MeshProtocol::ParseMigrationFinish(const transport::ReceivedMessageAccessor& message)
{
    if (g_piaMesh->IsHostMigrationEnabled() && transport::StationManager::s_pInstance->m_localStation != 0)
    {
        const unsigned char* data = message.m_message;
        bool succeeded = data[2] != 0;
        if (data[1] == g_piaMesh->GetHostStationIndex())
        {
            if (g_piaMesh->m_processHostMigrationJob->m_unknownB8)
                g_piaMesh->m_processHostMigrationJob->ReceiveFinish(succeeded);
        }
    }
}

void MeshProtocol::ParseGreetingResponse(const transport::ReceivedMessageAccessor& message)
{
    if (g_piaMesh->IsHostMigrationEnabled() && transport::StationManager::s_pInstance->m_localStation != 0)
    {
        StationIndex stationIndex = static_cast<StationIndex>(message.m_message[1]);
        if (g_piaMesh->m_processHostMigrationJob->m_greetingSent)
            g_piaMesh->m_processHostMigrationJob->ReceiveGreetingResponse(stationIndex);
    }
}

void MeshProtocol::ParseMigrationStart(const transport::ReceivedMessageAccessor& message)
{
    if (g_piaMesh->IsHostMigrationEnabled() && transport::StationManager::s_pInstance->m_localStation != 0
        && !g_piaMesh->m_leaveMeshJob->IsRunning())
    {
        const unsigned char* data = message.m_message;
        unsigned char senderIndex = data[1];
        if (senderIndex == g_piaMesh->GetHostStationIndex())
        {
            int newHostIndex = data[2];
            if (newHostIndex <= cStationIndexMax && newHostIndex != g_piaMesh->GetHostStationIndex())
            {
                ProcessHostMigrationJob* job = g_piaMesh->m_processHostMigrationJob;
                if (job->Startup(true, static_cast<StationIndex>(newHostIndex)))
                    job->Ready(false);
            }
        }
    }
}

void MeshProtocol::ParseMultiMigrationStart(const transport::ReceivedMessageAccessor& message)
{
    if (g_piaMesh->IsHostMigrationEnabled() && transport::StationManager::s_pInstance->m_localStation != 0
        && !g_piaMesh->IsHost() && !g_piaMesh->m_leaveMeshJob->IsRunning())
    {
        const unsigned char* data = message.m_message;
        unsigned char senderIndex = data[1];
        if (message.m_messageSize == g_piaMesh->m_unknown54 + cUpdateMeshHeaderSize
            && senderIndex == g_piaMesh->GetHostStationIndex())
        {
            unsigned int versionHigh = common::deserializeU32(data + 4);
            unsigned int versionLow = common::deserializeU32(data + 8);
            ProcessHostMigrationJob* job = g_piaMesh->m_processHostMigrationJob;
            if (job->StartupMulti(true, data + cUpdateMeshHeaderSize, versionHigh, versionLow))
                job->Ready(false);
        }
    }
}

void MeshProtocol::ParseMultiMigrationRankDecision(const transport::ReceivedMessageAccessor& message)
{
    if (g_piaMesh->IsHostMigrationEnabled() && transport::StationManager::s_pInstance->m_localStation != 0
        && message.m_messageSize == cRankDecisionReceivedSize)
    {
        const unsigned char* data = message.m_message;
        unsigned int ackId = transport::g_piaResendingMessageManager->ExtractAckIdFromMessage(
            data, cRankDecisionReceivedSize);
        unsigned char rawHigher = data[4];
        StationIndex senderIndex = static_cast<StationIndex>(data[1]);
        if (rawHigher == 0 || rawHigher == 1)
        {
            bool higher = rawHigher != 0;
            unsigned int meshValue = common::deserializeU32(data + 5);
            unsigned int relayValue = common::deserializeU32(data + 9);
            ProcessHostMigrationJob* job = g_piaMesh->m_processHostMigrationJob;
            bool firstIsHigher;
            unsigned int candidateMeshValue;
            unsigned int candidateRelayValue;
            Result result = job->CheckHostCandidateRanking(g_piaMesh->GetLocalStationIndex(), senderIndex,
                                                           &firstIsHigher, &candidateMeshValue, &candidateRelayValue);
            if (result == 0)
            {
                if (firstIsHigher != higher
                    && (meshValue < candidateMeshValue
                        || (meshValue <= candidateMeshValue && candidateRelayValue > relayValue)))
                {
                    higher = firstIsHigher;
                }
                static_cast<transport::StationProtocol*>(g_piaMesh->GetStationProtocol())->SendAck(ackId, senderIndex);
                job->SetRankDecision(senderIndex, higher);
            }
        }
    }
}

bool MeshProtocol::SendMultiMigrationStart(bool* sent)
{
    transport::Station* localStation = transport::StationManager::s_pInstance->m_localStation;
    if (localStation != 0)
    {
        int localIndex = static_cast<int>(g_piaMesh->GetLocalStationIndex());
        if (localIndex <= cStationIndexMax && localIndex == localStation->GetStationIndex())
        {
            unsigned char message[cMultiMigrationMessageSize] = {0};
            unsigned short stationNum = g_piaMesh->m_unknown54;
            unsigned int meshValue;
            unsigned int relayValue;
            Result result = g_piaMesh->m_processHostMigrationJob->MakeHostCandidateRanking(
                static_cast<StationIndex>(localIndex), message + cUpdateMeshHeaderSize, &meshValue, &relayValue, true);
            if (result == 0)
            {
                message[1] = static_cast<unsigned char>(localIndex);
                message[0] = cMessageTypeMultiMigrationStart;
                message[2] = g_piaMesh->GetStationNum();
                common::serializeU32(message + 4, meshValue);
                common::serializeU32(message + 8, relayValue);
                for (int index = 0; index < cStationNumMax; ++index)
                {
                    sent[index] = false;
                    if (index != localIndex && g_piaMesh->IsValidStationIndex(static_cast<StationIndex>(index)))
                    {
                        transport::ReliableSlidingWindow* window;
                        if (index < localIndex)
                            window = m_windows + index;
                        else
                            window = m_windows + (index - 1);
                        if (window->IsInCommunication())
                        {
                            result = window->PushData(message, stationNum + cUpdateMeshHeaderSize);
                            if (result == 0)
                            {
                                updateSendWatermark(*window);
                                sent[index] = true;
                            }
                        }
                    }
                }
                return true;
            }
        }
    }
    return false;
}

bool MeshProtocol::SendMultiMigrationRankDecision(StationIndex stationIndex, long long timeout, unsigned int* ackId)
{
    transport::Station* localStation = transport::StationManager::s_pInstance->m_localStation;
    if (localStation == 0)
        return false;
    int localIndex = static_cast<int>(g_piaMesh->GetLocalStationIndex());
    if (localIndex > cStationIndexMax)
        return false;
    if (localIndex != localStation->GetStationIndex())
        return false;
    if (transport::StationManager::s_pInstance->GetStation(stationIndex) == 0)
        return false;
    bool firstIsHigher;
    unsigned int meshValue;
    unsigned int relayValue;
    Result rankResult = g_piaMesh->m_processHostMigrationJob->CheckHostCandidateRanking(
        static_cast<StationIndex>(localIndex), stationIndex, &firstIsHigher, &meshValue, &relayValue);
    if (rankResult != 0)
        return false;
    unsigned char message[cRankDecisionMessageBufferSize] = {0};
    message[0] = cMessageTypeMultiMigrationRankDecision;
    message[1] = static_cast<unsigned char>(localIndex);
    message[2] = g_piaMesh->GetHostStationIndex();
    message[3] = g_piaMesh->GetStationNum();
    message[4] = !firstIsHigher;
    common::serializeU32(message + 5, meshValue);
    common::serializeU32(message + 9, relayValue);
    transport::ProtocolId protocolId(mProtocolId);
    return transport::g_piaResendingMessageManager->SetSendMessageByStationIndex(
               ackId, message, cRankDecisionMessageSize, stationIndex, protocolId, true, timeout)
           == 0;
}

unsigned char MeshProtocol::MakeJoinResponseData(StationIndex stationIndex, unsigned char** buffers,
                                                 unsigned int* sizes)
{
    transport::StationConnectionInfo info;
    unsigned int infoSize = info.GetSerializedSize();
    unsigned int hostIndex = g_piaMesh->GetHostStationIndex();
    std::memset(m_buffer, 0, m_bufferSize);
    unsigned int hostSlot = 0xff;
    unsigned int count = 0;
    unsigned int stationSlot = 0xff;
    unsigned char* buffer = m_buffer;
    if (g_piaSession != 0)
    {
        unsigned int current = g_piaSession->m_currentSessionIndex;
        if (g_piaSession->m_matchmakeSession[current] != 0)
            buffer[8] = g_piaSession->m_fieldc2[current];
        unsigned int other = current == 0;
        if (g_piaSession->m_matchmakeSession[other] != 0)
            buffer[9] = g_piaSession->m_fieldc2[other];
    }
    buffer[10] = g_piaMesh->m_unknown54;
    common::serializeU32(buffer + 12, g_piaMesh->m_unknown80);
    transport::StationConnectionInfoTable* table = transport::g_stationConnectionInfoTable;
    unsigned int offset = cJoinResponseHeaderSize;
    for (transport::Station** it = transport::StationManager::s_pInstance->m_stations.data;
         it
         != transport::StationManager::s_pInstance->m_stations.data
                + transport::StationManager::s_pInstance->m_stations.size;
         ++it)
    {
        transport::Station* station = *it;
        unsigned char index = station->GetStationIndex();
        if (index >= cStationNumMax)
        {
            station->Trace(cTraceStationUpdateLevel);
            continue;
        }
        if (index == hostIndex)
            hostSlot = count;
        if (index == stationIndex)
            stationSlot = count;
        unsigned int size = 0;
        table->GetStationConnectionInfo(station, &info);
        info.Serialize(buffer + offset, &size, infoSize);
        buffer[offset + size] = index;
        count = (count + 1) & 0xff;
        offset += size + 1 + (~infoSize & cStationEntryAlignMask);
    }

    unsigned int partitionNum = GetSystemMessagePartitionNum(cMessageTypeJoinResponse, count);
    char stationCount = count;
    if (partitionNum == 1)
    {
        if (mStationPacketHandler->GetPayloadSizeLimit() < offset)
            return 0;
        unsigned char* output = buffers[0];
        output[7] = 0;
        output[1] = stationCount;
        output[5] = 0;
        output[3] = stationSlot;
        output[6] = stationCount;
        output[2] = hostSlot;
        output[0] = cMessageTypeJoinResponse;
        output[4] = 1;
        std::memcpy(output + cJoinResponsePartHeaderSize, buffer + cJoinResponsePartHeaderSize,
                    offset - cJoinResponsePartHeaderSize);
        sizes[0] = offset;
        return 1;
    }

    if (partitionNum - 2 > 1)
        return 0;

    int consumed = 0;
    char partIndex = 0;
    unsigned int remaining = count;
    unsigned int perPart = (remaining / partitionNum + (remaining % partitionNum != 0)) & 0xff;
    unsigned char* header = buffer;
    for (unsigned int part = 0; part < partitionNum; ++part)
    {
        header[0] = cMessageTypeJoinResponse;
        header[1] = stationCount;
        int headerSize = cJoinResponsePartHeaderSize;
        header[7] = stationCount - static_cast<char>(remaining);
        header[2] = hostSlot;
        header[3] = stationSlot;
        header[5] = partIndex;
        header[6] = static_cast<char>(perPart);
        header[4] = static_cast<char>(partitionNum);
        if (partIndex == 0)
            headerSize = cJoinResponseHeaderSize;
        int partSize = (infoSize + (~infoSize & cStationEntryAlignMask) + 1) * perPart + headerSize;
        std::memcpy(buffers[part], header, partSize);
        remaining = (remaining - perPart) & 0xff;
        consumed += partSize - cJoinResponsePartHeaderSize;
        header = buffer + consumed;
        sizes[part] = partSize;
        if (remaining < perPart)
            perPart = remaining;
        ++partIndex;
    }
    return partitionNum;
}

void MeshProtocol::ParseHelper(const transport::ReceivedMessageAccessor& message)
{
    if (message.m_messageSize == 0)
        return;

    switch (message.m_message[0])
    {
    case cMessageTypeJoinRequest:
        ParseJoinRequest(message);
        break;
    case cMessageTypeJoinResponse:
        ParseJoinResponse(message);
        break;
    case cMessageTypeLeaveRequest:
        ParseLeaveRequest(message);
        break;
    case cMessageTypeLeaveResponse:
        ParseLeaveResponse(message);
        break;
    case cMessageTypeDestroyMesh:
        ParseDestroyMesh(message);
        break;
    case cMessageTypeDestroyResponse:
        ParseDestroyResponse(message);
        break;
    case cMessageTypeUpdateMesh:
        ParseUpdateMesh(message);
        break;
    case cMessageTypeKickoutNotice:
        ParseKickoutNotice(message);
        break;
    case cMessageTypeDummyMessage:
        ParseDummyMessage(message);
        break;
    case cMessageTypeDummyAck:
        ParseDummyAck(message);
        break;
    case cMessageTypeConnectionFailureNotice:
        ParseConnectionFailureNotice(message);
        break;
    case cMessageTypeGreeting:
        ParseGreeting(message);
        break;
    case cMessageTypeMigrationFinish:
        ParseMigrationFinish(message);
        break;
    case cMessageTypeGreetingResponse:
        ParseGreetingResponse(message);
        break;
    case cMessageTypeMigrationStart:
        ParseMigrationStart(message);
        break;
    case cMessageTypeMigrationResponse:
        ParseMigrationResponse(message);
        break;
    case cMessageTypeMultiMigrationStart:
        ParseMultiMigrationStart(message);
        break;
    case cMessageTypeMultiMigrationRankDecision:
        ParseMultiMigrationRankDecision(message);
        break;
    case cMessageTypeConnectionReport:
        ParseConnectionReport(message);
        break;
    case cMessageTypeRelayRouteDirections:
        ParseRelayRouteDirections(message);
        break;
    default:
    {
        const common::StationAddress* address = &message.m_stationAddress;
        address->Trace(cTraceMessageLevel);
        break;
    }
    }
}

Result MeshProtocol::Dispatch()
{
    transport::ReceivedMessageAccessor message;
    const StationIndex localIndex = g_piaMesh->GetLocalStationIndex();
    transport::ProtocolId protocolId(mProtocolId);
    transport::PacketHandler* handler = mStationPacketHandler;
    transport::PacketHandler::Iterator& iterator = handler->GetIterator(protocolId);
    handler->BeginIteration();
    while (!handler->IsEndIteration())
    {
        transport::ProtocolMessageReader* reader = iterator.GetMessageReader();
        message.m_message = reader->m_message + transport::ProtocolMessageReader::cMessageHeaderSize;
        message.m_messageSize = reader->m_messageSize;
        message.m_sourceStationId = reader->m_header.m_source;
        message.m_stationAddress = reader->m_stationAddress;
        message.m_field1c = reader->m_sendingPassId;
        ParseHelper(message);
        handler->NextIteration();
    }

    transport::ProtocolId secondaryProtocolId(static_cast<unsigned char>(GetProtocolType()),
                                              cSecondaryProtocolIdSuffix);
    transport::PacketHandler::Iterator& secondaryIterator = handler->GetIterator(secondaryProtocolId);
    handler->BeginIteration();
    while (!handler->IsEndIteration())
    {
        transport::ProtocolMessageReader* reader = secondaryIterator.GetMessageReader();
        transport::Station* station = transport::StationManager::s_pInstance->GetStationByStationIdWithoutConvert(
            reader->m_header.m_source);
        if (station != 0)
        {
            int windowIndex = station->GetStationIndex();
            if (localIndex < cStationNumMax && windowIndex < cStationNumMax)
            {
                if (localIndex <= windowIndex)
                    --windowIndex;
                transport::ReliableSlidingWindow* window = m_windows + windowIndex;
                if (window->IsInCommunication())
                {
                    Result result = window->AnalyzeProtocolMessage(*reader);
                    if (result == 0)
                    {
                        updateSendWatermark(*window);
                        updateReceiveWatermark(*window);
                    }
                }
            }
        }
        handler->NextIteration();
    }

    for (unsigned short index = 0; index < m_windowNum; ++index)
    {
        transport::ReliableSlidingWindow* window = m_windows + index;
        if (window->IsInCommunication())
        {
            transport::Station* station = transport::StationManager::s_pInstance->GetStationByStationIdWithoutConvert(
                window->m_remoteStationId);
            if (station != 0)
            {
                unsigned int size;
                while (window->PopData(m_buffer, &size, m_bufferSize) == 0)
                {
                    updateReceiveWatermark(*window);
                    message.m_message = m_buffer;
                    message.m_messageSize = size;
                    message.m_sourceStationId = station->m_stationId;
                    const common::StationAddress stationAddress(station->m_stationAddress);
                    message.m_stationAddress = stationAddress;
                    message.m_field1c = 0;
                    ParseHelper(message);
                }
            }
        }
    }

    for (unsigned short index = 0; index < m_windowNum; ++index)
    {
        if (m_windows[index].IsInCommunication())
            m_windows[index].Dispatch(static_cast<transport::StationPacketHandler*>(GetStationPacketHandler()), 0,
                                      cUnlimitedDispatchSize);
    }

    bool updateMeshRequired = false;
    bool hostLost = false;
    const common::Time now = transport::g_piaTransport->m_time;
    transport::StationManager* stationManager = transport::StationManager::s_pInstance;
    transport::Station* localStation = stationManager->m_localStation;
    if (localStation == 0)
        return 0;

    if (g_piaMesh->IsHost() && g_piaMesh->m_unknown24 != 0)
    {
        RelayRouteManageJob* relayRouteManageJob = g_piaMesh->m_unknown24;
        unsigned int refusedBitmap = relayRouteManageJob->m_refusedStationBitmap;
        if (refusedBitmap != 0)
        {
            unsigned int remainingBitmap = 0;
            unsigned int index = 0;
            unsigned int bit = 1;
            for (int count = cStationIndexMax; count != 0; --count)
            {
                if ((refusedBitmap & 1) != 0)
                {
                    Result result = g_piaMesh->m_kickoutManageJob->StartKickout(
                        static_cast<StationIndex>(index),
                        static_cast<KickoutManageJob::KickoutReason>(
                            relayRouteManageJob->m_failureReasons[static_cast<unsigned short>(index)]
                            + cKickoutReasonRelayFailureBase));
                    if (result != 0 && result != transport::cResultAlreadyExists)
                        remainingBitmap |= bit;
                }
                refusedBitmap >>= 1;
                if (refusedBitmap == 0)
                    break;
                ++index;
                bit <<= 1;
            }
            relayRouteManageJob->m_refusedStationBitmap = remainingBitmap;
        }
    }

    for (unsigned int stationOrder = 0; stationOrder < stationManager->m_stations.size; ++stationOrder)
    {
        transport::Station* station = stationManager->m_stations.data[stationOrder];
        if (station == localStation)
            continue;

        bool timedOut = false;
        if (station->m_field20 == cStationConnectedState)
        {
            int timeoutMsec = m_unknown24;
            transport::RelayRouteManager* relayRouteManager = transport::g_piaTransport->m_relayRouteManager;
            if (relayRouteManager != 0 && station->GetStationIndex() < cStationNumMax
                && localStation->GetStationIndex() < cStationNumMax)
            {
                StationIndex relay;
                Result result = relayRouteManager->GetRelayRoute(localStation->GetStationIndex(),
                                                                 station->GetStationIndex(), &relay);
                if (result == 0 && relay != station->GetStationIndex())
                    timeoutMsec += transport::g_piaTransport->GetKeepAliveInterval() * 2;
            }
            const unsigned long long lastReceivedTime = station->m_lastReceiveTime.m_tick;
            const long long timeoutTicks = common::TimeSpan::GetTicksPerMSec().m_tick * timeoutMsec;
            timedOut = lastReceivedTime + timeoutTicks < now.m_tick;
        }

        if (station->m_field68 == 0)
        {
            if (!timedOut && station->m_field20 != cStationReconnectState)
                continue;
        }
        else
        {
            timedOut = true;
        }

        const StationIndex stationIndex = station->GetStationIndex();
        if (g_piaMesh->IsValidStationIndex(stationIndex))
        {
            g_piaMesh->IsHost();
            if (stationIndex == g_piaMesh->GetHostStationIndex() && timedOut)
                hostLost = true;
            g_piaMesh->UnfixDisconnectedId(stationIndex);
            if (g_piaMesh->m_unknown0c->m_stationIndex != stationIndex)
                updateMeshRequired = true;
            else
                g_piaMesh->m_unknown0c->CancellationNotice(stationIndex);
        }

        transport::RelayRouteManager* relayRouteManager = transport::g_piaTransport->m_relayRouteManager;
        if (relayRouteManager != 0 && stationIndex < cStationNumMax && !hostLost)
            relayRouteManager->SearchRefugeRelayRoute(stationIndex);

        station->m_disconnectStationJob->ProcessDisconnection(station);
        transport::g_stationConnectionInfoTable->EraseFromTable(station);
        transport::IdentificationInfoTable::GetInstance()->EraseFromTable(station);
        station->CleanupJobs();
        station->Cleanup();
        stationManager->DestroyStation(station);
        g_piaMesh->m_processUpdateMeshJob->ClearStationIndex(stationIndex);
        transport::g_piaTransport->OutputStreamUpdateEvent();
    }

    if (g_piaMesh->IsHost())
    {
        unsigned int validBitmap = g_piaMesh->m_validStationBitmap;
        unsigned int index = 0;
        for (int count = cStationIndexMax; count != 0; --count)
        {
            if ((validBitmap & 1) != 0 && stationManager->GetStation(static_cast<StationIndex>(index)) == 0)
            {
                g_piaMesh->UnfixDisconnectedId(static_cast<StationIndex>(index));
                transport::RelayRouteManager* relayRouteManager;
                if (g_piaMesh->m_unknown0c->m_stationIndex == static_cast<StationIndex>(index))
                {
                    g_piaMesh->m_unknown0c->CancellationNotice(static_cast<StationIndex>(index));
                    relayRouteManager = transport::g_piaTransport->m_relayRouteManager;
                }
                else
                {
                    relayRouteManager = transport::g_piaTransport->m_relayRouteManager;
                    updateMeshRequired = true;
                }
                if (relayRouteManager != 0)
                    relayRouteManager->SearchRefugeRelayRoute(static_cast<StationIndex>(index));
            }
            validBitmap >>= 1;
            if (validBitmap == 0)
                break;
            ++index;
        }
    }

    if (updateMeshRequired)
    {
        if (!g_piaMesh->IsHost())
        {
            if (hostLost)
            {
                const unsigned int validBitmap = g_piaMesh->m_validStationBitmap;
                for (unsigned int index = 0; index < cStationIndexMax; ++index)
                {
                    if ((validBitmap & (1 << index)) != 0
                        && stationManager->GetStation(static_cast<StationIndex>(index)) == 0)
                    {
                        g_piaMesh->UnfixDisconnectedId(static_cast<StationIndex>(index));
                        transport::g_piaTransport->OutputStreamUpdateEvent();
                    }
                }
                if (!g_piaMesh->IsHostMigrationEnabled())
                {
                    g_piaMesh->CleanupStationsJobs();
                    if (!g_piaMesh->m_joinMeshJob->IsRunning())
                    {
                        g_piaMesh->m_disconnectReason = cDisconnectReason8;
                        g_piaMesh->ProcessingAtMeshEnd(static_cast<Mesh::DisconnectReason>(cDisconnectReason8));
                        g_piaMesh->CleanupStatus();
                    }
                    else
                    {
                        const Result cancelResult = cResultJoinMeshCancelled;
                        g_piaMesh->m_joinMeshJob->Cleanup(cancelResult);
                        g_piaMesh->m_joinMeshJob->Reset(true);
                        g_piaMesh->CleanupStatus();
                    }
                }
                else
                {
                    bool started;
                    if (g_piaMesh->m_hostMigrationMode == cHostMigrationModeMulti)
                        started = g_piaMesh->m_processHostMigrationJob->StartupMulti(false, 0, 0, 0);
                    else
                        started = g_piaMesh->m_processHostMigrationJob->Startup(
                            false, static_cast<StationIndex>(cStationIndexNone));
                    if (started)
                        g_piaMesh->m_processHostMigrationJob->Ready(false);
                }
            }
        }
        else if (!g_piaMesh->IsHostMigrationEnabled() || !g_piaMesh->m_processHostMigrationJob->m_started)
        {
            SendUpdateMesh(true, 1);
        }
        else
        {
            m_unknown38 = 1;
        }
    }
    else if (!g_piaMesh->IsHost())
    {
        if (g_piaMesh->IsHostMigrationEnabled() && g_piaMesh->IsHostMigrationStarted())
        {
            if (!g_piaMesh->m_processHostMigrationJob->m_started)
            {
                transport::Station* hostStation = stationManager->GetStation(g_piaMesh->GetHostStationIndex());
                if (hostStation != 0)
                {
                    if (g_piaMesh->IsValidStationIndex(hostStation->GetStationIndex()))
                        g_piaMesh->UnfixDisconnectedId(hostStation->GetStationIndex());
                    hostStation->m_disconnectStationJob->ProcessDisconnection(hostStation);
                    transport::g_stationConnectionInfoTable->EraseFromTable(hostStation);
                    transport::IdentificationInfoTable::GetInstance()->EraseFromTable(hostStation);
                    hostStation->CleanupJobs();
                    hostStation->Cleanup();
                    stationManager->DestroyStation(hostStation);
                    transport::g_piaTransport->OutputStreamUpdateEvent();
                }
                if (g_piaMesh->m_processHostMigrationJob->Startup(false, static_cast<StationIndex>(cStationIndexNone)))
                    g_piaMesh->m_processHostMigrationJob->Ready(false);
            }
            g_piaMesh->SetHostMigrationStarted(false);
        }
    }
    else
    {
        if (g_piaMesh->GetStationNum() > 1
            && (!g_piaMesh->IsHostMigrationEnabled() || !g_piaMesh->m_processHostMigrationJob->m_started))
        {
            if (!m_broadcastSupported)
            {
                if (m_unknown38)
                {
                    m_unknown38 = 0;
                    SendUpdateMesh(true, 1);
                }
                if (!g_piaMesh->IsProcessingLeaveMesh() && m_windowNum != 0)
                {
                    for (unsigned short index = 0; index < m_windowNum; ++index)
                    {
                        if (m_windows[index].IsInCommunication() && m_nextTimes[index] <= now)
                        {
                            transport::StationConnectionInfo connectionInfo;
                            const unsigned int infoSize = connectionInfo.GetSerializedSize();
                            Result result = m_windows[index].PushData(
                                m_updateMeshBuffer,
                                (infoSize
                                 + (cStationEntryAlignMask + 1
                                    - (connectionInfo.GetSerializedSize() & cStationEntryAlignMask)))
                                        * g_piaMesh->m_unknown54
                                    + cUpdateMeshHeaderSize);
                            if (result == 0)
                                updateSendWatermark(m_windows[index]);
                            m_nextTimes[index] = now + common::TimeSpan::FromMSec(m_unknown28);
                        }
                    }
                }
            }
            else if (!m_unknown38)
            {
                const int intervalMsec = m_unknown28;
                const common::TimeSpan interval(common::TimeSpan::GetTicksPerMSec().m_tick * intervalMsec);
                if (m_unknown30 + interval < now)
                    SendUpdateMesh(false, 1);
            }
            else
            {
                m_unknown38 = 0;
                SendUpdateMesh(true, 1);
            }
        }
        if (g_piaMesh->IsHostMigrationEnabled() && !g_piaMesh->m_processHostMigrationJob->m_started
            && g_piaMesh->m_processHostMigrationJob->IsFatalErrorOccur()
            && g_piaMesh->m_destroyMeshJob->Startup(0, true))
        {
            g_piaMesh->m_destroyMeshJob->Ready(false);
            g_piaMesh->m_disconnectReason = cDisconnectReason9;
        }
    }
    return 0;
}

void MeshProtocol::ParseUpdateMesh(const transport::ReceivedMessageAccessor& message)
{
    transport::StationManager* stationManager = transport::StationManager::s_pInstance;
    if (stationManager->m_localStation == 0)
        return;
    if (g_piaMesh->m_leaveMeshJob->IsRunning())
        return;
    if (g_piaMesh->m_processDestroyMeshJob->m_started)
        return;

    const unsigned char* data = message.m_message;
    const unsigned int size = message.m_messageSize;
    transport::Station* hostStation = stationManager->GetStation(g_piaMesh->GetHostStationIndex());
    if (hostStation == 0 || !(hostStation->m_stationAddress == message.m_stationAddress))
    {
        if (!g_piaMesh->m_processHostMigrationJob->m_started)
        {
            g_piaMesh->SetFatalError(static_cast<Mesh::DisconnectReason>(cDisconnectReason6));
            return;
        }
        g_piaMesh->SetFatalError(static_cast<Mesh::DisconnectReason>(cDisconnectReason9));
        return;
    }

    bool stationCountMismatch = false;
    bool unassignedStationFound = false;
    const unsigned char stationCount = data[1];
    const unsigned int version = common::deserializeU32(data + 4);
    if (!g_piaMesh->m_unknowncc && version <= g_piaMesh->m_unknownc8)
        return;

    if (g_piaMesh->m_unknown88 == 0 || g_piaMesh->m_unknown88 == message.m_sourceStationId)
    {
        if (version < g_piaMesh->m_unknown80)
            return;
        if (version <= g_piaMesh->m_unknown80)
        {
            if (stationCount == stationManager->GetValidStationNum())
            {
                bool found = false;
                for (unsigned int index = 0; index < stationManager->m_stations.size; ++index)
                {
                    transport::Station* station = stationManager->m_stations.data[index];
                    if (station->GetStationIndex() == cStationIndexNone)
                    {
                        station->Trace(cTraceStationLevel);
                        found = true;
                    }
                }
                if (!found)
                    return;
                unassignedStationFound = true;
            }
            else
            {
                stationCountMismatch = true;
            }
        }
    }
    else
    {
        unassignedStationFound = true;
    }

    g_piaMesh->m_unknownc8 = 0;
    g_piaMesh->m_unknowncc = 1;
    ProcessUpdateMeshJob* job = g_piaMesh->m_processUpdateMeshJob;
    int result;
    if (!job->m_started)
    {
        result = job->Startup(data, size, stationCountMismatch);
        if (result == ProcessUpdateMeshJob::cStartupResultStarted)
            job->Ready(false);
    }
    else
    {
        if (version <= job->m_updateVersion && !unassignedStationFound)
            return;
        result = job->UpdateStationDataList(data, size, unassignedStationFound);
    }
    if (result != ProcessUpdateMeshJob::cStartupResultStarted)
        return;
    g_piaMesh->m_unknown80 = version;
    g_piaMesh->m_unknown88 = message.m_sourceStationId;
}

void MeshProtocol::ParseConnectionFailureNotice(const transport::ReceivedMessageAccessor& message)
{
    if (message.m_sourceStationId == 0)
        return;
    if (message.m_messageSize != cConnectionFailureNoticeSize)
        return;

    const unsigned char* data = message.m_message;
    const unsigned char senderIndex = data[1];
    const unsigned char reporterIndex = data[2];
    const unsigned char failedIndex = data[3];
    const unsigned char reason = data[4];
    const unsigned int version = common::deserializeU32(data + 5);
    if (g_piaMesh->IsHost())
    {
        if (senderIndex == failedIndex)
        {
            if (version >= g_piaMesh->m_unknown80 && reporterIndex != g_piaMesh->GetLocalStationIndex())
            {
                SendConnectionFailureNotice(static_cast<StationIndex>(reporterIndex),
                                            static_cast<StationIndex>(reporterIndex),
                                            static_cast<StationIndex>(failedIndex), reason, version);
            }
        }
    }
    else if (senderIndex == g_piaMesh->GetHostStationIndex() && reporterIndex == g_piaMesh->GetLocalStationIndex())
    {
        ProcessUpdateMeshJob* job = g_piaMesh->m_processUpdateMeshJob;
        if (job->m_started && job->m_updateVersion == version)
            job->SetConnectionFailureNotice(static_cast<StationIndex>(failedIndex), reason);
    }
}

void MeshProtocol::ParseMigrationResponse(const transport::ReceivedMessageAccessor& message)
{
    if (g_piaMesh->IsHostMigrationEnabled() && transport::StationManager::s_pInstance->m_localStation != 0)
    {
        StationIndex stationIndex = static_cast<StationIndex>(message.m_message[1]);
        LeaveWithHostMigrationJob* job = g_piaMesh->m_leaveWithHostMigrationJob;
        if (job->m_migrationStarted)
            job->ReceiveMigrationResponse(stationIndex);
    }
}

void MeshProtocol::ParseConnectionReport(const transport::ReceivedMessageAccessor& message)
{
    if (transport::StationManager::s_pInstance->m_localStation == 0 || !g_piaMesh->IsHost())
        return;

    RelayRouteManageJob* job = g_piaMesh->m_unknown24;
    if (job == 0)
        return;

    const unsigned char* data = message.m_message;
    const unsigned int size = message.m_messageSize;
    const unsigned char hostIndex = data[1];
    if (hostIndex != g_piaMesh->GetHostStationIndex())
        return;

    const unsigned char reporterIndex = data[2];
    if (data[0xd] != 0 || data[0xe] != 0 || data[0xf] != 0)
        return;

    const unsigned int generation = common::deserializeU32(data + 4);
    const unsigned int reportSerial = common::deserializeU32(data + 8);
    if (!job->IsRunning())
    {
        if (!job->Startup(static_cast<StationIndex>(reporterIndex), generation, reportSerial))
            return;
        job->Ready(false);
    }
    job->UpdateConnectionReport(static_cast<StationIndex>(reporterIndex), data, size);
}

bool MeshProtocol::SendConnectionReport(StationIndex stationIndex, unsigned int versionHigh, unsigned int versionLow,
                                        unsigned int* stationBitmap)
{
    transport::StationManager* stationManager = transport::StationManager::s_pInstance;
    transport::Station* localStation = stationManager->m_localStation;
    if (localStation == 0)
        return false;
    const int localIndex = static_cast<int>(g_piaMesh->GetLocalStationIndex());
    if (localIndex > cStationIndexMax || localIndex != localStation->GetStationIndex())
        return false;
    if (stationManager->GetStation(stationIndex) == 0)
        return false;

    const unsigned char hostIndex = g_piaMesh->GetHostStationIndex();
    int windowIndex = static_cast<int>(stationIndex);
    if (localIndex <= windowIndex)
        --windowIndex;
    transport::ReliableSlidingWindow* window = m_windows + windowIndex;
    if (!window->IsInCommunication())
        return false;

    unsigned char* buffer = m_buffer;
    buffer[2] = static_cast<unsigned char>(localIndex);
    buffer[1] = hostIndex;
    buffer[0] = cMessageTypeConnectionReport;
    common::serializeU32(buffer + 4, versionHigh);
    common::serializeU32(buffer + 8, versionLow);
    buffer[0xe] = 0;
    buffer[0xc] = 0;
    buffer[0xf] = 0;
    buffer[0xd] = 0;

    unsigned char directCount = 0;
    unsigned int unmeasuredBitmap = 0;
    bool measuring = false;
    int offset = cConnectionReportHeaderSize;
    for (int index = 0; index < static_cast<int>(g_piaMesh->m_unknown54); ++index)
    {
        unsigned int rtt = 0;
        unsigned char value;
        transport::Station* station = stationManager->GetStation(static_cast<StationIndex>(index));
        if (station == 0 || station == localStation || station->m_field20 != cStationConnectedState)
        {
            value = 0xff;
            if (rtt >> 2 < 0xff)
                value = rtt >> 2;
        }
        else
        {
            rtt = transport::NetworkRttManager::sInstance->GetAverage(station->m_stationAddress) & 0xffff;
            if (rtt == 0)
            {
                if (!station->IsConnectionRouteRelay())
                {
                    measuring = true;
                    unmeasuredBitmap |= 1 << index;
                }
                value = 0xff;
                if (rtt >> 2 < 0xff)
                    value = rtt >> 2;
            }
            else if (!station->IsConnectionRouteDirect())
            {
                value = 0;
            }
            else
            {
                ++directCount;
                if (static_cast<int>(rtt) >> 2 != 0)
                {
                    value = 0xff;
                    if (rtt >> 2 < 0xff)
                        value = rtt >> 2;
                }
                else
                {
                    value = 1;
                }
            }
        }
        buffer[offset] = value;
        ++offset;
    }
    buffer[3] = directCount;
    if (measuring)
    {
        *stationBitmap = unmeasuredBitmap;
        return false;
    }

    Result result = window->PushData(m_buffer, offset);
    if (result == 0)
        updateSendWatermark(*window);
    return result == 0;
}

}
}
}
