#include "game/world/block/RedstoneTorchBlock.hpp"

#include <deque>
#include <typeinfo>
#include <boost/unordered_map.hpp>

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/core/Boxed.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/Direction.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/Property.hpp"
#include "game/world/block/TypedBoxed.hpp"

static boost::unordered_map<Level*, std::deque<RedstoneTorchBlock::Toggle>*> sToggleLists;

RedstoneTorchBlock::RedstoneTorchBlock(bool lit) : TorchBlock()
{
    ClearDerivedInit();
    DerivedInit();
    mLit = lit;
    setTicking(true);
}

void RedstoneTorchBlock::removeLevelReferences(Level* level)
{
    boost::unordered_map<Level*, std::deque<Toggle>*>::iterator it = sToggleLists.find(level);
    if (it != sToggleLists.end())
    {
        delete sToggleLists[level];
        sToggleLists.erase(level);
    }
}

bool RedstoneTorchBlock::isToggledTooFrequently(Level* level, const BlockPos& pos, bool addToggle)
{
    if (sToggleLists.find(level) == sToggleLists.end())
        sToggleLists[level] = new std::deque<Toggle>;
    if (addToggle)
        sToggleLists[level]->push_back(Toggle(pos, level->getGameTime()));
    int count = 0;
    std::deque<Toggle>::const_iterator end = sToggleLists[level]->end();
    for (std::deque<Toggle>::const_iterator it = sToggleLists[level]->begin(); it != end; ++it)
        if (it->pos.equals(pos) && ++count >= 8)
            return true;
    return false;
}

int RedstoneTorchBlock::getTickDelay(Level*)
{
    return 2;
}

void RedstoneTorchBlock::onPlace(Level* level, const BlockPos& pos, const BlockState*)
{
    if (mLit)
        for (std::vector<const Direction*>::const_iterator it = Direction::VALUES.begin();
             it != Direction::VALUES.end(); ++it)
            level->updateNeighborsAt(pos.relative(*it), this, false);
}

void RedstoneTorchBlock::onRemove(Level* level, const BlockPos& pos, const BlockState*)
{
    if (mLit)
        for (std::vector<const Direction*>::const_iterator it = Direction::VALUES.begin();
             it != Direction::VALUES.end(); ++it)
            level->updateNeighborsAt(pos.relative(*it), this, false);
}

int RedstoneTorchBlock::getSignal(const BlockState* state, LevelSource*, const BlockPos&, const Direction* direction)
{
    return mLit && state->getValue(FACING) != direction ? 15 : 0;
}

bool RedstoneTorchBlock::hasNeighborSignal(Level* level, const BlockPos& pos, const BlockState* state)
{
    const Direction* direction = state->getValue(FACING)->getOpposite();
    return level->hasSignal(pos.relative(direction), direction);
}

void RedstoneTorchBlock::randomTick(Level*, const BlockPos&, const BlockState*, Random*)
{
}

void RedstoneTorchBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random)
{
    bool powered = hasNeighborSignal(level, pos, state);

    if (sToggleLists.find(level) != sToggleLists.end())
    {
        std::deque<Toggle>* toggles = sToggleLists[level];
        while (!toggles->empty() && level->getGameTime() - toggles->front().time > 60)
            toggles->pop_front();
    }

    int flags = gApp.GetGameHostOption(eGameHostOption_MiniGame) ? 0x25 : 3;
    if (mLit)
    {
        if (powered)
        {
            level->setBlock(
                pos, Blocks::unlit_redstone_torch->defaultBlockState()->setValue(FACING, state->getValue(FACING)),
                flags, false);
            if (isToggledTooFrequently(level, pos, true))
            {
                level->playSound(boost::shared_ptr<Player>(), pos, SoundEvents::REDSTONE_TORCH_BURNOUT,
                                 SoundSource::eSoundSource_Blocks, 0.5f,
                                 2.6f + (level->mRandom->nextFloat() - level->mRandom->nextFloat()) * 0.8f, 16.0f);

                for (int i = 0; i < 5; ++i)
                {
                    double x = pos.x + random->nextDouble() * 0.6 + 0.2;
                    double y = pos.y + random->nextDouble() * 0.6 + 0.2;
                    double z = pos.z + random->nextDouble() * 0.6 + 0.2;
                    level->addParticle(PARTICLE_SMOKE, x, y, z, 0.0, 0.0, 0.0, arrayWithLength<int>());
                }

                level->addToTickNextTick(pos, level->getBlockState(pos)->getBlock(), 0xA0);
            }
        }
    }
    else if (!powered && !isToggledTooFrequently(level, pos, false))
    {
        level->setBlock(pos, Blocks::redstone_torch->defaultBlockState()->setValue(FACING, state->getValue(FACING)),
                        flags, false);
    }
}

void RedstoneTorchBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block*,
                                         const BlockPos&)
{
    if (!checkDoPop(level, pos, state) && hasNeighborSignal(level, pos, state) == mLit)
        level->addToTickNextTick(pos, this, getTickDelay(level));
}

int RedstoneTorchBlock::getDirectSignal(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                                        const Direction* direction)
{
    return direction == Direction::DOWN ? getSignal(state, levelSource, pos, direction) : 0;
}

Item* RedstoneTorchBlock::getResource(const BlockState*, Random*, int)
{
    return Item::byBlock(Blocks::redstone_torch);
}

bool RedstoneTorchBlock::isSignalSource(const BlockState*)
{
    return true;
}

void RedstoneTorchBlock::animateTick(const BlockState* state, Level* level, const BlockPos& pos, Random* random)
{
    if (!mLit)
        return;
    double x = pos.x + 0.5 + (random->nextDouble() - 0.5) * 0.2;
    double y = pos.y + 0.7 + (random->nextDouble() - 0.5) * 0.2;
    double z = pos.z + 0.5 + (random->nextDouble() - 0.5) * 0.2;
    const Direction* facing = state->getValue(FACING);
    if (facing->getAxis()->isHorizontal())
    {
        const Direction* opposite = facing->getOpposite();
        const double offset = 0.27;
        x += offset * opposite->getStepX();
        y += 0.22;
        z += offset * opposite->getStepZ();
    }
    level->addParticle(PARTICLE_RED_DUST, x, y, z, 0.0, 0.0, 0.0, arrayWithLength<int>());
}

not_null_ptr<ItemInstance> RedstoneTorchBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState*)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(Blocks::redstone_torch));
}

bool RedstoneTorchBlock::isMatching(Block* block)
{
    return block == Blocks::unlit_redstone_torch || block == Blocks::redstone_torch;
}
