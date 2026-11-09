#include "game/network/TexturePacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"

TexturePacket::TexturePacket()
{
    shouldDelay = true;
    textureName = L"";
    length = 0;
    data = NULL;
}

TexturePacket::TexturePacket(const std::wstring& textureName, unsigned char* data, unsigned int length)
{
    shouldDelay = true;
    this->textureName = textureName;
    this->data = data;
    this->length = length;
}

TexturePacket::~TexturePacket()
{
}

int TexturePacket::getPacketId()
{
    return 154;
}

void TexturePacket::handle(PacketListener* listener)
{
    listener->handleTexture(shared_from_this());
}

void TexturePacket::read(DataInputStream* input)
{
    textureName = Packet::readUtf(input, 32767);
    length = input->readShort();
    if (length != 0)
    {
        data = new unsigned char[length];
        unsigned int index = 0;
        while (index < length)
        {
            data[index] = input->readByte();
            ++index;
        }
    }
}

void TexturePacket::write(DataOutputStream* output)
{
    Packet::writeUtf(textureName, output);
    output->writeShort(static_cast<short>(length));
    unsigned int index = 0;
    while (index < length)
    {
        output->writeByte(data[index]);
        ++index;
    }
}

int TexturePacket::getEstimatedSize()
{
    return 4096;
}
