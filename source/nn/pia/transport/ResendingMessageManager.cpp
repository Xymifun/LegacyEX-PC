#include "nn/pia/transport/ResendingMessageManager.hpp"

#include <new>

#include <coreinit/memory.h>

#include "nn/pia/Transport.hpp"
#include "nn/pia/common/Memory.hpp"
#include "nn/pia/common/Time.hpp"
#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/common/StationAddress.hpp"
#include "nn/pia/transport/StationPacketHandler.hpp"
#include "platform/wiiu/pead.hpp"

namespace nn
{
namespace pia
{
namespace transport
{

ResendingMessageManager* g_piaResendingMessageManager = 0;

namespace
{
unsigned char* g_resendingMessageManagerStorage = 0;
const unsigned long long cResendInterval = static_cast<unsigned long long>(common::TimeSpan::GetTicksPerMSec().m_tick)
                                           * 500;
}

ResendingMessageManager::ResendingMessageManager()
{
    m_nextAckId = 0;
    m_nextSendTimes = 0;
    m_timeouts = 0;
    m_ackIds = 0;
    m_messages = 0;
    m_messageSizes = 0;
    m_stationIndices = 0;
    m_stationAddresses = 0;
    m_protocolIds = 0;
    m_isRelays = 0;
    m_entryNum = 0;
}

Result ResendingMessageManager::CreateInstance()
{
    if (g_piaResendingMessageManager != 0)
        return cResultAlreadyExists;

    g_resendingMessageManagerStorage = common::NewArray<unsigned char>(sizeof(ResendingMessageManager), 8);
    if (g_resendingMessageManagerStorage == 0)
        return cResultOutOfMemory;

    g_piaResendingMessageManager = new (g_resendingMessageManagerStorage) ResendingMessageManager();
    return 0;
}

void ResendingMessageManager::Finalize()
{
    if (m_nextSendTimes != 0)
    {
        common::DeleteArray(m_nextSendTimes);
        common::DeleteArray(m_timeouts);
        common::DeleteArray(m_ackIds);
        for (unsigned int index = 0; index < m_entryNum; ++index)
            common::DeleteArray(m_messages[index]);
        common::DeleteArray(m_messages);
        common::DeleteArray(m_messageSizes);
        common::DeleteArray(m_stationIndices);
        common::DeleteArray(m_stationAddresses);
        common::DeleteArray(m_protocolIds);
        common::DeleteArray(m_isRelays);

        m_nextSendTimes = 0;
        m_timeouts = 0;
        m_ackIds = 0;
        m_messages = 0;
        m_messageSizes = 0;
        m_stationIndices = 0;
        m_stationAddresses = 0;
        m_protocolIds = 0;
        m_isRelays = 0;
    }
}

ResendingMessageManager::~ResendingMessageManager()
{
}

void ResendingMessageManager::DestroyInstance()
{
    if (g_piaResendingMessageManager != 0)
    {
        g_piaResendingMessageManager->Finalize();
        g_piaResendingMessageManager->~ResendingMessageManager();
        g_piaResendingMessageManager = 0;
    }
    if (g_resendingMessageManagerStorage != 0)
    {
        common::DeleteArray(g_resendingMessageManagerStorage);
        g_resendingMessageManagerStorage = 0;
    }
}

Result ResendingMessageManager::Initialize(unsigned int entryNum)
{
    m_entryNum = entryNum;
    m_nextSendTimes = common::NewArray<long long>(m_entryNum);
    m_timeouts = common::NewArray<long long>(m_entryNum);
    m_ackIds = common::NewArray<unsigned int>(m_entryNum);
    m_messages = common::NewArray<unsigned char*>(m_entryNum);
    for (unsigned int index = 0; index < m_entryNum; ++index)
        m_messages[index] = common::NewArray<unsigned char>(cWorkBufferSize, 4);
    m_messageSizes = common::NewArray<unsigned int>(m_entryNum);
    m_stationIndices = common::NewArray<StationIndex>(m_entryNum);
    m_stationAddresses = common::NewArray<common::StationAddress>(m_entryNum);
    m_protocolIds = common::NewArray<ProtocolId>(m_entryNum);
    m_isRelays = common::NewArray<bool>(m_entryNum);
    OSBlockSet(m_ackIds, 0, sizeof(m_ackIds));
    return 0;
}

Result ResendingMessageManager::Startup(PacketHandler* packetHandler, bool isRelay)
{
    if (packetHandler == 0)
        return cResultInvalidArgument;

    m_packetHandler = packetHandler;
    m_isRelay = isRelay;
    return 0;
}

void ResendingMessageManager::Cleanup()
{
    m_packetHandler = 0;
    OSBlockSet(m_ackIds, 0, sizeof(m_ackIds));
}

Result ResendingMessageManager::Dispatch()
{
    common::Time now;
    now.SetNow();
    const long long currentTime = static_cast<long long>(now.m_tick);
    if (m_packetHandler == 0)
        return 0;

    for (unsigned int index = 0; index < m_entryNum; ++index)
    {
        if (m_ackIds[index] == 0)
            continue;
        if (m_timeouts[index] != 0 && currentTime >= m_timeouts[index])
        {
            m_ackIds[index] = 0;
            m_timeouts[index] = 0;
            m_nextSendTimes[index] = 0;
            continue;
        }
        if (currentTime < m_nextSendTimes[index])
            continue;

        ProtocolMessageWriter* writer;
        if (m_stationIndices[index] < 32)
        {
            writer = static_cast<StationPacketHandler*>(m_packetHandler)
                         ->AssignByStationIndex(m_protocolIds[index], m_stationIndices[index], m_messageSizes[index],
                                                false, false, m_isRelays[index]);
        }
        else
        {
            writer = m_packetHandler->AssignByStationAddress(m_protocolIds[index], m_stationAddresses[index],
                                                             m_messageSizes[index], false, false);
        }
        if (writer != 0)
        {
            writer->SetPayload(m_messages[index]);
            m_packetHandler->Commit();
            m_nextSendTimes[index] = currentTime + cResendInterval;
        }
    }
    return 0;
}

Result ResendingMessageManager::SetSendMessageCommon(unsigned int* ackId, const unsigned char* message,
                                                     unsigned int messageSize, ProtocolId protocolId, bool isRelay,
                                                     long long timeout, unsigned int* index)
{
    const unsigned int payloadSizeLimit = static_cast<unsigned int>(m_packetHandler->GetPayloadSizeLimit());
    if (messageSize > payloadSizeLimit - 4U)
        return cResultInvalidArgument;

    common::Time now;
    now.SetNow();
    if (timeout != 0 && timeout <= static_cast<long long>(now.m_tick))
        return cResultInvalidArgument;

    unsigned int selected;
    for (selected = 0; selected < m_entryNum; ++selected)
    {
        if (m_ackIds[selected] == 0)
            break;
    }
    if (selected >= m_entryNum)
        return cResultWindowOverflow;

    if (m_nextAckId == 0)
    {
        m_nextAckId = static_cast<unsigned int>(now.m_tick);
        if (m_nextAckId == 0)
            m_nextAckId = 1;
    }

    OSBlockMove(m_messages[selected], message, messageSize, FALSE);
    common::serializeU32(m_messages[selected] + messageSize, m_nextAckId);
    m_messageSizes[selected] = messageSize + 4;
    m_protocolIds[selected] = protocolId;
    m_isRelays[selected] = isRelay;
    m_nextSendTimes[selected] = now.m_tick;
    m_timeouts[selected] = timeout;
    m_ackIds[selected] = m_nextAckId;

    if (++m_nextAckId == 0)
        m_nextAckId = 1;

    *ackId = m_ackIds[selected];
    *index = selected;
    return 0;
}

Result ResendingMessageManager::SetSendMessageByStationAddress(unsigned int* ackId, const unsigned char* message,
                                                               unsigned int messageSize,
                                                               const common::StationAddress& stationAddress,
                                                               ProtocolId protocolId, long long timeout)
{
    unsigned int index = 0;
    Result result = SetSendMessageCommon(ackId, message, messageSize, protocolId, true, timeout, &index);
    if (result != 0)
        return result;

    m_stationAddresses[index] = stationAddress;
    m_stationIndices[index] = ::nn::pia::cStationIndexInvalid;
    return 0;
}

Result ResendingMessageManager::SetSendMessageByStationIndex(unsigned int* ackId, const unsigned char* message,
                                                             unsigned int messageSize, StationIndex stationIndex,
                                                             ProtocolId protocolId, bool isRelay, long long timeout)
{
    if (m_isRelay)
        return cResultInvalidState;

    unsigned int index = 0;
    Result result = SetSendMessageCommon(ackId, message, messageSize, protocolId, isRelay, timeout, &index);
    if (result != 0)
        return result;

    m_stationAddresses[index].Clear();
    m_stationIndices[index] = stationIndex;
    return 0;
}

bool ResendingMessageManager::CheckNowResending(unsigned int ackId) const
{
    if (ackId == 0)
        return false;

    for (unsigned int i = 0; i < m_entryNum; ++i)
    {
        if (m_ackIds[i] == ackId)
            return true;
    }
    return false;
}

unsigned int ResendingMessageManager::ExtractAckIdFromMessage(const unsigned char* message,
                                                              unsigned int messageSize) const
{
    unsigned int ackId = 0;
    if (messageSize >= 4)
        ackId = common::deserializeU32(message + messageSize - 4);
    return ackId;
}

bool ResendingMessageManager::StopResending(unsigned int ackId)
{
    if (ackId == 0)
        return false;

    bool stopped = false;
    for (unsigned int index = 0; index < m_entryNum; ++index)
    {
        if (m_ackIds[index] == ackId)
        {
            m_ackIds[index] = 0;
            m_timeouts[index] = 0;
            m_nextSendTimes[index] = 0;
            stopped = true;
            break;
        }
    }
    return stopped;
}

void ResendingMessageManager::Trace(unsigned long long) const
{
}

}
}
}
