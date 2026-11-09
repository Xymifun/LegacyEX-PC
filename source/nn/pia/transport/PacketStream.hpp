#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_PACKET_STREAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_PACKET_STREAM_HPP

#include "nn/pia/Reconstruction.hpp"
#include "nn/pia/common/Packet.hpp"

namespace nn
{
namespace pia
{
namespace transport
{

class PacketStream
{
public:
    class Accessor
    {
    public:
        class Iterator
        {
        public:
            Iterator(const Accessor* accessor, int index) : m_accessor(accessor), m_index(index) {}
            bool operator==(const Iterator& other) const
            {
                return m_accessor == other.m_accessor ? m_index == other.m_index : false;
            }
            bool operator!=(const Iterator& other) const { return !(*this == other); }
            Iterator& operator++()
            {
                ++m_index;
                if (m_index >= m_accessor->m_packetStream->m_packets.size())
                    m_index = 0;
                return *this;
            }
            common::Packet* operator->() const { return m_accessor->m_packetStream->m_packets.get(m_index); }
            common::Packet& operator*() const { return *operator->(); }

        private:
            const Accessor* m_accessor;
            int m_index;
        };

        Iterator Begin() const { return Iterator(this, m_field04); }
        Iterator End() const { return Iterator(this, m_field08); }
        common::Packet* Get(int index);
        void Reset(int position)
        {
            m_field04 = position;
            m_field08 = position;
            m_field0c = 0;
        }

        PacketStream* m_packetStream;
        volatile int m_field04;
        volatile int m_field08;
        unsigned int m_field0c;
    };

    class Writer : public Accessor
    {
    public:
        int AssignableNum() const;
        common::Packet* Assign();
        void Push();
    };

    class Reader : public Accessor
    {
    public:
        common::Packet* PullOne();
        void PullAll();
        void Release();
    };

    PacketStream();
    Result Initialize(unsigned int packetNum, unsigned int field28);
    void Finalize();
    Result Startup();
    void Cleanup();
    void Trace(unsigned long long value) const;

    pead::Buffer<common::Packet> m_packets;
    Writer m_writer;
    Reader m_reader;
    unsigned int m_field28;
};

}
}
}

#endif
