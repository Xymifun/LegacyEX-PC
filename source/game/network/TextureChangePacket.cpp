#include "game/network/TextureChangePacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/network/PacketListener.hpp"

TextureChangePacket::TextureChangePacket()
{
    mEntityId = -1;
    mChangeType = eTextureChangeType_Skin;
    mTextureName = L"";
}

TextureChangePacket::TextureChangePacket(boost::shared_ptr<Entity> entity, ETextureChangeType type,
                                         const std::wstring& textureName)
    : mEntityId(-1),
      mChangeType(type),
      mTextureName(textureName)
{
    mEntityId = entity->getId();
}

void TextureChangePacket::read(DataInputStream* input)
{
    mEntityId = input->readInt();
    mChangeType = static_cast<ETextureChangeType>(input->readByte());
    mTextureName = Packet::readUtf(input, 32767);
}

void TextureChangePacket::write(DataOutputStream* output)
{
    output->writeInt(mEntityId);
    output->writeByte(static_cast<unsigned char>(mChangeType));
    Packet::writeUtf(mTextureName, output);
}

void TextureChangePacket::handle(PacketListener* listener)
{
    listener->handleTextureChange(shared_from_this());
}

int TextureChangePacket::getEstimatedSize()
{
    return static_cast<int>(mTextureName.size()) + 5;
}

int TextureChangePacket::getPacketId()
{
    return 157;
}
