#include "game/world/biome/IceBiome.hpp"
#include "game/core/EntityType.hpp"

#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockId.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/generation/AbstractTreeFeature.hpp"
#include "game/world/generation/Feature.hpp"
#include "game/world/generation/IcePatchFeature.hpp"
#include "game/world/generation/IceSpikeFeature.hpp"
#include "game/world/generation/SpruceFeature.hpp"

IceBiome::IceBiome(EBiomeIDs biomeId, bool hasIceSpikes, BiomeProperties* properties)
    : Biome(biomeId, eBiomeType_Ice, properties)
{
    mIceSpikeFeature = new IceSpikeFeature();
    mIcePatchFeature = new IcePatchFeature(4);
    mHasIceSpikes = hasIceSpikes;

    if (hasIceSpikes)
    {
        mTopBlock = BlockPair(BlockId::SNOW,
                              Blocks::snow->convertBlockStateToLegacyData(Blocks::snow->defaultBlockState()));
    }

    mCreatures.clear();
    mCreatures.push_back(new MobSpawnerData(10, EntityType::RABBIT, 2, 3));
    mCreatures.push_back(new MobSpawnerData(1, EntityType::POLAR_BEAR, 1, 2));

    for (std::vector<MobSpawnerData*>::iterator monster = mMonsters.begin(); monster != mMonsters.end();)
    {
        if ((*monster)->mEntityBaseClass == EntityType::SKELETON)
        {
            monster = mMonsters.erase(monster);
        }
        else
        {
            ++monster;
        }
    }

    mMonsters.push_back(new MobSpawnerData(20, EntityType::SKELETON, 4, 4));
    mMonsters.push_back(new MobSpawnerData(80, EntityType::STRAY, 4, 4));
}

float IceBiome::getCreatureProbability()
{
    return 0.07F;
}

void IceBiome::decorate(Level* level, Random& random, const BlockPos& pos)
{
    if (mHasIceSpikes)
    {
        for (int i = 0; i < 3; ++i)
        {
            int x = random.nextInt(16) + 8;
            int z = random.nextInt(16) + 8;
            mIceSpikeFeature->place(level, random, level->getHeightmapPos(pos.offset(x, 0, z)));
        }

        for (int i = 0; i < 2; ++i)
        {
            int x = random.nextInt(16) + 8;
            int z = random.nextInt(16) + 8;
            mIcePatchFeature->place(level, random, level->getHeightmapPos(pos.offset(x, 0, z)));
        }
    }

    Biome::decorate(level, random, pos);
}

AbstractTreeFeature* IceBiome::getTreeFeature(Random& random)
{
    return new SpruceFeature(false);
}

Biome::EBiomeBaseClass IceBiome::getBaseClass()
{
    return eBiomeBaseClass_Ice;
}
