#include "game/world/block/OcelotSitOnBlockGoal.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/Ocelot.hpp"
#include "game/entity/ai/SitGoal.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BedBlock.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/ChestBlockEntity.hpp"

OcelotSitOnBlockGoal::OcelotSitOnBlockGoal(Ocelot* ocelot, double speedModifier)
    : MoveToBlockGoal(ocelot, speedModifier, 8, 1, 1.0),
      mOcelot(ocelot)
{
}

OcelotSitOnBlockGoal::~OcelotSitOnBlockGoal()
{
}

bool OcelotSitOnBlockGoal::canUse()
{
    return mOcelot->isTame() && !mOcelot->isSitting() && MoveToBlockGoal::canUse();
}

void OcelotSitOnBlockGoal::start()
{
    MoveToBlockGoal::start();
    mOcelot->getSitGoal()->wantToSit(false);
    mOcelot->setSittingOnBlock(true);
}

void OcelotSitOnBlockGoal::stop()
{
    Goal::stop();
    mOcelot->setSitting(false);
    mOcelot->setSittingOnBlock(false);
}

void OcelotSitOnBlockGoal::tick()
{
    MoveToBlockGoal::tick();
    mOcelot->getSitGoal()->wantToSit(false);
    if (!MoveToBlockGoal::isReachedTarget())
    {
        mOcelot->setSitting(false);
        return;
    }

    if (!mOcelot->isSitting())
    {
        mOcelot->setSitting(true);
    }
}

bool OcelotSitOnBlockGoal::isValidTarget(Level* level, const BlockPos& pos)
{
    const BlockState* state = level->getBlockState(pos);
    Block* block = state->getBlock();
    if (block == Blocks::chest)
    {
        boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(pos);
        boost::shared_ptr<ChestBlockEntity> chest = boost::dynamic_pointer_cast<ChestBlockEntity>(
            level->getBlockEntity(pos));
        if (chest != NULL && chest->mOpenCount < 1)
        {
            return true;
        }
    }
    else if (block == Blocks::lit_furnace)
    {
        return true;
    }
    else if (block == Blocks::bed && state->getValue(BedBlock::PART) != &gBedHead)
    {
        return true;
    }

    return false;
}
