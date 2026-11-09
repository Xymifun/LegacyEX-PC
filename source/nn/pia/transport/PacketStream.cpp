#include "nn/pia/transport/PacketStream.hpp"
#include "nn/pia/common/Packet.hpp"
#include "nn/pia/HeapManager.hpp"
#include "nn/pia/Transport.hpp"

#include <new>

namespace nn
{
namespace pia
{
namespace transport
{

common::Packet* PacketStream::Accessor::Get(int index)
{
    unsigned int packetIndex = static_cast<unsigned int>(index) + static_cast<unsigned int>(m_field04);
    if (static_cast<int>(packetIndex) >= m_packetStream->m_packets.size())
        packetIndex -= m_packetStream->m_packets.size();
    return m_packetStream->m_packets.get(packetIndex);
}

int PacketStream::Writer::AssignableNum() const
{
    const PacketStream* packetStream = m_packetStream;
    const int readerPosition = packetStream->m_reader.m_field04;
    const int writerPosition = m_field08;
    const int available = readerPosition - writerPosition - 1;
    if (readerPosition > writerPosition)
        return available;
    return available + packetStream->m_packets.size();
}

common::Packet* PacketStream::Writer::Assign()
{
    if (AssignableNum() < 1)
        return 0;

    const unsigned int packetIndex = static_cast<unsigned int>(m_field08);
    int nextPosition = m_field08 + 1;
    if (nextPosition >= m_packetStream->m_packets.size())
        nextPosition = 0;
    m_field08 = nextPosition;
    ++m_field0c;

    const int packetNum = m_packetStream->m_packets.size();
    const int usedCount = packetNum - AssignableNum();
    if (g_piaWatermarkManager != 0)
        g_piaWatermarkManager->GetWatermark(m_packetStream->m_field28)->Update(usedCount);

    return m_packetStream->m_packets.get(packetIndex);
}

void PacketStream::Writer::Push()
{
    m_field04 = m_field08;
    m_field0c = 0;
}

common::Packet* PacketStream::Reader::PullOne()
{
    const int writerPosition = m_packetStream->m_writer.m_field04;
    if (m_field08 == writerPosition)
        return 0;

    const int packetIndex = m_field08;
    ++m_field08;
    if (m_field08 >= m_packetStream->m_packets.size())
        m_field08 = 0;
    ++m_field0c;
    return m_packetStream->m_packets.get(packetIndex);
}

void PacketStream::Reader::PullAll()
{
    m_field08 = m_packetStream->m_writer.m_field04;
    m_field0c = m_field08 - m_field04;
    if (static_cast<int>(m_field0c) < 0)
        m_field0c += m_packetStream->m_packets.size();
}

void PacketStream::Reader::Release()
{
    m_field04 = m_field08;
    m_field0c = 0;
}

PacketStream::PacketStream()
{
    m_writer.m_packetStream = this;
    m_reader.m_packetStream = this;
    Cleanup();
}

Result PacketStream::Initialize(unsigned int packetNum, unsigned int field28)
{
    if (packetNum < 2)
        return cResultInvalidArgument;
    if (m_packets.isBufferReady())
        return cResultAlreadyInitialized;

    m_packets.tryAllocBuffer(packetNum, common::HeapManager::GetHeap(), 4);
    m_field28 = field28;
    Cleanup();
    return 0;
}

void PacketStream::Finalize()
{
    m_packets.freeBuffer();
    Cleanup();
}

Result PacketStream::Startup()
{
    if (!m_packets.isBufferReady())
        return cResultNotInitialized;

    m_writer.Reset(0);
    m_reader.Reset(0);
    return 0;
}

void PacketStream::Cleanup()
{
    m_writer.Reset(-1);
    m_reader.Reset(-1);
}

void PacketStream::Trace(unsigned long long value) const
{
}

}
}
}
