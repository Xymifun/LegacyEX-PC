#include "game/network/ClientboundBossEventPacket.hpp"

#include "game/core/BossEvent.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/Packet.hpp"
#include "game/network/PacketListener.hpp"

ClientboundBossEventPacket::ClientboundBossEventPacket()
{
    operation = ADD;
    id = L"";
    name = L"";
    nameId = -1;
    percent = 0.0F;
    color = BossEvent::PINK;
    overlay = BossEvent::PROGRESS;
    darkenScreen = false;
    playMusic = false;
    createWorldFog = false;
}

ClientboundBossEventPacket::ClientboundBossEventPacket(Operation operation, BossEvent* bossEvent)
{
    this->operation = operation;
    id = bossEvent->getId();
    name = bossEvent->getName();
    nameId = bossEvent->getNameId();
    percent = bossEvent->getPercent();
    color = bossEvent->getColor();
    overlay = bossEvent->getOverlay();
    darkenScreen = bossEvent->shouldDarkenScreen();
    playMusic = bossEvent->shouldPlayBossMusic();
    createWorldFog = bossEvent->shouldCreateWorldFog();
}

int ClientboundBossEventPacket::getPacketId()
{
    return 139;
}

void ClientboundBossEventPacket::decodeProperties(int properties)
{
    darkenScreen = (properties & 1) > 0;
    playMusic = (properties & 2) > 0;
    createWorldFog = (properties & 2) > 0;
}

int ClientboundBossEventPacket::encodeProperties()
{
    int properties = 0;

    if (darkenScreen)
    {
        properties |= 1;
    }

    if (playMusic)
    {
        properties |= 2;
    }

    if (createWorldFog)
    {
        properties |= 2;
    }

    return properties;
}

void ClientboundBossEventPacket::read(DataInputStream* input)
{
    id = input->readUTF();
    operation = static_cast<Operation>(input->readUnsignedByte());

    switch (operation)
    {
    case ADD:
        name = input->readUTF();
        nameId = input->readInt();
        percent = input->readFloat();
        color = static_cast<BossEvent::BossBarColor>(input->readUnsignedByte());
        overlay = static_cast<BossEvent::BossBarOverlay>(input->readUnsignedByte());
        decodeProperties(input->readUnsignedByte());
        break;

    case UPDATE_PROGRESS:
        percent = input->readFloat();
        break;

    case UPDATE_NAME:
        name = input->readUTF();
        break;

    case UPDATE_STYLE:
        color = static_cast<BossEvent::BossBarColor>(input->readUnsignedByte());
        overlay = static_cast<BossEvent::BossBarOverlay>(input->readUnsignedByte());
        break;

    case UPDATE_PROPERTIES:
        decodeProperties(input->readUnsignedByte());
        break;

    default:
        break;
    }
}

void ClientboundBossEventPacket::write(DataOutputStream* output)
{
    output->writeUTF(id);
    output->writeByte((unsigned char)operation);

    switch (operation)
    {
    case ADD:
        output->writeUTF(name);
        output->writeInt(nameId);
        output->writeFloat(percent);
        output->writeByte((unsigned char)color);
        output->writeByte((unsigned char)overlay);
        output->writeByte(encodeProperties());
        break;

    case UPDATE_PROGRESS:
        output->writeFloat(percent);
        break;

    case UPDATE_NAME:
        output->writeUTF(name);
        break;

    case UPDATE_STYLE:
        output->writeByte((unsigned char)color);
        output->writeByte((unsigned char)overlay);
        break;

    case UPDATE_PROPERTIES:
        output->writeByte(encodeProperties());
        break;

    default:
        break;
    }
}

void ClientboundBossEventPacket::handle(PacketListener* listener)
{
    listener->handleBossUpdate(shared_from_this());
}

std::wstring ClientboundBossEventPacket::getId()
{
    return id;
}

ClientboundBossEventPacket::Operation ClientboundBossEventPacket::getOperation()
{
    return operation;
}

std::wstring ClientboundBossEventPacket::getName()
{
    return name;
}

int ClientboundBossEventPacket::getNameId()
{
    return nameId;
}

float ClientboundBossEventPacket::getPercent()
{
    return percent;
}

BossEvent::BossBarColor ClientboundBossEventPacket::getColor()
{
    return color;
}

BossEvent::BossBarOverlay ClientboundBossEventPacket::getOverlay()
{
    return overlay;
}

bool ClientboundBossEventPacket::shouldDarkenScreen()
{
    return darkenScreen;
}

bool ClientboundBossEventPacket::shouldPlayMusic()
{
    return playMusic;
}

bool ClientboundBossEventPacket::shouldCreateWorldFog()
{
    return createWorldFog;
}

int ClientboundBossEventPacket::getEstimatedSize()
{
    switch (operation)
    {
    case REMOVE:
        return id.size() * 3 + 1;

    case UPDATE_PROGRESS:
        return id.size() * 3 + 5;

    case UPDATE_NAME:
        return (int)(id.size() * 3) + (int)(name.size() * 3) + 1;

    case UPDATE_STYLE:
        return id.size() * 3 + 3;

    case UPDATE_PROPERTIES:
        return id.size() * 3 + 2;

    default:
        return id.size() * 3 + 12;
    }
}
