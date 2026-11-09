#include "game/network/ClientboundExplodePacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"
#include "game/util/Vec3.hpp"

ClientboundExplodePacket::ClientboundExplodePacket()
{
    x = 0.0;
    y = 0.0;
    z = 0.0;
    power = 0.0f;
    knockbackOnly = false;
    knockbackX = 0.0f;
    knockbackY = 0.0f;
    knockbackZ = 0.0f;
}

ClientboundExplodePacket::ClientboundExplodePacket(double x, double y, double z, float power,
                                                   Explosion::ToBlowSet* toBlow, Vec3* knockback, bool knockbackOnly)
{
    this->x = x;
    this->y = y;
    this->z = z;
    this->power = power;
    this->knockbackOnly = knockbackOnly;
    if (toBlow != NULL)
    {
        this->toBlow.assign(toBlow->begin(), toBlow->end());
    }
    if (knockback != NULL)
    {
        knockbackX = (float)knockback->x;
        knockbackY = (float)knockback->y;
        knockbackZ = (float)knockback->z;
    }
}

void ClientboundExplodePacket::read(DataInputStream* input)
{
    knockbackOnly = input->readBoolean();
    if (!knockbackOnly)
    {
        x = input->readFloat();
        y = input->readFloat();
        z = input->readFloat();
        power = input->readFloat();
        int count = input->readInt();
        int baseX = (int)x;
        int baseY = (int)y;
        int baseZ = (int)z;
        for (int i = 0; i < count; ++i)
        {
            int blockX = (signed char)input->readByte() + baseX;
            int blockY = (signed char)input->readByte() + baseY;
            int blockZ = (signed char)input->readByte() + baseZ;
            toBlow.push_back(BlockPos(blockX, blockY, blockZ));
        }
    }
    knockbackX = input->readFloat();
    knockbackY = input->readFloat();
    knockbackZ = input->readFloat();
}

void ClientboundExplodePacket::write(DataOutputStream* output)
{
    output->writeBoolean(knockbackOnly);
    if (!knockbackOnly)
    {
        output->writeFloat((float)x);
        output->writeFloat((float)y);
        output->writeFloat((float)z);
        output->writeFloat(power);
        output->writeInt((int)toBlow.size());
        int baseX = (int)x;
        int baseY = (int)y;
        int baseZ = (int)z;
        for (std::vector<BlockPos, std::allocator<BlockPos> >::const_iterator it = toBlow.begin(); it != toBlow.end();
             ++it)
        {
            int offsetX = it->x - baseX;
            int offsetY = it->y - baseY;
            int offsetZ = it->z - baseZ;
            output->writeByte((unsigned char)offsetX);
            output->writeByte((unsigned char)offsetY);
            output->writeByte((unsigned char)offsetZ);
        }
    }
    output->writeFloat(knockbackX);
    output->writeFloat(knockbackY);
    output->writeFloat(knockbackZ);
}

void ClientboundExplodePacket::handle(PacketListener* listener)
{
    listener->handleExplosion(shared_from_this());
}

int ClientboundExplodePacket::getEstimatedSize()
{
    return (int)toBlow.size() * 3 + 0x2d;
}

double ClientboundExplodePacket::getX()
{
    return x;
}

double ClientboundExplodePacket::getY()
{
    return y;
}

double ClientboundExplodePacket::getZ()
{
    return z;
}

float ClientboundExplodePacket::getPower()
{
    return power;
}

std::vector<BlockPos, std::allocator<BlockPos> >* ClientboundExplodePacket::getToBlow()
{
    return &toBlow;
}

double ClientboundExplodePacket::getKnockbackX()
{
    return knockbackX;
}

double ClientboundExplodePacket::getKnockbackY()
{
    return knockbackY;
}

double ClientboundExplodePacket::getKnockbackZ()
{
    return knockbackZ;
}

int ClientboundExplodePacket::getPacketId()
{
    return 60;
}
