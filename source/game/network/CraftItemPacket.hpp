#ifndef GAME_NETWORK_CRAFTITEMPACKET_HPP
#define GAME_NETWORK_CRAFTITEMPACKET_HPP

#include <utility>
#include <vector>

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;

class CraftItemPacket : public Packet, public boost::enable_shared_from_this<CraftItemPacket>
{
public:
    CraftItemPacket();
    CraftItemPacket(int recipe, int craftType, short containerId);
    CraftItemPacket(std::vector<std::pair<int, int> >& stackedItems, int craftType, short containerId);
    CraftItemPacket(std::vector<int>& itemIds, short containerId);
    virtual ~CraftItemPacket();

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();

    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new CraftItemPacket()); }

private:
    friend class ServerGamePacketListenerImpl;

    std::vector<std::pair<int, int> > mStackedItems;
    std::vector<int> mItemIds;
    short mContainerId;
    unsigned char mCraftType;
};

#endif
