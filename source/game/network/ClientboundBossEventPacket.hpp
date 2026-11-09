#ifndef GAME_NETWORK_CLIENTBOUNDBOSSEVENTPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDBOSSEVENTPACKET_HPP

#include <stdint.h>
#include <string>

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/core/BossEvent.hpp"
#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;

class ClientboundBossEventPacket : public Packet, public boost::enable_shared_from_this<ClientboundBossEventPacket>
{
public:
    enum Operation
    {
        ADD,
        REMOVE,
        UPDATE_PROGRESS,
        UPDATE_NAME,
        UPDATE_STYLE,
        UPDATE_PROPERTIES
    };

    ClientboundBossEventPacket();
    ClientboundBossEventPacket(Operation operation, BossEvent* bossEvent);

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();

    std::wstring getId();
    Operation getOperation();
    std::wstring getName();
    int getNameId();
    float getPercent();
    BossEvent::BossBarColor getColor();
    BossEvent::BossBarOverlay getOverlay();
    bool shouldDarkenScreen();
    bool shouldPlayMusic();
    bool shouldCreateWorldFog();

    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new ClientboundBossEventPacket()); }

private:
    void decodeProperties(int properties);
    int encodeProperties();

    std::wstring id;
    Operation operation;
    std::wstring name;
    int nameId;
    float percent;
    BossEvent::BossBarColor color;
    BossEvent::BossBarOverlay overlay;
    bool darkenScreen;
    bool playMusic;
    bool createWorldFog;
};

#endif
