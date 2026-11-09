#include "game/world/biome/SwampBiome.hpp"
#include "game/core/EntityType.hpp"

#include "game/core/ColourTable.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/PerlinSimplexNoise.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/biome/BiomeDecorator.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/chunk/ChunkPrimer.hpp"
#include "game/world/generation/FossilFeature.hpp"
#include "game/world/generation/SwampTreeFeature.hpp"
#include "game/core/PIXNamedEvents.hpp"

static const Biome::BlockPair WATERLILY_BLOCK(111, 0);

SwampBiome::SwampBiome(EBiomeIDs biomeId, BiomeProperties* properties) : Biome(biomeId, eBiomeType_Swamp, properties)
{
    mDecorator->treesPerChunk = 2;
    mDecorator->flowersPerChunk = 1;
    mDecorator->deadBushPerChunk = 1;
    mDecorator->mushroomsPerChunk = 8;
    mDecorator->reedsPerChunk = 10;
    mDecorator->clayPerChunk = 1;
    mDecorator->waterlilyPerChunk = 4;
    mDecorator->sandPerChunk2 = 0;
    mDecorator->sandPerChunk = 0;
    mDecorator->grassPerChunk = 5;
    mDecorator->seagrassPerChunk = 10;
    mSlimes.push_back(new MobSpawnerData(1, EntityType::SLIME, 1, 1));
    mMonsters.push_back(new MobSpawnerData(5, EntityType::DROWNED, 2, 4));
}

AbstractTreeFeature* SwampBiome::getTreeFeature(Random& random)
{
    return new SwampTreeFeature();
}

unsigned int SwampBiome::getGrassColor(const BlockPos& pos)
{
    int z = pos.z;
    int x = pos.x;
    double noise = BIOME_INFO_NOISE->getValue((double)x * 0.0225, (double)z * 0.0225);
    ColourTable* colours = Minecraft::GetInstance()->getColourTable();

    if (noise < -0.1)
    {
        return colours->getColour(eMinecraftColour_Grass_Swamp1);
    }

    return colours->getColour(eMinecraftColour_Grass_Swamp2);
}

unsigned int SwampBiome::getFoliageColor(const BlockPos& pos)
{
    return Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_Foliage_Swampland);
}

FlowerBlock::Flower* SwampBiome::getRandomFlower(Random& random, const BlockPos& pos)
{
    return FlowerBlock::Flower::BLUE_ORCHID;
}

void SwampBiome::buildSurfaceAt(Level* level, Random& random, ChunkPrimer* primer, int x, int z, double noise)
{
    PIXBeginNamedEvent(0.0F, "SwampBiome::buildSurfaceAtDefault");
    double value = BIOME_INFO_NOISE->getValue((double)x * 0.25, (double)z * 0.25);
    if (value > 0.0)
    {
        const int localX = x & 15;
        const int localZ = z & 15;
        const int column = (localZ << 11) | (localX << 7);
        for (int y = 127; y >= 0; --y)
        {
            const int index = column | y;
            const BlockState* state = primer->getState(localZ, y, localX);
            Block* block = state->getBlock();
            if (block->mMaterial != Material::AIR)
            {
                if (y == 62 && block != Blocks::water)
                {
                    primer->setBlockAndData(index, WATER_BLOCK.blockId, WATER_BLOCK.blockData);
                    if (value < 0.12)
                    {
                        primer->setBlockAndData(index + 1, WATERLILY_BLOCK.blockId, WATERLILY_BLOCK.blockData);
                    }
                }
                break;
            }
        }
    }
    buildSurfaceAtDefault(level, random, primer, x, z, noise);
    PIXEndNamedEvent();
}

void SwampBiome::decorate(Level* level, Random& random, const BlockPos& pos)
{
    Biome::decorate(level, random, pos);
    if (random.nextInt(64) == 0)
    {
        FossilFeature fossil = FossilFeature();
        fossil.place(level, random, pos);
    }
}

Biome::EBiomeBaseClass SwampBiome::getBaseClass()
{
    return eBiomeBaseClass_Swamp;
}
