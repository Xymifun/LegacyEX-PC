#include "game/world/block/DeadBushBlock.hpp"

#include "game/core/GenericStats.hpp"
#include "game/core/Stats.hpp"
#include "game/util/AABB.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/generation/Player.hpp"

static const AABB* gDeadBushShape = AABB::newPermanent(0.1, 0.0, 0.1, 0.9, 0.8, 0.9);

DeadBushBlock::DeadBushBlock() : Bush(Material::REPLACEABLE_PLANT)
{
    DerivedInit();
}

const AABB* DeadBushBlock::getShape(const BlockState*, LevelSource*, const BlockPos&)
{
    return gDeadBushShape;
}

const MaterialColor* DeadBushBlock::getMapColor(const BlockState*, LevelSource*, const BlockPos&)
{
    return MaterialColor::WOOD;
}

bool DeadBushBlock::mayPlaceOn(Level*, const BlockPos&, const BlockState* state)
{
    return state->getBlock() == Blocks::sand || state->getBlock() == Blocks::hardened_clay
           || state->getBlock() == Blocks::stained_hardened_clay || state->getBlock() == Blocks::dirt;
}

bool DeadBushBlock::mayReplaceWithPlace(LevelSource*, const BlockPos&)
{
    return true;
}

Item* DeadBushBlock::getResource(const BlockState*, Random*, int)
{
    return Items::stick;
}

void DeadBushBlock::playerDestroy(Level* level, boost::shared_ptr<Player> player, const BlockPos& pos,
                                  const BlockState* state, boost::shared_ptr<BlockEntity> blockEntity,
                                  not_null_ptr<ItemInstance> tool)
{
    if (level->mIsClientSide || tool->getItem() != Items::shears)
    {
        Block::playerDestroy(level, player, pos, state, blockEntity, tool);
    }
    else
    {
        Player* awardee = player.operator->();
        Stat* stat = gStats->get_blocksMined(Block::getId(this));
        awardee->awardStat(stat, GenericStats::param_blocksMined(state, 1));
        Block::popResource(level, pos, not_null_ptr<ItemInstance>(new ItemInstance(Blocks::deadbush, 1, 0)));
    }
}
