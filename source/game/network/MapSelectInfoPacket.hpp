#ifndef GAME_NETWORK_MAPSELECTINFOPACKET_HPP
#define GAME_NETWORK_MAPSELECTINFOPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;

class MapSelectInfoPacket : public Packet, public boost::enable_shared_from_this<MapSelectInfoPacket>
{
public:
    MapSelectInfoPacket();
    MapSelectInfoPacket(arrayWithLength<unsigned char> data, bool ownsData);
    virtual ~MapSelectInfoPacket();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();
    virtual int getPacketId();
    arrayWithLength<unsigned char> getData() const { return mData; }
    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new MapSelectInfoPacket()); }

private:
    friend class ClientPacketListener;
    bool mOwnsData;
    unsigned int mSize;
    arrayWithLength<unsigned char> mData;
};

#endif
