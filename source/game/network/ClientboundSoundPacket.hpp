#ifndef GAME_NETWORK_CLIENTBOUNDSOUNDPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDSOUNDPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"
#include "game/client/audio/SoundSource.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;
class SoundEvent;

class ClientboundSoundPacket : public Packet, public boost::enable_shared_from_this<ClientboundSoundPacket>
{
public:
    enum ESoundInstances
    {
        eSoundInstances_None = -1,
        eSoundInstances_FollowLocalPlayer = 0,
        eSoundInstances_FollowEntity = 1
    };

    ClientboundSoundPacket();
    ClientboundSoundPacket(const SoundEvent* sound, SoundSource::ESoundSource source, double x, double y, double z,
                           float volume, float pitch);
    ClientboundSoundPacket(const SoundEvent* sound, float volume, float pitch);
    ClientboundSoundPacket(ESoundInstances soundInstance, const SoundEvent* sound, float volume, float pitch,
                           bool looping, int entityId);
    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();
    const SoundEvent* getSound();
    SoundSource::ESoundSource getSource();
    double getX();
    double getY();
    double getZ();
    double getVolume();
    double getPitch();
    bool getPlayUI();
    int getEntityId();
    bool getLooping() const;
    int getSoundInstance();
    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new ClientboundSoundPacket()); }

private:
    const SoundEvent* sound;
    SoundSource::ESoundSource source;
    int x;
    int y;
    int z;
    float volume;
    float pitch;
    bool playUI;
    bool looping;
    int soundInstance;
    int entityId;
};

#endif
