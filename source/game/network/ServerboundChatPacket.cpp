#include "game/network/ServerboundChatPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"

ServerboundChatPacket::ServerboundChatPacket()
{
}

ServerboundChatPacket::ServerboundChatPacket(const std::wstring& message)
{
    if (message.length() > 256)
    {
        this->message = message.substr(0, 256);
    }
    else
    {
        this->message = message;
    }
}

ServerboundChatPacket::~ServerboundChatPacket()
{
}

void ServerboundChatPacket::read(DataInputStream* input)
{
    message = Packet::readUtf(input, 256);
}

void ServerboundChatPacket::write(DataOutputStream* output)
{
    Packet::writeUtf(message, output);
}

void ServerboundChatPacket::handle(PacketListener* listener)
{
    listener->handleChat(shared_from_this());
}

int ServerboundChatPacket::getPacketId()
{
    return 222;
}

std::wstring ServerboundChatPacket::getMessage()
{
    return message;
}
