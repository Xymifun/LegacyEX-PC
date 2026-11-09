#include "nn/pia/common/Packet.hpp"

#include "platform/wiiu/pead.hpp"

#include <coreinit/memory.h>

namespace nn
{
namespace pia
{
namespace common
{

namespace
{

enum
{
    cResultInvalidPacket = 0x10c08,
    cResultUnsupportedMode = 0xc0b,
    cResultDecryptInvalidPacket = 0x2c03
};

}

void Packet::PacketData::Copy(const PacketData& packetData)
{
    m_header.m_signature = packetData.m_header.m_signature;
    m_header.m_version = packetData.m_header.m_version;
    m_header.m_passId = packetData.m_header.m_passId;
    m_header.m_sequenceId = packetData.m_header.m_sequenceId;
    m_header.m_rttResponseFlag = packetData.m_header.m_rttResponseFlag;
    m_header.m_rttMeasurement = packetData.m_header.m_rttMeasurement;
    unsigned int index;
    for (index = 0; index < sizeof(m_header.m_cryptoNonce); ++index)
        m_header.m_cryptoNonce[index] = packetData.m_header.m_cryptoNonce[index];

    OSBlockMove(m_data, packetData.m_data, cDataSizeMax, FALSE);
}

unsigned char* Packet::PacketData::GetPayload()
{
    return m_data + cHeaderSize;
}

const unsigned char* Packet::PacketData::GetPayload(unsigned int offset) const
{
    return m_data + cHeaderSize + offset;
}

void Packet::PacketData::SetPayload(const void* payload, unsigned int size)
{
    OSBlockMove(m_data + cHeaderSize, payload, size, FALSE);
}

void Packet::PacketData::SerializeHeader()
{
    unsigned int size;
    m_header.Serialize(m_data, &size);
}

void Packet::PacketData::DeserializeHeader()
{
    m_header.Deserialize(m_data);
}

Packet::Packet()
{
    Reset();
}

Packet::~Packet()
{
}

void Packet::Reset()
{
    m_packetData.m_header.m_signature = cSignature;
    m_packetData.m_header.m_version = 1;
    m_packetData.m_header.m_passId = 0;
    m_packetData.m_header.m_sequenceId = 0;
    m_packetData.m_header.m_rttResponseFlag = 0;
    m_packetData.m_header.m_rttMeasurement = 0;
    OSBlockSet(m_packetData.m_header.m_cryptoNonce, 0, sizeof(m_packetData.m_header.m_cryptoNonce));
    OSBlockSet(m_packetData.m_header.m_cryptoSignature, 0, sizeof(m_packetData.m_header.m_cryptoSignature));

    m_size = cHeaderSize;
    m_stationIndex = 0xfd;
    m_stationBitmap = 0;
    m_field5f4.Clear();
    m_field600 = 0;
    m_field601 = 0;
    m_field602 = 0;
    m_field603 = 0;
    m_field604 = 1;
    m_field608.Clear();
    m_field614 = 0;
}

unsigned char Packet::IsValid() const
{
    return m_packetData.m_header.m_signature == cSignature
           && (m_packetData.m_header.m_version == 1 || m_packetData.m_header.m_version == 2) && m_size >= cHeaderSize
           && m_size <= cDataSizeMax;
}

unsigned char* Packet::AssignPayload(unsigned int size)
{
    const unsigned int assignedSize = m_size;
    const unsigned int newSize = assignedSize + size;
    if (newSize > cDataSizeMax)
        return 0;
    m_size = newSize;
    return m_packetData.m_data + assignedSize;
}

void Packet::CopyFrom(const Packet& packet)
{
    m_packetData.Copy(packet.m_packetData);
    m_size = packet.m_size;
    m_stationIndex = packet.m_stationIndex;
    m_stationBitmap = packet.m_stationBitmap;
    m_field5f4 = packet.m_field5f4;
    m_field600 = packet.m_field600;
    m_field601 = packet.m_field601;
    m_field602 = packet.m_field602;
    m_field603 = packet.m_field603;
    m_field604 = packet.m_field604;
    m_field608 = packet.m_field608;
    m_field614 = packet.m_field614;
}

Result Packet::Encrypt(const Crypto::Setting& setting)
{
    if (!IsValid())
        return cResultInvalidPacket;
    if (m_packetData.m_header.m_version != 1)
        return cResultInvalidPacket;
    if (setting.m_mode == Crypto::cModeNone)
        return 0;
    if (setting.m_mode != Crypto::cModeAesGcm)
        return cResultUnsupportedMode;
    if (m_size <= cHeaderSize)
        return 0;

    const unsigned int payloadSize = m_size - cHeaderSize;
    const unsigned int blockSize = Crypto::GetBlockSize(setting.m_mode);
    const unsigned int encryptedSize = (payloadSize + blockSize - 1) / blockSize * blockSize;
    if (encryptedSize > cPayloadSizeMax)
        return cResultInvalidPacket;

    unsigned char source[cPayloadSizeMax];
    OSBlockMove(source, m_packetData.GetPayload(), payloadSize, FALSE);
    const int paddingSize = encryptedSize - payloadSize;
    if (paddingSize > 0)
        OSBlockSet(source + payloadSize, 0xff, paddingSize);

    const Result result = Crypto::Encrypt(m_packetData.GetPayload(), source, encryptedSize,
                                          m_packetData.m_header.m_cryptoSignature,
                                          sizeof(m_packetData.m_header.m_cryptoSignature), setting);
    if (result != 0)
        return result;

    m_packetData.m_header.m_version = 2;
    m_size += paddingSize;
    return 0;
}

Result Packet::Decrypt(const Crypto::Setting& setting)
{
    if (!IsValid())
        return cResultDecryptInvalidPacket;
    if (m_packetData.m_header.m_version == 1)
        return 0;
    if (setting.m_mode != Crypto::cModeAesGcm)
        return cResultUnsupportedMode;
    if (m_packetData.m_header.m_version != 2)
        return cResultDecryptInvalidPacket;

    const unsigned int encryptedSize = m_size - cHeaderSize;
    if (encryptedSize == 0)
    {
        m_packetData.m_header.m_version = 1;
        return 0;
    }

    const unsigned int blockSize = Crypto::GetBlockSize(setting.m_mode);
    if (encryptedSize % blockSize != 0)
        return cResultDecryptInvalidPacket;
    if (encryptedSize > cPayloadSizeMax)
        return cResultDecryptInvalidPacket;

    unsigned char destination[cPayloadSizeMax];
    const Result result = Crypto::Decrypt(destination, m_packetData.GetPayload(), encryptedSize,
                                          m_packetData.m_header.m_cryptoSignature,
                                          sizeof(m_packetData.m_header.m_cryptoSignature), setting);
    if (result != 0)
        return result;

    m_packetData.SetPayload(destination, encryptedSize);
    m_packetData.m_header.m_version = 1;
    return 0;
}

void Packet::Trace(unsigned long long value, bool isVerbose) const
{
}

int Packet::GetPacketNumInNetwork() const
{
    const int packetNum = static_cast<int>(pead::BitFlagUtil::countOnBit(m_stationBitmap));
    if (packetNum < 1)
        return 1;
    return packetNum;
}

}
}
}
