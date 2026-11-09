#include "game/world/chunk/ChunkPos.hpp"

#include <boost/functional/hash.hpp>
#include "game/entity/mob/Entity.hpp"

ChunkPos::ChunkPos(int x, int z) : x(x), z(z)
{
}

ChunkPos::ChunkPos(const BlockPos& pos) : x(pos.x >> 4), z(pos.z >> 4)
{
}

long long ChunkPos::asLong(int x, int z)
{
    return (static_cast<long long>(x) & 0xFFFFFFFFLL) | ((static_cast<long long>(z) & 0xFFFFFFFFLL) << 32);
}

std::size_t ChunkPos::hashCode() const
{
    static boost::hash<long long> hasher = boost::hash<long long>();
    return hasher(asLong(x, z));
}

double ChunkPos::distanceToSqr(boost::shared_ptr<Entity> entity)
{
    double dx = static_cast<double>((x << 4) + 8) - entity->mX;
    double dz = static_cast<double>((z << 4) + 8) - entity->mZ;
    return dx * dx + dz * dz;
}

double ChunkPos::distanceToSqr(double x, double z)
{
    double dz2 = static_cast<double>((this->z << 4) + 8) - z;
    double dx2 = static_cast<double>((this->x << 4) + 8) - x;
    return dx2 * dx2 + dz2 * dz2;
}

int ChunkPos::getMiddleBlockX()
{
    return (x << 4) + 8;
}

int ChunkPos::getMiddleBlockZ()
{
    return (z << 4) + 8;
}

int ChunkPos::getMinBlockX() const
{
    return x << 4;
}

int ChunkPos::getMinBlockZ() const
{
    return z << 4;
}

int ChunkPos::getMaxBlockX() const
{
    return (x << 4) + 15;
}

int ChunkPos::getMaxBlockZ() const
{
    return (z << 4) + 15;
}

BlockPos ChunkPos::getMiddleBlockPosition(int y)
{
    return BlockPos(getMiddleBlockX(), y, getMiddleBlockZ());
}

BlockPos ChunkPos::getWorldPosition() const
{
    return BlockPos(x << 4, 0, z << 4);
}

bool ChunkPos::eq_test(const ChunkPos& lhs, const ChunkPos& rhs)
{
    return lhs.x == rhs.x && lhs.z == rhs.z;
}
