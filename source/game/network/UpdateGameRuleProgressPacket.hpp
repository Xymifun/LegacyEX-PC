#ifndef GAME_NETWORK_UPDATEGAMERULEPROGRESSPACKET_HPP
#define GAME_NETWORK_UPDATEGAMERULEPROGRESSPACKET_HPP

#include <string>

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/data/GameRuleSaveInterface.hpp"
#include "game/network/Packet.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;

class UpdateGameRuleProgressPacket : public Packet, public boost::enable_shared_from_this<UpdateGameRuleProgressPacket>
{
public:
    UpdateGameRuleProgressPacket();
    UpdateGameRuleProgressPacket(ConsoleGameRules::EGameRuleType ruleType, const std::wstring& description, int itemId,
                                 int auxValue, int dataTag, void* payload, int payloadLength);

    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new UpdateGameRuleProgressPacket()); }
    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();

private:
    friend class ClientPacketListener;
    ConsoleGameRules::EGameRuleType mRuleType;
    std::wstring mDescription;
    int mItemId;
    int mAuxValue;
    int mDataTag;
    arrayWithLength<unsigned char> mPayload;
};

#endif
