#include "game/item/FireChargeItem.hpp"

#include <string>

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/core/IconRegister.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/generation/Player.hpp"

FireChargeItem::FireChargeItem() : Item(), mIcon2(NULL)
{
}

InteractionResult::EInteractionResult FireChargeItem::useOn(boost::shared_ptr<Player> player, Level* level,
                                                            const BlockPos& pos, InteractionHand::EInteractionHand hand,
                                                            const Direction* direction, float hitX, float hitY,
                                                            float hitZ, bool simulate)
{
    if (level->mIsClientSide && !simulate)
    {
        return InteractionResult::eInteractionResult_Success;
    }

    BlockPos target = pos;
    target = target.relative(direction);
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);

    if (!player->mayUseItemAt(target, direction, itemInstance))
    {
        return InteractionResult::eInteractionResult_Fail;
    }

    if (!simulate)
    {
        if (level->getBlockState(target)->getMaterial() == Material::AIR)
        {
            level->playSound(boost::shared_ptr<Player>(), target, SoundEvents::FIREBALL_SHOOT,
                             SoundSource::eSoundSource_Blocks, 1.0f,
                             (random->nextFloat() - random->nextFloat()) * 0.2f + 1.0f, 16.0f);
            level->setBlockAndUpdate(target, Blocks::fire->defaultBlockState());
        }
        if (!player->mAbilities.mInstabuild)
        {
            itemInstance->shrink(1);
        }
    }
    else
    {
        return level->getBlockState(target)->getBlock()->mMaterial == Material::AIR
                       && Blocks::fire->mayPlace(level, target)
                   ? InteractionResult::eInteractionResult_Pass
                   : InteractionResult::eInteractionResult_Fail;
    }
    return InteractionResult::eInteractionResult_Success;
}

TextureAtlasSprite* FireChargeItem::getIcon(int auxValue)
{
    if (auxValue > 0)
    {
        return mIcon2;
    }
    return Item::getIcon(auxValue);
}

void FireChargeItem::registerIcons(IconRegister* iconRegister)
{
    Item::registerIcons(iconRegister);
    mIcon2 = iconRegister->registerIcon(L"dragonFireball");
}
