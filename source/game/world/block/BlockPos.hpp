#ifndef GAME_WORLD_BLOCK_BLOCKPOS_HPP
#define GAME_WORLD_BLOCK_BLOCKPOS_HPP

#include <boost/shared_ptr.hpp>

#include "game/core/Direction.hpp"
#include "game/util/Vec3i.hpp"

class Entity;
class MutableBlockPos;
class Rotation;
class Vec3;
class BlockPos;

template <class T> class Iterator;

class BlockPos : public Vec3i
{
public:
    static const BlockPos ZERO;

    BlockPos();
    BlockPos(int x, int y, int z);
    BlockPos(double x, double y, double z);
    BlockPos(const boost::shared_ptr<Entity>& entity);
    BlockPos(Entity* entity);
    BlockPos(Vec3* pos);
    BlockPos(const Vec3i& other);

    bool equals(const BlockPos& other) const;
    bool equals(const BlockPos* other) const;
    int hashCode() const;
    int compareTo(const BlockPos& other) const;
    int randomSeed() const;

    BlockPos offset(double dx, double dy, double dz) const;
    BlockPos offset(int dx, int dy, int dz) const;
    BlockPos offset(const Vec3i& delta) const;
    BlockPos subtract(const Vec3i& delta) const;
    BlockPos multiply(int scale) const;
    BlockPos cross(const Vec3i& other) const;

    BlockPos relative(const Direction* direction, int steps) const;
    BlockPos above(int steps) const;
    BlockPos above() const;
    BlockPos below(int steps) const;
    BlockPos below() const;
    BlockPos north(int steps) const;
    BlockPos north() const;
    BlockPos south(int steps) const;
    BlockPos south() const;
    BlockPos west(int steps) const;
    BlockPos west() const;
    BlockPos east(int steps) const;
    BlockPos east() const;
    BlockPos relative(const Direction* direction) const;
    BlockPos relative(Direction::Axis* axis, int steps) const;
    BlockPos rotate(Rotation* rotation) const;

    long long compressBlockPos() const;
    int getRandomSeed() const;

    static BlockPos decompressBlockPos(long long packed);
    static BlockPos* decompressBlockPosAsPointer(long long packed);

    static Iterator<BlockPos*>* between(const BlockPos& from, const BlockPos& to);
    static Iterator<MutableBlockPos*>* mutablesBetween(int minX, int minY, int minZ, int maxX, int maxY, int maxZ);
    static Iterator<MutableBlockPos*>* mutablesBetween(const BlockPos& from, const BlockPos& to);
    static Iterator<MutableBlockPos*>* pooledMutablesBetween(const BlockPos& from, const BlockPos& to);
    static Iterator<MutableBlockPos*>* pooledMutablesBetween(int minX, int minY, int minZ, int maxX, int maxY,
                                                             int maxZ);

    bool operator==(const BlockPos& other) const;
    bool operator!=(const BlockPos& other) const;
    BlockPos operator+(const BlockPos& other) const;
    BlockPos operator-(const BlockPos& other) const;
    Vec3* operator-(const Vec3* other) const;

private:
    static const int PACKED_X_LENGTH;
    static const int PACKED_Z_LENGTH;
    static const int PACKED_Y_LENGTH;
    static const long long PACKED_X_MASK;
    static const long long PACKED_Y_MASK;
    static const long long PACKED_Z_MASK;
    static const int Y_OFFSET;
    static const int X_OFFSET;
};

inline unsigned int hash_value(const BlockPos& pos)
{
    return static_cast<unsigned int>(pos.hashCode());
}

#endif
