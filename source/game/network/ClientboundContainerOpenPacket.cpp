#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/network/ClientboundContainerOpenPacket.hpp"

#include "game/core/CMinecraftApp.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/network/PacketListener.hpp"

ClientboundContainerOpenPacket::ClientboundContainerOpenPacket() : mNameData(L"", L"", true, false)
{
    mContainerId = 0;
    mType = 0;
    mSize = 0;
    mEntityId = 0;
    mHasCustomName = false;
}

ClientboundContainerOpenPacket::ClientboundContainerOpenPacket(int containerId, int type,
                                                               InteractionObject::NameData data)
    : mNameData(L"", L"", true, false)
{
    _init(containerId, type, data, 0, 0);
}

ClientboundContainerOpenPacket::ClientboundContainerOpenPacket(int containerId, int type,
                                                               InteractionObject::NameData data, int size)
    : mNameData(L"", L"", true, false)
{
    _init(containerId, type, data, size, 0);
}

ClientboundContainerOpenPacket::ClientboundContainerOpenPacket(int containerId, int type,
                                                               InteractionObject::NameData data, int size, int entityId)
    : mNameData(L"", L"", true, false)
{
    _init(containerId, type, data, size, entityId);
}

ClientboundContainerOpenPacket::~ClientboundContainerOpenPacket()
{
}

void ClientboundContainerOpenPacket::_init(int containerId, int type, InteractionObject::NameData data, int size,
                                           int entityId)
{
    mContainerId = containerId;
    mType = type;
    mNameData = data;
    mSize = size;
    mEntityId = entityId;
    mHasCustomName = data.name.length() > 0;
}

void ClientboundContainerOpenPacket::handle(PacketListener* listener)
{
    listener->handleContainerOpen(shared_from_this());
}

void ClientboundContainerOpenPacket::read(DataInputStream* input)
{
    mContainerId = input->readByte() & 0xff;
    mType = input->readByte() & 0xff;
    mSize = input->readByte() & 0xff;
    mHasCustomName = input->readBoolean();
    if (mType == HORSE || mType == TRADER_NPC)
    {
        mEntityId = input->readInt();
    }
    if (mHasCustomName)
    {
        mNameData.name = Packet::readUtf(input, 64);
        mNameData.ownerUuid = Packet::readUtf(input, 64);
        mNameData.createdByRestrictedPlayer = input->readBoolean();
        mNameData.createdOnHost = input->readBoolean();
    }
}

void ClientboundContainerOpenPacket::write(DataOutputStream* output)
{
    output->writeByte(mContainerId & 0xff);
    output->writeByte(mType & 0xff);
    output->writeByte(mSize & 0xff);
    output->writeBoolean(mHasCustomName);
    if (mType == HORSE || mType == TRADER_NPC)
    {
        output->writeInt(mEntityId);
    }
    if (mHasCustomName)
    {
        Packet::writeUtf(mNameData.name, output);
        Packet::writeUtf(mNameData.ownerUuid, output);
        output->writeBoolean(mNameData.createdByRestrictedPlayer);
        output->writeBoolean(mNameData.createdOnHost);
    }
}

int ClientboundContainerOpenPacket::getEstimatedSize()
{
    if (mType == 11 || mType == 6)
    {
        return 10;
    }
    return 6;
}

int ClientboundContainerOpenPacket::getContainerId()
{
    return mContainerId;
}

int ClientboundContainerOpenPacket::getType()
{
    return mType;
}

bool ClientboundContainerOpenPacket::hasCustomName()
{
    if (mHasCustomName)
    {
#ifdef MINECRAFT_PC
        bool censored = gApp.IsConsoleChatCensored();
#else
        bool censored = gApp.IsConsoleChatCensored() || mNameData.createdByRestrictedPlayer;
#endif
        if (!censored || gApp.IsUUIDLocal(mNameData.ownerUuid)
            || (CGameNetworkManager::sInstance.IsHost() && mNameData.createdOnHost))
        {
            return true;
        }
    }
    return false;
}

std::wstring ClientboundContainerOpenPacket::getTitle()
{
    if (hasCustomName())
    {
#ifdef MINECRAFT_PC
        bool canShow = !gApp.IsConsoleChatCensored()
#else
        bool canShow = (!gApp.IsConsoleChatCensored() && !mNameData.createdByRestrictedPlayer)
#endif
                       || gApp.IsUUIDLocal(mNameData.ownerUuid)
                       || (CGameNetworkManager::sInstance.IsHost() && mNameData.createdOnHost);
        if (!canShow)
        {
            return L"";
        }
        return mNameData.name;
    }
    return L"";
}

int ClientboundContainerOpenPacket::getSize()
{
    return mSize;
}

int ClientboundContainerOpenPacket::getEntityId()
{
    return mEntityId;
}

uint32_t ClientboundContainerOpenPacket::isContainer()
{
    return mSize > 0;
}

int ClientboundContainerOpenPacket::getPacketId()
{
    return 100;
}
