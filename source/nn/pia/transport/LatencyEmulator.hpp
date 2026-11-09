#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_LATENCY_EMULATOR_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_LATENCY_EMULATOR_HPP

#include "nn/pia.h"
#include "nn/pia/common/Packet.hpp"
#include "nn/pia/common/IPacketInput.hpp"
#include "nn/pia/common/IPacketOutput.hpp"
#include "nn/pia/common/Time.hpp"
#include "platform/wiiu/pead.hpp"

namespace nn
{
namespace pia
{
namespace transport
{

class LatencyEmulator
{
public:
    class Entry
    {
    public:
        Entry() {}
        Entry(const common::Packet& packet, const common::Time& dispatchTime) : m_dispatchTime(dispatchTime)
        {
            m_packet.CopyFrom(packet);
        }

        Entry& operator=(const Entry& other)
        {
            m_packet.CopyFrom(other.m_packet);
            m_dispatchTime = other.m_dispatchTime;
            return *this;
        }

        common::Packet m_packet;
        common::Time m_dispatchTime;
    };

    explicit LatencyEmulator(unsigned int entryNum);
    LatencyEmulator(common::IPacketInput* packetInput, unsigned int entryNum);
    LatencyEmulator(common::IPacketOutput* packetOutput, unsigned int entryNum);
    ~LatencyEmulator();
    void SetInput(common::IPacketInput* packetInput);
    void SetOutput(common::IPacketOutput* packetOutput);
    void SetLatency(int minimumLatency, int maximumLatency);
    void Clear();
    Result Read(common::Packet* packet);
    Result Write(const common::Packet& packet);
    Result Dispatch();
    void Trace(unsigned long long value) const;

    pead::RingBuffer<Entry> m_entries;
    Entry* m_buffer;
    common::IPacketInput* m_packetInput;
    common::IPacketOutput* m_packetOutput;
    int m_minimumLatency;
    int m_maximumLatency;
    mutable common::CriticalSection m_criticalSection;
    pead::Random m_random;
    Entry m_entry;

private:
    bool allocateBuffer(unsigned int entryNum);
    void freeBuffer();
    void init(unsigned int entryNum);
    void getLatency(int* minimumLatency, int* maximumLatency) const;
    int calcLatency();
    Result writeDispatch();
    Result readDispatch();
};

}
}
}

#endif
