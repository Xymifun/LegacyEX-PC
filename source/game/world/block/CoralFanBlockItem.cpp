#include "game/world/block/CoralFanBlockItem.hpp"

#include <math.h>

#include "game/client/audio/SoundSource.hpp"
#include "game/client/audio/SoundType.hpp"
#include "game/core/CoralFan.hpp"
#include "game/core/CoralFanHang.hpp"
#include "game/core/Direction.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockId.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/LiquidBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/generation/Player.hpp"

CoralFanBlockItem::CoralFanBlockItem(Block* block) : BlockItem(block), mIsDead(block == Blocks::coral_fan_dead)
{
    setStackedByData(true);
    setMaxDamage(0);
}

int CoralFanBlockItem::getDescriptionId(not_null_ptr<ItemInstance> itemInstance)
{
    int auxValue = itemInstance->getAuxValue();
    if (mIsDead)
    {
        return Blocks::coral_fan_dead->getDescriptionId(auxValue);
    }
    return Blocks::coral_fan->getDescriptionId(auxValue);
}

int CoralFanBlockItem::getUseDescriptionId(not_null_ptr<ItemInstance>)
{
    if (mIsDead)
    {
        return Blocks::coral_fan_dead->getUseDescriptionId(-1);
    }
    return Blocks::coral_fan->getUseDescriptionId(-1);
}

InteractionResult::EInteractionResult CoralFanBlockItem::useOn(boost::shared_ptr<Player> player, Level* level,
                                                               const BlockPos& pos,
                                                               InteractionHand::EInteractionHand hand,
                                                               const Direction* direction, float, float, float,
                                                               bool simulate)
{
    BlockPos placePos = pos.relative(direction);
    bool isUp = true;
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    int variant = itemInstance->getAuxValue() & 7;

    if (!level->isEmptyBlock(pos.relative(direction)) && !level->isEmptyWaterBlock(pos.relative(direction)))
        return InteractionResult::eInteractionResult_Fail;
    if (level->getLiquidBlock(placePos)->mMaterial == Material::WATER
        && static_cast<unsigned int>(level->getLiquidBlockState(placePos)->getValue(LiquidBlock::LEVEL) - 1) < 7)
    {
        return InteractionResult::eInteractionResult_Fail;
    }

    if (direction != Direction::UP)
        isUp = false;
    bool underWater = level->isUnderWater(placePos, BlockId::CORAL_FAN, 0);
    const BlockState* state;
    int fanDirection;
    if (isUp)
    {
        state = Blocks::coral_fan->defaultBlockState();
        if (direction == Direction::UP)
        {
            if (!state->getBlock()->mayPlaceOn(level, pos, state))
                return InteractionResult::eInteractionResult_Fail;

            Vec3* facing = (pos - player->getPos(0.0f))->normalize();
            float angle = static_cast<float>(atan2(facing->z, facing->x));
            if ((angle > 0.7853982f && angle < 2.3561945f) || (angle > 3.926991f && angle < 5.4977875f))
                fanDirection = 0;
            else
                fanDirection = 1;
            if (!underWater || itemInstance->getItem() == Item::byBlock(Blocks::coral_fan_dead))
                state = Blocks::coral_fan_dead->defaultBlockState();
            state = state->setValue(CoralFan::DIRECTION, fanDirection)->setValue(CoralFan::VARIANT, variant);
        }
        else
            return InteractionResult::eInteractionResult_Fail;
    }
    else
    {
        state = Blocks::coral_fan_hang->defaultBlockState();
        switch (variant)
        {
        case 0:
            state = Blocks::coral_fan_hang->defaultBlockState()->setValue(CoralFanHang::VARIANT, 0);
            break;
        case 1:
            state = Blocks::coral_fan_hang->defaultBlockState()->setValue(CoralFanHang::VARIANT, 1);
            break;
        case 2:
            state = Blocks::coral_fan_hang2->defaultBlockState()->setValue(CoralFanHang::VARIANT, 0);
            break;
        case 3:
            state = Blocks::coral_fan_hang2->defaultBlockState()->setValue(CoralFanHang::VARIANT, 1);
            break;
        case 4:
        case 5:
            state = Blocks::coral_fan_hang3->defaultBlockState()->setValue(CoralFanHang::VARIANT, 0);
            break;
        }

        const BlockState* support = level->getBlockState(pos);
        if (!support->isSolidBlockingBlock() && !support->getBlock()->isSlabBlock())
            return InteractionResult::eInteractionResult_Fail;
        if (direction == Direction::UP || direction == Direction::DOWN)
            return InteractionResult::eInteractionResult_Fail;
        if (!state->getBlock()->mayPlaceOn(level, pos, state))
            return InteractionResult::eInteractionResult_Fail;

        if (direction == Direction::NORTH)
            state = state->setValue(CoralFanHang::DIRECTION, 2);
        else if (direction == Direction::EAST)
            state = state->setValue(CoralFanHang::DIRECTION, 1);
        else if (direction == Direction::SOUTH)
            state = state->setValue(CoralFanHang::DIRECTION, 3);
        else
            state = state->setValue(CoralFanHang::DIRECTION, 0);
        if (!underWater || itemInstance->getItem() == Item::byBlock(Blocks::coral_fan_dead))
            state = state->setValue(CoralFanHang::DEAD, true);
    }

    if (simulate)
        return InteractionResult::eInteractionResult_Success;

    level->setBlock(placePos, state, Block::UPDATE_ALL, false);
    itemInstance->shrink(1);
    level->getBlockState(pos);
    player->swing(InteractionHand::eInteractionHand_MainHand);
    const SoundType* soundType = mBlock->getSoundType();
    level->playSound(player, pos, soundType->getPlaceSound(), SoundSource::eSoundSource_Blocks,
                     (soundType->getVolume() + 1.0f) * 0.5f, soundType->getPitch() * 0.8f, 16.0f);
    return InteractionResult::eInteractionResult_Success;
}

int CoralFanBlockItem::getLevelDataForAuxValue(int auxValue)
{
    return auxValue;
}
