#include "game/network/GameCommandPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"

GameCommandPacket::GameCommandPacket() : dataLength(0), data()
{
}

GameCommandPacket::GameCommandPacket(EGameCommand command, arrayWithLength<unsigned char> data)
{
    this->command = command;
    this->data = data;

    if (data.data != NULL)
    {
        this->dataLength = data.length;
    }
    else
    {
        this->dataLength = 0;
    }
}

GameCommandPacket::~GameCommandPacket()
{
    delete[] this->data.data;
}

int GameCommandPacket::getPacketId()
{
    return 167;
}

void GameCommandPacket::read(DataInputStream* input)
{
    this->command = (EGameCommand)input->readInt();
    this->dataLength = input->readShort();

    if (this->dataLength > 0 && this->dataLength < 0x7FFF)
    {
        if (this->data.data != NULL)
        {
            delete[] this->data.data;
        }

        this->data = arrayWithLength<unsigned char>(this->dataLength, true);

        input->readBytes(this->data);
    }
}

void GameCommandPacket::write(DataOutputStream* output)
{
    output->writeInt(this->command);
    output->writeShort((short)this->dataLength);

    if (this->data.data != NULL)
    {
        output->writeBytes(this->data);
    }
}

void GameCommandPacket::handle(PacketListener* listener)
{
    listener->handleGameCommand(shared_from_this());
}

int GameCommandPacket::getEstimatedSize()
{
    return this->dataLength + 4;
}
