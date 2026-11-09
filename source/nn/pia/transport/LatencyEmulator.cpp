#include "nn/pia/transport/LatencyEmulator.hpp"
#include "nn/pia/common/Memory.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/common/TimeSpan.hpp"

namespace nn
{
namespace pia
{
namespace transport
{

bool LatencyEmulator::allocateBuffer(unsigned int entryNum)
{
    if (entryNum != 0)
    {
        m_buffer = common::NewArray<Entry>(entryNum);
        if (m_buffer != 0)
        {
            m_entries.setBuffer(entryNum, m_buffer);
            return true;
        }
    }
    return false;
}

void LatencyEmulator::init(unsigned int entryNum)
{
    m_buffer = 0;
    m_minimumLatency = 0;
    m_maximumLatency = 0;
    allocateBuffer(entryNum);
}

LatencyEmulator::LatencyEmulator(unsigned int entryNum)
    : m_entries(),
      m_packetInput(0),
      m_packetOutput(0),
      m_criticalSection(-1),
      m_random()
{
    init(entryNum);
}

void LatencyEmulator::freeBuffer()
{
    if (m_buffer != 0)
    {
        common::DeleteArray(m_buffer);
        m_buffer = 0;
    }
}

LatencyEmulator::~LatencyEmulator()
{
    freeBuffer();
}

void LatencyEmulator::SetInput(common::IPacketInput* packetInput)
{
    m_packetInput = packetInput;
}

void LatencyEmulator::SetOutput(common::IPacketOutput* packetOutput)
{
    m_packetOutput = packetOutput;
}

void LatencyEmulator::Clear()
{
    m_entries.clear();
}

Result LatencyEmulator::Read(common::Packet* packet)
{
    if (packet == 0)
        return ResultInvalidArgument();
    if (m_packetInput == 0)
        return cResultInvalidState;
    if (m_entries.isEmpty())
        return cResultDataNotReady;

    common::Time now;
    now.SetNow();
    if (m_entries.front().m_dispatchTime < now)
    {
        packet->CopyFrom(m_entries.front().m_packet);
        m_entries.popFront();
        return 0;
    }
    return cResultDataNotReady;
}

void LatencyEmulator::getLatency(int* minimumLatency, int* maximumLatency) const
{
    m_criticalSection.Lock();
    *minimumLatency = m_minimumLatency;
    *maximumLatency = m_maximumLatency;
    m_criticalSection.Unlock();
}

int LatencyEmulator::calcLatency()
{
    int minimumLatency = 0;
    int maximumLatency = 0;
    getLatency(&minimumLatency, &maximumLatency);
    return m_random.getS32Range(minimumLatency, maximumLatency);
}

Result LatencyEmulator::Write(const common::Packet& packet)
{
    common::Time now;
    now.SetNow();
    common::Time dispatchTime = now + common::TimeSpan::FromMSec(calcLatency());
    if (m_entries.pushBack(Entry(packet, dispatchTime)))
        return 0;
    return cResultWindowOverflow;
}

Result LatencyEmulator::Dispatch()
{
    if (m_packetInput == 0)
    {
        if (m_packetOutput != 0)
            return writeDispatch();
    }
    else if (m_packetOutput == 0)
    {
        return readDispatch();
    }
    return cResultInvalidState;
}

Result LatencyEmulator::writeDispatch()
{
    common::Time now;
    now.SetNow();
    Result result = 0;

    while (!m_entries.isEmpty())
    {
        if (!(m_entries.front().m_dispatchTime < now))
            return result;

        result = m_packetOutput->Write(m_entries.front().m_packet);
        m_entries.popFront();
        if (result != 0)
        {
            if (result == cResultWindowOverflow)
                result = 0;
            break;
        }
    }
    return result;
}

Result LatencyEmulator::readDispatch()
{
    common::Time now;
    now.SetNow();
    Result result = 0;

    while (!m_entries.isFull())
    {
        result = m_packetInput->Read(&m_entry.m_packet);
        if (result == 0)
        {
            m_entry.m_dispatchTime = now + common::TimeSpan::FromMSec(calcLatency());
            if (!m_entries.pushBack(m_entry))
                break;
        }
        else
        {
            if (result == cResultDataNotReady)
                result = 0;
            break;
        }
    }
    return result;
}

void LatencyEmulator::SetLatency(int minimumLatency, int maximumLatency)
{
    m_criticalSection.Lock();
    m_minimumLatency = minimumLatency;
    m_maximumLatency = maximumLatency;
    m_criticalSection.Unlock();
}

}
}
}
