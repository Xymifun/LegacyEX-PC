#ifndef GAME_NETWORK_CLIENTBOUNDEXPLODEPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDEXPLODEPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>

#include "game/core/Explosion.hpp"
#include "game/network/Packet.hpp"
#include "game/world/block/BlockPos.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;
class Vec3;

class ClientboundExplodePacket : public Packet, public boost::enable_shared_from_this<ClientboundExplodePacket>
{
public:
    ClientboundExplodePacket();
    ClientboundExplodePacket(double x, double y, double z, float power, Explosion::ToBlowSet* toBlow, Vec3* knockback,
                             bool knockbackOnly);

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();

    double getX();
    double getY();
    double getZ();
    float getPower();
    std::vector<BlockPos, std::allocator<BlockPos> >* getToBlow();
    double getKnockbackX();
    double getKnockbackY();
    double getKnockbackZ();

    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new ClientboundExplodePacket()); }

private:
    double x;
    double y;
    double z;
    float power;
    std::vector<BlockPos, std::allocator<BlockPos> > toBlow;

public:
    bool knockbackOnly;

private:
    float knockbackX;
    float knockbackY;
    float knockbackZ;
};

#endif
