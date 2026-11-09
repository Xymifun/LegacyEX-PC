#include "game/world/block/SlabBlockItem.hpp"

#include "game/client/audio/SoundSource.hpp"
#include "game/client/audio/SoundType.hpp"
#include "game/core/Direction.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/HalfSlabBlock.hpp"
#include "game/world/block/Property.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/generation/Player.hpp"

SlabBlockItem::SlabBlockItem(Block* block, HalfSlabBlock* halfSlab, HalfSlabBlock* fullSlab)
    : BlockItem(block),
      mHalfSlab(halfSlab),
      mFullSlab(fullSlab)
{
    setMaxDamage(0);
    setStackedByData(true);
}

int SlabBlockItem::getLevelDataForAuxValue(int auxValue)
{
    return auxValue;
}

int SlabBlockItem::getDescriptionId(not_null_ptr<ItemInstance> itemInstance)
{
    return mHalfSlab->getAuxName(itemInstance->getAuxValue());
}

const BlockState* SlabBlockItem::makeState(const Property* property, Boxed* value)
{
    return mFullSlab->defaultBlockState()->setBoxedValue(property, value);
}

bool SlabBlockItem::tryConvertTargetBlock(boost::shared_ptr<Player> player, not_null_ptr<ItemInstance> itemInstance,
                                          Level* level, const BlockPos& pos, Boxed* type, bool simulate)
{
    const BlockState* state = level->getBlockState(pos);
    Property* property = mHalfSlab->getTypeProperty();
    if (state->getBlock() == mHalfSlab)
    {
        Boxed* currentType = state->getBoxedValue(property);
        if (*currentType == type)
        {
            if (!simulate)
            {
                const BlockState* fullState = mFullSlab->defaultBlockState()->setBoxedValue(property, currentType);
                const AABB* box = mFullSlab->getClipAABB(fullState, level, pos);
                if (level->isUnobstructed(box) && level->setBlock(pos, fullState, Block::UPDATE_ALL, false))
                {
                    const SoundType* soundType = mFullSlab->mSoundType;
                    level->playSound(player, pos, soundType->getPlaceSound(), SoundSource::eSoundSource_Blocks,
                                     (soundType->getVolume() + 1.0f) * 0.5f, soundType->getPitch() * 0.8f, 16.0f);
                    itemInstance->shrink(1);
                }
            }
            return true;
        }
    }

    return false;
}

InteractionResult::EInteractionResult SlabBlockItem::useOn(boost::shared_ptr<Player> player, Level* level,
                                                           const BlockPos& pos, InteractionHand::EInteractionHand hand,
                                                           const Direction* direction, float hitX, float hitY,
                                                           float hitZ, bool simulate)
{
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    if (itemInstance->isEmpty() || !player->mayUseItemAt(pos, direction, itemInstance))
    {
        return InteractionResult::eInteractionResult_Fail;
    }

    Boxed* type = mHalfSlab->getTypeForItem(itemInstance);

    const BlockState* state = level->getBlockState(pos);
    if (state->getBlock() == mHalfSlab)
    {
        Property* property = mHalfSlab->getTypeProperty();
        Boxed* currentType = state->getBoxedValue(property);
        HalfSlabBlock::Half* half = state->getValue(HalfSlabBlock::TYPE);
        if (((direction == Direction::UP && half == HalfSlabBlock::BOTTOM)
             || (direction == Direction::DOWN && half == HalfSlabBlock::TOP))
            && *currentType == type)
        {
            const BlockState* fullState = makeState(property, currentType);
            delete type;
            if (simulate)
            {
                return InteractionResult::eInteractionResult_Success;
            }

            const AABB* shape = fullState->getClipAABB(level, pos);
            if (shape != Block::NULL_AABB && level->isUnobstructed(shape->move(pos))
                && level->setBlock(pos, fullState, Block::UPDATE_ALL | Block::UPDATE_IMMEDIATE, false))
            {
                const SoundType* soundType = mFullSlab->getSoundType();
                level->playSound(player, pos, soundType->getPlaceSound(), SoundSource::eSoundSource_Blocks,
                                 (soundType->getVolume() + 1.0f) * 0.5f, soundType->getPitch() * 0.8f, 16.0f);
                itemInstance->shrink(1);
            }
            return InteractionResult::eInteractionResult_Success;
        }
    }

    if (tryConvertTargetBlock(player, itemInstance, level, pos.relative(direction), type, simulate))
    {
        delete type;
        return InteractionResult::eInteractionResult_Success;
    }

    delete type;
    return BlockItem::useOn(player, level, pos, hand, direction, hitX, hitY, hitZ, simulate);
}

bool SlabBlockItem::mayPlace(Level* level, const BlockPos& pos, const Direction* direction,
                             boost::shared_ptr<Player> player, not_null_ptr<ItemInstance> itemInstance)
{
    Property* property = mHalfSlab->getTypeProperty();
    Boxed* type = mHalfSlab->getTypeForItem(itemInstance);

    const BlockState* state = level->getBlockState(pos);
    if (state->getBlock() == mHalfSlab)
    {
        bool top = state->getValue(HalfSlabBlock::TYPE) == HalfSlabBlock::TOP;
        if (((direction == Direction::UP && !top) || (direction == Direction::DOWN && top))
            && *type == state->getBoxedValue(property))
        {
            delete type;
            return true;
        }
    }

    const BlockState* adjacent = level->getBlockState(pos.relative(direction));
    if (adjacent->getBlock() == mHalfSlab && *type == adjacent->getBoxedValue(property))
    {
        delete type;
        return true;
    }

    delete type;
    return BlockItem::mayPlace(level, pos, direction, player, itemInstance);
}
