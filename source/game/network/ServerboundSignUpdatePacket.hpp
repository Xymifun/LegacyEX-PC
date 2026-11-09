#ifndef GAME_NETWORK_SERVERBOUNDSIGNUPDATEPACKET_HPP
#define GAME_NETWORK_SERVERBOUNDSIGNUPDATEPACKET_HPP

#include <string>

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"
#include "game/world/block/BlockPos.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;

class ServerboundSignUpdatePacket : public Packet, public boost::enable_shared_from_this<ServerboundSignUpdatePacket>
{
public:
    ServerboundSignUpdatePacket();
    ServerboundSignUpdatePacket(const BlockPos& pos, std::wstring* lines, bool createdByRestrictedPlayer);
    virtual ~ServerboundSignUpdatePacket();

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);

    BlockPos getPos();
    std::wstring* getLines();
    bool isCreatedByRestrictedPlayer();

    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new ServerboundSignUpdatePacket()); }

private:
    BlockPos pos;
    std::wstring* lines;
    bool createdByRestrictedPlayer;
};

#endif
