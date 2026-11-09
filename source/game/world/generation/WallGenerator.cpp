#include "game/world/generation/WallGenerator.hpp"

#include <cmath>
#include <vector>

#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/PlanksBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/block/BlockTransaction.hpp"
#include "game/core/GenerationWriteTask.hpp"

WallGenerator::Wall::Wall(AABB* area, bool internal, bool boundary)
    : mArea(new AABB(*area)),
      mInternal(internal),
      mBoundary(boundary)
{
}

WallGenerator::Wall::~Wall()
{
    delete mArea;
}

WallGenerator::WallGenerator(Random random, std::vector<AABB*>* areas, std::vector<AABB*>* walls) : MapGenerator(random)
{
    mAreas = *areas;
    mBounds = NULL;
    std::vector<AABB*>::iterator area = mAreas.begin();
    while (area != mAreas.end())
    {
        if (mBounds != NULL)
        {
            mBounds = mBounds->minmax(*area);
        }
        else
        {
            mBounds = *area;
        }
        ++area;
    }

    if (walls != NULL)
    {
        std::vector<AABB*>::iterator wall = walls->begin();
        while (wall != walls->end())
        {
            mBounds = mBounds->minmax(*wall);
            mWalls.push_back(new Wall(*wall, true, false));
            ++wall;
        }
    }
    mBounds = mBounds->grow(2.0, 0.0, 2.0);
}

WallGenerator::~WallGenerator()
{
    std::vector<Wall*>::iterator wall = mWalls.begin();
    while (wall != mWalls.end())
    {
        delete *wall;
        ++wall;
    }
}

void WallGenerator::GetInternalWalls(std::vector<AABB*>* walls)
{
    std::vector<Wall*>::iterator wall = mWalls.begin();
    while (wall != mWalls.end())
    {
        if ((*wall)->mInternal)
        {
            walls->push_back((*wall)->mArea);
        }
        ++wall;
    }
}

bool WallGenerator::IsPointInternal(int x, int z, int size)
{
    return x > 0 && x < size && z > 0 && z < size;
}

void WallGenerator::StartGeneration(Level* level)
{
    AABB* grown = mBounds->grow(2.0, 1.0, 2.0);
    BlockTransaction* transaction = BlockTransaction::CreateTransaction(
        BlockPos(grown->mMinX, grown->mMinY, grown->mMinZ), BlockPos(grown->mMaxX, grown->mMaxY, grown->mMaxZ));

    if (mWalls.empty())
    {
        const int size = static_cast<int>(std::floor((mBounds->mMaxX - mBounds->mMinX) * 0.5));
        for (int x = 0; x < 3; ++x)
        {
            for (int z = 0; z < 3; ++z)
            {
                const bool internal = IsPointInternal(x, z, 2);
                AABB* post = AABB::newTemp(mBounds->mMinX + x * size, mBounds->mMinY, mBounds->mMinZ + z * size,
                                           mBounds->mMinX + x * size + 1.0, mBounds->mMinY + 16.0,
                                           mBounds->mMinZ + z * size + 1.0);
                mWalls.push_back(new Wall(post, internal, true));

                if (x + 1 <= 2)
                {
                    const bool boundary = internal || IsPointInternal(x + 1, z, 2);
                    const double wallZ = mBounds->mMinZ + z * size;
                    AABB* side = AABB::newTemp(mBounds->mMinX + x * size + 1.0, mBounds->mMinY, wallZ,
                                               mBounds->mMinX + (x + 1) * size, mBounds->mMinY + 16.0, wallZ + 1.0);
                    mWalls.push_back(new Wall(side, boundary, false));
                }
                if (z + 1 <= 2)
                {
                    const bool boundary = internal || IsPointInternal(x, z + 1, 2);
                    const double wallX = mBounds->mMinX + x * size;
                    AABB* side = AABB::newTemp(wallX, mBounds->mMinY, mBounds->mMinZ + z * size + 1.0, wallX + 1.0,
                                               mBounds->mMinY + 16.0, mBounds->mMinZ + (z + 1) * size);
                    mWalls.push_back(new Wall(side, boundary, false));
                }
            }
        }

        const BlockState* voidState = Blocks::structure_void->defaultBlockState();
        SetArea(level, transaction,
                AABB::newTemp(grown->mMinX, grown->mMinY, grown->mMinZ, grown->mMaxX, grown->mMinY + 1.0, grown->mMaxZ),
                voidState);
        SetArea(level, transaction, grown, Blocks::planks->defaultBlockState());

        for (std::vector<AABB*>::iterator area = mAreas.begin(); area != mAreas.end(); ++area)
        {
            AABB* bounds = *area;
            const BlockState* floor = Blocks::planks->defaultBlockState()->setValue(PlanksBlock::VARIANT,
                                                                                    PlanksBlock::Variant::SPRUCE);
            SetArea(level, transaction,
                    AABB::newTemp(bounds->mMinX, bounds->mMinY - 1.0, bounds->mMinZ, bounds->mMaxX, bounds->mMinY,
                                  bounds->mMaxZ),
                    floor);
        }
    }

    for (std::vector<Wall*>::iterator wall = mWalls.begin(); wall != mWalls.end(); ++wall)
    {
        const BlockState* state = ((*wall)->mBoundary || (*wall)->mInternal) ? Blocks::planks->defaultBlockState()
                                                                             : Blocks::stonebrick->defaultBlockState();
        SetArea(level, transaction, (*wall)->mArea, state);
    }

    BlockPos origin(static_cast<int>(grown->mMinX), static_cast<int>(grown->mMinY), static_cast<int>(grown->mMinZ));
    mGenerationWriteTasks.push_back(new GenerationWriteTask(level, transaction, origin, NULL));
}

AABB* WallGenerator::GetSpawnArea() const
{
    return mAreas.empty() ? NULL : mAreas.front();
}

void WallGenerator::CleanUp(Level*)
{
}

AABB* WallGenerator::GetLayerBB(unsigned int)
{
    return 0;
}

int WallGenerator::GetNumberOfLayers()
{
    return 0;
}

void WallGenerator::GetLayerExtents(unsigned int, float*, float*)
{
}

void WallGenerator::SetArea(Level*, BlockTransaction*, AABB*, const BlockState*)
{
}
