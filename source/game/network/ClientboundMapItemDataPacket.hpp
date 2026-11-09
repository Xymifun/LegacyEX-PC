#ifndef GAME_NETWORK_CLIENTBOUNDMAPITEMDATAPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDMAPITEMDATAPACKET_HPP

#include <deque>
#include <vector>

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"
#include "game/core/MapDecoration.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class DataInputStream;
class DataOutputStream;
class MapItemSavedData;
class PacketListener;

class ClientboundMapItemDataPacket : public Packet, public boost::enable_shared_from_this<ClientboundMapItemDataPacket>
{
public:
    ClientboundMapItemDataPacket();
    ClientboundMapItemDataPacket(int mapId, unsigned char scale, bool trackingPosition,
                                 const std::vector<MapDecoration>& decorations, arrayWithLength<unsigned char> colors,
                                 int startX, int startY, int width, int height);
    virtual ~ClientboundMapItemDataPacket();

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual bool tryReplaceDuplicatePacket(
        std::deque<boost::shared_ptr<Packet>, std::allocator<boost::shared_ptr<Packet> > >* queuedPackets);

    int getMapId();
    void applyToMap(MapItemSavedData* mapData);
    bool isDuplicatePacket(boost::shared_ptr<ClientboundMapItemDataPacket> packet);
    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new ClientboundMapItemDataPacket()); }
    unsigned char getScale();
    int getWidth();
    int getHeight();
    int getStartX();
    int getStartY();

private:
    int mMapId;
    unsigned char mScale;
    bool mTrackingPosition;
    std::vector<MapDecoration> mDecorations;
    int mStartX;
    int mStartY;
    int mWidth;
    int mHeight;
    arrayWithLength<unsigned char> mColors;
};

#endif
