#ifndef GAME_NETWORK_CLIENTBOUNDADDENTITYPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDADDENTITYPACKET_HPP

#include <string>

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"
#include "game/world/block/BlockPos.hpp"

class DataInputStream;
class DataOutputStream;
class Entity;
class PacketListener;

class ClientboundAddEntityPacket : public Packet, public boost::enable_shared_from_this<ClientboundAddEntityPacket>
{
public:
    enum
    {
        BOAT = 1,
        ITEM = 2,
        AREA_EFFECT_CLOUD = 3,
        MINECART = 10,
        PRIMED_TNT = 50,
        ENDER_CRYSTAL = 51,
        ARROW = 60,
        SNOWBALL = 61,
        EGG = 62,
        FIREBALL = 63,
        SMALL_FIREBALL = 64,
        THROWN_ENDERPEARL = 65,
        WITHER_SKULL = 66,
        SHULKER_BULLET = 67,
        LLAMA_SPIT = 68,
        FALLING_ALT = 69,
        FALLING = 70,
        ITEM_FRAME = 71,
        EYE_OF_ENDER = 72,
        POTION = 73,
        EXPERIENCE_BOTTLE_ALT = 74,
        EXPERIENCE_BOTTLE = 75,
        FIREWORKS = 76,
        LEASH_KNOT = 77,
        ARMOR_STAND = 78,
        EVOCATION_FANGS = 79,
        FISHING_HOOK_FIRST = 80,
        FISHING_HOOK = 90,
        SPECTRAL_ARROW = 91,
        DRAGON_FIREBALL_ALT = 92,
        DRAGON_FIREBALL = 93,
        TRIDENT = 94
    };

    ClientboundAddEntityPacket();
    ClientboundAddEntityPacket(boost::shared_ptr<Entity> entity, int type, int data);
    ClientboundAddEntityPacket(boost::shared_ptr<Entity> entity, int type, int yRot, int xRot, int xa, int ya, int za);
    ClientboundAddEntityPacket(boost::shared_ptr<Entity> entity, int type, int data, int yRot, int xRot, int xa, int ya,
                               int za);
    ClientboundAddEntityPacket(boost::shared_ptr<Entity> entity, int type, int data, const BlockPos& pos, int yRot,
                               int xRot, int xa, int ya, int za);

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();

    void _init(boost::shared_ptr<Entity> entity, int type, int data, int xa, int ya, int za, int yRot, int xRot);
    int getId();
    std::wstring getUUID();
    int getX();
    int getY();
    int getZ();
    double getXa();
    double getYa();
    double getZa();
    unsigned char getxRot();
    unsigned char getyRot();
    int getType();
    int getData();
    void setXa(int xa);
    void setYa(int ya);
    void setZa(int za);
    void setData(int data);

    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new ClientboundAddEntityPacket()); }

private:
    int mId;
    std::wstring mUUID;
    int mX;
    int mY;
    int mZ;
    int mXa;
    int mYa;
    int mZa;
    int mType;
    int mData;
    unsigned char mYRot;
    unsigned char mXRot;
};

#endif
