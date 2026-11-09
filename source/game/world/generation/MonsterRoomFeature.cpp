#include "game/world/generation/MonsterRoomFeature.hpp"
#include "game/core/EntityType.hpp"

#include <string>
#include <vector>

#include "game/core/BuiltInLootTables.hpp"
#include "game/core/Direction.hpp"
#include "game/entity/mob/EntityIO.hpp"
#include "game/util/Random.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/ChestBlock.hpp"
#include "game/world/block/ChestBlockEntity.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/MobSpawnerBlockEntity.hpp"

std::wstring MonsterRoomFeature::randomEntityId(Random& random)
{
    static std::wstring entityIds[] = {
        EntityIO::getKey(EntityType::SKELETON).toString(),
        EntityIO::getKey(EntityType::ZOMBIE).toString(),
        EntityIO::getKey(EntityType::ZOMBIE).toString(),
        EntityIO::getKey(EntityType::SPIDER).toString(),
    };

    return entityIds[random.nextInt(4)];
}

bool MonsterRoomFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    int xExtent = random.nextInt(2) + 2;
    int xMin = -xExtent - 1;
    int xMax = xExtent + 1;
    int zExtent = random.nextInt(2) + 2;
    int zMin = -zExtent - 1;
    int zMax = zExtent + 1;
    int openings = 0;

    for (int x = xMin; x <= xMax; ++x)
    {
        for (int y = -1; y <= 4; ++y)
        {
            for (int z = zMin; z <= zMax; ++z)
            {
                BlockPos pos = origin.offset(x, y, z);
                bool solid = level->getBlockState(pos)->getMaterial()->isSolid();
                if ((y == -1 || y == 4) && !solid)
                    return false;

                if ((x == xMin || x == xMax || z == zMin || z == zMax) && y == 0 && level->isEmptyBlock(pos)
                    && level->isEmptyBlock(pos.above()))
                    ++openings;
            }
        }
    }

    if (openings < 1 || openings > 5)
        return false;

    for (int x = xMin; x <= xMax; ++x)
    {
        for (int y = 3; y >= -1; --y)
        {
            for (int z = zMin; z <= zMax; ++z)
            {
                BlockPos pos = origin.offset(x, y, z);
                if (x == xMin || y == -1 || z == zMin || x == xMax || z == zMax)
                {
                    if (pos.y >= 0 && !level->getBlockState(pos.below())->getMaterial()->isSolid())
                    {
                        level->removeBlock(pos);
                    }
                    else if (level->getBlockState(pos)->getMaterial()->isSolid()
                             && level->getBlockState(pos)->getBlock() != Blocks::chest)
                    {
                        if (y == -1 && random.nextInt(4) != 0)
                            level->setBlock(pos, Blocks::mossy_cobblestone->defaultBlockState(), Block::UPDATE_CLIENTS,
                                            false);
                        else
                            level->setBlock(pos, Blocks::cobblestone->defaultBlockState(), Block::UPDATE_CLIENTS,
                                            false);
                    }
                }
                else if (level->getBlockState(pos)->getBlock() != Blocks::chest)
                {
                    level->removeBlock(pos);
                }
            }
        }
    }

    for (int chest = 0; chest < 2; ++chest)
    {
        for (int attempt = 0; attempt < 3; ++attempt)
        {
            int chestX = origin.x + random.nextInt(xExtent * 2 + 1) - xExtent;
            int chestY = origin.y;
            int chestZ = origin.z + random.nextInt(zExtent * 2 + 1) - zExtent;
            const BlockPos& pos = BlockPos(chestX, chestY, chestZ);
            if (!level->isEmptyBlock(pos))
                continue;

            int solidNeighbours = 0;
            arrayWithLength<const Direction*> directions = Direction::Plane::HORIZONTAL->iterator();
            for (unsigned int i = 0; i < directions.length; ++i)
            {
                if (level->getBlockState(pos.relative(directions.data[i]))->getMaterial()->isSolid())
                    ++solidNeighbours;
            }
            if (solidNeighbours != 1)
                continue;

            level->setBlock(
                pos, static_cast<ChestBlock*>(Blocks::chest)->reorient(level, pos, Blocks::chest->defaultBlockState()),
                Block::UPDATE_CLIENTS, false);
            boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(pos);
            if (eTYPE_DERIVED_FROM(BlockEntityType::CHEST, blockEntity->GetType()))
            {
                boost::shared_ptr<ChestBlockEntity> chestEntity = boost::static_pointer_cast<ChestBlockEntity>(
                    blockEntity);
                chestEntity->setLootTable(&BuiltInLootTables::SIMPLE_DUNGEON, random.nextLong());
            }
            break;
        }
    }

    level->setBlock(origin, Blocks::mob_spawner->defaultBlockState(), Block::UPDATE_CLIENTS, false);
    boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(origin);
    if (eTYPE_DERIVED_FROM(BlockEntityType::MOB_SPAWNER, blockEntity->GetType()))
    {
        boost::shared_ptr<MobSpawnerBlockEntity> spawnerBlockEntity = boost::static_pointer_cast<MobSpawnerBlockEntity>(
            blockEntity);
        spawnerBlockEntity->getSpawner()->setEntityId(ResourceLocation(randomEntityId(random)));
    }
    return true;
}
