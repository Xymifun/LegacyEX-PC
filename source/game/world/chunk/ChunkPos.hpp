#ifndef GAME_WORLD_CHUNK_CHUNKPOS_HPP
#define GAME_WORLD_CHUNK_CHUNKPOS_HPP

#include <cstddef>
#include <string>

#include <boost/shared_ptr.hpp>

#include "game/world/block/BlockPos.hpp"

class Entity;

class ChunkPos
{
public:
    int x;
    int z;

    ChunkPos(int x, int z);
    ChunkPos(const BlockPos& pos);
    static long long asLong(int x, int z);
    std::size_t hashCode() const;
    double distanceToSqr(boost::shared_ptr<Entity> entity);
    double distanceToSqr(double x, double z);
    int getMiddleBlockX();
    int getMiddleBlockZ();
    int getMinBlockX() const;
    int getMinBlockZ() const;
    int getMaxBlockX() const;
    int getMaxBlockZ() const;
    BlockPos getMiddleBlockPosition(int y);
    BlockPos getBlockAt(int x, int y, int z);
    std::wstring toString();
    BlockPos getWorldPosition() const;
    bool operator==(const ChunkPos& other) const { return x == other.x && z == other.z; }
    static bool eq_test(const ChunkPos& lhs, const ChunkPos& rhs);
};

class ChunkPosKeyHash
{
public:
    std::size_t operator()(const ChunkPos& pos) const { return pos.hashCode(); }
};

class ChunkPosKeyEq
{
public:
    bool operator()(const ChunkPos& left, const ChunkPos& right) const { return ChunkPos::eq_test(left, right); }
};

#endif
