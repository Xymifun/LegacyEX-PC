#include "game/network/UpdateGameRuleProgressPacket.hpp"

#include <cstring>

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"

UpdateGameRuleProgressPacket::UpdateGameRuleProgressPacket()
{
    mDescription = L"";
    mItemId = -1;
    mRuleType = ConsoleGameRules::eGameRuleType_LevelRules;
    mAuxValue = 0;
    mDataTag = 0;
}

UpdateGameRuleProgressPacket::UpdateGameRuleProgressPacket(ConsoleGameRules::EGameRuleType ruleType,
                                                           const std::wstring& description, int itemId, int auxValue,
                                                           int dataTag, void* payload, int payloadLength)
{
    mRuleType = ruleType;
    mDescription = description;
    mItemId = itemId;
    mAuxValue = auxValue;
    mDataTag = dataTag;
    if (payloadLength > 0)
    {
        mPayload = arrayWithLength<unsigned char>(static_cast<unsigned int>(payloadLength), true);
        std::memcpy(mPayload.data, payload, payloadLength);
    }
    else
    {
        mPayload = arrayWithLength<unsigned char>();
    }
}

int UpdateGameRuleProgressPacket::getPacketId()
{
    return 158;
}

void UpdateGameRuleProgressPacket::read(DataInputStream* input)
{
    mRuleType = static_cast<ConsoleGameRules::EGameRuleType>(input->readInt());
    mDescription = Packet::readUtf(input, 64);
    mItemId = input->readInt();
    mAuxValue = input->readByte();
    mDataTag = input->readInt();
    int payloadLength = input->readInt();
    if (payloadLength > 0)
    {
        mPayload = arrayWithLength<unsigned char>(static_cast<unsigned int>(payloadLength), true);
        input->readBytes(mPayload);
    }
    else
    {
        mPayload = arrayWithLength<unsigned char>();
    }
}

void UpdateGameRuleProgressPacket::write(DataOutputStream* output)
{
    output->writeInt(static_cast<int>(mRuleType));
    Packet::writeUtf(mDescription, output);
    output->writeInt(mItemId);
    output->writeByte(mAuxValue);
    output->writeInt(mDataTag);
    output->writeInt(static_cast<int>(mPayload.length));
    output->writeBytes(mPayload);
}

void UpdateGameRuleProgressPacket::handle(PacketListener* listener)
{
    listener->handleUpdateGameRuleProgressPacket(shared_from_this());
}

int UpdateGameRuleProgressPacket::getEstimatedSize()
{
    return (int)mDescription.length() + 4 + mPayload.length;
}
