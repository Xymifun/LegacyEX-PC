#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_PACKET_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_PACKET_HPP

#include "nn/pia/common/Crypto.hpp"
#include "nn/pia/common/ISerializable.hpp"
#include "nn/pia/common/StationAddress.hpp"

namespace nn
{
namespace pia
{
namespace common
{

class Packet
{
public:
    enum
    {
        cHeaderSize = 36,
        cDataSizeMax = 1472,
        cPayloadSizeMax = 1436,
        cSignature = 0x32ab9864
    };

    class Header : public ISerializable
    {
    public:
        enum
        {
            cSerializedSize = 36,
            cResultInvalidArgument = 0x10c07
        };

        virtual ~Header() {}

        virtual Result Serialize(unsigned char* buffer, unsigned int* size) const
        {
            if (buffer == 0)
                return cResultInvalidArgument;
            if (size == 0)
                return cResultInvalidArgument;

            unsigned char* cursor = buffer;
            const unsigned int signature = m_signature;
            cursor[0] = static_cast<unsigned char>((signature >> 24) & 0xff);
            cursor[1] = static_cast<unsigned char>((signature >> 16) & 0xff);
            cursor[2] = static_cast<unsigned char>((signature >> 8) & 0xff);
            cursor[3] = static_cast<unsigned char>(signature & 0xff);
            cursor += sizeof(m_signature);
            *cursor++ = m_version;
            *cursor++ = m_passId;
            const unsigned short sequenceId = m_sequenceId;
            cursor[0] = static_cast<unsigned char>((sequenceId >> 8) & 0xff);
            cursor[1] = static_cast<unsigned char>(sequenceId & 0xff);
            cursor += sizeof(m_sequenceId);
            const unsigned short rttResponseFlag = m_rttResponseFlag;
            cursor[0] = static_cast<unsigned char>((rttResponseFlag >> 8) & 0xff);
            cursor[1] = static_cast<unsigned char>(rttResponseFlag & 0xff);
            cursor += sizeof(m_rttResponseFlag);
            const unsigned short rttMeasurement = m_rttMeasurement;
            cursor[0] = static_cast<unsigned char>((rttMeasurement >> 8) & 0xff);
            cursor[1] = static_cast<unsigned char>(rttMeasurement & 0xff);
            cursor += sizeof(m_rttMeasurement);
            unsigned int index;
            for (index = 0; index < sizeof(m_cryptoNonce); ++index)
                *cursor++ = m_cryptoNonce[index];
            for (index = 0; index < sizeof(m_cryptoSignature); ++index)
                *cursor++ = m_cryptoSignature[index];

            *size = static_cast<unsigned int>(cursor - buffer);
            return 0;
        }

        virtual Result Deserialize(const unsigned char* buffer)
        {
            if (buffer == 0)
                return cResultInvalidArgument;

            const unsigned char* cursor = buffer;
            m_signature = (static_cast<unsigned int>(cursor[0]) << 24) | (static_cast<unsigned int>(cursor[1]) << 16)
                          | (static_cast<unsigned int>(cursor[2]) << 8) | cursor[3];
            cursor += sizeof(m_signature);
            m_version = *cursor++;
            m_passId = *cursor++;
            m_sequenceId = static_cast<unsigned short>(cursor[0] << 8 | cursor[1]);
            cursor += sizeof(m_sequenceId);
            m_rttResponseFlag = static_cast<unsigned short>(cursor[0] << 8 | cursor[1]);
            cursor += sizeof(m_rttResponseFlag);
            m_rttMeasurement = static_cast<unsigned short>(cursor[0] << 8 | cursor[1]);
            cursor += sizeof(m_rttMeasurement);
            unsigned int index;
            for (index = 0; index < sizeof(m_cryptoNonce); ++index)
                m_cryptoNonce[index] = *cursor++;
            for (index = 0; index < sizeof(m_cryptoSignature); ++index)
                m_cryptoSignature[index] = *cursor++;

            return 0;
        }

        virtual unsigned int GetSerializedSize() const { return cSerializedSize; }

        unsigned int m_signature;
        unsigned char m_version;
        unsigned char m_passId;
        unsigned short m_sequenceId;
        unsigned short m_rttResponseFlag;
        unsigned short m_rttMeasurement;
        unsigned char m_cryptoNonce[8];
        unsigned char m_cryptoSignature[16];
    };

    class PacketData
    {
    public:
        void Copy(const PacketData& packetData);
        unsigned char* GetPayload();
        const unsigned char* GetPayload() const;
        const unsigned char* GetPayload(unsigned int offset) const;
        void SetPayload(const void* payload, unsigned int size);
        void SerializeHeader();
        void DeserializeHeader();

        Header m_header;
        unsigned char m_data[cDataSizeMax];
    };

    Packet();
    ~Packet();
    void Reset();
    unsigned char IsValid() const;
    unsigned char* AssignPayload(unsigned int size);
    void CopyFrom(const Packet& packet);
    Result Encrypt(const Crypto::Setting& setting);
    Result Decrypt(const Crypto::Setting& setting);
    void Trace(unsigned long long value, bool isVerbose) const;
    int GetPacketNumInNetwork() const;

    PacketData m_packetData;
    unsigned int m_size;
    unsigned int m_stationIndex;
    unsigned int m_stationBitmap;
    StationAddress m_field5f4;
    unsigned char m_field600;
    unsigned char m_field601;
    unsigned char m_field602;
    unsigned char m_field603;
    unsigned char m_field604;
    StationAddress m_field608;
    unsigned char m_field614;
};

}
}
}

#endif
