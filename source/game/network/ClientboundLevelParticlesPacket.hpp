#ifndef GAME_NETWORK_CLIENTBOUNDLEVELPARTICLESPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDLEVELPARTICLESPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;
class ParticleType;

class ClientboundLevelParticlesPacket : public Packet,
                                        public boost::enable_shared_from_this<ClientboundLevelParticlesPacket>
{
public:
    ClientboundLevelParticlesPacket();
    ClientboundLevelParticlesPacket(const ParticleType* type, bool overrideLimiter, float x, float y, float z,
                                    float xDist, float yDist, float zDist, float maxSpeed, int count,
                                    arrayWithLength<int> params);
    ClientboundLevelParticlesPacket(const ParticleType* type, bool overrideLimiter, float x, float y, float z,
                                    float xDist, float yDist, float zDist, float maxSpeed, int count,
                                    arrayWithLength<int> params, float xAux, float yAux, float zAux);
    virtual ~ClientboundLevelParticlesPacket();

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();

    float getX();
    float getY();
    float getZ();
    float getXDist();
    float getYDist();
    float getZDist();
    float getMaxSpeed();
    int getCount();
    const ParticleType* getType();
    bool isOverrideLimiter();
    arrayWithLength<int> getParams();
    float getXAux();
    float getYAux();
    float getZAux();
    bool getHasAux();

    static boost::shared_ptr<Packet> create()
    {
        return boost::shared_ptr<Packet>(new ClientboundLevelParticlesPacket());
    }

private:
    void Init(const ParticleType* type, bool overrideLimiter, float x, float y, float z, float xDist, float yDist,
              float zDist, float maxSpeed, int count, arrayWithLength<int> params, bool hasAux, float xAux, float yAux,
              float zAux);

    const ParticleType* type;
    float x;
    float y;
    float z;
    float xDist;
    float yDist;
    float zDist;
    float maxSpeed;
    int count;
    bool overrideLimiter;
    arrayWithLength<int> params;
    bool hasAux;
    float xAux;
    float yAux;
    float zAux;
};

#endif
