#include "game/world/biome/BiomeDecorator.hpp"

#include "game/core/Bush.hpp"
#include "game/core/CustomizableSourceSettings.hpp"
#include "game/core/MemSect.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/core/SuperflatConfig.hpp"
#include "game/world/block/Material.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/FlowerBlock.hpp"
#include "game/world/block/MutableBlockPos.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/StoneBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/generation/AbstractTreeFeature.hpp"
#include "game/world/generation/BlueIceFeature.hpp"
#include "game/world/generation/BushFeature.hpp"
#include "game/world/generation/CactusFeature.hpp"
#include "game/world/generation/ClayFeature.hpp"
#include "game/world/generation/CoralCrustFeature.hpp"
#include "game/world/generation/CoralFeature.hpp"
#include "game/world/generation/CoralHangFeature.hpp"
#include "game/world/generation/DeadBushFeature.hpp"
#include "game/world/generation/FlowerFeature.hpp"
#include "game/world/generation/HugeMushroomFeature.hpp"
#include "game/world/generation/IcebergFeature.hpp"
#include "game/world/generation/OreFeature.hpp"
#include "game/world/generation/PumpkinFeature.hpp"
#include "game/world/generation/ReedsFeature.hpp"
#include "game/world/generation/SandFeature.hpp"
#include "game/world/generation/SeaAnemoneFeature.hpp"
#include "game/world/generation/SeagrassFeature.hpp"
#include "game/world/generation/SeaPickleFeature.hpp"
#include "game/world/generation/SpringFeature.hpp"
#include "game/world/generation/WaterlilyFeature.hpp"

BiomeDecorator::BiomeDecorator()
{
    clayGen = new ClayFeature(4);
    sandGen = new SandFeature(Blocks::sand, 7);
    gravelAsSandGen = new SandFeature(Blocks::gravel, 6);
    dirtGen = NULL;
    gravelGen = NULL;
    graniteGen = NULL;
    dioriteGen = NULL;
    andesiteGen = NULL;
    coalGen = NULL;
    ironGen = NULL;
    goldGen = NULL;
    redstoneGen = NULL;
    diamondGen = NULL;
    lapisGen = NULL;
    flowerGen = new FlowerFeature(static_cast<FlowerBlock*>(Blocks::yellow_flower), FlowerBlock::Flower::DANDELION);
    brownMushroomGen = new BushFeature(static_cast<Bush*>(Blocks::brown_mushroom));
    redMushroomGen = new BushFeature(static_cast<Bush*>(Blocks::red_mushroom));
    bigMushroomGen = new HugeMushroomFeature();
    reedGen = new ReedsFeature();
    cactusGen = new CactusFeature();
    waterlilyGen = new WaterlilyFeature();
    kelpGen = new KelpFeature();
    seagrassGen = new SeagrassFeature();
    coralGen = new CoralFeature();
    seaPickleGen = new SeaPickleFeature();
    icebergGen = new IcebergFeature();
    blueIceGen = new BlueIceFeature();
    coralCrustGen = new CoralCrustFeature();
    coralHangGen = new CoralHangFeature();
    seaAnemoneGen = new SeaAnemoneFeature();
    mushroomsPerChunk = 0;
    deadBushPerChunk = 0;
    waterlilyPerChunk = 0;
    treesPerChunk = 0;
    extraTreeChance = 0.1f;
    flowersPerChunk = 2;
    grassPerChunk = 1;
    reedsPerChunk = 0;
    cactiPerChunk = 0;
    bigMushroomsPerChunk = 0;
    kelpPerChunk = 0;
    sandPerChunk = 1;
    sandPerChunk2 = 3;
    clayPerChunk = 1;
    seaGrassPerChunk = 0;
    seagrassPerChunk = 0;
    decorating = false;
    settings = NULL;
    generateLakes = true;
}

BiomeDecorator::~BiomeDecorator()
{
    delete clayGen;
    delete sandGen;
    delete gravelAsSandGen;
    delete dirtGen;
    delete gravelGen;
    delete graniteGen;
    delete dioriteGen;
    delete andesiteGen;
    delete coalGen;
    delete ironGen;
    delete goldGen;
    delete redstoneGen;
    delete diamondGen;
    delete lapisGen;
    delete flowerGen;
    delete brownMushroomGen;
    delete redMushroomGen;
    delete bigMushroomGen;
    delete reedGen;
    delete cactusGen;
    delete waterlilyGen;
    delete settings;
    delete kelpGen;
    delete seagrassGen;
    delete coralGen;
    delete coralCrustGen;
    delete coralHangGen;
    delete seaAnemoneGen;
    delete seaPickleGen;
    delete icebergGen;
    delete blueIceGen;
}

void BiomeDecorator::decorate(Level* level, Random* random, Biome* biome, const BlockPos& origin)
{
    if (settings != NULL)
        delete settings;
    CustomizableSourceSettings::Builder* builder = CustomizableSourceSettings::Builder::fromString(
        level->getLevelData()->getGeneratorOptions());
    settings = builder->build();
    position = origin;

    delete dirtGen;
    delete gravelGen;
    delete graniteGen;
    delete dioriteGen;
    delete andesiteGen;
    delete coalGen;
    delete ironGen;
    delete goldGen;
    delete redstoneGen;
    delete diamondGen;
    delete lapisGen;

    dirtGen = new OreFeature(Blocks::dirt->defaultBlockState(), settings->dirtCount);
    gravelGen = new OreFeature(Blocks::gravel->defaultBlockState(), settings->gravelCount);
    graniteGen = new OreFeature(
        Blocks::stone->defaultBlockState()->setValue(StoneBlock::VARIANT, StoneBlock::Variant::GRANITE),
        settings->dioriteCount);
    dioriteGen = new OreFeature(
        Blocks::stone->defaultBlockState()->setValue(StoneBlock::VARIANT, StoneBlock::Variant::DIORITE),
        settings->graniteCount);
    andesiteGen = new OreFeature(
        Blocks::stone->defaultBlockState()->setValue(StoneBlock::VARIANT, StoneBlock::Variant::ANDESITE),
        settings->andesiteCount);
    coalGen = new OreFeature(Blocks::coal_ore->defaultBlockState(), settings->coalCount);
    ironGen = new OreFeature(Blocks::iron_ore->defaultBlockState(), settings->ironCount);
    goldGen = new OreFeature(Blocks::gold_ore->defaultBlockState(), settings->goldCount);
    redstoneGen = new OreFeature(Blocks::redstone_ore->defaultBlockState(), settings->redstoneCount);
    diamondGen = new OreFeature(Blocks::diamond_ore->defaultBlockState(), settings->diamondCount);
    lapisGen = new OreFeature(Blocks::lapis_ore->defaultBlockState(), settings->lapisCount);

    decorate(biome, level, random);
    decorating = false;
    delete builder;
}

void BiomeDecorator::decorateDepthSpan(Level* level, Random* random, int count, Feature* feature, int minY, int maxY)
{
    if (maxY < minY)
    {
        int swap = minY;
        minY = maxY;
        maxY = swap;
    }
    else if (maxY == minY)
    {
        if (minY < 255)
        {
            ++maxY;
        }
        else
        {
            --minY;
        }
    }

    const int originX = position.x;
    const int originZ = position.z;
    MutableBlockPos pos(0, 0, 0);
    for (int i = 0; i < count; ++i)
    {
        pos.set(originX + random->nextInt(16), random->nextInt(maxY - minY) + minY, originZ + random->nextInt(16));
        feature->place(level, *random, pos);
    }
}

void BiomeDecorator::decorateDepthAverage(Level* level, Random* random, int count, Feature* feature, int centreY,
                                          int radius)
{
    const int originX = position.x;
    const int originZ = position.z;
    MutableBlockPos pos(0, 0, 0);
    for (int i = 0; i < count; ++i)
    {
        pos.set(originX + random->nextInt(16), random->nextInt(radius) + random->nextInt(radius) + centreY - radius,
                originZ + random->nextInt(16));
        feature->place(level, *random, pos);
    }
}

void BiomeDecorator::decorateOres(Level* level, Random* random)
{
    Level::setInstaTick(true);
    decorateDepthSpan(level, random, settings->dirtSize, dirtGen, settings->dirtMinHeight, settings->dirtMaxHeight);
    decorateDepthSpan(level, random, settings->gravelSize, gravelGen, settings->gravelMinHeight,
                      settings->gravelMaxHeight);
    decorateDepthSpan(level, random, settings->graniteSize, dioriteGen, settings->graniteMinHeight,
                      settings->graniteMaxHeight);
    decorateDepthSpan(level, random, settings->dioriteSize, graniteGen, settings->dioriteMinHeight,
                      settings->dioriteMaxHeight);
    decorateDepthSpan(level, random, settings->andesiteSize, andesiteGen, settings->andesiteMinHeight,
                      settings->andesiteMaxHeight);
    decorateDepthSpan(level, random, settings->coalSize, coalGen, settings->coalMinHeight, settings->coalMaxHeight);
    decorateDepthSpan(level, random, settings->ironSize, ironGen, settings->ironMinHeight, settings->ironMaxHeight);
    decorateDepthSpan(level, random, settings->goldSize, goldGen, settings->goldMinHeight, settings->goldMaxHeight);
    decorateDepthSpan(level, random, settings->redstoneSize, redstoneGen, settings->redstoneMinHeight,
                      settings->redstoneMaxHeight);
    decorateDepthSpan(level, random, settings->diamondSize, diamondGen, settings->diamondMinHeight,
                      settings->diamondMaxHeight);
    decorateDepthAverage(level, random, settings->lapisSize, lapisGen, settings->lapisMinHeight,
                         settings->lapisMaxHeight);
    Level::setInstaTick(false);
}

void BiomeDecorator::decorate(Biome* biome, Level* level, Random* random)
{
    PIXBeginNamedEvent(0.0f, "Decorate ores");
    decorateOres(level, random);
    PIXEndNamedEvent();

    PIXBeginNamedEvent(0.0f, "Decorate sand/clay/gravel");
    for (int i = 0; i < sandPerChunk2; ++i)
    {
        const int x = random->nextInt(16);
        const int z = random->nextInt(16);
        sandGen->place(level, *random, level->getTopSolidBlockPos(position.offset(x + 8, 0, z + 8)));
    }
    for (int i = 0; i < clayPerChunk; ++i)
    {
        const int x = random->nextInt(16);
        const int z = random->nextInt(16);
        clayGen->place(level, *random, level->getTopSolidBlockPos(position.offset(x + 8, 0, z + 8)));
    }
    for (int i = 0; i < sandPerChunk; ++i)
    {
        const int x = random->nextInt(16);
        const int z = random->nextInt(16);
        gravelAsSandGen->place(level, *random, level->getTopSolidBlockPos(position.offset(x + 8, 0, z + 8)));
    }
    PIXEndNamedEvent();

    PIXBeginNamedEvent(0.0f, "Decorate forests");
    int treeCount = treesPerChunk;
    if (random->nextFloat() < extraTreeChance)
        ++treeCount;
    for (int i = 0; i < treeCount; ++i)
    {
        const int x = random->nextInt(16);
        const int z = random->nextInt(16);
        AbstractTreeFeature* tree = biome->getTreeFeature(*random);
        tree->init();
        BlockPos pos = level->getHeightmapPos(position.offset(x + 8, 0, z + 8));
        if (tree->place(level, *random, pos))
            tree->postPlaceTree(level, *random, pos);
        delete tree;
    }
    PIXEndNamedEvent();

    PIXBeginNamedEvent(0.0f, "Decorate mushrooms/flowers/grass");
    for (int i = 0; i < bigMushroomsPerChunk; ++i)
    {
        const int x = random->nextInt(16);
        const int z = random->nextInt(16);
        bigMushroomGen->place(level, *random, level->getHeightmapPos(position.offset(x + 8, 0, z + 8)));
    }
    for (int i = 0; i < flowersPerChunk; ++i)
    {
        int x = random->nextInt(16);
        int z = random->nextInt(16);
        BlockPos top = level->getHeightmapPos(position.offset(x + 8, 0, z + 8));
        if (top.y + 32 > 0)
        {
            BlockPos pos = position.offset(x + 8, random->nextInt(top.y + 32), z + 8);
            FlowerBlock::Flower* flower = biome->getRandomFlower(*random, pos);
            Block* block = flower->getCategory()->getBlock();
            if (block->mMaterial != Material::AIR)
            {
                flowerGen->setNextFlower(static_cast<FlowerBlock*>(block), flower);
                flowerGen->place(level, *random, pos);
            }
        }
    }
    for (int i = 0; i < grassPerChunk; ++i)
    {
        int x = random->nextInt(16);
        int z = random->nextInt(16);
        BlockPos top = level->getHeightmapPos(position.offset(x + 8, 0, z + 8));
        if (top.y * 2 > 0)
        {
            const int y = random->nextInt(top.y * 2);
            MemSect(50);
            Feature* grass = biome->getGrassFeature(*random);
            MemSect(0);
            grass->place(level, *random, position.offset(x + 8, y, z + 8));
            delete grass;
        }
    }
    for (int i = 0; i < seagrassPerChunk; ++i)
    {
        const int x = random->nextInt(16);
        const int z = random->nextInt(16);
        seagrassGen->place(level, *random, position.offset(x, 54, z));
    }
    PIXEndNamedEvent();

    PIXBeginNamedEvent(0.0f, "Decorate bush/waterlily/mushroom/reeds/pumpkins/cactuses");
    DeadBushFeature* deadBush = NULL;
    if (deadBushPerChunk > 0)
        deadBush = new DeadBushFeature(Blocks::deadbush);
    for (int i = 0; i < deadBushPerChunk; ++i)
    {
        int x = random->nextInt(16);
        int z = random->nextInt(16);
        BlockPos top = level->getHeightmapPos(position.offset(x + 8, 0, z + 8));
        if (top.y * 2 > 0)
        {
            const int y = random->nextInt(top.y * 2);
            deadBush->place(level, *random, position.offset(x + 8, y, z + 8));
        }
    }
    delete deadBush;

    for (int i = 0; i < waterlilyPerChunk; ++i)
    {
        int x = random->nextInt(16);
        int z = random->nextInt(16);
        BlockPos top = level->getHeightmapPos(position.offset(x + 8, 0, z + 8));
        if (top.y * 2 > 0)
        {
            BlockPos pos = position.offset(x + 8, random->nextInt(top.y * 2), z + 8);
            while (pos.y > 0)
            {
                BlockPos below = pos.below();
                if (!level->isEmptyBlock(below))
                    break;
                pos = below;
            }
            waterlilyGen->place(level, *random, pos);
        }
    }

    for (int i = 0; i < mushroomsPerChunk; ++i)
    {
        if (random->nextInt(4) == 0)
        {
            const int x = random->nextInt(16);
            const int z = random->nextInt(16);
            BlockPos pos = level->getHeightmapPos(position.offset(x + 8, 0, z + 8));
            brownMushroomGen->place(level, *random, pos);
        }
        if (random->nextInt(8) == 0)
        {
            int x = random->nextInt(16) + 8;
            int z = random->nextInt(16) + 8;
            BlockPos top = level->getHeightmapPos(position.offset(x, 0, z));
            if (top.y * 2 > 0)
            {
                BlockPos pos = position.offset(x, random->nextInt(top.y * 2), z);
                redMushroomGen->place(level, *random, pos);
            }
        }
    }
    if (random->nextInt(4) == 0)
    {
        int x = random->nextInt(16);
        int z = random->nextInt(16);
        BlockPos top = level->getHeightmapPos(position.offset(x + 8, 0, z + 8));
        if (top.y * 2 > 0)
        {
            const int y = random->nextInt(top.y * 2);
            brownMushroomGen->place(level, *random, position.offset(x + 8, y, z + 8));
        }
    }
    if (random->nextInt(8) == 0)
    {
        int x = random->nextInt(16);
        int z = random->nextInt(16);
        BlockPos top = level->getHeightmapPos(position.offset(x + 8, 0, z + 8));
        if (top.y * 2 > 0)
        {
            const int y = random->nextInt(top.y * 2);
            redMushroomGen->place(level, *random, position.offset(x + 8, y, z + 8));
        }
    }
    if (random->nextInt(18) == 0)
    {
        for (int i = 0; i < kelpPerChunk; ++i)
        {
            const int x = random->nextInt(35);
            const int z = random->nextInt(35);
            kelpGen->place(level, *random, position.offset(x, 43, z));
        }
    }
    for (int i = 0; i < reedsPerChunk; ++i)
    {
        int x = random->nextInt(16);
        int z = random->nextInt(16);
        BlockPos top = level->getHeightmapPos(position.offset(x + 8, 0, z + 8));
        if (top.y * 2 > 0)
        {
            const int y = random->nextInt(top.y * 2);
            reedGen->place(level, *random, position.offset(x + 8, y, z + 8));
        }
    }
    for (int i = 0; i < 10; ++i)
    {
        int x = random->nextInt(16);
        int z = random->nextInt(16);
        BlockPos top = level->getHeightmapPos(position.offset(x + 8, 0, z + 8));
        if (top.y * 2 > 0)
        {
            const int y = random->nextInt(top.y * 2);
            reedGen->place(level, *random, position.offset(x + 8, y, z + 8));
        }
    }
    if (random->nextInt(32) == 0)
    {
        int x = random->nextInt(16);
        int z = random->nextInt(16);
        BlockPos top = level->getHeightmapPos(position.offset(x + 8, 0, z + 8));
        if (top.y * 2 > 0)
        {
            const int y = random->nextInt(top.y * 2);
            Feature* pumpkin = new PumpkinFeature();
            pumpkin->place(level, *random, position.offset(x + 8, y, z + 8));
            delete pumpkin;
        }
    }
    for (int i = 0; i < cactiPerChunk; ++i)
    {
        int x = random->nextInt(16);
        int z = random->nextInt(16);
        BlockPos top = level->getHeightmapPos(position.offset(x + 8, 0, z + 8));
        if (top.y * 2 > 0)
        {
            const int y = random->nextInt(top.y * 2);
            cactusGen->place(level, *random, position.offset(x + 8, y, z + 8));
        }
    }
    PIXEndNamedEvent();

    PIXBeginNamedEvent(0.0f, "Decorate liquids");
    if (generateLakes)
    {
        Feature* waterSpring = new SpringFeature(Blocks::flowing_water);
        for (int i = 0; i < 50; ++i)
        {
            const int x = random->nextInt(16);
            const int z = random->nextInt(16);
            const int range = random->nextInt(120) + 8;
            if (range > 0)
            {
                const int y = random->nextInt(range);
                BlockPos pos = position.offset(x + 8, y, z + 8);
                waterSpring->place(level, *random, pos);
            }
        }

        delete waterSpring;

        Feature* lavaSpring = new SpringFeature(Blocks::flowing_lava);
        for (int i = 0; i < 20; ++i)
        {
            const int x = random->nextInt(16) + 8;
            const int z = random->nextInt(16) + 8;
            const int range = random->nextInt(random->nextInt(112) + 8) + 8;
            const int y = random->nextInt(range);
            BlockPos pos = position.offset(x, y, z);
            lavaSpring->place(level, *random, pos);
        }
        delete lavaSpring;
    }
    PIXEndNamedEvent();
}
