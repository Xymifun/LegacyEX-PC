#include "game/network/ClientboundSetTitlesPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/Packet.hpp"
#include "game/network/PacketListener.hpp"

ClientboundSetTitlesPacket::ClientboundSetTitlesPacket() : text()
{
}

ClientboundSetTitlesPacket::ClientboundSetTitlesPacket(Type type, const std::wstring& text, int fadeIn, int stay,
                                                       int fadeOut)
{
    this->type = type;
    this->text = text;
    this->fadeIn = fadeIn;
    this->stay = stay;
    this->fadeOut = fadeOut;
}

int ClientboundSetTitlesPacket::getPacketId()
{
    return 219;
}

void ClientboundSetTitlesPacket::read(DataInputStream* input)
{
    type = input->readInt();

    if (type <= ACTIONBAR)
    {
        text = Packet::readUtf(input, 32767);
    }

    if (type == TIMES)
    {
        fadeIn = input->readInt();
        stay = input->readInt();
        fadeOut = input->readInt();
    }
}

void ClientboundSetTitlesPacket::write(DataOutputStream* output)
{
    output->writeInt(type);

    if (type <= ACTIONBAR)
    {
        Packet::writeUtf(text, output);
    }

    if (type == TIMES)
    {
        output->writeInt(fadeIn);
        output->writeInt(stay);
        output->writeInt(fadeOut);
    }
}

void ClientboundSetTitlesPacket::handle(PacketListener* listener)
{
    listener->handleSetTitles(shared_from_this());
}
