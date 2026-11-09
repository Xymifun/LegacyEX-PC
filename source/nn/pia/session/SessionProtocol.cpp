#include "nn/pia/session/SessionProtocol.hpp"

#include <new>

#include "nn/pia/HeapManager.hpp"
#include "nn/pia/StationIdTable.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/session/JointSessionJob.hpp"
#include "nn/pia/session/Session.hpp"
#include "nn/pia/session/StationIdStatusTable.hpp"
#include "nn/pia/transport/PacketHandler.hpp"
#include "nn/pia/transport/ProtocolEvent.hpp"
#include "nn/pia/transport/ProtocolMessageReader.hpp"
#include "nn/pia/transport/ReliableSlidingWindow.hpp"
#include "nn/pia/transport/Station.hpp"
#include "nn/pia/transport/StationManager.hpp"
#include "nn/pia/transport/StationPacketHandler.hpp"
#include "nn/pia/transport/StationProtocol.hpp"
#include "platform/wiiu/pead.hpp"

namespace nn
{
namespace pia
{
namespace session
{

namespace
{
enum SessionProtocolResult
{
    cResultInvalidArgument = 0x10c07,
    cResultInvalidState = 0x10c08,
    cResultNotInitialized = 0x10c0c,
    cResultSendBufferFull = 0x4c0d,
    cResultNoInvitationTarget = 0x2c0a,
    cResultInvitationSendFailed = 0x8c56,
    cResultSessionProcessFailed = 0x6c59
};

enum
{
    cStationIndexNone = 0xfd,
    cStationNumMax = transport::StationManager::cStationNumMax,
    cStationIndexMax = cStationNumMax - 1,
    cStationStateConnected = 5,
    cTraceStationLevel = 0x4000000,
    cWindowSendCapacity = 2,
    cWindowReceiveCapacity = 2,
    cStationIdSize = 8,
    cMessageTypeOffset = 0,
    cProcessTypeOffset = 1,
    cAnswerResultOffset = 2,
    cAnswerStationIdOffset = 3,
    cAnswerHeaderSize = 3,
    cInvitationStationIdOffset = 2,
    cInvitationStationIdNumOffset = 10,
    cInvitationStationIdListOffset = 11,
    cInvitationHeaderSize = 3,
    cInvitationStationIdMax = 0x20,
    cPreparedResultOffset = 2,
    cPreparedSessionIdOffset = 3,
    cPreparedStationIdOffset = 7,
    cPreparedHeaderSize = 7,
    cCompletionFailureStationIdOffset = 2,
    cCompletionFailureHeaderSize = 2,
    cCompletionMigrateResultOffset = 2,
    cCompletionMigrateStationIdOffset = 3,
    cCompletionMigrateHeaderSize = 3,
    cNextSessionIdSessionIdOffset = 2,
    cNextSessionIdNextSessionIdOffset = 6,
    cNextSessionIdStationIdOffset = 10,
    cNextSessionIdSessionIndexLength = 1,
    cNextSessionIdOwnerLength = 8,
    cNextSessionIdReservedLength = 1,
    cNextSessionIdApplicationDataSizeLength = 1,
    cNextSessionIdParseOwnerOffset = 0x13,
    cNextSessionIdParseApplicationDataSizeOffset = 0x1c,
    cNextSessionIdParseApplicationDataOffset = 0x1d,
    cNextSessionIdParseSessionIndexOffset = 0x12
};

const unsigned int cUnlimitedDispatchSize = 0xffffffffu;
}

SessionProtocol::SessionProtocol()
    : transport::Protocol(),
      m_stationIndex(cStationIndexNone),
      m_stationId(0),
      m_windowNum(0),
      m_windows(0),
      m_dispatchWindowIndex(0)
{
}

SessionProtocol::~SessionProtocol()
{
}

void SessionProtocol::Finalize()
{
    if (m_windows != 0)
    {
        transport::ReliableSlidingWindow* windows = m_windows;
        unsigned int count = pead::getArraySize(windows) / sizeof(transport::ReliableSlidingWindow);
        for (unsigned int index = 0; index < count; ++index)
            windows[index].~ReliableSlidingWindow();
        pead::freeToSeadHeap(windows);
        m_windows = 0;
    }
    m_windowNum = 0;
}

Result SessionProtocol::Initialize(unsigned short stationNum)
{
    unsigned short count = stationNum - 1;
    m_windowNum = count;
    void* buffer = pead::allocateFromSeadHeap(count * sizeof(transport::ReliableSlidingWindow),
                                              common::HeapManager::GetHeap());
    transport::ReliableSlidingWindow* windows;
    if (buffer == 0)
    {
        windows = 0;
    }
    else
    {
        windows = static_cast<transport::ReliableSlidingWindow*>(buffer);
        transport::ReliableSlidingWindow* window = windows;
        for (int remaining = count; remaining != 0; --remaining)
        {
            new (window) transport::ReliableSlidingWindow();
            ++window;
        }
    }
    m_windows = windows;

    for (unsigned short index = 0; index < m_windowNum; ++index)
    {
        Result result = m_windows[index].Initialize(cWindowSendCapacity, cWindowReceiveCapacity);
        if (result != 0)
        {
            Finalize();
            return result;
        }
    }
    return 0;
}

Result SessionProtocol::StartupWithStationAddress(const common::StationAddress& address)
{
    if (m_windows == 0)
        return cResultNotInitialized;
    if (m_stationIndex != cStationIndexNone)
        return cResultInvalidState;

    transport::Station* station = transport::StationManager::s_pInstance->GetStation(address);
    if (station == 0)
        return cResultInvalidArgument;

    int stationIndex = static_cast<int>(station->m_field10);
    if (stationIndex < 0 || stationIndex > m_windowNum)
        return cResultInvalidArgument;

    m_dispatchWindowIndex = 0;
    m_stationId = station->m_stationId;
    m_stationIndex = stationIndex;
    return 0;
}

void SessionProtocol::CleanupWithStationAddress()
{
    if (m_stationIndex == cStationIndexNone)
        return;

    m_stationIndex = cStationIndexNone;
    m_stationId = 0;
    for (unsigned int index = 0; index < m_windowNum; ++index)
    {
        if (m_windows[index].IsInCommunication())
            m_windows[index].Cleanup();
    }
}

unsigned long long SessionProtocol::DeserializeStationId(const unsigned char* buffer)
{
    return common::deserializeU64(buffer);
}

unsigned int SessionProtocol::SerializeStationId(unsigned long long stationId, unsigned char* buffer,
                                                 unsigned int bufferSize)
{
    if (bufferSize < cStationIdSize)
        return 0;
    common::serializeU64(buffer, stationId);
    return cStationIdSize;
}

Result SessionProtocol::SendPreparedForMigrateSession(unsigned char processType, unsigned long long stationId,
                                                      unsigned int sessionId, bool value)
{
    transport::StationIdTable::Entry entry;
    Result result;
    result = transport::g_piaTransport->m_stationIdTable->FindByStationIndex(&entry,
                                                                             static_cast<StationIndex>(m_stationIndex));
    if (result != 0)
        return result;
    unsigned long long localStationId = entry.stationId;
    result = transport::g_piaTransport->m_stationIdTable->FindByStationId(&entry, stationId);
    if (result != 0)
        return result;

    int windowIndex = static_cast<int>(entry.stationIndex);
    if (windowIndex >= m_stationIndex)
        --windowIndex;
    transport::ReliableSlidingWindow* window = m_windows + windowIndex;
    if (!window->IsInCommunication())
        return cResultInvalidState;

    m_messageBuffer[cProcessTypeOffset] = processType;
    m_messageBuffer[cMessageTypeOffset] = cMessageTypePreparedForMigrateSession;
    m_messageBuffer[cPreparedResultOffset] = value;
    common::serializeU32(m_messageBuffer + cPreparedSessionIdOffset, sessionId);
    unsigned int size = SerializeStationId(localStationId, m_messageBuffer + cPreparedStationIdOffset,
                                           sizeof(m_messageBuffer) - cPreparedStationIdOffset);
    if (sessionId == 0 || size == 0)
        return cResultInvalidState;
    return window->PushData(m_messageBuffer, size + cPreparedHeaderSize);
}

void SessionProtocol::ParseNextSessionId(const transport::ReceivedMessageAccessor& message)
{
    unsigned long long sourceStationId = message.m_sourceStationId;
    if (sourceStationId == 0)
        return;

    const unsigned char* data = message.m_message;
    unsigned char messageType = data[cMessageTypeOffset];
    Session::ProcessType processType = static_cast<Session::ProcessType>(data[cProcessTypeOffset]);
    unsigned int sessionId = common::deserializeU32(data + cNextSessionIdSessionIdOffset);
    unsigned int nextSessionId = common::deserializeU32(data + cNextSessionIdNextSessionIdOffset);
    unsigned long long stationId = DeserializeStationId(data + cNextSessionIdStationIdOffset);
    unsigned int sessionIndex = data[cNextSessionIdParseSessionIndexOffset];
    unsigned long long matchmakeOwnerUniqueId = 0;
    unsigned char applicationDataSize = 0;
    const unsigned char* applicationData = 0;
    if (message.m_messageSize > cNextSessionIdParseSessionIndexOffset)
    {
        matchmakeOwnerUniqueId = common::deserializeU64(data + cNextSessionIdParseOwnerOffset);
        applicationDataSize = data[cNextSessionIdParseApplicationDataSizeOffset];
        applicationData = data + cNextSessionIdParseApplicationDataOffset;
    }

    transport::StationIdTable::Entry entry;
    Result result = transport::g_piaTransport->m_stationIdTable->FindByStationId(&entry, sourceStationId);
    if (result != 0 || sessionIndex == 0)
        return;

    if (sessionId != g_piaSession->m_sessionId[g_piaSession->m_currentSessionIndex])
        return;
    JointSessionJob* job = g_piaSession->m_jointSessionJob;
    if (job == 0)
        return;

    if (job->m_processType != 0)
    {
        if (job->m_field60 == messageType)
        {
            g_piaSession->m_fieldc2[g_piaSession->m_currentSessionIndex == 0 ? 1 : 0] = static_cast<unsigned char>(
                sessionIndex);
            job->ReceiveNextSessionId(processType, nextSessionId, matchmakeOwnerUniqueId, applicationData,
                                      applicationDataSize, stationId);
        }
    }
    else
    {
        SendPreparedForMigrateSession(processType, stationId, nextSessionId, false);
    }
}

void SessionProtocol::ParsePreparedForMigrateSession(const transport::ReceivedMessageAccessor& message)
{
    unsigned long long sourceStationId = message.m_sourceStationId;
    if (sourceStationId == 0)
        return;

    const unsigned char* data = message.m_message;
    unsigned char processType = data[cProcessTypeOffset];
    bool prepared = data[cPreparedResultOffset] != 0;
    unsigned int sessionId = common::deserializeU32(data + cPreparedSessionIdOffset);
    unsigned long long stationId = DeserializeStationId(data + cPreparedStationIdOffset);

    transport::StationIdTable::Entry entry;
    Result result = transport::g_piaTransport->m_stationIdTable->FindByStationId(&entry, sourceStationId);
    if (result != 0)
        return;

    unsigned int expectedSessionId;
    if (processType == Session::cProcessTypeDestroyJointSession
        || processType == Session::cProcessTypeLeaveJointSession)
        expectedSessionId = g_piaSession->m_sessionId[g_piaSession->m_currentSessionIndex];
    else
        expectedSessionId = g_piaSession->m_sessionId[g_piaSession->m_currentSessionIndex == 0 ? 1 : 0];
    if (sessionId != expectedSessionId)
        return;
    g_piaSession->m_stationIdStatusTable->SetPrepareStatus(stationId, prepared);
}

Result SessionProtocol::SendAnswerToInvitationCore(MessageType messageType, unsigned char processType,
                                                   unsigned long long stationId, bool result)
{
    transport::StationIdTable::Entry entry;
    Result findResult;
    findResult = transport::g_piaTransport->m_stationIdTable->FindByStationIndex(
        &entry, static_cast<StationIndex>(m_stationIndex));
    if (findResult != 0)
        return findResult;
    unsigned long long localStationId = entry.stationId;
    findResult = transport::g_piaTransport->m_stationIdTable->FindByStationId(&entry, stationId);
    if (findResult != 0)
        return findResult;
    if (entry.stationIndex == cStationIndexNone)
        return cResultInvalidState;

    int windowIndex = static_cast<int>(entry.stationIndex);
    if (windowIndex >= m_stationIndex)
        --windowIndex;
    transport::ReliableSlidingWindow* window = m_windows + windowIndex;
    if (!window->IsInCommunication())
        return cResultInvalidState;

    unsigned char* message = m_messageBuffer;
    message[cMessageTypeOffset] = messageType;
    message[cProcessTypeOffset] = processType;
    message[cAnswerResultOffset] = result;
    unsigned int size = SerializeStationId(localStationId, message + cAnswerStationIdOffset,
                                           sizeof(m_messageBuffer) - cAnswerStationIdOffset);
    const unsigned int messageSize = size + cAnswerHeaderSize;
    if (size == 0)
        return cResultInvalidState;
    return window->PushData(message, messageSize);
}

Result SessionProtocol::SendAnswerToInvitation(unsigned char processType, unsigned long long stationId, bool result)
{
    return SendAnswerToInvitationCore(cMessageTypeAnswerToInvitation, processType, stationId, result);
}

void SessionProtocol::ParseInvitationAsCompanion(const transport::ReceivedMessageAccessor& message)
{
    unsigned long long sourceStationId = message.m_sourceStationId;
    if (sourceStationId == 0)
        return;

    const unsigned char* data = message.m_message;
    MessageType messageType = static_cast<MessageType>(data[cMessageTypeOffset]);
    Session::ProcessType processType = static_cast<Session::ProcessType>(data[cProcessTypeOffset]);
    unsigned long long hostStationId = DeserializeStationId(data + cInvitationStationIdOffset);
    unsigned short stationIdNum = data[cInvitationStationIdNumOffset];
    unsigned int offset = cInvitationStationIdListOffset;
    JointSessionJob* job = g_piaSession->m_jointSessionJob;
    if (job == 0)
        return;
    if (job->m_failurePending)
        return;

    unsigned long long stationIds[cInvitationStationIdMax];
    if (job->m_processType != 0)
    {
        if (job->m_processType == processType)
        {
            if (processType == Session::cProcessTypeDestroyJointSession && job->m_leaveStationId == 0)
            {
                if (stationIdNum <= cInvitationStationIdMax)
                {
                    for (unsigned int index = 0; index < stationIdNum; ++index)
                    {
                        stationIds[index] = DeserializeStationId(data + offset);
                        offset += cStationIdSize;
                    }
                    Result result = job->ReceiveLeaveInvitation(processType, stationIds, stationIdNum, hostStationId);
                    if (result == 0)
                        return;
                }
            }
            else if (job->m_leaveStationId == hostStationId)
            {
                SendAnswerToInvitation(processType, hostStationId, true);
                return;
            }
        }
    }
    else if (hostStationId == sourceStationId && stationIdNum <= cInvitationStationIdMax)
    {
        for (unsigned int index = 0; index < stationIdNum; ++index)
        {
            stationIds[index] = DeserializeStationId(data + offset);
            offset += cStationIdSize;
        }
        Result result = job->StartupPassiveProcess(processType, messageType, stationIds, stationIdNum, hostStationId);
        if (result == 0)
        {
            job->Ready(false);
            return;
        }
    }

    if (job->m_processType != 0)
    {
        job->m_failurePending = 1;
        job->m_pendingFailureResult = cResultSessionProcessFailed;
        return;
    }
    if (g_piaSession->IsProcessingJoin())
        g_piaSession->CancelJoinProcessByFatalError();
    else
        g_piaSession->ChangeStatusToDisconnectedSession();
}

Result SessionProtocol::SendAnswerToDestroyInvitation(unsigned char processType, unsigned long long stationId,
                                                      bool result)
{
    return SendAnswerToInvitationCore(cMessageTypeAnswerToDestroyInvitation, processType, stationId, result);
}

void SessionProtocol::ParseDestroyInvitation(const transport::ReceivedMessageAccessor& message)
{
    unsigned long long sourceStationId = message.m_sourceStationId;
    if (sourceStationId == 0)
        return;

    const unsigned char* data = message.m_message;
    MessageType messageType = static_cast<MessageType>(data[cMessageTypeOffset]);
    Session::ProcessType processType = static_cast<Session::ProcessType>(data[cProcessTypeOffset]);
    unsigned long long hostStationId = DeserializeStationId(data + cInvitationStationIdOffset);
    unsigned int offset = cInvitationStationIdListOffset;
    unsigned short stationIdNum = data[cInvitationStationIdNumOffset];
    if (processType != Session::cProcessTypeDestroyJointSession)
        return;
    JointSessionJob* job = g_piaSession->m_jointSessionJob;
    if (job == 0 || job->m_failurePending)
        return;

    if (job->m_processType != 0)
    {
        if (job->m_processType == Session::cProcessTypeDestroyJointSession
            || job->m_processType == Session::cProcessTypeLeaveJointSession)
        {
            if (g_piaSession->GetJointSessionHostStationId() != hostStationId)
                return;
            SendAnswerToDestroyInvitation(processType, hostStationId, true);
            return;
        }
    }
    else if (hostStationId == sourceStationId && stationIdNum <= cInvitationStationIdMax)
    {
        unsigned long long stationIds[cInvitationStationIdMax];
        for (unsigned int index = 0; index < stationIdNum; ++index)
        {
            stationIds[index] = DeserializeStationId(data + offset);
            offset += cStationIdSize;
        }
        Result result = job->StartupPassiveProcess(processType, messageType, stationIds, stationIdNum, hostStationId);
        if (result == 0)
        {
            job->Ready(false);
            return;
        }
    }

    if (job->m_processType != 0)
    {
        job->m_failurePending = 1;
        job->m_pendingFailureResult = cResultSessionProcessFailed;
        return;
    }
    if (g_piaSession->IsProcessingJoin())
        g_piaSession->CancelJoinProcessByFatalError();
    else
        g_piaSession->ChangeStatusToDisconnectedSession();
}

void SessionProtocol::ParseLeaveMeshInvitation(const transport::ReceivedMessageAccessor& message)
{
    if (message.m_sourceStationId == 0)
        return;

    const unsigned char* data = message.m_message;
    unsigned char messageType = data[cMessageTypeOffset];
    Session::ProcessType processType = static_cast<Session::ProcessType>(data[cProcessTypeOffset]);
    unsigned long long hostStationId = DeserializeStationId(data + cInvitationStationIdOffset);
    unsigned int stationIdNum = data[cInvitationStationIdNumOffset];
    unsigned int offset = cInvitationStationIdListOffset;
    unsigned long long stationIds[cInvitationStationIdMax];
    for (unsigned int index = 0; index < stationIdNum; ++index)
    {
        stationIds[index] = DeserializeStationId(data + offset);
        offset += cStationIdSize;
    }

    JointSessionJob* job = g_piaSession->m_jointSessionJob;
    if (job != 0 && !job->m_failurePending && job->m_processType != 0 && job->m_processType == processType
        && job->m_leaveStationId == hostStationId && job->m_field60 == messageType)
    {
        job->ReceiveLeaveMeshInvitation(processType, hostStationId);
        for (unsigned int index = 0; index < stationIdNum; ++index)
            g_piaSession->m_stationIdStatusTable->SetPrepareStatus(stationIds[index], true);
        SendAnswerToLeaveMeshInvitation(processType, hostStationId, true);
    }
}

void SessionProtocol::ParseAnswerToInvitation(const transport::ReceivedMessageAccessor& message)
{
    unsigned long long sourceStationId = message.m_sourceStationId;
    if (sourceStationId == 0)
        return;

    const unsigned char* data = message.m_message;
    bool answer = data[cAnswerResultOffset] != 0;
    unsigned char messageType = data[cMessageTypeOffset];
    unsigned char processType = data[cProcessTypeOffset];
    unsigned long long stationId = DeserializeStationId(data + cAnswerStationIdOffset);

    transport::StationIdTable::Entry entry;
    Result result = transport::g_piaTransport->m_stationIdTable->FindByStationId(&entry, sourceStationId);
    if (result != 0)
        return;
    JointSessionJob* job = g_piaSession->m_jointSessionJob;
    if (job == 0)
        return;
    bool isProcessing = job->m_processType != 0;
    if (isProcessing && job->m_processType == processType && job->m_field60 == messageType)
        g_piaSession->m_stationIdStatusTable->SetAnswer(stationId, answer);
}

Result SessionProtocol::SendInvitationCore(MessageType messageType, unsigned char processType,
                                           unsigned long long* stationIds, unsigned short stationIdNum,
                                           unsigned long long stationId, unsigned long long* companionStationIds,
                                           unsigned short companionStationIdNum)
{
    m_messageBuffer[cMessageTypeOffset] = messageType;
    m_messageBuffer[cProcessTypeOffset] = processType;
    unsigned int size = SerializeStationId(stationId, m_messageBuffer + cInvitationStationIdOffset,
                                           sizeof(m_messageBuffer) - cInvitationStationIdOffset);
    m_messageBuffer[cInvitationStationIdOffset + size] = static_cast<unsigned char>(stationIdNum);
    unsigned int messageSize = size + cInvitationHeaderSize;
    if (size == 0 || stationIdNum > cInvitationStationIdMax)
        return cResultInvalidState;

    for (unsigned int index = 0; index < stationIdNum; ++index)
    {
        unsigned int written = SerializeStationId(stationIds[index], m_messageBuffer + messageSize,
                                                  sizeof(m_messageBuffer) - messageSize);
        if (written == 0)
            return cResultInvalidState;
        messageSize += written;
    }

    const unsigned long long* targets = companionStationIds;
    unsigned int targetNum = companionStationIdNum;
    if (targets == 0)
    {
        targets = stationIds;
        targetNum = stationIdNum;
    }

    transport::StationIdTable* table = transport::g_piaTransport->m_stationIdTable;
    transport::ReliableSlidingWindow* windows[cStationNumMax] = {0};
    unsigned short windowNum = 0;
    Result result;
    for (unsigned int index = 0; index < targetNum; ++index)
    {
        if (targets[index] == stationId)
            continue;
        transport::StationIdTable::Entry entry;
        result = table->FindByStationId(&entry, targets[index]);
        if (result != 0 || static_cast<int>(entry.stationIndex) > cInvitationStationIdMax)
            continue;
        int windowIndex = static_cast<int>(entry.stationIndex);
        if (windowIndex >= m_stationIndex)
            --windowIndex;
        transport::ReliableSlidingWindow* window = m_windows + windowIndex;
        windows[windowNum] = window;
        if (!window->IsInCommunication())
        {
            result = cResultNotInitialized;
            continue;
        }
        if (!window->CanPushData(messageSize))
        {
            window->Trace(cTraceStationLevel);
            result = cResultSendBufferFull;
        }
        else
        {
            result = 0;
        }
        if (result == cResultSendBufferFull)
            return result;
        g_piaSession->m_stationIdStatusTable->SetSendInvitationStatus(targets[index], true);
        ++windowNum;
    }

    if (windowNum == 0)
        return cResultNoInvitationTarget;

    unsigned short sentNum = 0;
    for (unsigned short index = 0; index < windowNum; ++index)
    {
        result = windows[index]->PushData(m_messageBuffer, messageSize);
        if (result == 0)
            ++sentNum;
    }
    if (sentNum != 0)
        return 0;
    return cResultInvitationSendFailed;
}

Result SessionProtocol::SendCompletionInvitation(unsigned char processType, unsigned long long* stationIds,
                                                 unsigned short stationIdNum, unsigned long long stationId,
                                                 unsigned long long* companionStationIds,
                                                 unsigned short companionStationIdNum)
{
    return SendInvitationCore(cMessageTypeCompletionInvitation, processType, stationIds, stationIdNum, stationId,
                              companionStationIds, companionStationIdNum);
}

Result SessionProtocol::SendCompletionFailureResponse(unsigned char processType, unsigned long long stationId)
{
    transport::StationIdTable::Entry entry;
    Result result;
    result = transport::g_piaTransport->m_stationIdTable->FindByStationIndex(&entry,
                                                                             static_cast<StationIndex>(m_stationIndex));
    if (result != 0)
        return result;
    unsigned long long localStationId = entry.stationId;
    result = transport::g_piaTransport->m_stationIdTable->FindByStationId(&entry, stationId);
    if (result != 0)
        return result;

    int windowIndex = static_cast<int>(entry.stationIndex);
    if (windowIndex >= m_stationIndex)
        --windowIndex;
    transport::ReliableSlidingWindow* window = m_windows + windowIndex;
    if (!window->IsInCommunication())
        return cResultInvalidState;

    m_messageBuffer[cProcessTypeOffset] = processType;
    m_messageBuffer[cMessageTypeOffset] = cMessageTypeCompletionFailureResponse;
    unsigned int size = SerializeStationId(localStationId, m_messageBuffer + cCompletionFailureStationIdOffset,
                                           sizeof(m_messageBuffer) - cCompletionFailureStationIdOffset);
    return window->PushData(m_messageBuffer, size + cCompletionFailureHeaderSize);
}

void SessionProtocol::ParseCompletionForMigrateSession(const transport::ReceivedMessageAccessor& message)
{
    unsigned long long sourceStationId = message.m_sourceStationId;
    if (sourceStationId == 0)
        return;

    const unsigned char* data = message.m_message;
    bool completed = data[cCompletionMigrateResultOffset] != 0;
    Session::ProcessType processType = static_cast<Session::ProcessType>(data[cProcessTypeOffset]);
    unsigned long long stationId = DeserializeStationId(data + cCompletionMigrateStationIdOffset);

    transport::StationIdTable::Entry entry;
    Result result = transport::g_piaTransport->m_stationIdTable->FindByStationId(&entry, sourceStationId);
    if (result != 0)
        return;

    g_piaSession->m_stationIdStatusTable->SetMigrationCompleteStatus(stationId, completed);
    JointSessionJob* job = g_piaSession->m_jointSessionJob;
    if (job == 0 || job->m_failurePending)
        return;

    if (job->m_processType != 0)
    {
        if (job->m_fielda0 != 0)
            SendCompletionInvitation(processType, &stationId, 1, g_piaSession->m_localStationId, 0, 0);
    }
    else if (g_piaSession->GetStatus() == Session::cConnectionStatusDisconnectedNetwork
             || g_piaSession->GetStatus() == Session::cConnectionStatusConnected)
    {
        SendCompletionInvitation(processType, &stationId, 1, g_piaSession->m_localStationId, 0, 0);
    }
    else
    {
        SendCompletionFailureResponse(processType, stationId);
    }
}

void SessionProtocol::ParseCompletionFailureResponse(const transport::ReceivedMessageAccessor& message)
{
    unsigned long long sourceStationId = message.m_sourceStationId;
    if (sourceStationId == 0)
        return;

    DeserializeStationId(message.m_message + cCompletionFailureStationIdOffset);
    transport::StationIdTable::Entry entry;
    Result result = transport::g_piaTransport->m_stationIdTable->FindByStationId(&entry, sourceStationId);
    if (result != 0)
        return;
    JointSessionJob* job = g_piaSession->m_jointSessionJob;
    if (job != 0 && !job->m_failurePending && job->m_processType != 0)
    {
        job->m_failurePending = 1;
        job->m_pendingFailureResult = cResultSessionProcessFailed;
    }
}

Result SessionProtocol::SendAnswerToCompletionInvitation(unsigned char processType, unsigned long long stationId,
                                                         bool result)
{
    return SendAnswerToInvitationCore(cMessageTypeAnswerToCompletionInvitation, processType, stationId, result);
}

void SessionProtocol::ParseCompletionInvitation(const transport::ReceivedMessageAccessor& message)
{
    if (message.m_sourceStationId == 0)
        return;

    unsigned char messageType = message.m_message[cMessageTypeOffset];
    Session::ProcessType processType = static_cast<Session::ProcessType>(message.m_message[cProcessTypeOffset]);
    unsigned long long hostStationId = DeserializeStationId(message.m_message + cInvitationStationIdOffset);
    JointSessionJob* job = g_piaSession->m_jointSessionJob;
    if (job != 0 && !job->m_failurePending)
    {
        if (job->m_processType != 0 && job->m_processType == processType && job->m_leaveStationId == hostStationId
            && job->m_field60 == messageType)
            job->ReceiveCompletionResponse(processType, hostStationId);
        SendAnswerToCompletionInvitation(processType, hostStationId, true);
    }
}

void SessionProtocol::ParseHelper(const transport::ReceivedMessageAccessor& message)
{
    if (message.m_messageSize == 0)
        return;

    unsigned char messageType = message.m_message[cMessageTypeOffset];
    switch (messageType)
    {
    case cMessageTypeNextSessionId:
        ParseNextSessionId(message);
        break;
    case cMessageTypePreparedForMigrateSession:
        ParsePreparedForMigrateSession(message);
        break;
    case cMessageTypeInvitationAsCompanion:
        ParseInvitationAsCompanion(message);
        break;
    case cMessageTypeDestroyInvitation:
        ParseDestroyInvitation(message);
        break;
    case cMessageTypeLeaveMeshInvitation:
        ParseLeaveMeshInvitation(message);
        break;
    case cMessageTypeAnswerToInvitation:
    case cMessageTypeAnswerToDestroyInvitation:
    case cMessageTypeAnswerToLeaveMeshInvitation:
    case cMessageTypeAnswerToCompletionInvitation:
        ParseAnswerToInvitation(message);
        break;
    case cMessageTypeCompletionForMigrateSession:
        ParseCompletionForMigrateSession(message);
        break;
    case cMessageTypeCompletionFailureResponse:
        ParseCompletionFailureResponse(message);
        break;
    case cMessageTypeCompletionInvitation:
        ParseCompletionInvitation(message);
        break;
    default:
        break;
    }
}

Result SessionProtocol::Dispatch()
{
    if (m_stationIndex == cStationIndexNone)
        return 0;

    transport::ReceivedMessageAccessor message;
    transport::StationPacketHandler* handler = static_cast<transport::StationPacketHandler*>(mStationPacketHandler);
    transport::ProtocolId protocolId(static_cast<unsigned char>(GetProtocolType()), 1);
    transport::PacketHandler::Iterator& iterator = handler->GetIterator(protocolId);
    iterator.BeginIteration();
    for (; !iterator.IsEndIteration(); iterator.NextIteration())
    {
        transport::ProtocolMessageReader* reader = iterator.GetMessageReader();
        transport::Station* station = transport::StationManager::s_pInstance->GetStationByStationIdWithoutConvert(
            reader->m_header.m_source);
        if (station == 0 || static_cast<int>(station->m_field10) > cStationIndexMax)
            continue;
        int stationIndex = static_cast<int>(station->m_field10);
        transport::ReliableSlidingWindow* window;
        if (stationIndex < m_stationIndex)
            window = m_windows + stationIndex;
        else if (m_stationIndex < stationIndex)
            window = m_windows + (stationIndex - 1);
        else
            continue;
        if (window->IsInCommunication())
            window->AnalyzeProtocolMessage(*reader);
    }

    for (unsigned short index = 0; index < m_windowNum; ++index)
    {
        transport::ReliableSlidingWindow* window = m_windows + index;
        if (!window->IsInCommunication())
            continue;
        transport::Station* station = transport::StationManager::s_pInstance->GetStationByStationIdWithoutConvert(
            window->m_remoteStationId);
        if (station == 0 || station->m_field20 != cStationStateConnected)
            continue;
        unsigned int size;
        for (;;)
        {
            Result popResult = window->PopData(m_messageBuffer, &size, sizeof(m_messageBuffer));
            if (popResult != 0)
                break;
            message.m_message = m_messageBuffer;
            message.m_messageSize = size;
            message.m_sourceStationId = station->m_stationId;
            common::StationAddress stationAddress(station->m_stationAddress);
            message.m_stationAddress = stationAddress;
            message.m_field1c = 0;
            ParseHelper(message);
        }
    }

    ++m_dispatchWindowIndex;
    if (m_dispatchWindowIndex >= m_windowNum)
        m_dispatchWindowIndex = 0;
    for (unsigned short index = m_dispatchWindowIndex; index < m_windowNum; ++index)
    {
        if (m_windows[index].IsInCommunication())
            m_windows[index].Dispatch(static_cast<transport::StationPacketHandler*>(GetStationPacketHandler()), 0,
                                      cUnlimitedDispatchSize);
    }
    for (unsigned short index = 0; index < m_dispatchWindowIndex; ++index)
    {
        if (m_windows[index].IsInCommunication())
            m_windows[index].Dispatch(static_cast<transport::StationPacketHandler*>(GetStationPacketHandler()), 0,
                                      cUnlimitedDispatchSize);
    }
    return 0;
}

Result SessionProtocol::UpdateProtocolEvent(const transport::ProtocolEvent& event)
{
    transport::Station* station = transport::StationManager::s_pInstance->GetStation(event.m_stationAddress);
    int stationIndex = static_cast<int>(station->m_field10);
    if (m_stationIndex > cStationIndexMax)
        return cResultInvalidState;
    if (stationIndex < 0 || stationIndex >= static_cast<int>(m_windowNum) + 1)
        return cResultInvalidArgument;

    transport::ReliableSlidingWindow* window;
    if (stationIndex > m_stationIndex)
    {
        window = m_windows + (stationIndex - 1);
    }
    else
    {
        if (stationIndex >= m_stationIndex)
            return cResultInvalidArgument;
        window = m_windows + stationIndex;
    }

    switch (event.m_type)
    {
    case 0:
    {
        transport::ProtocolId protocolId(static_cast<unsigned char>(GetProtocolType()), 1);
        Result result = window->Startup(static_cast<transport::StationPacketHandler*>(GetStationPacketHandler()),
                                        protocolId, m_stationId, station->m_stationId);
        return result;
    }
    case 1:
        window->Cleanup();
        return 0;
    default:
        return cResultInvalidArgument;
    }
}

Result SessionProtocol::SendNextSessionId(unsigned char processType, unsigned int sessionId, unsigned int nextSessionId,
                                          unsigned long long matchmakeOwnerUniqueId,
                                          const unsigned char* applicationData, unsigned int applicationDataSize,
                                          unsigned long long* stationIds, unsigned short stationIdNum)
{
    transport::StationIdTable::Entry entry;
    transport::StationIdTable* table = transport::g_piaTransport->m_stationIdTable;
    Result result = table->FindByStationIndex(&entry, static_cast<StationIndex>(m_stationIndex));
    if (result != 0)
        return result;
    unsigned long long localStationId = entry.stationId;

    m_messageBuffer[cProcessTypeOffset] = processType;
    m_messageBuffer[cMessageTypeOffset] = cMessageTypeNextSessionId;
    common::serializeU32(m_messageBuffer + cNextSessionIdSessionIdOffset, sessionId);
    common::serializeU32(m_messageBuffer + cNextSessionIdNextSessionIdOffset, nextSessionId);
    unsigned int size = SerializeStationId(localStationId, m_messageBuffer + cNextSessionIdStationIdOffset,
                                           sizeof(m_messageBuffer) - cNextSessionIdStationIdOffset);

    unsigned int messageSize = cNextSessionIdStationIdOffset + size;
    if (processType >= Session::cProcessTypeCreateJointSession
        && processType <= Session::cProcessTypeJoinRandomJointSession)
        m_messageBuffer[messageSize] = static_cast<unsigned char>(
            g_piaSession->m_fieldc2[g_piaSession->m_currentSessionIndex == 0 ? 1 : 0]);
    else
        m_messageBuffer[messageSize] = static_cast<unsigned char>(
            g_piaSession->m_fieldc2[g_piaSession->m_currentSessionIndex]);
    messageSize += cNextSessionIdSessionIndexLength;
    common::serializeU64(m_messageBuffer + messageSize, matchmakeOwnerUniqueId);
    messageSize += cNextSessionIdOwnerLength + cNextSessionIdReservedLength;
    m_messageBuffer[messageSize] = static_cast<unsigned char>(applicationDataSize);
    messageSize += cNextSessionIdApplicationDataSizeLength;
    for (unsigned int index = 0; index < applicationDataSize; ++index)
    {
        m_messageBuffer[messageSize] = applicationData[index];
        ++messageSize;
    }

    if (sessionId == 0 || nextSessionId == 0 || size == 0)
        return cResultInvalidState;

    transport::ReliableSlidingWindow* windows[cStationNumMax] = {0};
    unsigned short windowNum = 0;
    for (unsigned int index = 0; index < stationIdNum; ++index)
    {
        if (stationIds[index] == localStationId)
            continue;
        result = table->FindByStationId(&entry, stationIds[index]);
        if (result != 0 || static_cast<int>(entry.stationIndex) > cInvitationStationIdMax)
            continue;
        int windowIndex = static_cast<int>(entry.stationIndex);
        if (windowIndex >= m_stationIndex)
            --windowIndex;
        transport::ReliableSlidingWindow* window = m_windows + windowIndex;
        windows[windowNum] = window;
        if (!window->IsInCommunication())
        {
            result = cResultNotInitialized;
            continue;
        }
        if (!window->CanPushData(messageSize))
        {
            window->Trace(cTraceStationLevel);
            result = cResultSendBufferFull;
        }
        else
        {
            result = 0;
        }
        if (result == cResultSendBufferFull)
            return result;
        g_piaSession->m_stationIdStatusTable->SetSendInvitationStatus(stationIds[index], true);
        ++windowNum;
    }

    if (windowNum == 0)
        return cResultNoInvitationTarget;

    unsigned short sentNum = 0;
    for (unsigned short index = 0; index < windowNum; ++index)
    {
        result = windows[index]->PushData(m_messageBuffer, messageSize);
        if (result == 0)
            ++sentNum;
    }
    if (sentNum != 0)
        return 0;
    return cResultInvitationSendFailed;
}

Result SessionProtocol::SendInvitationAsCompanion(unsigned char processType, unsigned long long* stationIds,
                                                  unsigned short stationIdNum, unsigned long long stationId,
                                                  unsigned long long* companionStationIds,
                                                  unsigned short companionStationIdNum)
{
    return SendInvitationCore(cMessageTypeInvitationAsCompanion, processType, stationIds, stationIdNum, stationId,
                              companionStationIds, companionStationIdNum);
}

Result SessionProtocol::SendDestroyInvitation(unsigned char processType, unsigned long long* stationIds,
                                              unsigned short stationIdNum, unsigned long long stationId,
                                              unsigned long long* companionStationIds,
                                              unsigned short companionStationIdNum)
{
    return SendInvitationCore(cMessageTypeDestroyInvitation, processType, stationIds, stationIdNum, stationId,
                              companionStationIds, companionStationIdNum);
}

Result SessionProtocol::SendLeaveMeshInvitation(unsigned char processType, unsigned long long* stationIds,
                                                unsigned short stationIdNum, unsigned long long stationId,
                                                unsigned long long* companionStationIds,
                                                unsigned short companionStationIdNum)
{
    return SendInvitationCore(cMessageTypeLeaveMeshInvitation, processType, stationIds, stationIdNum, stationId,
                              companionStationIds, companionStationIdNum);
}

Result SessionProtocol::SendAnswerToLeaveMeshInvitation(unsigned char processType, unsigned long long stationId,
                                                        bool result)
{
    return SendAnswerToInvitationCore(cMessageTypeAnswerToLeaveMeshInvitation, processType, stationId, result);
}

Result SessionProtocol::SendCompletionForMigrateSession(unsigned char processType, unsigned long long stationId,
                                                        bool result)
{
    transport::StationIdTable::Entry entry;
    Result findResult;
    findResult = transport::g_piaTransport->m_stationIdTable->FindByStationIndex(
        &entry, static_cast<StationIndex>(m_stationIndex));
    if (findResult != 0)
        return findResult;
    unsigned long long localStationId = entry.stationId;
    findResult = transport::g_piaTransport->m_stationIdTable->FindByStationId(&entry, stationId);
    if (findResult != 0)
        return findResult;

    int windowIndex = static_cast<int>(entry.stationIndex);
    if (windowIndex >= m_stationIndex)
        --windowIndex;
    transport::ReliableSlidingWindow* window = m_windows + windowIndex;
    if (!window->IsInCommunication())
        return cResultInvalidState;

    m_messageBuffer[cProcessTypeOffset] = processType;
    m_messageBuffer[cMessageTypeOffset] = cMessageTypeCompletionForMigrateSession;
    m_messageBuffer[cCompletionMigrateResultOffset] = result;
    unsigned int size = SerializeStationId(localStationId, m_messageBuffer + cCompletionMigrateStationIdOffset,
                                           sizeof(m_messageBuffer) - cCompletionMigrateStationIdOffset);
    return window->PushData(m_messageBuffer, size + cCompletionMigrateHeaderSize);
}

Result SessionProtocol::CheckIsAbleToSendReliableMessage(StationIndex stationIndex, unsigned int size)
{
    int windowIndex = stationIndex;
    if (windowIndex >= m_stationIndex)
        --windowIndex;

    transport::ReliableSlidingWindow& window = m_windows[windowIndex];
    if (!window.IsInCommunication())
        return cResultNotInitialized;
    if (!window.CanPushData(size))
    {
        window.Trace(cTraceStationLevel);
        return cResultSendBufferFull;
    }
    return 0;
}

void SessionProtocol::Trace(unsigned long long value) const
{
}

unsigned short SessionProtocol::GetProtocolType() const
{
    return cProtocolType;
}

}
}
}
