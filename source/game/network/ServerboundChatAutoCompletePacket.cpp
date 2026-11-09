#include "game/network/ServerboundChatAutoCompletePacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"

ServerboundChatAutoCompletePacket::ServerboundChatAutoCompletePacket() : lookAtPos(NULL)
{
}

ServerboundChatAutoCompletePacket::ServerboundChatAutoCompletePacket(const std::wstring& message,
                                                                     const BlockPos& lookAtPos)
    : message(message),
      command(false),
      lookAtPos(new BlockPos(lookAtPos))
{
}

ServerboundChatAutoCompletePacket::ServerboundChatAutoCompletePacket(const std::wstring& message,
                                                                     const BlockPos& lookAtPos, bool assumeCommand)
    : message(message),
      command(assumeCommand),
      lookAtPos(new BlockPos(lookAtPos))
{
}

ServerboundChatAutoCompletePacket::~ServerboundChatAutoCompletePacket()
{
    delete lookAtPos;
}

void ServerboundChatAutoCompletePacket::read(DataInputStream* input)
{
    message = Packet::readUtf(input, 32767);
    command = input->readBoolean();

    if (input->readBoolean())
    {
        delete lookAtPos;
        lookAtPos = new BlockPos(input->readBlockPos());
    }
}

void ServerboundChatAutoCompletePacket::write(DataOutputStream* output)
{
    Packet::writeUtf(message, output);
    output->writeBoolean(command);
    bool hasLookAtPos = lookAtPos != NULL;
    output->writeBoolean(hasLookAtPos);

    if (hasLookAtPos)
    {
        output->writeBlockPos(*lookAtPos);
    }
}

void ServerboundChatAutoCompletePacket::handle(PacketListener* listener)
{
    listener->handleChatAutoComplete(shared_from_this());
}

int ServerboundChatAutoCompletePacket::getPacketId()
{
    return 221;
}

std::wstring ServerboundChatAutoCompletePacket::getMessage()
{
    return message;
}

BlockPos* ServerboundChatAutoCompletePacket::getLookAtPos()
{
    return lookAtPos;
}

bool ServerboundChatAutoCompletePacket::assumeCommand()
{
    return command;
}
