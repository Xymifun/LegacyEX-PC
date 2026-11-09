#include "game/network/ServerboundPlayerActionPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/Direction.hpp"
#include "game/network/PacketListener.hpp"

ServerboundPlayerActionPacket::ServerboundPlayerActionPacket() : mPos(), mDirection(NULL), mUseTime(0)
{
}

ServerboundPlayerActionPacket::ServerboundPlayerActionPacket(Action action, const BlockPos& pos,
                                                             const Direction* direction, int useTime)
{
    mAction = action;
    mPos = pos;
    mDirection = direction;
    mUseTime = useTime;
}

void ServerboundPlayerActionPacket::read(DataInputStream* input)
{
    mAction = static_cast<Action>(input->readInt());
    mPos = input->readBlockPos();
    mDirection = Direction::from3DDataValue(input->readUnsignedByte());
    mUseTime = input->readInt();
}

void ServerboundPlayerActionPacket::write(DataOutputStream* output)
{
    output->writeInt(static_cast<int>(mAction));
    output->writeBlockPos(mPos);
    output->writeByte(static_cast<unsigned char>(mDirection->get3DDataValue()));
    output->writeInt(mUseTime);
}

void ServerboundPlayerActionPacket::handle(PacketListener* listener)
{
    listener->handlePlayerAction(shared_from_this());
}

int ServerboundPlayerActionPacket::getEstimatedSize()
{
    return 11;
}

int ServerboundPlayerActionPacket::getPacketId()
{
    return 14;
}

BlockPos ServerboundPlayerActionPacket::getPos()
{
    return mPos;
}

const Direction* ServerboundPlayerActionPacket::getDirection()
{
    return mDirection;
}

ServerboundPlayerActionPacket::Action ServerboundPlayerActionPacket::getAction()
{
    return mAction;
}

int ServerboundPlayerActionPacket::getUseTime() const
{
    return mUseTime;
}
