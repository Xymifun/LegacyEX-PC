#include "game/world/TheEndDimension.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <limits>

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/ColourTable.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/EndDragonFight.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/TheEndLevelSource.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/biome/FixedBiomeSource.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/chunk/ChunkPos.hpp"

int aaDistance(int x, int z, const ChunkPos* pos)
{
    return std::max(std::abs(x - pos->x), std::abs(z - pos->z));
}

ChunkPos TheEndDimension::AREAS[2] = {ChunkPos(0, 0), ChunkPos(0, 25)};

TheEndDimension::TheEndDimension() : Dimension(), mDragonFight(NULL), mModernEnd(true)
{
    const int size = getXZSize();
    mChunkData = new char[size * size];
    std::memset(mChunkData, -1, size * size);
}

TheEndDimension::~TheEndDimension()
{
    delete mDragonFight;
    delete[] mChunkData;
}

EndDragonFight* TheEndDimension::getDragonFight()
{
    return mDragonFight;
}

int TheEndDimension::getXZSize()
{
    return mModernEnd ? 128 : 18;
}

void TheEndDimension::init()
{
    mBiomeSource = new FixedBiomeSource(Biome::THE_END, 0.5f);
    CompoundTag* data = mLevel->getLevelData()->getDimensionData(DimensionType::THE_END);
    mModernEnd = mLevel->getLevelData()->getModernEnd();
    mDragonFight = !mLevel->mIsClientSide
                       ? new EndDragonFight(static_cast<ServerLevel*>(mLevel), data->getCompound(L"DragonFight"))
                       : NULL;
    gApp.SetExtraGameData(eGameExtraData_ModernEnd, mModernEnd);
}

ChunkGenerator* TheEndDimension::createRandomLevelGenerator() const
{
    LevelData* levelData = mLevel->getLevelData();
    return new TheEndLevelSource(mLevel, levelData->isGenerateMapFeatures(), mLevel->getSeed(),
                                 getSpawnPos() != NULL ? *getSpawnPos() : BlockPos(0, 0, 0));
}

float TheEndDimension::getTimeOfDay(long long time, float partialTick) const
{
    return 0.0f;
}

float* TheEndDimension::getSunriseColor(float celestialAngle, float timeOfDay)
{
    return NULL;
}

Vec3* TheEndDimension::getFogColor(float celestialAngle, float timeOfDay)
{
    float brightness = Mth::cos(celestialAngle * 6.2831855f) * 2.0f + 0.5f;
    brightness = Mth::clamp(brightness, 0.0f, 1.0f);
    float red = 0.627451f;
    float green = 0.5019608f;
    float blue = 0.627451f;
    red = red * (brightness * 0.0f + 0.15f);
    green = green * (brightness * 0.0f + 0.15f);
    blue = blue * (brightness * 0.0f + 0.15f);
    return Vec3::newTemp(red, green, blue);
}

bool TheEndDimension::hasGround()
{
    return false;
}
bool TheEndDimension::mayRespawn() const
{
    return false;
}
bool TheEndDimension::isNaturalDimension()
{
    return false;
}
float TheEndDimension::getCloudHeight()
{
    return 8.0f;
}

bool TheEndDimension::isValidSpawn(int x, int z) const
{
    const BlockState* state = mLevel->getTopBlockState(BlockPos(x, 0, z));
    return state->getMaterial()->blocksMotion();
}

BlockPos* TheEndDimension::getSpawnPos() const
{
    static BlockPos modernSpawnPos(60, 50, 60);
    static BlockPos legacySpawnPos(100, 50, 0);
    return mModernEnd ? &modernSpawnPos : &legacySpawnPos;
}

int TheEndDimension::getSpawnYPosition()
{
    return 50;
}
bool TheEndDimension::isFoggyAt(int x, int z)
{
    return false;
}

const DimensionType* TheEndDimension::getType() const
{
    return DimensionType::THE_END;
}

void TheEndDimension::saveData()
{
    CompoundTag* data = new CompoundTag();
    if (mDragonFight != NULL)
        data->put(L"DragonFight", mDragonFight->saveData());
    mLevel->getLevelData()->setDimensionData(DimensionType::THE_END, data);
}

void TheEndDimension::tick()
{
    if (mDragonFight != NULL)
        mDragonFight->tick();
}

ChunkPos* TheEndDimension::closestAreaToPoint(int x, int z)
{
    ChunkPos* closest = &AREAS[0];
    unsigned int closestDistance = aaDistance(x, z, &AREAS[0]);
    ChunkPos* area = &AREAS[1];
    if (static_cast<unsigned int>(aaDistance(x, z, area)) < closestDistance)
        closest = area;
    return closest;
}

AABB* TheEndDimension::getPlayerConstraints(const BlockPos& pos)
{
    if (mModernEnd)
    {
        BlockPos origin = closestAreaToPoint(pos.x >> 4, pos.z >> 4)->getWorldPosition();
        return AABB::newTemp(-96.0, -std::numeric_limits<double>::max(), -96.0, 96.0,
                             std::numeric_limits<double>::max(), 96.0)
            ->move(origin);
    }
    AABB* bounds = Dimension::getPlayerConstraints(pos);
    return bounds->set(bounds->mMinX, -std::numeric_limits<double>::max(), bounds->mMinZ, bounds->mMaxX,
                       std::numeric_limits<double>::max(), bounds->mMaxZ);
}

bool TheEndDimension::containsChunk(int x, int z)
{
    bool result = Dimension::containsChunk(x, z);
    if (result && mModernEnd)
    {
        const int size = getXZSize();
        char& cached = mChunkData[(x + size / 2) * size + (z + size / 2)];
        if (cached == -1)
        {
            ChunkPos* area = closestAreaToPoint(x, z);
            int dx = x - area->x;
            int dz = z - area->z;
            cached = (dx >= -6 && dx < 6 && dz >= -6 && dz < 6) ? 1 : 0;
        }
        return cached == 1;
    }
    return result;
}

int TheEndDimension::getBlockLightColour()
{
    return static_cast<int>(
        Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_BlockLight_EndDimension));
}
