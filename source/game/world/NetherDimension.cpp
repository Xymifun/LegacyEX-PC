#include "game/world/NetherDimension.hpp"

#include <cmath>

#include "game/core/ColourTable.hpp"
#include "game/core/Minecraft.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/LevelType.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/biome/FixedBiomeSource.hpp"

#include "game/world/NetherFlatLevelSource.hpp"
#include "game/world/NetherLevelSource.hpp"

void NetherDimension::init()
{
    mBiomeSource = new FixedBiomeSource(Biome::HELL, 0.0f);
    mUltraWarm = 1;
    mHasCeiling = 1;
}

Vec3* NetherDimension::getFogColor(float celestialAngle, float timeOfDay)
{
    return Vec3::newTemp(0.2f, 0.03f, 0.03f);
}

void NetherDimension::updateLightRamp()
{
    const float ambientLight = 0.1f;
    for (int i = 0; i <= 15; ++i)
    {
        float v = 1.0f - (float)i / 15.0f;
        mBrightnessRamp[i] = ((1.0f - v) / (v * 3.0f + 1.0f)) * (1.0f - ambientLight) + ambientLight;
    }
}

ChunkGenerator* NetherDimension::createRandomLevelGenerator() const
{
    if (mLevelType == LevelType::FLAT)
        return new NetherFlatLevelSource(mLevel, mLevel->getLevelData()->isGenerateMapFeatures(), mLevel->getSeed());
    return new NetherLevelSource(mLevel, mLevel->getLevelData()->isGenerateMapFeatures(), mLevel->getSeed());
}

bool NetherDimension::isNaturalDimension()
{
    return false;
}

bool NetherDimension::isValidSpawn(int x, int z) const
{
    return false;
}

float NetherDimension::getTimeOfDay(long long time, float partialTick) const
{
    return 0.5f;
}

bool NetherDimension::mayRespawn() const
{
    return false;
}

bool NetherDimension::isFoggyAt(int x, int z)
{
    return true;
}

WorldBorder* NetherDimension::createWorldBorder(Level* level)
{
    return new NetherDimensionWorldBorder(level);
}

const DimensionType* NetherDimension::getType() const
{
    return DimensionType::NETHER;
}

int NetherDimension::getXZSize()
{
    float quotient = static_cast<float>(mLevel->getLevelData()->getXZSize())
                     / static_cast<float>(mLevel->getLevelData()->getHellScale());
    float result = ceilf(quotient);
    return static_cast<int>(result);
}

AABB* NetherDimension::getPlayerConstraints(const BlockPos& pos)
{
    AABB* bounds = Dimension::getPlayerConstraints(pos);
    bounds->mMaxY = 127.0;
    return bounds;
}

int NetherDimension::getBlockLightColour()
{
    return static_cast<int>(
        Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_BlockLight_HellDimension));
}

NetherDimension::NetherDimensionWorldBorder::NetherDimensionWorldBorder(Level* level) : WorldBorder(level)
{
}

double NetherDimension::NetherDimensionWorldBorder::getCenterX()
{
    return WorldBorder::getCenterX() * 0.125;
}

double NetherDimension::NetherDimensionWorldBorder::getCenterZ()
{
    return WorldBorder::getCenterZ() * 0.125;
}
