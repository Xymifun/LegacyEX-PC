#include "nn/pia/transport/BandwidthCheckProtocol.hpp"

#include "nn/pia/Assert.hpp"
#include "nn/pia/PayloadSizeManager.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/common/Math.hpp"
#include "nn/pia/common/Memory.hpp"
#include "nn/pia/common/SessionBeginMonitoringContent.hpp"
#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/transport/PacketHandler.hpp"
#include "nn/pia/transport/ProtocolEvent.hpp"
#include "nn/pia/transport/ProtocolMessageReader.hpp"
#include "nn/pia/transport/ProtocolMessageWriter.hpp"
#include "nn/pia/transport/Station.hpp"
#include "nn/pia/transport/StationManager.hpp"
#include "nn/pia/transport/StationPacketHandler.hpp"

namespace nn { namespace pia { namespace transport {

namespace
{
const unsigned int cBulkBufferSize = 0x584;
const int cPacketOverheadSize = 0x58;
const int cIpUdpHeaderSize = 0x1c;
const int cHeaderSize = 16;
const unsigned long long cTraceCategoryBandwidth = 0x8000;

inline common::Time currentTime()
{
    common::Time now;
    now.SetNow();
    return now;
}

inline int elapsedMilliseconds(const common::Time& start)
{
    common::Time now;
    now.SetNow();
    return static_cast<int>(static_cast<long long>(now.m_tick - start.m_tick) /
                            common::TimeSpan::GetTicksPerMSec().m_tick);
}

inline void writeU32(unsigned char* buffer, unsigned int value)
{
    buffer[0] = static_cast<unsigned char>(value >> 24);
    buffer[1] = static_cast<unsigned char>((value >> 16) & 0xff);
    buffer[2] = static_cast<unsigned char>((value >> 8) & 0xff);
    buffer[3] = static_cast<unsigned char>(value);
}

inline unsigned int readU32(const unsigned char* buffer)
{
    return (static_cast<unsigned int>(buffer[0]) << 24) |
           (static_cast<unsigned int>(buffer[1]) << 16) |
           (static_cast<unsigned int>(buffer[2]) << 8) |
           static_cast<unsigned int>(buffer[3]);
}
}

Result BandwidthCheckProtocol::Header::Serialize(unsigned char* buffer,
                                                 unsigned int* size) const
{
    if (buffer == 0)
        return cResultInvalidArgument;
    if (size == 0)
        return cResultInvalidArgument;
    unsigned char* cursor = buffer;
    writeU32(cursor, m_type);
    cursor += 4;
    writeU32(cursor, m_sequence);
    cursor += 4;
    writeU32(cursor, m_offset);
    cursor += 4;
    writeU32(cursor, m_flags);
    cursor += 4;
    *size = static_cast<unsigned int>(cursor - buffer);
    return 0;
}

Result BandwidthCheckProtocol::Header::Deserialize(const unsigned char* buffer)
{
    if (buffer == 0)
        return cResultInvalidArgument;
    const unsigned char* cursor = buffer;
    m_type = readU32(cursor);
    cursor += 4;
    m_sequence = readU32(cursor);
    cursor += 4;
    m_offset = readU32(cursor);
    cursor += 4;
    m_flags = readU32(cursor);
    return 0;
}

unsigned int BandwidthCheckProtocol::Header::GetSerializedSize() const
{
    return 16;
}

BandwidthCheckProtocol::Header::~Header()
{
}

BandwidthCheckProtocol::BandwidthCheckProtocol()
    : Protocol(),
      m_initialized(false), m_stationNum(0), m_activated(false), m_isServer(false),
      m_bulkBuffer(0), m_activeStationBitmap(0), m_bandwidthCheckedBitmap(0),
      m_completionTimeout(0), m_measurementCount(0), m_senderStartTime(), m_bulkStartTime(),
      m_receiverStartTime(), m_bulkArrivalTime(), m_responseSentTime(), m_stationAddress(),
      m_ackSentTime()
{
}

BandwidthCheckProtocol::~BandwidthCheckProtocol()
{
    PIA_ASSERT(!m_initialized);
}

void BandwidthCheckProtocol::initSender()
{
    m_senderState = SenderState_None;
    m_senderStartTime.m_tick = 0;
    m_ackReceived = false;
    m_bulkStartTime.m_tick = 0;
    m_uplinkBitRate = -1;
    m_senderStation = pia::cStationIndexInvalid;
    m_bulkPacketSequence = 0;
    m_measurementResult = -1;
    m_cancelRequested = false;
}

void BandwidthCheckProtocol::initReceiver()
{
    m_receiverState = ReceiverState_None;
    m_receiverStartTime.m_tick = 0;
    m_bulkArrivalTime.m_tick = 0;
    m_responseSentTime.m_tick = 0;
    m_receivedBytes = 0;
    m_receiverStation = pia::cStationIndexInvalid;
    m_stationAddress.Clear();
    m_ackSentTime.m_tick = 0;
    m_receiverActive = false;
    m_firstBulkPacketSequence = -1;
    m_receivedBulkPacketCount = 0;
    m_lastBulkPacketSequence = -1;
}

Result BandwidthCheckProtocol::Initialize()
{
    if (m_initialized)
        return cResultAlreadyInitialized;
    m_bulkBuffer = common::NewArray<unsigned char>(cBulkBufferSize);
    if (m_bulkBuffer == 0)
        return cResultOutOfMemory;
    m_stationNum = g_piaTransport->m_stationNum;
    m_activeStationBitmap = 0;
    m_bandwidthCheckedBitmap = 0;
    initSender();
    initReceiver();
    m_completionTimeout = 0;
    m_measurementTime = -1;
    m_isServer = false;
    m_initialized = true;
    m_bulkPacketSize = 0;
    m_measurementCount = 0;
    m_activated = false;
    return 0;
}

void BandwidthCheckProtocol::Finalize()
{
    if (!m_initialized)
        return;
    m_initialized = false;
    m_activated = false;
    common::DeleteArray(m_bulkBuffer);
    m_bulkBuffer = 0;
}

Result BandwidthCheckProtocol::Startup()
{
    m_activeStationBitmap = 0;
    m_bandwidthCheckedBitmap = 0;
    return 0;
}

void BandwidthCheckProtocol::Cleanup()
{
    m_activeStationBitmap = 0;
    m_bandwidthCheckedBitmap = 0;
}

int BandwidthCheckProtocol::GetMaxIpPacketSize() const
{
    return static_cast<int>(common::g_payloadSizeManager->m_mtuSize) + cIpUdpHeaderSize;
}

int BandwidthCheckProtocol::GetMinIpPacketSize() const
{
    return static_cast<int>(common::g_payloadSizeManager->m_signatureSize) + cPacketOverheadSize +
           cHeaderSize;
}

unsigned int BandwidthCheckProtocol::IsSetupParamValid(int measurementTime,
                                                       unsigned int ipPacketSize,
                                                       int measurementCount) const
{
    if (measurementTime < 1 || measurementTime > 1000000)
        return false;
    if ((ipPacketSize & 3u) != 0)
        return false;
    if (static_cast<unsigned int>(GetMaxIpPacketSize()) < ipPacketSize)
        return false;
    if (ipPacketSize < static_cast<unsigned int>(GetMinIpPacketSize()))
        return false;
    if (measurementCount <= 0)
        return false;
    return true;
}

Result BandwidthCheckProtocol::Activate(int measurementTime, unsigned int ipPacketSize,
                                        bool isServer, int measurementCount)
{
    if (!IsSetupParamValid(measurementTime, ipPacketSize, measurementCount))
        return cResultInvalidArgument;
    if (!m_initialized)
        return cResultNotInitialized;
    m_measurementTime = measurementTime;
    common::g_sessionBeginMonitoringContent.m_metric135 = static_cast<unsigned int>(measurementTime);
    m_bulkPacketSize = ipPacketSize;
    common::g_sessionBeginMonitoringContent.m_metric133 = static_cast<unsigned short>(ipPacketSize);
    m_activated = true;
    m_measurementCount = measurementCount;
    m_completionTimeout = static_cast<unsigned int>(measurementCount + 0xfa);
    m_isServer = isServer;
    return 0;
}

void BandwidthCheckProtocol::Inactivate()
{
    m_activated = false;
    m_isServer = false;
    m_measurementTime = -1;
    m_bulkPacketSize = 0;
    m_completionTimeout = 0;
    m_measurementCount = 0;
    initSender();
    initReceiver();
}

}}}

namespace
{
    bool isUnavailableStationIndex(nn::pia::StationIndex stationIndex)
    {
        return stationIndex > 0x1f;
    }
}

namespace nn { namespace pia { namespace transport {

bool BandwidthCheckProtocol::IsReady() const
{
    if (m_initialized && m_activated &&
        (m_senderState == SenderState_None ||
         m_senderState == SenderState_Completed ||
         m_senderState == SenderState_Canceled))
        return true;
    return false;
}

Result BandwidthCheckProtocol::StartSend(StationIndex stationIndex)
{
    if (isUnavailableStationIndex(stationIndex))
        return cResultInvalidArgument;
    if (!m_initialized)
        return cResultNotInitialized;
    if (!IsReady())
        return cResultInvalidState;
    initSender();
    m_senderStation = stationIndex;
    return 0;
}

void BandwidthCheckProtocol::CancelSend()
{
    if (static_cast<unsigned int>(m_senderState) - SenderState_WaitingForAck <=
        SenderState_WaitingForResponse - SenderState_WaitingForAck)
        m_cancelRequested = true;
}

bool BandwidthCheckProtocol::IsCompleted() const
{
    if (m_initialized &&
        (m_senderState == SenderState_Completed ||
         m_senderState == SenderState_Canceled))
        return true;
    return false;
}

Result BandwidthCheckProtocol::GetUplinkBitRate(int* result) const
{
    if (result == 0)
        return cResultInvalidArgument;
    if (!m_initialized)
        return cResultNotInitialized;
    if (!IsCompleted())
        return cResultInvalidState;
    if (m_senderState == SenderState_Completed)
        *result = m_uplinkBitRate;
    else
    {
        if (m_senderState != SenderState_Canceled)
            Trace(cTraceCategoryBandwidth);
        *result = -1;
    }
    return 0;
}

void BandwidthCheckProtocol::SetBandwidthChecked(StationIndex stationIndex)
{
    m_bandwidthCheckedBitmap |= 1 << stationIndex;
}

int BandwidthCheckProtocol::getBulkPacketSize() const
{
    return static_cast<int>(m_bulkPacketSize);
}

bool BandwidthCheckProtocol::IsBandwidthChecked(StationIndex stationIndex) const
{
    if (stationIndex > 31)
        return false;
    return (m_bandwidthCheckedBitmap & (1 << stationIndex)) != 0;
}

bool BandwidthCheckProtocol::isStartSendCalled() const
{
    return m_senderStation != 0xfd;
}

void BandwidthCheckProtocol::processCancel()
{
    m_senderState = SenderState_Canceled;
    m_cancelRequested = false;
}

bool BandwidthCheckProtocol::isBulkDataArrived() const
{
    return m_firstBulkPacketSequence != -1;
}

bool BandwidthCheckProtocol::isExist(StationIndex stationIndex) const
{
    return StationManager::s_pInstance->GetStation(stationIndex) != 0;
}

Result BandwidthCheckProtocol::send(StationIndex stationIndex,
                                    const unsigned char* payload,
                                    unsigned int payloadSize)
{
    StationPacketHandler* handler = static_cast<StationPacketHandler*>(GetStationPacketHandler());
    if (!isExist(stationIndex))
        return 0x2c0a;
    ProtocolMessageWriter* writer = handler->AssignByStationIndex(
        GetProtocolId(), stationIndex, payloadSize, false, false, true);
    if (writer == 0)
        return 0;
    writer->SetPayload(payload);
    return handler->Commit();
}

Result BandwidthCheckProtocol::sendHeaderOnlyPacket(StationIndex stationIndex,
                                                    unsigned int type,
                                                    int sequence, int offset,
                                                    int flags)
{
    Header header;
    header.m_type = type;
    header.m_sequence = static_cast<unsigned int>(sequence);
    header.m_offset = static_cast<unsigned int>(offset);
    header.m_flags = static_cast<unsigned int>(flags);
    unsigned char bytes[16];
    unsigned int size = 0;
    header.Serialize(bytes, &size);
    return send(stationIndex, bytes, sizeof(bytes));
}

Result BandwidthCheckProtocol::sendSynPacket()
{
    return sendHeaderOnlyPacket(m_senderStation, 3, -1, -1, -1);
}

int BandwidthCheckProtocol::getPayloadSize(unsigned int ipPacketSize) const
{
    const int headerSize =
        static_cast<int>(common::g_payloadSizeManager->m_signatureSize) + cPacketOverheadSize;
    return static_cast<int>(ipPacketSize) - headerSize;
}

Result BandwidthCheckProtocol::sendBulkPacket()
{
    Header header;
    header.m_type = 1;
    header.m_sequence = 0xffffffffu;
    header.m_offset = static_cast<unsigned int>(m_bulkPacketSequence);
    header.m_flags = 0xffffffffu;
    unsigned int serializedSize = 0;
    header.Serialize(m_bulkBuffer, &serializedSize);
    PacketHandler* handler = mStationPacketHandler;
    unsigned int payloadSize = getPayloadSize(m_bulkPacketSize);
    if (handler->GetPayloadSizeLimit() < payloadSize)
    {
        Trace(cTraceCategoryBandwidth);
        payloadSize = handler->GetPayloadSizeLimit();
    }
    const Result result = send(m_senderStation, m_bulkBuffer, payloadSize);
    if (result == 0)
    {
        ++m_bulkPacketSequence;
        return 0;
    }
    return result;
}

Result BandwidthCheckProtocol::sendAckPacket()
{
    return sendHeaderOnlyPacket(m_receiverStation, 4, -1, -1, -1);
}

Result BandwidthCheckProtocol::sendResponse()
{
    const float bits = static_cast<float>(static_cast<int>(m_receivedBytes)) * 8.0f;
    const int rate = static_cast<int>(bits * 1000.0f / static_cast<float>(static_cast<int>(m_measurementCount)));
    return sendHeaderOnlyPacket(m_receiverStation, 2, rate, -1, m_receivedBulkPacketCount);
}

void BandwidthCheckProtocol::processSend()
{
    if (m_senderState == SenderState_WaitingForAck)
    {
        common::Time now;
        now.SetNow();
        if (static_cast<int>(static_cast<long long>(now.m_tick - m_senderStartTime.m_tick) /
                         common::TimeSpan::GetTicksPerMSec().m_tick) > 150)
        {
            const Result result = sendSynPacket();
            if (result != 0 && result == 0x2c0a)
                m_cancelRequested = true;
            common::Time sentTime;
            sentTime.SetNow();
            m_senderStartTime = sentTime;
        }
    }
    else if (m_senderState == SenderState_Measuring)
    {
        common::Time now;
        now.SetNow();
        const int elapsed = static_cast<int>(static_cast<long long>(now.m_tick - m_bulkStartTime.m_tick) /
                         common::TimeSpan::GetTicksPerMSec().m_tick);
        const int sentBits = m_bulkPacketSequence * getBulkPacketSize() * 8;
        for (int packet = 0; packet < 4; ++packet)
        {
            if (sentBits < static_cast<int>(m_measurementTime * 130.0f / 100.0f * elapsed / 1000.0f))
            {
                const Result result = sendBulkPacket();
                if (result != 0 && result == 0x2c0a)
                    m_cancelRequested = true;
            }
        }
    }
    if (m_receiverState == ReceiverState_WaitingForBulk)
    {
        common::Time now;
        now.SetNow();
        if (static_cast<int>(static_cast<long long>(now.m_tick - m_ackSentTime.m_tick) /
                         common::TimeSpan::GetTicksPerMSec().m_tick) > 100)
        {
            sendAckPacket();
            common::Time sentTime;
            sentTime.SetNow();
            m_ackSentTime = sentTime;
        }
    }
    else if (m_receiverState == ReceiverState_SendingResponse)
    {
        common::Time now;
        now.SetNow();
        if (static_cast<int>(static_cast<long long>(now.m_tick - m_responseSentTime.m_tick) /
                         common::TimeSpan::GetTicksPerMSec().m_tick) > 100)
        {
            sendResponse();
            common::Time sentTime;
            sentTime.SetNow();
            m_responseSentTime = sentTime;
        }
    }
}

int BandwidthCheckProtocol::getIpPacketSize(unsigned int payloadSize) const
{
    const int signatureSize = static_cast<int>(common::g_payloadSizeManager->m_signatureSize);
    return static_cast<int>(payloadSize) + signatureSize + cPacketOverheadSize;
}

int BandwidthCheckProtocol::getSendPacketNumInMeasuringTime() const
{
    return static_cast<int>(1.3f * (m_measurementTime * 0.125f / m_bulkPacketSize) * (m_measurementCount / 1000.0f)) + 1;
}

void BandwidthCheckProtocol::senderStateTransition()
{
    switch (m_senderState)
    {
    case SenderState_None:
        if (isStartSendCalled())
            m_senderState = SenderState_WaitingForAck;
        break;
    case SenderState_WaitingForAck:
        if (m_cancelRequested)
            processCancel();
        else if (m_ackReceived)
        {
            m_senderState = SenderState_Measuring;
            m_bulkStartTime = currentTime();
        }
        break;
    case SenderState_Measuring:
        if (m_cancelRequested)
            processCancel();
        else if (m_uplinkBitRate != -1)
            m_senderState = SenderState_Completed;
        else if (elapsedMilliseconds(m_bulkStartTime) > m_completionTimeout)
            m_senderState = SenderState_WaitingForResponse;
        break;
    case SenderState_WaitingForResponse:
        if (m_cancelRequested)
            processCancel();
        else if (m_uplinkBitRate != -1)
            m_senderState = SenderState_Completed;
        else if (elapsedMilliseconds(m_bulkStartTime) > m_completionTimeout + 2000)
            m_senderState = SenderState_Canceled;
        break;
    default:
        break;
    }
}

void BandwidthCheckProtocol::receiverStateTransition()
{
    switch (m_receiverState)
    {
    case ReceiverState_None:
        if (m_receiverActive)
        {
            common::Time now;
            now.SetNow();
            m_receiverState = ReceiverState_WaitingForBulk;
            m_receiverStartTime = now;
        }
        break;
    case ReceiverState_WaitingForBulk:
        if (isBulkDataArrived())
        {
            common::Time now;
            now.SetNow();
            m_receiverState = ReceiverState_Measuring;
            m_bulkArrivalTime = now;
        }
        else
        {
            common::Time now;
            now.SetNow();
            if (static_cast<int>(static_cast<long long>(now.m_tick - m_receiverStartTime.m_tick) /
                                 common::TimeSpan::GetTicksPerMSec().m_tick) > 2000)
                initReceiver();
        }
        break;
    case ReceiverState_Measuring:
        if (elapsedMilliseconds(m_bulkArrivalTime) > m_measurementCount)
            m_receiverState = ReceiverState_SendingResponse;
        break;
    case ReceiverState_SendingResponse:
        if (elapsedMilliseconds(m_bulkArrivalTime) > m_measurementCount + 1000)
        {
            if (m_uplinkBitRate == -1 && !isStartSendCalled() && !m_isServer)
                StartSend(m_receiverStation);
            SetBandwidthChecked(m_receiverStation);
            initReceiver();
        }
        break;
    }
}

void BandwidthCheckProtocol::processReceive()
{
    PacketHandler* handler = mStationPacketHandler;
    bool isTransitioned = false;
    PacketHandler::Iterator& iterator = handler->GetIterator(GetProtocolId());
    while (!iterator.IsEndIteration())
    {
        ProtocolMessageReader* reader = iterator.GetMessageReader();
        const StationIndex stationIndex =
            g_piaTransport->ConvertToStationIndexUnsafe(reader->m_header.m_source);
        if ((m_activeStationBitmap & (1 << stationIndex)) == 0)
        {
            iterator.NextIteration();
            continue;
        }
        Header header;
        header.Deserialize(reader->m_message + ProtocolMessageReader::cMessageHeaderSize);
        switch (header.m_type)
        {
        case 3:
            if (m_receiverState == ReceiverState_None && !IsBandwidthChecked(stationIndex))
            {
                m_receiverStation = stationIndex;
                m_stationAddress = reader->m_stationAddress;
                m_receiverActive = true;
                Trace(cTraceCategoryBandwidth);
            }
            break;
        case 4:
            if (m_senderState == SenderState_WaitingForAck)
            {
                if (m_senderStation == stationIndex)
                {
                    m_ackReceived = true;
                    Trace(cTraceCategoryBandwidth);
                }
                else
                    Trace(cTraceCategoryBandwidth);
            }
            break;
        case 1:
            if (m_receiverState == ReceiverState_WaitingForBulk)
            {
                if (stationIndex == m_receiverStation &&
                    m_stationAddress == reader->m_stationAddress)
                {
                    m_firstBulkPacketSequence = static_cast<int>(header.m_offset);
                    m_lastBulkPacketSequence = static_cast<int>(header.m_offset);
                    Trace(cTraceCategoryBandwidth);
                }
            }
            else if (m_receiverState == ReceiverState_Measuring)
            {
                if (stationIndex == m_receiverStation &&
                    m_stationAddress == reader->m_stationAddress)
                {
                    m_receivedBytes += getIpPacketSize(reader->m_messageSize);
                    ++m_receivedBulkPacketCount;
                }
                else
                    reader->m_stationAddress.Trace(cTraceCategoryBandwidth);
            }
            break;
        case 2:
            if (m_senderState == SenderState_Measuring ||
                m_senderState == SenderState_WaitingForResponse)
            {
                const int lostPackets = static_cast<int>(header.m_flags) * 10000;
                m_uplinkBitRate = static_cast<int>(header.m_sequence);
                common::g_sessionBeginMonitoringContent.m_metric136 = header.m_sequence;
                const int packetCount = getSendPacketNumInMeasuringTime();
                m_measurementResult = common::maximum(0, 10000 - lostPackets / packetCount);
                common::g_sessionBeginMonitoringContent.m_metric134 =
                    static_cast<unsigned short>(m_measurementResult);
            }
            break;
        default:
            break;
        }
        senderStateTransition();
        receiverStateTransition();
        iterator.NextIteration();
        isTransitioned = true;
    }
    if (!isTransitioned)
    {
        senderStateTransition();
        receiverStateTransition();
    }
}

Result BandwidthCheckProtocol::Dispatch()
{
    if (!m_initialized)
        return cResultNotInitialized;
    if (!m_activated)
        return 0;
    processSend();
    processReceive();
    return 0;
}

Result BandwidthCheckProtocol::UpdateProtocolEvent(const ProtocolEvent& event)
{
    const Station* station = StationManager::s_pInstance->GetStation(event.m_stationAddress);
    const StationIndex stationIndex = station->GetStationIndex();
    if (stationIndex < 0 || stationIndex >= m_stationNum)
        return cResultInvalidArgument;
    switch (event.m_type)
    {
    case 0:
        m_activeStationBitmap |= 1 << stationIndex;
        break;
    case 1:
    {
        const unsigned int bit = 1 << stationIndex;
        m_activeStationBitmap &= ~bit;
        m_bandwidthCheckedBitmap &= ~bit;
        if (stationIndex == m_receiverStation)
            initReceiver();
        if (stationIndex == m_senderStation && !IsCompleted())
            m_senderState = SenderState_Canceled;
        break;
    }
    default:
        return cResultInvalidArgument;
    }
    return 0;
}

void BandwidthCheckProtocol::Trace(unsigned long long) const
{
}

unsigned short BandwidthCheckProtocol::GetProtocolType() const
{
    return cProtocolType;
}

}}}
