#include "game/world/Dimension.hpp"

#include <cmath>

#include "game/core/ColourTable.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/core/SuperflatConfig.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/LevelType.hpp"
#include "game/world/FlatLevelSource.hpp"
#include "game/world/OverworldLevelSource.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/WorldBorder.hpp"
#include "game/world/biome/BiomeSource.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/biome/FixedBiomeSource.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/generation/FlatGeneratorInfo.hpp"
#include "game/world/chunk/OldChunkStorage.hpp"

const float Dimension::MOON_BRIGHTNESS_PER_PHASE[8] = {1.0f, 0.75f, 0.5f, 0.25f, 0.0f, 0.25f, 0.5f, 0.75f};

Dimension::Dimension() : mUltraWarm(0), mHasCeiling(0), mBrightnessRamp(new float[16])
{
    mHasSkyLight = 0;
    mSeaLevel = 0;
    mSuperflatConfig = NULL;
    mLevel = NULL;
}

Dimension::~Dimension()
{
    delete[] mBrightnessRamp;
    delete mBiomeSource;
}

void Dimension::init(Level* level)
{
    mLevel = level;
    init();
    mLevelType = level->getLevelData()->getGeneratorType();
    mSuperflatConfig = level->getLevelData()->getGeneratorOptions();
    updateLightRamp();
    mXZSize = level->getLevelData()->getXZSize();
}

void Dimension::updateLightRamp()
{
    const float base = 1.0f;
    const float scale = 15.0f;
    const float ambient = 3.0f;
    for (int i = 0; i < 16; ++i)
    {
        float light = base - (float)i / scale;
        mBrightnessRamp[i] = (base - light) / (light * ambient + base);
    }
}

void Dimension::init()
{
    mHasSkyLight = 1;
    if (mLevel->getLevelData()->getGeneratorType() == LevelType::FLAT)
    {
        SuperflatConfig* config = mLevel->getLevelData()->getGeneratorOptions();
        if (config == NULL)
        {
            config = new SuperflatConfig();
            config->makeDefaults();
            mLevel->getLevelData()->setGeneratorOptions(config);
        }
        Biome* biome = Biome::getBiome(FlatGeneratorInfo(config).getBiome(), Biome::DEFAULT);
        mBiomeSource = new FixedBiomeSource(biome, biome->getDownfall());
    }
    else
    {
        mBiomeSource = new BiomeSource(mLevel->getLevelData());
    }
}

ChunkGenerator* Dimension::createRandomLevelGenerator() const
{
    LevelData* levelData = mLevel->getLevelData();
    if (mLevelType == LevelType::FLAT)
        return new FlatLevelSource(mLevel, mLevel->getSeed(), levelData->isGenerateMapFeatures(), mSuperflatConfig);
    if (mLevelType == LevelType::CUSTOMIZED)
        return new OverworldLevelSource(mLevel, mLevel->getSeed(), levelData->isGenerateMapFeatures(),
                                        mSuperflatConfig);
    return new OverworldLevelSource(mLevel, mLevel->getSeed(), levelData->isGenerateMapFeatures(), mSuperflatConfig);
}

ChunkGenerator* Dimension::createFlatLevelSource() const
{
    LevelData* levelData = mLevel->getLevelData();
    return new FlatLevelSource(mLevel, mLevel->getSeed(), levelData->isGenerateMapFeatures(), mSuperflatConfig);
}

OldChunkStorage* Dimension::createStorage(File file)
{
    return new OldChunkStorage(file, true);
}

bool Dimension::isValidSpawn(int x, int z) const
{
    BlockPos pos(x, 0, z);
    if (mLevel->getBiome(pos)->isAlwaysValidSpawn())
        return true;
    return mLevel->getTopBlockState(pos)->getBlock() == Blocks::grass;
}

float Dimension::getTimeOfDay(long long time, float partialTick) const
{
    float value = ((float)(int)(time % 24000) + partialTick) / 24000.0f - 0.25f;
    if (value < 0.0f)
        value += 1.0f;
    if (value > 1.0f)
        value -= 1.0f;
    float cosine = cosf(value * 3.1415927f);
    return value + (-((cosine + 1.0f) * 0.5f - 1.0f) - value) / 3.0f;
}

int Dimension::getMoonPhase(long long time) const
{
    int phase = (int)((time / 24000) % 8);
    return (phase + 8) % 8;
}

bool Dimension::isNaturalDimension()
{
    return true;
}

float* Dimension::getSunriseColor(float celestialAngle, float timeOfDay)
{
    unsigned int dark = Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_Sky_Dawn_Dark);
    unsigned int bright = Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_Sky_Dawn_Bright);
    double darkRed = ((dark >> 16) & 255) / 255.0f;
    double darkGreen = ((dark >> 8) & 255) / 255.0;
    double darkBlue = (dark & 255) / 255.0;
    double brightRed = ((bright >> 16) & 255) / 255.0f;
    double brightGreen = ((bright >> 8) & 255) / 255.0;
    double brightBlue = (bright & 255) / 255.0;
    float blend = Mth::cos(celestialAngle * 6.2831855f);
    if (blend < -0.4f || blend > 0.4f)
        return NULL;
    blend = blend / 0.4f * 0.5f + 0.5f;
    float alpha = 1.0f - (1.0f - sinf(blend * 3.1415927f)) * 0.99f;
    mSunriseColor[0] = blend * (brightRed - darkRed) + darkRed;
    mSunriseColor[1] = blend * (brightGreen - darkGreen) + darkGreen;
    mSunriseColor[2] = blend * (brightBlue - darkBlue) + darkBlue;
    mSunriseColor[3] = alpha * alpha;
    return mSunriseColor;
}

Vec3* Dimension::getFogColor(float celestialAngle, float timeOfDay)
{
    float brightness = Mth::clamp(Mth::cos(celestialAngle * 6.2831855f) * 2.0f + 0.5f, 0.0f, 1.0f);
    float redGreenScale = brightness * 0.94f + 0.06f;
    return Vec3::newTemp(0.7529412f * redGreenScale, 0.84705883f * redGreenScale, brightness * 0.91f + 0.09f);
}

bool Dimension::mayRespawn() const
{
    return true;
}

float Dimension::getCloudHeight()
{
    return (float)mLevel->getCloudHeight();
}
bool Dimension::hasGround()
{
    return true;
}
BlockPos* Dimension::getSpawnPos() const
{
    return NULL;
}
int Dimension::getSpawnYPosition()
{
    return mLevelType == LevelType::FLAT ? 4 : 64;
}
bool Dimension::isFoggyAt(int x, int z)
{
    return false;
}
WorldBorder* Dimension::createWorldBorder(Level* level)
{
    return new WorldBorder(level);
}
void Dimension::saveData()
{
    return;
}
void Dimension::tick()
{
    return;
}
bool Dimension::canUnloadChunk(int x, int z)
{
    return true;
}
int Dimension::getXZSize()
{
    return mXZSize;
}

AABB* Dimension::getPlayerConstraints(const BlockPos& pos)
{
    double size = (double)getXZSize() * 0.5 * 16.0;
    return AABB::newTemp(-size, 0.0, -size, size, 512.0, size);
}

bool Dimension::inPlayerConstraints(const BlockPos& pos)
{
    AABB* bounds = getPlayerConstraints(pos);
    int minX = (int)bounds->mMinX + (bounds->mMinX >= (double)(int)bounds->mMinX ? 1 : 0);
    int minY = (int)bounds->mMinY + (bounds->mMinY >= (double)(int)bounds->mMinY ? 1 : 0);
    int minZ = (int)bounds->mMinZ + (bounds->mMinZ >= (double)(int)bounds->mMinZ ? 1 : 0);
    int maxX = (int)bounds->mMaxX - (bounds->mMaxX < (double)(int)bounds->mMaxX ? 3 : 2);
    int maxY = (int)bounds->mMaxY - (bounds->mMaxY < (double)(int)bounds->mMaxY ? 2 : 1);
    int maxZ = (int)bounds->mMaxZ - (bounds->mMaxZ < (double)(int)bounds->mMaxZ ? 3 : 2);
    return pos.x >= minX && pos.x <= maxX && pos.y >= minY && pos.y <= maxY && pos.z >= minZ && pos.z <= maxZ;
}

bool Dimension::containsChunk(int x, int z)
{
    int size = getXZSize() / 2;
    return x >= -size && x < size && z >= -size && z < size;
}

int Dimension::getBlockLightColour()
{
    return (int)Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_BlockLight_OverworldDimension);
}

double Dimension::getClearColorScale()
{
    return mLevelType == LevelType::FLAT ? 1.0 : 0.03125;
}
BiomeSource* Dimension::getBiomeSource()
{
    return mBiomeSource;
}
unsigned char Dimension::isUltraWarm()
{
    return mUltraWarm;
}
unsigned char Dimension::isHasCeiling()
{
    return mHasCeiling;
}
unsigned char Dimension::isHasSkyLight()
{
    return mHasSkyLight;
}
float* Dimension::getBrightnessRamp()
{
    return mBrightnessRamp;
}
void Dimension::onPlayerJoin(const boost::shared_ptr<ServerPlayer>& player)
{
    (void)player;
}
void Dimension::onPlayerLeave(const boost::shared_ptr<ServerPlayer>& player)
{
    (void)player;
}
int Dimension::getSeaLevel()
{
    return mSeaLevel;
}
