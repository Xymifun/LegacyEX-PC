#include "game/network/ClientboundCustomPayloadPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/Packet.hpp"
#include "game/network/PacketListener.hpp"
#include "game/world/biome/arrayWithLength.hpp"

ClientboundCustomPayloadPacket::ClientboundCustomPayloadPacket()
{
}

ClientboundCustomPayloadPacket::ClientboundCustomPayloadPacket(const std::wstring& identifier,
                                                               arrayWithLength<unsigned char> data)
{
    this->identifier = identifier;
    this->data = data;
    this->dataLength = data.data != NULL ? data.length : 0;
}

ClientboundCustomPayloadPacket::~ClientboundCustomPayloadPacket()
{
    delete[] data.data;
}

int ClientboundCustomPayloadPacket::getPacketId()
{
    return 250;
}

void ClientboundCustomPayloadPacket::read(DataInputStream* input)
{
    identifier = Packet::readUtf(input, 25);
    dataLength = input->readShort();

    if (dataLength > 0 && dataLength < 32767)
    {
        delete[] data.data;
        arrayWithLength<unsigned char> buffer(dataLength, true);
        data = buffer;
        input->readFully(buffer);
    }
}

void ClientboundCustomPayloadPacket::write(DataOutputStream* output)
{
    Packet::writeUtf(identifier, output);
    output->writeShort((short)dataLength);

    if (data.data != NULL)
    {
        output->write(data);
    }
}

void ClientboundCustomPayloadPacket::handle(PacketListener* listener)
{
    listener->handleCustomPayload(shared_from_this());
}

int ClientboundCustomPayloadPacket::getEstimatedSize()
{
    return identifier.size() * 2 + dataLength + 4;
}

std::wstring ClientboundCustomPayloadPacket::getIdentifier()
{
    return identifier;
}

arrayWithLength<unsigned char> ClientboundCustomPayloadPacket::getData()
{
    return data;
}
