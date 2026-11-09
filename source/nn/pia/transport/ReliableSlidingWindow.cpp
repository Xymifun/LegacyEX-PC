#include "nn/pia/transport/ReliableSlidingWindow.hpp"
#include "nn/pia/transport/StationPacketHandler.hpp"
#include "nn/pia/transport/ProtocolMessageReader.hpp"
#include "nn/pia/transport/ProtocolMessageWriter.hpp"
#include "nn/pia/transport/Station.hpp"
#include "nn/pia/transport/StationManager.hpp"
#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/HeapManager.hpp"
#include "platform/wiiu/pead.hpp"

#include <coreinit/memory.h>
#include <coreinit/systeminfo.h>
#include <coreinit/time.h>
#include <new>
#include <string.h>

namespace nn
{
namespace pia
{
namespace transport
{

float ReliableSlidingWindow::s_retransmissionTimeOutRate = 1.25f;
unsigned int ReliableSlidingWindow::s_rttSamplingNum = 8;
int (*ReliableSlidingWindow::s_resendTimeCallback)(int, int) = 0;

namespace
{

enum
{
    cProfiledStationNum = 4,
    cProfiledSizeMin = 102400,
    cResendCountReset = 65535
};
const unsigned int cInitialSequence = 4294965295u;
const unsigned long long cNeverResendTime = 18446744073709551615ULL;
const unsigned long long cHighestReceivedBit = 9223372036854775808ULL;

unsigned int g_profileByteCount[cProfiledStationNum];
unsigned int g_profileDurationMs[cProfiledStationNum];
unsigned short g_maximumResendCount;

}

template <typename T>
ReliableSlidingWindow::Buffer<T>::Buffer() : m_data(0),
                                             m_capacity(0),
                                             m_head(0),
                                             m_sequence(0),
                                             m_count(0)
{
}

template <typename T>
ReliableSlidingWindow::Buffer<T>::~Buffer()
{
    Finalize();
}

template <typename T>
Result ReliableSlidingWindow::Buffer<T>::Initialize(unsigned int capacity)
{
    if (m_data != 0)
        return cResultAlreadyInitialized;
    if (capacity == 0)
        return cResultInvalidArgument;
    T* data = static_cast<T*>(pead::allocateFromSeadHeap(capacity * sizeof(T), common::HeapManager::GetHeap()));
    if (data != 0)
    {
        for (unsigned int index = 0; index < capacity; ++index)
            new (data + index) T();
    }
    m_data = data;
    m_capacity = capacity;
    return 0;
}

template <typename T>
void ReliableSlidingWindow::Buffer<T>::Finalize()
{
    if (m_data != 0)
    {
        pead::getArraySize(m_data);
        pead::freeToSeadHeap(m_data);
        m_data = 0;
    }
    m_capacity = 0;
}

template <typename T>
void ReliableSlidingWindow::Buffer<T>::Clear()
{
    m_head = 0;
    m_sequence = cInitialSequence;
    m_count = 0;
}

template <typename T>
void ReliableSlidingWindow::Buffer<T>::Withdraw(unsigned int count)
{
    m_head += count;
    if (m_head >= m_capacity)
        m_head -= m_capacity;
    m_sequence += count;
    m_count -= count;
}

template <typename T>
T* ReliableSlidingWindow::Buffer<T>::GetData(unsigned int sequence)
{
    const unsigned int offset = sequence - m_sequence;
    if (static_cast<int>(offset) < 0 || offset >= m_count)
        return 0;
    unsigned int index = m_head + offset;
    if (index >= m_capacity)
        index -= m_capacity;
    return m_data + index;
}

template class ReliableSlidingWindow::Buffer<ReliableSlidingWindow::SendData>;
template class ReliableSlidingWindow::Buffer<ReliableSlidingWindow::ReceiveData>;

ReliableSlidingWindow::ReliableSlidingWindow()
    : m_protocolId(ProtocolId::INVALID),
      m_localStationId(0),
      m_remoteStationId(0),
      m_payloadLimit(0),
      m_byteCount(0),
      m_profileStart(0)
{
}

void ReliableSlidingWindow::Finalize()
{
    m_receiveBuffer.Finalize();
    m_sendBuffer.Finalize();
}

ReliableSlidingWindow::~ReliableSlidingWindow()
{
    Finalize();
}

Result ReliableSlidingWindow::Initialize(unsigned int sendCapacity, unsigned int receiveCapacity)
{
    int result = m_sendBuffer.Initialize(sendCapacity);
    if (result != 0)
        return result;
    result = m_receiveBuffer.Initialize(receiveCapacity);
    if (result != 0)
    {
        m_sendBuffer.Finalize();
        return result;
    }
    m_retrySpan = pead::TickSpan::cFrequency * 500 / 1000;
    return 0;
}

Result ReliableSlidingWindow::Startup(StationPacketHandler* handler, unsigned short protocolId,
                                      unsigned long long localStationId, unsigned long long remoteStationId)
{
    if (m_sendBuffer.m_capacity == 0)
        return cResultNotInitialized;
    if (m_protocolId != ProtocolId::INVALID)
        return cResultInvalidState;
    if (protocolId == ProtocolId::INVALID)
        return cResultInvalidArgument;
    if (localStationId == 0)
        return cResultInvalidArgument;
    if (remoteStationId == 0)
        return cResultInvalidArgument;
    if (localStationId == remoteStationId)
        return cResultInvalidArgument;
    m_localStationId = localStationId;
    m_protocolId = protocolId;
    m_remoteStationId = remoteStationId;
    m_payloadLimit = (handler->GetPayloadSizeLimit() & ~7u) - cHeaderSize;
    m_sendBuffer.Clear();
    m_receiveBuffer.Clear();
    m_time = static_cast<unsigned long long>(OSGetSystemTime());
    m_sequence = cInitialSequence;
    m_receivedBits = 0;
    m_field68 = false;
    m_byteCount = 0;
    m_profileStart = 0;
    return 0;
}

void ReliableSlidingWindow::Cleanup()
{
    if (m_protocolId == ProtocolId::INVALID)
        return;
    m_protocolId = ProtocolId::INVALID;
    m_remoteStationId = 0;
    m_localStationId = 0;
    m_payloadLimit = 0;
}

bool ReliableSlidingWindow::IsInCommunication() const
{
    return m_protocolId != ProtocolId::INVALID;
}

ReliableSlidingWindow::ReceiveData* ReliableSlidingWindow::ReceiveBuffer::Assign(unsigned int sequence)
{
    const unsigned int offset = sequence - m_sequence;
    while (m_count <= offset)
    {
        unsigned int index = m_head + m_count;
        if (index >= m_capacity)
            index -= m_capacity;
        m_data[index].m_valid = false;
        ++m_count;
    }
    unsigned int index = m_head + offset;
    if (index >= m_capacity)
        index -= m_capacity;
    return m_data + index;
}

void ReliableSlidingWindow::MessageData::DeserializeHeader(const void* buffer)
{
    m_header.Deserialize(static_cast<const unsigned char*>(buffer));
}

void ReliableSlidingWindow::MessageData::CopyPayload(void* destination, const void* source, unsigned int size)
{
    OSBlockMove(destination, static_cast<const unsigned char*>(source) + cHeaderSize, size, FALSE);
}

Result ReliableSlidingWindow::AnalyzeProtocolMessage(const ProtocolMessageReader& reader)
{
    if (m_protocolId == ProtocolId::INVALID)
        return cResultInvalidState;
    const unsigned int messageSize = reader.m_messageSize;
    if (messageSize < cHeaderSize)
        return cResultDataSizeInvalid;

    const unsigned char* body = reader.m_message + ProtocolMessageReader::cMessageHeaderSize;
    MessageData message;
    message.DeserializeHeader(body);

    if (m_localStationId != 0)
    {
        const int acknowledged = static_cast<int>(message.m_header.m_field10 - m_sendBuffer.m_sequence);
        if (acknowledged >= 0)
        {
            if (m_sendBuffer.m_count < static_cast<unsigned int>(acknowledged))
                return cResultDataSizeInvalid;
            if (acknowledged > 0)
                m_sendBuffer.Withdraw(acknowledged);
            unsigned long long received = message.m_header.m_field18;
            if (m_sendBuffer.m_count != 0)
            {
                for (unsigned int index = 1; index < m_sendBuffer.m_count; ++index)
                {
                    if ((received & 1) != 0)
                    {
                        unsigned int slot = m_sendBuffer.m_head + index;
                        if (slot >= m_sendBuffer.m_capacity)
                            slot -= m_sendBuffer.m_capacity;
                        m_sendBuffer.m_data[slot].m_sendTime = cNeverResendTime;
                    }
                    received >>= 1;
                    if (received == 0)
                        break;
                }
            }
        }
    }

    if (m_byteCount != 0 && m_sendBuffer.m_count == 0)
    {
        const StationIndex stationIndex = g_piaTransport->ConvertToStationIndexUnsafe(m_remoteStationId);
        if (stationIndex < cProfiledStationNum)
        {
            g_profileByteCount[stationIndex] = m_byteCount;
            g_profileDurationMs[stationIndex] = static_cast<unsigned int>(
                static_cast<long long>(g_piaTransport->m_time.m_tick - m_profileStart)
                / common::TimeSpan::GetTicksPerMSec().m_tick);
        }
        m_byteCount = 0;
        m_profileStart = 0;
    }

    if ((message.m_header.m_flags & 1) != 0)
    {
        const unsigned int payloadSize = message.m_header.m_size;
        if (messageSize != payloadSize + cHeaderSize)
            return cResultDataSizeInvalid;
        if (payloadSize > cPayloadSizeMax)
            return cResultDataSizeInvalid;
        const int offset = static_cast<int>(message.m_header.m_sequenceId - m_receiveBuffer.m_sequence);
        if (offset < 0)
        {
            m_field68 = true;
            return 0;
        }
        if (static_cast<unsigned int>(offset) >= m_receiveBuffer.m_capacity)
            return cResultWindowOverflow;

        ReceiveData* fragment = m_receiveBuffer.Assign(message.m_header.m_sequenceId);
        m_field68 = true;
        if (fragment->m_valid)
            return 0;
        MessageData::CopyPayload(fragment->m_payload, body, payloadSize);
        fragment->m_size = message.m_header.m_size;
        fragment->m_valid = true;
        fragment->m_end = ((message.m_header.m_flags >> 1) & 1) != 0;

        const unsigned int distance = message.m_header.m_sequenceId - m_sequence;
        if (distance == 0)
        {
            bool continuous;
            do
            {
                continuous = (m_receivedBits & 1) != 0;
                ++m_sequence;
                m_receivedBits >>= 1;
                ReceiveData* ahead = m_receiveBuffer.GetData(m_sequence + 64);
                if (ahead != 0 && ahead->m_valid)
                    m_receivedBits |= cHighestReceivedBit;
            } while (continuous);
        }
        else if (distance - 1 < 64)
            m_receivedBits |= 1ULL << (distance - 1);
    }
    return 0;
}

void ReliableSlidingWindow::MessageData::SerializeHeader()
{
    unsigned int size;
    m_header.Serialize(m_data, &size);
}

Result ReliableSlidingWindow::Dispatch(StationPacketHandler* handler, unsigned int* size, unsigned int limit)
{
    if (m_protocolId == ProtocolId::INVALID)
        return cResultInvalidState;
    m_time = static_cast<unsigned long long>(OSGetSystemTime());

    int rtt = -1;
    long long resendTime = m_retrySpan;
    Station* station = StationManager::s_pInstance->GetStationByStationIdWithoutConvert(m_remoteStationId);
    if (station != 0)
    {
        rtt = station->GetRtt(s_rttSamplingNum);
        if (rtt < 1)
            resendTime = m_time + m_retrySpan;
        else
        {
            const int timeout = static_cast<int>(static_cast<float>(rtt) * s_retransmissionTimeOutRate);
            resendTime = static_cast<long long>(timeout) * pead::TickSpan::cFrequency / 1000 + m_time;
        }
    }

    const unsigned int sequence = m_sequence;
    const unsigned long long receivedBits = m_receivedBits;
    unsigned int total = 0;
    for (unsigned int index = 0; index < m_sendBuffer.m_count; ++index)
    {
        unsigned int slot = m_sendBuffer.m_head + index;
        if (slot >= m_sendBuffer.m_capacity)
            slot -= m_sendBuffer.m_capacity;
        SendData* fragment = m_sendBuffer.m_data + slot;
        if (fragment->m_sendTime > m_time)
            continue;
        if (limit < total + fragment->m_size)
        {
            if (size != 0)
                *size = total;
            return 0;
        }
        ProtocolMessageWriter* writer = handler->AssignByStationId(ProtocolId(m_protocolId), m_remoteStationId,
                                                                   fragment->m_size, false, false, true);
        if (writer == 0)
        {
            if (size != 0)
                *size = total;
            return 0;
        }
        fragment->m_message.m_header.m_field10 = sequence;
        fragment->m_message.m_header.m_field18 = receivedBits;
        fragment->m_message.SerializeHeader();
        writer->SetPayload(fragment->m_message.m_data);
        handler->Commit();
        total += fragment->m_size;

        if (s_resendTimeCallback == 0)
            fragment->m_sendTime = resendTime;
        else
        {
            const int timeout = s_resendTimeCallback(rtt, fragment->m_field0c);
            fragment->m_sendTime = static_cast<long long>(timeout) * pead::TickSpan::cFrequency / 1000 + m_time;
        }
        m_field68 = false;
        if (g_maximumResendCount == cResendCountReset || g_maximumResendCount < fragment->m_field0c)
            g_maximumResendCount = fragment->m_field0c;
        ++fragment->m_field0c;
    }

    unsigned int written = total;
    if (m_field68)
    {
        written = total + cHeaderSize;
        if (limit < written)
        {
            if (size != 0)
                *size = total;
            return 0;
        }
        ProtocolMessageWriter* writer = handler->AssignByStationId(ProtocolId(m_protocolId), m_remoteStationId,
                                                                   cHeaderSize, false, false, true);
        if (writer == 0)
        {
            if (size != 0)
                *size = total;
            return 0;
        }
        MessageHeader header;
        header.m_flags = 0;
        header.m_size = 0;
        header.m_field08 = 0;
        header.m_sequenceId = 0;
        header.m_field10 = sequence;
        header.m_field18 = receivedBits;
        unsigned char buffer[cHeaderSize];
        unsigned int headerSize;
        header.Serialize(buffer, &headerSize);
        writer->SetPayload(buffer);
        handler->Commit();
        m_field68 = false;
    }
    if (size != 0)
        *size = written;
    return 0;
}

Result ReliableSlidingWindow::PopData(void* buffer, unsigned int* size, unsigned int bufferSize)
{
    if (m_protocolId == ProtocolId::INVALID)
        return cResultInvalidState;
    if (buffer == 0 || size == 0)
        return cResultInvalidArgument;
    unsigned int total = 0;
    unsigned int count = 0;
    while (count < m_receiveBuffer.m_count)
    {
        unsigned int index = m_receiveBuffer.m_head + count;
        if (index >= m_receiveBuffer.m_capacity)
            index -= m_receiveBuffer.m_capacity;
        ReceiveData* fragment = m_receiveBuffer.m_data + index;
        if (!fragment->m_valid)
            return cResultDataNotReady;
        total += fragment->m_size;
        if (fragment->m_end)
        {
            if (bufferSize < total)
                return cResultDataSizeInvalid;
            unsigned char* destination = static_cast<unsigned char*>(buffer);
            for (unsigned int offset = 0; offset <= count; ++offset)
            {
                index = m_receiveBuffer.m_head + offset;
                if (index >= m_receiveBuffer.m_capacity)
                    index -= m_receiveBuffer.m_capacity;
                fragment = m_receiveBuffer.m_data + index;
                OSBlockMove(destination, fragment->m_payload, fragment->m_size, FALSE);
                destination += fragment->m_size;
            }
            *size = total;
            m_receiveBuffer.Withdraw(count + 1);
            return 0;
        }
        ++count;
    }
    return count < m_receiveBuffer.m_capacity ? cResultDataNotReady : cResultDataSizeInvalid;
}

ReliableSlidingWindow::SendData* ReliableSlidingWindow::SendBuffer::Assign()
{
    unsigned int index = m_head + m_count;
    if (index >= m_capacity)
        index -= m_capacity;
    SendData* data = m_data + index;
    data->m_message.m_header.m_flags = 1;
    data->m_message.m_header.m_sequenceId = m_sequence + m_count;
    ++m_count;
    return data;
}

void ReliableSlidingWindow::MessageData::SetPayload(const void* data, unsigned int size)
{
    OSBlockMove(m_data + cHeaderSize, data, size, FALSE);
}

Result ReliableSlidingWindow::PushData(const void* data, unsigned int size)
{
    if (m_protocolId == ProtocolId::INVALID)
        return cResultInvalidState;
    if (size != 0 && data == 0)
        return cResultInvalidArgument;
    const unsigned int count = size == 0 ? 1 : (size - 1) / m_payloadLimit + 1;
    if (count > m_sendBuffer.m_capacity - m_sendBuffer.m_count)
        return cResultWindowOverflow;
    if (m_byteCount != 0 || size >= cProfiledSizeMin)
    {
        if (m_byteCount == 0)
            m_profileStart = g_piaTransport->m_time.m_tick;
        m_byteCount += size;
    }
    const unsigned char* source = static_cast<const unsigned char*>(data);
    for (unsigned int index = 0; index < count; ++index)
    {
        SendData* fragment = m_sendBuffer.Assign();
        const unsigned int fragmentSize = index + 1 == count ? size : m_payloadLimit;
        if (fragmentSize != 0)
            fragment->m_message.SetPayload(source, fragmentSize);
        fragment->m_message.m_header.m_size = static_cast<unsigned short>(fragmentSize);
        fragment->m_sendTime = 0;
        fragment->m_size = fragmentSize + cHeaderSize;
        fragment->m_field0c = 0;
        if (index + 1 == count)
            fragment->m_message.m_header.m_flags |= 2;
        else
        {
            source += fragmentSize;
            size -= fragmentSize;
        }
    }
    return 0;
}

unsigned int ReliableSlidingWindow::CanPushData(unsigned int size)
{
    if (!IsInCommunication())
        return false;
    const unsigned int count = size == 0 ? 1 : (size - 1) / m_payloadLimit + 1;
    return count <= m_sendBuffer.m_capacity - m_sendBuffer.m_count;
}

void ReliableSlidingWindow::Trace(unsigned long long) const
{
}

void ReliableSlidingWindow::RegisterResendTimeCallback(int (*callback)(int, int))
{
    s_resendTimeCallback = callback;
}

}
}
}
