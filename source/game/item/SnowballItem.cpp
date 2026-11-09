#include "game/item/SnowballItem.hpp"
#include "game/core/StringIds.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/Snowball.hpp"
#include "game/core/Minecraft.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/ui/ItemTooltipDataHolder.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"

SnowballItem::SnowballItem() : Item()
{
    mMaxStackSize = 16;
}

InteractionResultHolder SnowballItem::use(Level* level, boost::shared_ptr<Player> player,
                                          InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);

    if (!player->mAbilities.mInstabuild)
        itemInstance->shrink(1);

    level->playSound(boost::shared_ptr<Player>(), player->mX, player->mY, player->mZ, SoundEvents::SNOWBALL_THROW,
                     SoundSource::eSoundSource_Neutral, 0.5f, 0.4f / (random->nextFloat() * 0.4f + 0.8f), 16.0f);

    if (!level->mIsClientSide)
    {
        boost::shared_ptr<Snowball> snowball(new Snowball(level, player));
        snowball->shootFromRotation(player, player->mXRot, player->mYRot, 0.0f, 1.5f, 1.0f);
        level->addEntity(snowball);
    }
    else if (Minecraft::GetInstance()->GetMiniGame()->HasFeatureOverrideHandSwing())
    {
        player->swing(InteractionHand::eInteractionHand_MainHand);
    }

    player->awardStat(GenericStats::itemsUsed(this), GenericStats::param_itemsUsed(player, itemInstance));

    return InteractionResultHolder(InteractionResult::eInteractionResult_Success, itemInstance);
}

unsigned int SnowballItem::GetUseTooltip(const ItemTooltipDataHolder&)
{
    return IDS_TOOLTIPS_THROW;
}
