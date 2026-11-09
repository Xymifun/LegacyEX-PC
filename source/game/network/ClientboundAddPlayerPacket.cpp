#include "game/network/ClientboundAddPlayerPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/GameType.hpp"
#include "game/network/PacketListener.hpp"
#include "game/world/generation/Player.hpp"

ClientboundAddPlayerPacket::ClientboundAddPlayerPacket() : mPlayerId(0), mProfileId(0)
{
    mEntityId = 0xffffffff;
    mName = L"";
    mX = 0;
    mY = 0;
    mZ = 0;
    myRot = 0;
    mxRot = 0;
    mPlayerId = 0;
    mPlayerIndex = 0;
    mSkinId = 0;
    mCapeId = 0;
    mPlayerPrivilege = 0;
    mGameType = GameType::NOT_SET;
    mEntityData = boost::shared_ptr<SynchedEntityData>();
    mUnpackedData = NULL;
}

ClientboundAddPlayerPacket::ClientboundAddPlayerPacket(boost::shared_ptr<Player> player, PlayerUID playerId,
                                                       PlayerUID profileId, int x, int y, int z, int yRot, int xRot,
                                                       int yHeadRot)
    : mPlayerId(0),
      mProfileId(0)
{
    mEntityId = player->getId();
    mName = player->getName();
    mX = x;
    mY = y;
    mZ = z;
    myRot = (unsigned char)yRot;
    mxRot = (unsigned char)xRot;
    myHeadRot = (unsigned char)yHeadRot;
    mPlayerId = playerId;
    mProfileId = profileId;
    mPlayerIndex = (unsigned char)player->mPlayerIndex;
    mSkinId = player->mCustomSkin;
    mCapeId = player->mCustomCape;
    mPlayerPrivilege = player->getPlayerGamePrivilege(Player::ePlayerGamePrivilege_All);
    mGameType = player->GetGameType();
    mEntityData = player->getEntityData();
    mUnpackedData = NULL;
}

ClientboundAddPlayerPacket::~ClientboundAddPlayerPacket()
{
    if (mUnpackedData != NULL)
    {
        for (unsigned int i = 0; i < mUnpackedData->size(); ++i)
        {
            delete mUnpackedData->at(i);
        }
        delete mUnpackedData;
    }
}

unsigned int ClientboundAddPlayerPacket::getEntityId()
{
    return mEntityId;
}

PlayerUID ClientboundAddPlayerPacket::getPlayerId()
{
    return mPlayerId;
}

int ClientboundAddPlayerPacket::getX()
{
    return (unsigned int)mX;
}

int ClientboundAddPlayerPacket::getY()
{
    return (unsigned int)mY;
}

int ClientboundAddPlayerPacket::getZ()
{
    return (unsigned int)mZ;
}

unsigned char ClientboundAddPlayerPacket::getyRot()
{
    return myRot;
}

unsigned char ClientboundAddPlayerPacket::getxRot()
{
    return mxRot;
}

std::wstring ClientboundAddPlayerPacket::getName()
{
    return mName;
}

void ClientboundAddPlayerPacket::read(DataInputStream* input)
{
    mEntityId = (unsigned int)input->readInt();
    mName = Packet::readUtf(input, 20);
    mX = input->readInt();
    mY = input->readInt();
    mZ = input->readInt();
    myRot = input->readByte();
    mxRot = input->readByte();
    myHeadRot = input->readByte();
    mPlayerId = input->readPlayerUID();
    mProfileId = input->readPlayerUID();
    mPlayerIndex = input->readByte();
    int skinId = input->readInt();
    mSkinId = *(unsigned int*)&skinId;
    int capeId = input->readInt();
    mCapeId = *(unsigned int*)&capeId;
    int privileges = input->readInt();
    mPlayerPrivilege = *(unsigned int*)&privileges;
    mGameType = GameType::byId(input->readInt());
    MemSect(1);
    mUnpackedData = SynchedEntityData::unpack(input);
    MemSect(0);
}

void ClientboundAddPlayerPacket::write(DataOutputStream* output)
{
    output->writeInt((int)mEntityId);
    Packet::writeUtf(mName, output);
    output->writeInt(mX);
    output->writeInt(mY);
    output->writeInt(mZ);
    output->writeByte(myRot);
    output->writeByte(mxRot);
    output->writeByte(myHeadRot);
    output->writePlayerUID(mPlayerId);
    output->writePlayerUID(mProfileId);
    output->writeByte(mPlayerIndex);
    output->writeInt(mSkinId);
    output->writeInt(mCapeId);
    output->writeInt(mPlayerPrivilege);
    output->writeInt(mGameType->getId());
    mEntityData->packAll(output);
}

void ClientboundAddPlayerPacket::handle(PacketListener* listener)
{
    listener->handleAddPlayer(shared_from_this());
}

int ClientboundAddPlayerPacket::getEstimatedSize()
{
    int size = 0x5c;

    if (mEntityData)
    {
        size += mEntityData->getItemCount() * 8;
    }

    return size;
}

std::vector<DataItem*>* ClientboundAddPlayerPacket::getUnpackedData()
{
    if (mUnpackedData == NULL)
    {
        mUnpackedData = mEntityData->getAll();
    }
    return mUnpackedData;
}

int ClientboundAddPlayerPacket::getPacketId()
{
    return 20;
}
