#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_RELIABLESLIDINGWINDOW_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_RELIABLESLIDINGWINDOW_HPP

#include "nn/pia/Common.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/common/ISerializable.hpp"

namespace nn
{
namespace pia
{
namespace transport
{

class StationPacketHandler;
class ProtocolMessageReader;

class ReliableSlidingWindow
{
public:
    enum
    {
        cHeaderSize = 24,
        cPayloadSizeMax = 1384
    };

    class MessageHeader : public common::ISerializable
    {
    public:
        virtual ~MessageHeader() {}

        virtual Result Serialize(unsigned char* buffer, unsigned int* size) const
        {
            if (buffer == 0)
                return cResultInvalidArgument;
            if (size == 0)
                return cResultInvalidArgument;

            unsigned char* cursor = buffer;
            const unsigned short flags = m_flags;
            cursor[0] = static_cast<unsigned char>((flags >> 8) & 0xff);
            cursor[1] = static_cast<unsigned char>(flags & 0xff);
            cursor += sizeof(m_flags);
            const unsigned short messageSize = m_size;
            cursor[0] = static_cast<unsigned char>((messageSize >> 8) & 0xff);
            cursor[1] = static_cast<unsigned char>(messageSize & 0xff);
            cursor += sizeof(m_size);
            const unsigned int field08 = m_field08;
            cursor[0] = static_cast<unsigned char>((field08 >> 24) & 0xff);
            cursor[1] = static_cast<unsigned char>((field08 >> 16) & 0xff);
            cursor[2] = static_cast<unsigned char>((field08 >> 8) & 0xff);
            cursor[3] = static_cast<unsigned char>(field08 & 0xff);
            cursor += sizeof(m_field08);
            const unsigned int sequenceId = m_sequenceId;
            cursor[0] = static_cast<unsigned char>((sequenceId >> 24) & 0xff);
            cursor[1] = static_cast<unsigned char>((sequenceId >> 16) & 0xff);
            cursor[2] = static_cast<unsigned char>((sequenceId >> 8) & 0xff);
            cursor[3] = static_cast<unsigned char>(sequenceId & 0xff);
            cursor += sizeof(m_sequenceId);
            const unsigned int field10 = m_field10;
            cursor[0] = static_cast<unsigned char>((field10 >> 24) & 0xff);
            cursor[1] = static_cast<unsigned char>((field10 >> 16) & 0xff);
            cursor[2] = static_cast<unsigned char>((field10 >> 8) & 0xff);
            cursor[3] = static_cast<unsigned char>(field10 & 0xff);
            cursor += sizeof(m_field10);
            const unsigned long long field18 = m_field18;
            cursor[0] = static_cast<unsigned char>((field18 >> 56) & 0xff);
            cursor[1] = static_cast<unsigned char>((field18 >> 48) & 0xff);
            cursor[2] = static_cast<unsigned char>((field18 >> 40) & 0xff);
            cursor[3] = static_cast<unsigned char>((field18 >> 32) & 0xff);
            cursor[4] = static_cast<unsigned char>((field18 >> 24) & 0xff);
            cursor[5] = static_cast<unsigned char>((field18 >> 16) & 0xff);
            cursor[6] = static_cast<unsigned char>((field18 >> 8) & 0xff);
            cursor[7] = static_cast<unsigned char>(field18 & 0xff);
            cursor += sizeof(m_field18);

            *size = static_cast<unsigned int>(cursor - buffer);
            return 0;
        }

        virtual Result Deserialize(const unsigned char* buffer)
        {
            const Result invalid = cResultInvalidArgument;
            if (buffer == 0)
                return invalid;

            const unsigned char* cursor = buffer;
            m_flags = static_cast<unsigned short>(cursor[0] << 8 | cursor[1]);
            cursor += sizeof(m_flags);
            m_size = static_cast<unsigned short>(cursor[0] << 8 | cursor[1]);
            cursor += sizeof(m_size);
            m_field08 = static_cast<unsigned int>(cursor[0]) << 24 | static_cast<unsigned int>(cursor[1]) << 16
                        | static_cast<unsigned int>(cursor[2]) << 8 | cursor[3];
            cursor += sizeof(m_field08);
            m_sequenceId = static_cast<unsigned int>(cursor[0]) << 24 | static_cast<unsigned int>(cursor[1]) << 16
                           | static_cast<unsigned int>(cursor[2]) << 8 | cursor[3];
            cursor += sizeof(m_sequenceId);
            m_field10 = static_cast<unsigned int>(cursor[0]) << 24 | static_cast<unsigned int>(cursor[1]) << 16
                        | static_cast<unsigned int>(cursor[2]) << 8 | cursor[3];
            cursor += sizeof(m_field10);
            m_field18 =
                static_cast<unsigned long long>(cursor[0]) << 56 | static_cast<unsigned long long>(cursor[1]) << 48
                | static_cast<unsigned long long>(cursor[2]) << 40 | static_cast<unsigned long long>(cursor[3]) << 32
                | static_cast<unsigned long long>(cursor[4]) << 24 | static_cast<unsigned long long>(cursor[5]) << 16
                | static_cast<unsigned long long>(cursor[6]) << 8 | cursor[7];
            return 0;
        }

        virtual unsigned int GetSerializedSize() const { return cHeaderSize; }

        unsigned short m_flags;
        unsigned short m_size;
        unsigned int m_field08;
        unsigned int m_sequenceId;
        unsigned int m_field10;
        unsigned long long m_field18;
    };

    class MessageData
    {
    public:
        void DeserializeHeader(const void* buffer);
        void SerializeHeader();
        static void CopyPayload(void* destination, const void* source, unsigned int size);
        void SetPayload(const void* data, unsigned int size);

        MessageHeader m_header;
        unsigned char m_data[cHeaderSize + cPayloadSizeMax];
    };

    struct SendData
    {
        unsigned long long m_sendTime;
        unsigned int m_size;
        unsigned short m_field0c;
        MessageData m_message;
    };

    struct ReceiveData
    {
        bool m_valid;
        bool m_end;
        unsigned short m_size;
        unsigned char m_payload[cPayloadSizeMax];
    };

    template <typename T> class Buffer
    {
    public:
        Buffer();
        ~Buffer();
        Result Initialize(unsigned int capacity);
        void Finalize();
        void Clear();
        void Withdraw(unsigned int count);
        T* GetData(unsigned int sequence);

        T* m_data;
        unsigned int m_capacity;
        unsigned int m_head;
        unsigned int m_sequence;
        unsigned int m_count;
    };

    class SendBuffer : public Buffer<SendData>
    {
    public:
        SendData* Assign();
    };

    class ReceiveBuffer : public Buffer<ReceiveData>
    {
    public:
        ReceiveData* Assign(unsigned int sequence);
    };

    ReliableSlidingWindow();
    virtual ~ReliableSlidingWindow();
    virtual void Trace(unsigned long long value) const;
    Result Initialize(unsigned int sendCapacity, unsigned int receiveCapacity);
    void Finalize();
    void Cleanup();
    bool IsInCommunication() const;
    Result PopData(void* buffer, unsigned int* size, unsigned int bufferSize);
    Result PushData(const void* data, unsigned int size);
    unsigned int CanPushData(unsigned int size);
    Result Startup(StationPacketHandler* handler, unsigned short protocolId, unsigned long long localStationId,
                   unsigned long long remoteStationId);
    Result AnalyzeProtocolMessage(const ProtocolMessageReader& reader);
    Result Dispatch(StationPacketHandler* handler, unsigned int* size, unsigned int limit);
    static void RegisterResendTimeCallback(int (*callback)(int, int));

    ProtocolId m_protocolId;
    unsigned long long m_localStationId;
    unsigned long long m_remoteStationId;
    SendBuffer m_sendBuffer;
    ReceiveBuffer m_receiveBuffer;
    unsigned int m_payloadLimit;
    unsigned long long m_time;
    unsigned long long m_retrySpan;
    unsigned int m_sequence;
    unsigned long long m_receivedBits;
    bool m_field68;
    unsigned int m_byteCount;
    unsigned long long m_profileStart;

private:
    static float s_retransmissionTimeOutRate;
    static unsigned int s_rttSamplingNum;
    static int (*s_resendTimeCallback)(int, int);
};


}
}
}

#endif
