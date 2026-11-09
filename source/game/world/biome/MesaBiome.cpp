#include "game/world/biome/MesaBiome.hpp"
#include "game/world/block/BlockId.hpp"
#include "game/core/ColourTable.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/PerlinSimplexNoise.hpp"
#include "game/util/Math.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/ColoredBlock.hpp"
#include "game/world/block/DirtBlock.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/SandBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/chunk/ChunkPrimer.hpp"
#include "game/world/generation/OreFeature.hpp"
#include "game/world/generation/TreeFeature.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include <cmath>

static Biome::BlockPair sCoarseDirt(BlockId::DIRT, 1);
static Biome::BlockPair sGrass(BlockId::GRASS, 0);
static Biome::BlockPair sClay(BlockId::HARDENED_CLAY, 0);
static Biome::BlockPair sStainedClay(BlockId::STAINED_HARDENED_CLAY, 0);
static Biome::BlockPair sOrange(BlockId::STAINED_HARDENED_CLAY, 1);
static Biome::BlockPair sRedSand(BlockId::SAND, 1);

MesaBiome::MesaBiome(EBiomeIDs id, bool brycePillars, bool hasForest, BiomeProperties* properties)
    : Biome(id, eBiomeType_Mesa, properties)
{
    mBrycePillars = brycePillars;
    mHasForest = hasForest;
    mPillarNoise = NULL;
    mPillarRoofNoise = NULL;
    mNoise = NULL;
    mClayBands = NULL;
    mSeed = 0;
    mCreatures.clear();
    mTopBlock = sRedSand;
    mFillerBlock = sStainedClay;
    delete mDecorator;
    mDecorator = new MesaDecorator();
    mDecorator->treesPerChunk = -999;
    mDecorator->deadBushPerChunk = 20;
    mDecorator->reedsPerChunk = 3;
    mDecorator->cactiPerChunk = 5;
    mDecorator->flowersPerChunk = 0;
    mCreatures.clear();

    if (hasForest)
    {
        mDecorator->treesPerChunk = 5;
    }
}

MesaBiome::~MesaBiome()
{
    delete mPillarNoise;
    delete mPillarRoofNoise;
    delete mNoise;
    delete[] mClayBands;
}

void MesaBiome::staticCtor()
{
    sCoarseDirt = BlockPair(BlockId::DIRT,
                            Blocks::dirt->convertBlockStateToLegacyData(Blocks::dirt->defaultBlockState()->setValue(
                                DirtBlock::VARIANT, DirtBlock::Variant::COARSE_DIRT)));
    sGrass = BlockPair(BlockId::GRASS,
                       Blocks::grass->convertBlockStateToLegacyData(Blocks::grass->defaultBlockState()));
    sClay = BlockPair(BlockId::HARDENED_CLAY,
                      Blocks::hardened_clay->convertBlockStateToLegacyData(Blocks::hardened_clay->defaultBlockState()));
    sStainedClay = BlockPair(BlockId::STAINED_HARDENED_CLAY,
                             Blocks::stained_hardened_clay->convertBlockStateToLegacyData(
                                 Blocks::stained_hardened_clay->defaultBlockState()));
    sOrange = BlockPair(
        BlockId::STAINED_HARDENED_CLAY,
        Blocks::stained_hardened_clay->convertBlockStateToLegacyData(
            Blocks::stained_hardened_clay->defaultBlockState()->setValue(ColoredBlock::COLOR, DyeColor::ORANGE)));
    sRedSand = BlockPair(
        BlockId::SAND, Blocks::sand->convertBlockStateToLegacyData(
                           Blocks::sand->defaultBlockState()->setValue(SandBlock::TYPE, SandBlock::Variant::RED_SAND)));
}

AbstractTreeFeature* MesaBiome::getTreeFeature(Random& random)
{
    return new TreeFeature(false);
}

unsigned int MesaBiome::getFoliageColor(const BlockPos& pos)
{
    return Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_Foliage_Mesa);
}

unsigned int MesaBiome::getGrassColor(const BlockPos& pos)
{
    return Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_Grass_Mesa);
}

Biome::BlockPair MesaBiome::getBand(int x, int y, int z)
{
    int offset = (int)Math::round(mNoise->getValue(x * 0.001953125, x * 0.001953125) * 2.0);
    return mClayBands[(y + offset + 64) % 64];
}

void MesaBiome::generateBands(long long seed)
{
    delete[] mClayBands;
    mClayBands = new BlockPair[64];

    for (int i = 0; i < 64; ++i)
    {
        mClayBands[i] = sClay;
    }

    Random random(seed);
    mNoise = new PerlinSimplexNoise(&random, 1);

    for (int i = 0; i < 64; ++i)
    {
        i += random.nextInt(5) + 1;
        if (i < 64)
        {
            mClayBands[i] = sOrange;
        }
    }

    BlockPair yellowBand(
        BlockId::STAINED_HARDENED_CLAY,
        Blocks::stained_hardened_clay->convertBlockStateToLegacyData(
            Blocks::stained_hardened_clay->defaultBlockState()->setValue(ColoredBlock::COLOR, DyeColor::YELLOW)));
    int yellowCount = random.nextInt(4) + 2;
    for (int i = 0; i < yellowCount; ++i)
    {
        int length = random.nextInt(3) + 1;
        int start = random.nextInt(64);
        for (int j = 0; start + j < 64 && j < length; ++j)
        {
            mClayBands[start + j] = yellowBand;
        }
    }

    BlockPair brownBand(
        BlockId::STAINED_HARDENED_CLAY,
        Blocks::stained_hardened_clay->convertBlockStateToLegacyData(
            Blocks::stained_hardened_clay->defaultBlockState()->setValue(ColoredBlock::COLOR, DyeColor::BROWN)));
    int brownCount = random.nextInt(4) + 2;
    for (int i = 0; i < brownCount; ++i)
    {
        int length = random.nextInt(3) + 2;
        int start = random.nextInt(64);
        for (int j = 0; start + j < 64 && j < length; ++j)
        {
            mClayBands[start + j] = brownBand;
        }
    }

    BlockPair redBand(BlockId::STAINED_HARDENED_CLAY, Blocks::stained_hardened_clay->convertBlockStateToLegacyData(
                                                          Blocks::stained_hardened_clay->defaultBlockState()->setValue(
                                                              ColoredBlock::COLOR, DyeColor::RED)));
    int redCount = random.nextInt(4) + 2;
    for (int i = 0; i < redCount; ++i)
    {
        int length = random.nextInt(3) + 1;
        int start = random.nextInt(64);
        for (int j = 0; start + j < 64 && j < length; ++j)
        {
            mClayBands[start + j] = redBand;
        }
    }

    BlockPair silverBand(
        BlockId::STAINED_HARDENED_CLAY,
        Blocks::stained_hardened_clay->convertBlockStateToLegacyData(
            Blocks::stained_hardened_clay->defaultBlockState()->setValue(ColoredBlock::COLOR, DyeColor::SILVER)));
    BlockPair whiteBand(
        BlockId::STAINED_HARDENED_CLAY,
        Blocks::stained_hardened_clay->convertBlockStateToLegacyData(
            Blocks::stained_hardened_clay->defaultBlockState()->setValue(ColoredBlock::COLOR, DyeColor::WHITE)));
    int whiteCount = random.nextInt(3) + 3;
    int band = 0;
    for (int i = 0; i < whiteCount; ++i)
    {
        int length = 1;
        band += random.nextInt(16) + 4;
        for (int j = 0; band + j < 64 && j < length; ++j)
        {
            mClayBands[band + j] = whiteBand;
            if (band + j > 1 && random.nextBoolean())
            {
                mClayBands[band + j - 1] = silverBand;
            }
            if (band + j < 63 && random.nextBoolean())
            {
                mClayBands[band + j + 1] = silverBand;
            }
        }
    }
}

void MesaBiome::buildSurfaceAt(Level* level, Random& random, ChunkPrimer* primer, int x, int z, double noise)
{
    PIXBeginNamedEvent(0.0F, "MesaBiome::buildSurfaceAtDefault");

    if (mClayBands == NULL || mSeed != level->getSeed())
    {
        generateBands(level->getSeed());
    }

    if (mPillarNoise == NULL || mPillarRoofNoise == NULL || mSeed != level->getSeed())
    {
        Random* pillarRandom = new Random(mSeed);
        mPillarNoise = new PerlinSimplexNoise(pillarRandom, 4);
        mPillarRoofNoise = new PerlinSimplexNoise(pillarRandom, 1);
    }

    mSeed = level->getSeed();

    double pillarHeight = 0.0;
    if (mBrycePillars)
    {
        int pillarX = (x & -16) + (z & 15);
        int pillarZ = (z & -16) + (x & 15);
        double pillar = std::min(std::fabs(noise),
                                 mPillarNoise->getValue((double)pillarX * 0.25, (double)pillarZ * 0.25));

        if (pillar > 0.0)
        {
            double roof = std::fabs(
                mPillarRoofNoise->getValue((double)pillarX * 0.001953125, (double)pillarZ * 0.001953125));
            pillarHeight = pillar * pillar * 2.5;
            double cap = std::ceil(roof * 50.0) + 14.0;
            if (pillarHeight > cap)
            {
                pillarHeight = cap;
            }
            pillarHeight += 64.0;
        }
    }

    int localX = x & 15;
    int localZ = z & 15;
    BlockPair filler = mFillerBlock;
    int depth = (int)(noise / 3.0 + 3.0 + random.nextDouble() * 0.25);
    bool coarse = std::cos(noise / 3.0 * 3.1415927f) > 0.0;
    int runDepth = -1;
    bool orangeRun = false;
    int surfaceBlocks = 0;

    for (int y = 127; y >= 0; --y)
    {
        int index = localZ << 11 | localX << 7 | y;

        if (primer->getState(localZ, y, localX)->getBlock()->mMaterial == Material::AIR && y < (int)pillarHeight)
        {
            primer->setBlockAndData(index, STONE_BLOCK.blockId, STONE_BLOCK.blockData);
        }

        if (y <= random.nextInt(2) + 1)
        {
            primer->setBlockAndData(index, BEDROCK_BLOCK.blockId, BEDROCK_BLOCK.blockData);
        }
        else if (surfaceBlocks < 15 || mBrycePillars)
        {
            const BlockState* state = primer->getState(localZ, y, localX);

            if (state->getBlock()->mMaterial == Material::AIR)
            {
                runDepth = -1;
            }
            else if (state->getBlock() == Blocks::stone)
            {
                if (runDepth == -1)
                {
                    orangeRun = false;

                    if (depth <= 0)
                    {
                        filler = STONE_BLOCK;
                    }
                    else if (y >= 59 && y <= 64)
                    {
                        filler = mFillerBlock;
                    }

                    runDepth = depth + std::max(0, y - 63);

                    if (y >= 62)
                    {
                        if (mHasForest && y > 86 + depth * 2)
                        {
                            if (coarse)
                            {
                                primer->setBlockAndData(index, sCoarseDirt.blockId, sCoarseDirt.blockData);
                            }
                            else
                            {
                                primer->setBlockAndData(index, sGrass.blockId, sGrass.blockData);
                            }
                        }
                        else if (y > 66 + depth)
                        {
                            BlockPair surface;
                            if (y < 64 || y > 127)
                            {
                                surface = sOrange;
                            }
                            else if (coarse)
                            {
                                surface = sClay;
                            }
                            else
                            {
                                surface = getBand(x, y, z);
                            }
                            primer->setBlockAndData(index, surface.blockId, surface.blockData);
                        }
                        else
                        {
                            primer->setBlockAndData(index, mTopBlock.blockId, mTopBlock.blockData);
                            orangeRun = true;
                        }
                    }
                    else
                    {
                        primer->setBlockAndData(index, filler.blockId, filler.blockData);

                        if (filler.blockId == BlockId::STAINED_HARDENED_CLAY)
                        {
                            primer->setBlockAndData(index, sOrange.blockId, sOrange.blockData);
                        }
                    }
                }
                else if (runDepth > 0)
                {
                    --runDepth;

                    if (orangeRun)
                    {
                        primer->setBlockAndData(index, sOrange.blockId, sOrange.blockData);
                    }
                    else
                    {
                        BlockPair band = getBand(x, y, z);
                        primer->setBlockAndData(index, band.blockId, band.blockData);
                    }
                }

                ++surfaceBlocks;
            }
        }
    }

    PIXEndNamedEvent();
}

Biome::EBiomeBaseClass MesaBiome::getBaseClass()
{
    return eBiomeBaseClass_Mesa;
}

void MesaBiome::MesaDecorator::decorateOres(Level* level, Random* random)
{
    BiomeDecorator::decorateOres(level, random);
    decorateDepthSpan(level, random, 20, goldGen, 32, 80);
}
