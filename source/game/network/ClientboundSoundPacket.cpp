#include "game/network/ClientboundSoundPacket.hpp"

#include <climits>

#include "game/client/audio/SoundEvent.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"

ClientboundSoundPacket::ClientboundSoundPacket()
{
    sound = NULL;
    source = SoundSource::eSoundSource_Neutral;
    x = 0;
    y = INT_MAX;
    z = 0;
    volume = 0.0f;
    pitch = 0.0f;
    soundInstance = eSoundInstances_None;
    entityId = -1;
    playUI = false;
    looping = false;
}

ClientboundSoundPacket::ClientboundSoundPacket(const SoundEvent* sound, SoundSource::ESoundSource source, double x,
                                               double y, double z, float volume, float pitch)
{
    this->sound = sound;
    this->source = source;
    this->x = (int)(x * 8.0);
    this->y = (int)(y * 8.0);
    this->z = (int)(z * 8.0);
    this->volume = volume;
    this->pitch = pitch;
    soundInstance = eSoundInstances_None;
    entityId = -1;
    playUI = false;
    looping = false;
}

ClientboundSoundPacket::ClientboundSoundPacket(const SoundEvent* sound, float volume, float pitch)
{
    playUI = true;
    looping = false;
    this->volume = volume;
    this->sound = sound;
    this->pitch = pitch;
    entityId = -1;
    soundInstance = eSoundInstances_None;
}

ClientboundSoundPacket::ClientboundSoundPacket(ESoundInstances soundInstance, const SoundEvent* sound, float volume,
                                               float pitch, bool looping, int entityId)
    : sound(sound),
      volume(volume),
      pitch(pitch),
      playUI(false),
      soundInstance((int)soundInstance),
      entityId(entityId)
{
    this->looping = looping;
}

void ClientboundSoundPacket::read(DataInputStream* input)
{
    int id = input->readVarInt();
    sound = SoundEvents::soundEventRegistry.byId(id);
    source = (SoundSource::ESoundSource)input->readVarInt();
    x = input->readInt();
    y = input->readInt();
    z = input->readInt();
    volume = input->readFloat();
    pitch = input->readFloat();
    playUI = input->readBoolean();
    soundInstance = input->readInt();
    looping = input->readBoolean();
    entityId = input->readInt();
}

void ClientboundSoundPacket::write(DataOutputStream* output)
{
    output->writeVarInt(SoundEvents::soundEventRegistry.getId(sound));
    output->writeVarInt((int)source);
    output->writeInt(x);
    output->writeInt(y);
    output->writeInt(z);
    output->writeFloat(volume);
    output->writeFloat(pitch);
    output->writeBoolean(playUI);
    output->writeInt(soundInstance);
    output->writeBoolean(looping);
    output->writeInt(entityId);
}

const SoundEvent* ClientboundSoundPacket::getSound()
{
    return sound;
}
SoundSource::ESoundSource ClientboundSoundPacket::getSource()
{
    return source;
}
double ClientboundSoundPacket::getX()
{
    return (float)x * 0.125f;
}
double ClientboundSoundPacket::getY()
{
    return (float)y * 0.125f;
}
double ClientboundSoundPacket::getZ()
{
    return (float)z * 0.125f;
}
double ClientboundSoundPacket::getVolume()
{
    return (double)volume;
}
double ClientboundSoundPacket::getPitch()
{
    return (double)pitch;
}
bool ClientboundSoundPacket::getPlayUI()
{
    return playUI;
}
int ClientboundSoundPacket::getEntityId()
{
    return entityId;
}
bool ClientboundSoundPacket::getLooping() const
{
    return looping;
}
int ClientboundSoundPacket::getSoundInstance()
{
    return soundInstance;
}

void ClientboundSoundPacket::handle(PacketListener* listener)
{
    listener->handleSoundEvent(shared_from_this());
}

int ClientboundSoundPacket::getEstimatedSize()
{
    return 24;
}
int ClientboundSoundPacket::getPacketId()
{
    return 62;
}
