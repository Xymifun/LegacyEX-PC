#include "game/network/ServerboundCustomPayloadPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/Packet.hpp"
#include "game/network/PacketListener.hpp"
#include "game/world/biome/arrayWithLength.hpp"

const std::wstring ServerboundCustomPayloadPacket::CUSTOM_BOOK_PACKET = L"MC|BEdit";
const std::wstring ServerboundCustomPayloadPacket::CUSTOM_BOOK_SIGN_PACKET = L"MC|BSign";
const std::wstring ServerboundCustomPayloadPacket::TRADER_SELECTION_PACKET = L"MC|TrSel";
const std::wstring ServerboundCustomPayloadPacket::SET_ADVENTURE_COMMAND_PACKET = L"MC|AdvCmd";
const std::wstring ServerboundCustomPayloadPacket::SET_AUTO_COMMAND_PACKET = L"MC|AutoCmd";
const std::wstring ServerboundCustomPayloadPacket::SET_STRUCTURE_PACKET = L"MC|Struct";
const std::wstring ServerboundCustomPayloadPacket::SET_BEACON_PACKET = L"MC|Beacon";
const std::wstring ServerboundCustomPayloadPacket::SET_ITEM_NAME_PACKET = L"MC|ItemName";
const std::wstring ServerboundCustomPayloadPacket::SET_RENAME_ITEM_PACKET = L"MC|RIName";
const std::wstring ServerboundCustomPayloadPacket::PICK_ITEM_PACKET = L"MC|PickItem";
const std::wstring ServerboundCustomPayloadPacket::CLIENT_BRAND_PACKET = L"MC|Brand";

ServerboundCustomPayloadPacket::ServerboundCustomPayloadPacket()
#ifdef MINECRAFT_PC
    : mPcOversizedBytes(0)
#endif
{
}

ServerboundCustomPayloadPacket::ServerboundCustomPayloadPacket(const std::wstring& identifier,
                                                               arrayWithLength<unsigned char> data)
#ifdef MINECRAFT_PC
    : mPcOversizedBytes(0)
#endif
{
    this->identifier = identifier;
    this->data = data;
}

ServerboundCustomPayloadPacket::~ServerboundCustomPayloadPacket()
{
    delete[] data.data;
}

int ServerboundCustomPayloadPacket::getPacketId()
{
    return 225;
}

void ServerboundCustomPayloadPacket::read(DataInputStream* input)
{
    identifier = Packet::readUtf(input, 20);
    int length = input->readShort();

#ifdef MINECRAFT_PC
    const unsigned int size = static_cast<unsigned short>(length);
    if (size >= 32767)
    {
        arrayWithLength<unsigned char> discarded(size, true);
        input->readFully(discarded);
        delete[] discarded.data;
        mPcOversizedBytes = size;
        return;
    }
#endif
    if (length > 0 && length < 32767)
    {
        if (data.data != NULL)
        {
            delete[] data.data;
        }

        data = arrayWithLength<unsigned char>(length, true);
        input->readFully(data);
    }
}

void ServerboundCustomPayloadPacket::write(DataOutputStream* output)
{
    Packet::writeUtf(identifier, output);
    output->writeShort((short)data.length);

    if (data.data != NULL)
    {
        output->write(data);
    }
}

void ServerboundCustomPayloadPacket::handle(PacketListener* listener)
{
    listener->handleCustomPayload(shared_from_this());
}

std::wstring ServerboundCustomPayloadPacket::getIdentifier()
{
    return identifier;
}

arrayWithLength<unsigned char> ServerboundCustomPayloadPacket::getData()
{
    return data;
}
