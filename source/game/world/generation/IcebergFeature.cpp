#include "game/world/generation/IcebergFeature.hpp"
#include "game/world/block/BlockId.hpp"

#include <algorithm>
#include <cmath>

#include "game/core/Mth.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"

IcebergFeature::IcebergFeature()
    : Feature(false),
      mSnowOnTop(false),
      mIsEllipse(false),
      mAngle(3.1415927410125732),
      mEllipseRadius(0),
      mEllipseC(0),
      mLargestOffset(0),
      mIcebergState(NULL)
{
}

IcebergFeature::~IcebergFeature()
{
}

void IcebergFeature::setSnowOnTop(bool snowOnTop)
{
    mSnowOnTop = snowOnTop;
}

void IcebergFeature::setLargestOffset(int largestOffset)
{
    mLargestOffset = largestOffset;
}

int IcebergFeature::heightDependentRadiusEllipse(int height, int totalHeight, int radius)
{
    float falloff = pow((float)height, 2) / (float)totalHeight;
    float scaledRadius = (1.0f - falloff) * (float)radius;
    return (int)ceilf(scaledRadius * 0.5f);
}

int IcebergFeature::heightDependentRadiusRound(Random& random, int height, int totalHeight, int radius)
{
    float spread = 3.5f - random.nextFloat();
    float scaledRadius = (1.0f - pow((float)height, 2) / ((float)totalHeight * spread)) * (float)radius;
    if (totalHeight > 15 + random.nextInt(5))
    {
        int y = height < 3 + random.nextInt(6) ? height / 2 : height;
        scaledRadius = (1.0f - (float)y / ((float)totalHeight * spread * 0.4f)) * (float)radius;
    }
    return (int)ceilf(scaledRadius * 0.5f);
}

int IcebergFeature::heightDependentRadiusSteep(Random& random, int height, int totalHeight, int radius)
{
    float spread = 1.0f + random.nextFloat() * 0.5f;
    float falloff = (float)height / ((float)totalHeight * spread);
    float scaledRadius = (1.0f - falloff) * (float)radius;
    return (int)ceilf(scaledRadius * 0.5f);
}

void IcebergFeature::initEllipse(Random& random)
{
    mIsEllipse = random.nextDouble() > 0.8;
    if (mIsEllipse)
    {
        mAngle = random.nextDouble() * 2.0 * 3.1415927410125732;
        mEllipseRadius = 15 - mLargestOffset - random.nextInt(4);
        mEllipseC = random.nextInt(3) + 3;
    }
}

double IcebergFeature::signedDistanceEllipse(int x, int z, const BlockPos& origin, int radius, int c, double angle)
{
    return pow(((double)(x - origin.x) * cos(angle) - (double)(z - origin.z) * sin(angle)) / (double)radius, 2)
           + pow(((double)(x - origin.x) * sin(angle) + (double)(z - origin.z) * cos(angle)) / (double)c, 2) - 1.0;
}

int IcebergFeature::getEllipseC(int height, int totalHeight)
{
    int c = mEllipseC;
    if (height > 0 && totalHeight - height <= 3)
        c += totalHeight - height - 4;
    return c;
}

double IcebergFeature::signedDistanceCircle(int x, int z, const BlockPos& origin, int radius, Random& random)
{
    float wobble = 10.0f * Mth::clamp(random.nextFloat(), 0.2f, 0.8f) / (float)radius;
    return wobble + pow((float)(x - origin.x), 2) + pow((float)(z - origin.z), 2) - pow((float)radius, 2);
}

void IcebergFeature::setIcebergBlock(const BlockPos& pos, Level* level, Random& random, int height, int totalHeight)
{
    const BlockState* state = level->getBlockState(pos);
    Block* block = state->getBlock();
    if (state->getMaterial() == Material::AIR || block == Blocks::snow || block == Blocks::ice || block == Blocks::water
        || block == Blocks::flowing_water)
    {
        bool placeSnow = !mIsEllipse || random.nextDouble() > 0.05;
        int divisor = mIsEllipse ? 3 : 2;
        if (mSnowOnTop && block != Blocks::water
            && (double)height <= (double)random.nextInt(std::max(1, totalHeight / divisor)) + (double)totalHeight * 0.6
            && placeSnow)
            setBlock(level, pos, Blocks::snow->defaultBlockState(), Block::UPDATE_NONE);
        else
            setBlock(level, pos, mIcebergState, Block::UPDATE_NONE);
    }
}

void IcebergFeature::generateIcebergBlock(Level* level, Random& random, const BlockPos& origin, int height, int x,
                                          int y, int z, int radius, int ellipseRadius)
{
    BlockPos centre(0, 0, 0);
    double distance = mIsEllipse ? signedDistanceEllipse(x, z, centre, ellipseRadius, getEllipseC(y, height), mAngle)
                                 : signedDistanceCircle(x, z, centre, radius, random);
    if (distance < 0.0)
    {
        BlockPos pos = origin.offset(x, y, z);
        double edge = mIsEllipse ? -0.5 : (double)(-6 - random.nextInt(3));
        if (distance > edge && random.nextDouble() > 0.9)
            return;
        setIcebergBlock(pos, level, random, height - y, height);
    }
}

bool IcebergFeature::isIcebergBlock(Block* block)
{
    return block == Blocks::packed_ice || block == Blocks::snow || block == Blocks::blue_ice;
}

bool IcebergFeature::belowIsAir(Level* level, const BlockPos& pos)
{
    return level->getBlockState(pos.below())->getMaterial() == Material::AIR;
}

void IcebergFeature::smooth(Level* level, const BlockPos& origin, int radius, int height)
{
    int extent = mIsEllipse ? mEllipseRadius : radius / 2;
    for (int x = -extent; x <= extent; ++x)
    {
        for (int z = -extent; z <= extent; ++z)
        {
            for (int y = 0; y <= height; ++y)
            {
                BlockPos pos = origin.offset(x, y, z);
                Block* block = level->getBlockState(pos)->getBlock();
                if (!isIcebergBlock(block) && block != Blocks::snow_layer)
                    continue;

                if (belowIsAir(level, pos))
                {
                    setBlock(level, pos, Blocks::air->defaultBlockState(), Block::UPDATE_NONE);
                    setBlock(level, pos.above(), Blocks::air->defaultBlockState(), Block::UPDATE_NONE);
                }
                else if (isIcebergBlock(block))
                {
                    Block* neighbours[4];
                    neighbours[0] = level->getBlockState(pos.west())->getBlock();
                    neighbours[1] = level->getBlockState(pos.east())->getBlock();
                    neighbours[2] = level->getBlockState(pos.north())->getBlock();
                    neighbours[3] = level->getBlockState(pos.south())->getBlock();
                    int exposedSides = 0;
                    for (int i = 0; i < 4; ++i)
                    {
                        if (!isIcebergBlock(neighbours[i]))
                            ++exposedSides;
                    }
                    if (exposedSides >= 3)
                        setBlock(level, pos, Blocks::air->defaultBlockState(), Block::UPDATE_NONE);
                }
            }
        }
    }
}

void IcebergFeature::removeFloatingSnowLayer(Level* level, const BlockPos& pos)
{
    if (level->getBlockId(pos.x, pos.y + 1, pos.z) == BlockId::SNOW_LAYER)
        setBlock(level, pos.above(), Blocks::air->defaultBlockState(), Block::UPDATE_NONE);
}

void IcebergFeature::carve(int radius, int y, const BlockPos& origin, Level* level, bool placeWater, double angle,
                           const BlockPos& cutCentre)
{
    int extent = radius + 1 + mEllipseRadius / 3;
    int c = std::min(radius - 3, 3) + mEllipseC / 2 - 1;
    for (int x = -extent; x < extent; ++x)
    {
        for (int z = -extent; z < extent; ++z)
        {
            double distance = signedDistanceEllipse(x, z, cutCentre, extent, c, angle);
            if (distance < 0.0)
            {
                BlockPos pos = origin.offset(x, y, z);
                Block* block = level->getBlockState(pos)->getBlock();
                if (isIcebergBlock(block) || block == Blocks::snow_layer)
                {
                    if (placeWater)
                    {
                        setBlock(level, pos, Blocks::water->defaultBlockState(), Block::UPDATE_NONE);
                    }
                    else
                    {
                        setBlock(level, pos, Blocks::air->defaultBlockState(), Block::UPDATE_NONE);
                        removeFloatingSnowLayer(level, pos);
                    }
                }
            }
        }
    }
}

void IcebergFeature::generateCutOut(Random& random, Level* level, int radius, int height, const BlockPos& origin)
{
    int xSign = random.nextBoolean() ? -1 : 1;
    int zSign = random.nextBoolean() ? -1 : 1;
    int x = random.nextInt(std::max(radius / 2 - 2, 1));
    if (random.nextBoolean())
        x = radius / 2 + 1 - random.nextInt(std::max(radius - radius / 2 - 1, 1));
    int z = random.nextInt(std::max(radius / 2 - 2, 1));
    if (random.nextBoolean())
        z = radius / 2 + 1 - random.nextInt(std::max(radius - radius / 2 - 1, 1));
    if (mIsEllipse)
        x = z = random.nextInt(std::max(mEllipseRadius - 5, 1));

    BlockPos cutCentre = BlockPos(0, 0, 0).offset(xSign * x, 0, zSign * z);
    double angle = mIsEllipse ? mAngle + 1.5707963705062866 : random.nextDouble() * 2.0 * 3.1415927410125732;

    for (int y = 0; y < height - 3; ++y)
    {
        int layerRadius = heightDependentRadiusRound(random, y, height, radius);
        carve(layerRadius, y, origin, level, false, angle, cutCentre);
    }

    for (int y = -1; y > -height + random.nextInt(5); --y)
    {
        int layerRadius = heightDependentRadiusSteep(random, -y, height, radius);
        carve(layerRadius, y, origin, level, true, angle, cutCentre);
    }
}

bool IcebergFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    setSnowOnTop(random.nextDouble() > 0.7);
    initEllipse(random);
    if (random.nextDouble() > 0.01)
        mIcebergState = Blocks::packed_ice->defaultBlockState();
    else
        mIcebergState = Blocks::blue_ice->defaultBlockState();

    BlockPos pos(origin.x, level->getSeaLevel(), origin.z);
    int height = mIsEllipse ? random.nextInt(4) + 6 : random.nextInt(11) + 3;
    if (!mIsEllipse && random.nextDouble() > 0.9)
        height += random.nextInt(15) + 7;
    int depth = std::min(height + random.nextInt(11), 18);
    int radius = std::min(height + random.nextInt(7) - random.nextInt(5), 11);
    int extent = mIsEllipse ? mEllipseRadius : 11;

    for (int x = -extent; x < extent; ++x)
    {
        for (int z = -extent; z < extent; ++z)
        {
            for (int y = 0; y < height; ++y)
            {
                int layerRadius = mIsEllipse ? heightDependentRadiusEllipse(y, height, radius)
                                             : heightDependentRadiusRound(random, y, height, radius);
                if (mIsEllipse || x < layerRadius)
                    generateIcebergBlock(level, random, pos, height, x, y, z, layerRadius, extent);
            }
        }
    }

    smooth(level, pos, radius, height);

    for (int x = -extent; x < extent; ++x)
    {
        for (int z = -extent; z < extent; ++z)
        {
            for (int y = -1; y > -depth; --y)
            {
                int ellipseRadius = mIsEllipse
                                        ? (int)ceilf((float)extent * (1.0f - pow((float)y, 2) / ((float)depth * 8.0f)))
                                        : extent;
                int layerRadius = heightDependentRadiusSteep(random, -y, depth, radius);
                if (x < layerRadius)
                    generateIcebergBlock(level, random, pos, depth, x, y, z, layerRadius, ellipseRadius);
            }
        }
    }

    bool cutOut = mIsEllipse ? random.nextDouble() > 0.1 : random.nextDouble() > 0.7;
    if (cutOut)
        generateCutOut(random, level, radius, height, pos);
    return true;
}
