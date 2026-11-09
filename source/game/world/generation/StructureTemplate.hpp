#ifndef GAME_WORLD_GENERATION_STRUCTURETEMPLATE_HPP
#define GAME_WORLD_GENERATION_STRUCTURETEMPLATE_HPP

#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <boost/functional/hash.hpp>
#include <boost/unordered_map.hpp>

#include "game/world/block/BlockPos.hpp"

class BlockState;
class BoundingBox;
class CompoundTag;
class DataFixerUpper;
class Direction;
class Level;
class Mirror;
class Rotation;
class StructurePlaceSettings;
class StructureProcessor;
class Vec3;


class StructureTemplate
{
public:
    class StructureBlockInfo
    {
    public:
        struct byColumnSorter
        {
            bool operator()(const StructureBlockInfo* left, const StructureBlockInfo* right) const
            {
                return (left->mPos.x << 16 | (left->mPos.z & 0xFFFF))
                       < (right->mPos.x << 16 | (right->mPos.z & 0xFFFF));
            }
        };

        StructureBlockInfo(const BlockPos& pos, const BlockState* state, CompoundTag* nbt)
        {
            mPos = pos;
            mState = state;
            mNbt = nbt;
        }

        ~StructureBlockInfo();

        BlockPos mPos;
        const BlockState* mState;
        CompoundTag* mNbt;
    };

    class StructureEntityInfo
    {
    public:
        StructureEntityInfo(Vec3* pos, const BlockPos& blockPos, CompoundTag* nbt)
        {
            mPos = pos;
            mBlockPos = blockPos;
            mNbt = nbt;
        }

        ~StructureEntityInfo();

        Vec3* mPos;
        BlockPos mBlockPos;
        CompoundTag* mNbt;
    };

    typedef boost::unordered_map<const BlockPos, std::wstring, boost::hash<const BlockPos>,
                                 std::equal_to<const BlockPos>,
                                 std::allocator<std::pair<const BlockPos, std::wstring> > >
        MarkerMap;

    StructureTemplate();
    virtual ~StructureTemplate();

    BlockPos getSize() const;

    void setAuthor(const std::wstring& author);
    std::wstring getAuthor();

    static BlockPos transform(const BlockPos& pos, Mirror* mirror, Rotation* rotation);
    static BlockPos transform(const BlockPos& pos, const Direction* direction);
    static Vec3* transform(Vec3* pos, Mirror* mirror, Rotation* rotation);

    static BlockPos calculateRelativePosition(const StructurePlaceSettings* settings, const BlockPos& pos);

    MarkerMap getMarkers(const BlockPos& pos, StructurePlaceSettings* settings) const;

    BlockPos calculateConnectedPosition(const StructurePlaceSettings* settings, const BlockPos& pos,
                                        const StructurePlaceSettings* connectedSettings,
                                        const BlockPos& connectedPos) const;

    void placeEntities(Level* level, const BlockPos& pos, Mirror* mirror, Rotation* rotation,
                       BoundingBox* boundingBox) const;

    void placeInWorld(Level* level, const BlockPos& pos, StructureProcessor* processor,
                      StructurePlaceSettings* settings, int flags) const;
    void placeInWorld(Level* level, const BlockPos& pos, StructurePlaceSettings* settings) const;
    void placeInWorldChunk(Level* level, const BlockPos& pos, StructurePlaceSettings* settings) const;
    void placeInWorld(Level* level, const BlockPos& pos, StructurePlaceSettings* settings, int flags) const;

    BlockPos getSize(Rotation* rotation) const;

    static BlockPos getZeroPositionWithTransform(const BlockPos& pos, Mirror* mirror, Rotation* rotation, int sizeX,
                                                 int sizeZ);
    BlockPos getZeroPositionWithTransform(const BlockPos& pos, Mirror* mirror, Rotation* rotation) const;

    static void addWalkers(DataFixerUpper* fixerUpper);

    void load(CompoundTag* tag);

private:
    std::vector<const BlockState*> mPalette;
    std::vector<StructureBlockInfo*> mBlocks;
    std::vector<StructureEntityInfo*> mEntities;
    BlockPos mSize;
    std::wstring mAuthor;
};

#endif
