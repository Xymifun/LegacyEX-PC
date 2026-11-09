#include "game/network/ClientboundLevelParticlesPacket.hpp"

#include <cstring>

#include "game/client/render/ParticleType.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/Packet.hpp"
#include "game/network/PacketListener.hpp"

ClientboundLevelParticlesPacket::ClientboundLevelParticlesPacket()
{
    type = NULL;
    x = 0.0f;
    y = 0.0f;
    z = 0.0f;
    xDist = 0.0f;
    yDist = 0.0f;
    zDist = 0.0f;
    maxSpeed = 0.0f;
    count = 0;
    overrideLimiter = false;
    params = arrayWithLength<int>();
    hasAux = false;
    xAux = 0.0f;
    yAux = 0.0f;
    zAux = 0.0f;
}

ClientboundLevelParticlesPacket::ClientboundLevelParticlesPacket(const ParticleType* type, bool overrideLimiter,
                                                                 float x, float y, float z, float xDist, float yDist,
                                                                 float zDist, float maxSpeed, int count,
                                                                 arrayWithLength<int> params)
{
    Init(type, overrideLimiter, x, y, z, xDist, yDist, zDist, maxSpeed, count, params, false, 0.0f, 0.0f, 0.0f);
}

ClientboundLevelParticlesPacket::ClientboundLevelParticlesPacket(const ParticleType* type, bool overrideLimiter,
                                                                 float x, float y, float z, float xDist, float yDist,
                                                                 float zDist, float maxSpeed, int count,
                                                                 arrayWithLength<int> params, float xAux, float yAux,
                                                                 float zAux)
{
    Init(type, overrideLimiter, x, y, z, xDist, yDist, zDist, maxSpeed, count, params, true, xAux, yAux, zAux);
}

ClientboundLevelParticlesPacket::~ClientboundLevelParticlesPacket()
{
    if (params.data != NULL)
    {
        delete[] params.data;
    }
}

void ClientboundLevelParticlesPacket::Init(const ParticleType* type, bool overrideLimiter, float x, float y, float z,
                                           float xDist, float yDist, float zDist, float maxSpeed, int count,
                                           arrayWithLength<int> params, bool hasAux, float xAux, float yAux, float zAux)
{
    this->type = type;
    this->overrideLimiter = overrideLimiter;
    this->x = x;
    this->y = y;
    this->z = z;
    this->xDist = xDist;
    this->yDist = yDist;
    this->zDist = zDist;
    this->maxSpeed = maxSpeed;
    this->count = count;

    this->params.length = params.length;

    if (params.length > 0)
    {
        this->params.data = new int[params.length];
        memcpy(this->params.data, params.data, params.length * sizeof(int));
    }
    else
    {
        this->params.data = NULL;
    }

    this->hasAux = hasAux;
    this->xAux = xAux;
    this->yAux = yAux;
    this->zAux = zAux;
}

int ClientboundLevelParticlesPacket::getPacketId()
{
    return 63;
}

void ClientboundLevelParticlesPacket::read(DataInputStream* input)
{
    type = ParticleType::byId((ePARTICLE_TYPE)input->readInt());

    if (type == NULL)
    {
        type = PARTICLE_BARRIER;
    }

    overrideLimiter = input->readBoolean();
    x = input->readFloat();
    y = input->readFloat();
    z = input->readFloat();
    xDist = input->readFloat();
    yDist = input->readFloat();
    zDist = input->readFloat();
    maxSpeed = input->readFloat();
    count = input->readInt();

    int paramCount = type->getParamCount();

    if (paramCount > 0)
    {
        params = arrayWithLength<int>(paramCount, true);

        for (int i = 0; i < paramCount; i++)
        {
            params.data[i] = input->readInt();
        }
    }

    hasAux = input->readBoolean();

    if (hasAux)
    {
        xAux = input->readFloat();
        yAux = input->readFloat();
        zAux = input->readFloat();
    }
}

void ClientboundLevelParticlesPacket::write(DataOutputStream* output)
{
    output->writeInt(type->getId());
    output->writeBoolean(overrideLimiter);
    output->writeFloat(x);
    output->writeFloat(y);
    output->writeFloat(z);
    output->writeFloat(xDist);
    output->writeFloat(yDist);
    output->writeFloat(zDist);
    output->writeFloat(maxSpeed);
    output->writeInt(count);

    unsigned int paramCount = (unsigned int)type->getParamCount();

    for (unsigned int i = 0; i < paramCount; i++)
    {
        int param = i < params.length ? params.data[i] : 0;
        output->writeInt(param);
    }

    output->writeBoolean(hasAux);

    if (hasAux)
    {
        output->writeFloat(xAux);
        output->writeFloat(yAux);
        output->writeFloat(zAux);
    }
}

void ClientboundLevelParticlesPacket::handle(PacketListener* listener)
{
    listener->handleParticleEvent(shared_from_this());
}

int ClientboundLevelParticlesPacket::getEstimatedSize()
{
    return 64;
}

float ClientboundLevelParticlesPacket::getX()
{
    return x;
}

float ClientboundLevelParticlesPacket::getY()
{
    return y;
}

float ClientboundLevelParticlesPacket::getZ()
{
    return z;
}

float ClientboundLevelParticlesPacket::getXDist()
{
    return xDist;
}

float ClientboundLevelParticlesPacket::getYDist()
{
    return yDist;
}

float ClientboundLevelParticlesPacket::getZDist()
{
    return zDist;
}

float ClientboundLevelParticlesPacket::getMaxSpeed()
{
    return maxSpeed;
}

int ClientboundLevelParticlesPacket::getCount()
{
    return count;
}

const ParticleType* ClientboundLevelParticlesPacket::getType()
{
    return type;
}

bool ClientboundLevelParticlesPacket::isOverrideLimiter()
{
    return overrideLimiter;
}

arrayWithLength<int> ClientboundLevelParticlesPacket::getParams()
{
    return params;
}

float ClientboundLevelParticlesPacket::getXAux()
{
    return xAux;
}

float ClientboundLevelParticlesPacket::getYAux()
{
    return yAux;
}

float ClientboundLevelParticlesPacket::getZAux()
{
    return zAux;
}

bool ClientboundLevelParticlesPacket::getHasAux()
{
    return hasAux;
}
