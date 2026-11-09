#include "game/world/block/SeaPickleBlockItem.hpp"
#include "game/world/block/BlockId.hpp"

#include "game/client/audio/SoundSource.hpp"
#include "game/client/audio/SoundType.hpp"
#include "game/core/Direction.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/SeaPickle.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockProperties.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/LiquidBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/generation/Player.hpp"

SeaPickleBlockItem::SeaPickleBlockItem(Block* block) : BlockItem(block)
{
    Item::setMaxDamage(0);
    Item::setStackedByData(true);
}

int SeaPickleBlockItem::getLevelDataForAuxValue(int auxValue)
{
    return auxValue;
}

InteractionResult::EInteractionResult SeaPickleBlockItem::useOn(boost::shared_ptr<Player> player, Level* level,
                                                                const BlockPos& pos,
                                                                InteractionHand::EInteractionHand hand,
                                                                const Direction* direction, float, float, float,
                                                                bool simulate)
{
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    if (itemInstance->isEmpty() || !player->mayUseItemAt(pos, direction, itemInstance))
        return InteractionResult::eInteractionResult_Fail;

    BlockPos placePos = pos.relative(direction);
    BlockPos targetPos = pos.relative(direction);
    if (level->getBlockId(pos.x, pos.y, pos.z) != BlockId::SEA_GRASS)
    {
        if (direction == Direction::UP)
            targetPos = targetPos.below();
        else if (direction != Direction::DOWN)
            targetPos = targetPos.relative(direction->getOpposite());
    }

    const BlockState* targetState = level->getBlockState(targetPos);
    const BlockState* defaultState = Blocks::sea_pickle->defaultBlockState();
    const BlockState* newState = defaultState;
    if (targetState->getBlock() == Blocks::sea_pickle)
    {
        int count = targetState->getValue(SeaPickle::COUNT) + 1;
        if (count > 3)
            return InteractionResult::eInteractionResult_Fail;
        newState = newState->setValue(SeaPickle::COUNT, count);
        placePos = targetPos;
        if (!level->isUnderWater(targetPos, defaultState))
            newState = newState->setValue(SeaPickle::FLANGE, 1);
    }
    else
    {
        if (level->getLiquidBlock(placePos)->mMaterial == Material::WATER
            && static_cast<unsigned int>(level->getLiquidBlockState(placePos)->getValue(LiquidBlock::LEVEL) - 1) < 7)
        {
            return InteractionResult::eInteractionResult_Fail;
        }
        if ((!level->isConsideredSolidBlock(placePos.below()) && targetState->getBlock() != Blocks::glass
             && targetState->getBlock() != Blocks::stained_glass)
            || !level->canProvideSupport(placePos.below(), Direction::UP, BLOCK_SUPPORT_CENTER)
            || (!level->isEmptyBlock(placePos) && !level->isEmptyWaterBlock(placePos)
                && targetState->getBlock() != Blocks::sea_grass))
        {
            return InteractionResult::eInteractionResult_Fail;
        }
        if (!level->isUnderWater(placePos, defaultState))
            newState = newState->setValue(SeaPickle::FLANGE, 1);
    }

    if (simulate)
        return InteractionResult::eInteractionResult_Success;

    if (level->isUnobstructed(Blocks::sea_pickle->getShape(targetState, level, placePos))
        && level->setBlock(placePos, newState, Block::UPDATE_ALL, false))
    {
        const SoundType* soundType = Blocks::sea_pickle->getSoundType();
        level->playSound(boost::shared_ptr<Player>(), placePos, soundType->getPlaceSound(),
                         SoundSource::eSoundSource_Blocks, (soundType->getVolume() + 1.0f) * 0.5f,
                         soundType->getPitch(), 16.0f);
        itemInstance->shrink(1);
        if (newState->getValue(SeaPickle::COUNT) == 3)
            player->awardStat(GenericStats::_1234Pickle(), GenericStats::param_1234Pickle());
        return InteractionResult::eInteractionResult_Success;
    }
    return InteractionResult::eInteractionResult_Fail;
}
