#ifndef GAME_NETWORK_BLOCKREGIONUPDATEPACKET_HPP
#define GAME_NETWORK_BLOCKREGIONUPDATEPACKET_HPP

#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "game/network/Packet.hpp"
#include "game/world/chunk/LevelChunk.hpp"

class DataInputStream;
class DataOutputStream;
class ClientPacketListener;
class Level;
class PacketListener;

class BlockRegionUpdatePacket : public Packet, public boost::enable_shared_from_this<BlockRegionUpdatePacket>
{
public:
    BlockRegionUpdatePacket();
    BlockRegionUpdatePacket(int x, int y, int z, int width, int height, int depth, Level* level, bool hasBlockChanges,
                            const std::vector<short>* blockChanges);
    virtual ~BlockRegionUpdatePacket();

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();

    void SetChanges(std::vector<LevelChunk::BlockChange>* changes);
    std::vector<LevelChunk::BlockChange>* GetChanges() const;

    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new BlockRegionUpdatePacket()); }

private:
    friend class ClientPacketListener;

    int mX;
    int mY;
    int mZ;
    int mWidth;
    int mHeight;
    int mDepth;
    arrayWithLength<unsigned char> mData;
    int mDimension;
    bool mFullChunk;
    bool mHasBlockChanges;
    std::vector<LevelChunk::BlockChange>* mChanges;
    int mSize;
};

#endif
