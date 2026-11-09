#include "game/world/block/BlockPos.hpp"

#include "game/world/block/MutableBlockPosIterator.hpp"

#include "game/world/block/BlockPosIterator.hpp"

#include "game/core/Rotation.hpp"

#include <algorithm>

#include <boost/shared_ptr.hpp>

#include "game/core/Direction.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/Vec3.hpp"
#include "game/util/Vec3i.hpp"

const BlockPos BlockPos::ZERO(0, 0, 0);

const int BlockPos::PACKED_X_LENGTH = 1 + Mth::log2(Mth::smallestEncompassingPowerOfTwo(30000000));
const int BlockPos::PACKED_Z_LENGTH = BlockPos::PACKED_X_LENGTH;
const int BlockPos::PACKED_Y_LENGTH = 64 - BlockPos::PACKED_X_LENGTH - BlockPos::PACKED_Z_LENGTH;
const long long BlockPos::PACKED_X_MASK = (1LL << BlockPos::PACKED_X_LENGTH) - 1LL;
const long long BlockPos::PACKED_Y_MASK = (1LL << BlockPos::PACKED_Y_LENGTH) - 1LL;
const long long BlockPos::PACKED_Z_MASK = (1LL << BlockPos::PACKED_Z_LENGTH) - 1LL;
const int BlockPos::Y_OFFSET = BlockPos::PACKED_Z_LENGTH;
const int BlockPos::X_OFFSET = BlockPos::PACKED_Y_LENGTH + BlockPos::PACKED_Z_LENGTH;

BlockPos::BlockPos() : Vec3i(0, 0, 0)
{
}

BlockPos::BlockPos(int x, int y, int z) : Vec3i(x, y, z)
{
}

BlockPos::BlockPos(double x, double y, double z) : Vec3i(x, y, z)
{
}

BlockPos::BlockPos(const boost::shared_ptr<Entity>& entity) : Vec3i(entity->mX, entity->mY, entity->mZ)
{
}

BlockPos::BlockPos(Entity* entity) : Vec3i(entity->mX, entity->mY, entity->mZ)
{
}

BlockPos::BlockPos(Vec3* pos) : Vec3i(pos->x, pos->y, pos->z)
{
}

BlockPos::BlockPos(const Vec3i& other) : Vec3i(other.x, other.y, other.z)
{
}

bool BlockPos::equals(const BlockPos& other) const
{
    if (x == other.x && y == other.y && z == other.z)
    {
        return true;
    }

    return false;
}

bool BlockPos::equals(const BlockPos* other) const
{
    if (this != other && (other == NULL || x != other->x || y != other->y || z != other->z))
    {
        return false;
    }

    return true;
}

int BlockPos::hashCode() const
{
    return (y + z * 31) * 31 + x;
}

int BlockPos::compareTo(const BlockPos& other) const
{
    if (y != other.y)
    {
        return y < other.y ? -1 : 1;
    }

    if (z != other.z)
    {
        return z < other.z ? -1 : 1;
    }

    if (x != other.x)
    {
        return x < other.x ? -1 : 1;
    }

    return 0;
}

int BlockPos::randomSeed() const
{
    int seed = x * 3129871 ^ z * 116129781 ^ y;

    return seed * seed * 42317861 + seed * 11;
}

BlockPos BlockPos::offset(double dx, double dy, double dz) const
{
    return BlockPos((double)x + dx, (double)y + dy, (double)z + dz);
}

BlockPos BlockPos::offset(int dx, int dy, int dz) const
{
    return BlockPos(x + dx, y + dy, z + dz);
}

BlockPos BlockPos::offset(const Vec3i& delta) const
{
    return offset(delta.x, delta.y, delta.z);
}

BlockPos BlockPos::subtract(const Vec3i& delta) const
{
    return offset(-delta.x, -delta.y, -delta.z);
}

BlockPos BlockPos::multiply(int scale) const
{
    if (scale == 1)
    {
        return *this;
    }

    if (scale == 0)
    {
        return ZERO;
    }

    return BlockPos(x * scale, y * scale, z * scale);
}

BlockPos BlockPos::relative(const Direction* direction, int steps) const
{
    return BlockPos(x + direction->getStepX() * steps, y + direction->getStepY() * steps,
                    z + direction->getStepZ() * steps);
}

BlockPos BlockPos::above(int steps) const
{
    return relative(Direction::UP, steps);
}

BlockPos BlockPos::above() const
{
    return above(1);
}

BlockPos BlockPos::below(int steps) const
{
    return relative(Direction::DOWN, steps);
}

BlockPos BlockPos::below() const
{
    return below(1);
}

BlockPos BlockPos::north(int steps) const
{
    return relative(Direction::NORTH, steps);
}

BlockPos BlockPos::north() const
{
    return north(1);
}

BlockPos BlockPos::south(int steps) const
{
    return relative(Direction::SOUTH, steps);
}

BlockPos BlockPos::south() const
{
    return south(1);
}

BlockPos BlockPos::west(int steps) const
{
    return relative(Direction::WEST, steps);
}

BlockPos BlockPos::west() const
{
    return west(1);
}

BlockPos BlockPos::east(int steps) const
{
    return relative(Direction::EAST, steps);
}

BlockPos BlockPos::east() const
{
    return east(1);
}

BlockPos BlockPos::relative(const Direction* direction) const
{
    return relative(direction, 1);
}

BlockPos BlockPos::relative(Direction::Axis* axis, int steps) const
{
    if (axis == Direction::Axis::X)
    {
        return offset(steps, 0, 0);
    }

    if (axis == Direction::Axis::Y)
    {
        return offset(0, steps, 0);
    }

    return offset(0, 0, steps);
}

BlockPos BlockPos::rotate(Rotation* rotation) const
{
    switch (rotation->getType())
    {
    default:
        return *this;

    case Rotation::eTYPE_Clockwise90:
        return BlockPos(-z, y, x);

    case Rotation::eTYPE_Clockwise180:
        return BlockPos(-x, y, -z);

    case Rotation::eTYPE_CounterClockwise90:
        return BlockPos(z, y, -x);
    }
}

long long BlockPos::compressBlockPos() const
{
    return (((long long)x & PACKED_X_MASK) << X_OFFSET) | (((long long)y & PACKED_Y_MASK) << Y_OFFSET)
           | ((long long)z & PACKED_Z_MASK);
}

int BlockPos::getRandomSeed() const
{
    int seed = Mth::fastFloor(x) * 3129871 ^ Mth::fastFloor(z) * 116129781 ^ Mth::fastFloor(y);

    return seed * seed * 42317861 + seed * 11;
}

BlockPos BlockPos::cross(const Vec3i& other) const
{
    return BlockPos(y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x);
}

BlockPos BlockPos::decompressBlockPos(long long packed)
{
    int x = (int)(packed << (64 - X_OFFSET - PACKED_X_LENGTH) >> (64 - PACKED_X_LENGTH));
    int y = (int)(packed << (64 - Y_OFFSET - PACKED_Y_LENGTH) >> (64 - PACKED_Y_LENGTH));
    int z = (int)(packed << (64 - PACKED_Z_LENGTH) >> (64 - PACKED_Z_LENGTH));

    return BlockPos(x, y, z);
}

BlockPos* BlockPos::decompressBlockPosAsPointer(long long packed)
{
    int x = (int)(packed << (64 - X_OFFSET - PACKED_X_LENGTH) >> (64 - PACKED_X_LENGTH));
    int y = (int)(packed << (64 - Y_OFFSET - PACKED_Y_LENGTH) >> (64 - PACKED_Y_LENGTH));
    int z = (int)(packed << (64 - PACKED_Z_LENGTH) >> (64 - PACKED_Z_LENGTH));

    return new BlockPos(x, y, z);
}

Iterator<BlockPos*>* BlockPos::between(const BlockPos& from, const BlockPos& to)
{
    return new BlockPosIterator(from, to);
}

Iterator<MutableBlockPos*>* BlockPos::mutablesBetween(int minX, int minY, int minZ, int maxX, int maxY, int maxZ)
{
    return new MutableBlockPosIterator(minX, minY, minZ, maxX, maxY, maxZ);
}

Iterator<MutableBlockPos*>* BlockPos::mutablesBetween(const BlockPos& from, const BlockPos& to)
{
    int minX = to.x < from.x ? to.x : from.x;
    int minY = to.y < from.y ? to.y : from.y;
    int minZ = to.z < from.z ? to.z : from.z;
    int maxX = to.x;
    if (maxX < from.x)
    {
        maxX = from.x;
    }
    int maxY = from.y < to.y ? to.y : from.y;
    int maxZ = from.z < to.z ? to.z : from.z;

    return mutablesBetween(minX, minY, minZ, maxX, maxY, maxZ);
}

Iterator<MutableBlockPos*>* BlockPos::pooledMutablesBetween(const BlockPos& from, const BlockPos& to)
{
    return pooledMutablesBetween(std::min(from.x, to.x), std::min(from.y, to.y), std::min(from.z, to.z),
                                 std::max(from.x, to.x), std::max(from.y, to.y), std::max(from.z, to.z));
}

Iterator<MutableBlockPos*>* BlockPos::pooledMutablesBetween(int minX, int minY, int minZ, int maxX, int maxY, int maxZ)
{
    return new MutableBlockPosIterator(minX, minY, minZ, maxX, maxY, maxZ);
}

bool BlockPos::operator==(const BlockPos& other) const
{
    return equals(other);
}

bool BlockPos::operator!=(const BlockPos& other) const
{
    return !equals(other);
}

BlockPos BlockPos::operator+(const BlockPos& other) const
{
    return BlockPos(x + other.x, y + other.y, z + other.z);
}

BlockPos BlockPos::operator-(const BlockPos& other) const
{
    return BlockPos(x - other.x, y - other.y, z - other.z);
}

Vec3* BlockPos::operator-(const Vec3* other) const
{
    return Vec3::newTemp((double)x - other->x, (double)y - other->y, (double)z - other->z);
}
