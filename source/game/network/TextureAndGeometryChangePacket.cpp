#include "game/network/TextureAndGeometryChangePacket.hpp"

#include <sstream>

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/network/PacketListener.hpp"

TextureAndGeometryChangePacket::TextureAndGeometryChangePacket()
{
    entityId = -1;
    geometryName = L"";
    geometryId = 0;
}

TextureAndGeometryChangePacket::TextureAndGeometryChangePacket(boost::shared_ptr<Entity> entity,
                                                               const std::wstring& geometryName)
{
    entityId = entity->getId();
    this->geometryName = geometryName;

    std::wstring identifier = geometryName.substr(7, geometryName.length());
    identifier = identifier.substr(0, identifier.find(L'.'));

    std::wstringstream stream;
    stream << std::dec << identifier.c_str();
    stream >> geometryId;
    geometryId |= 0x80000000u;
}

int TextureAndGeometryChangePacket::getPacketId()
{
    return 161;
}

void TextureAndGeometryChangePacket::read(DataInputStream* input)
{
    entityId = input->readInt();
    geometryId = input->readInt();
    geometryName = Packet::readUtf(input, 32767);
}

void TextureAndGeometryChangePacket::write(DataOutputStream* output)
{
    output->writeInt(entityId);
    output->writeInt(geometryId);
    Packet::writeUtf(geometryName, output);
}

void TextureAndGeometryChangePacket::handle(PacketListener* listener)
{
    listener->handleTextureAndGeometryChange(shared_from_this());
}

int TextureAndGeometryChangePacket::getEstimatedSize()
{
    return geometryName.length() + 8;
}
