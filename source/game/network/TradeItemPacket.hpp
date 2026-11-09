#ifndef GAME_NETWORK_TRADEITEMPACKET_HPP
#define GAME_NETWORK_TRADEITEMPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;

class TradeItemPacket : public Packet, public boost::enable_shared_from_this<TradeItemPacket>
{
public:
    TradeItemPacket();
    TradeItemPacket(int containerId, int itemIndex);
    virtual void handle(PacketListener* listener);
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual int getEstimatedSize();
    virtual int getPacketId();
    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new TradeItemPacket()); }

public:
    int containerId;
    int itemIndex;
};

#endif
