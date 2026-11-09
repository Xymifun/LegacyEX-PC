#include "game/world/biome/DesertBiome.hpp"
#include "game/core/EntityType.hpp"

#include <vector>

#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/biome/BiomeDecorator.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockId.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/generation/DesertWellFeature.hpp"
#include "game/world/generation/FossilFeature.hpp"

DesertBiome::DesertBiome(EBiomeIDs biomeId, BiomeProperties* properties) : Biome(biomeId, eBiomeType_Desert, properties)
{
    mCreatures.clear();
    mCreatures.push_back(new MobSpawnerData(4, EntityType::RABBIT, 2, 3));
    mChickens.clear();
    mWolves.clear();

    mTopBlock = BlockPair(BlockId::SAND,
                          Blocks::sand->convertBlockStateToLegacyData(Blocks::sand->defaultBlockState()));
    mFillerBlock = BlockPair(BlockId::SAND,
                             Blocks::sand->convertBlockStateToLegacyData(Blocks::sand->defaultBlockState()));

    mDecorator->treesPerChunk = -999;
    mDecorator->deadBushPerChunk = 2;
    mDecorator->reedsPerChunk = 50;
    mDecorator->cactiPerChunk = 10;

    for (std::vector<MobSpawnerData*>::iterator iterator = mMonsters.begin(); iterator != mMonsters.end(); ++iterator)
    {
        MobSpawnerData* data = *iterator;
        if (data->mEntityBaseClass == EntityType::ZOMBIE || data->mEntityBaseClass == EntityType::ZOMBIE_VILLAGER)
        {
            iterator = mMonsters.erase(iterator);
            delete data;
        }
    }

    mMonsters.push_back(new MobSpawnerData(19, EntityType::ZOMBIE, 4, 4));
    mMonsters.push_back(new MobSpawnerData(1, EntityType::ZOMBIE_VILLAGER, 1, 1));
    mMonsters.push_back(new MobSpawnerData(80, EntityType::HUSK, 4, 4));
}

void DesertBiome::decorate(Level* level, Random& random, const BlockPos& origin)
{
    Biome::decorate(level, random, origin);

    if (random.nextInt(1000) == 0)
    {
        const int x = random.nextInt(16) + 8;
        const int z = random.nextInt(16) + 8;
        const BlockPos wellPosition = level->getHeightmapPos(origin.offset(x, 0, z)).above();
        DesertWellFeature().place(level, random, wellPosition);
    }

    if (random.nextInt(64) == 0)
    {
        FossilFeature().place(level, random, origin);
    }
}

Biome::EBiomeBaseClass DesertBiome::getBaseClass()
{
    return eBiomeBaseClass_Desert;
}
