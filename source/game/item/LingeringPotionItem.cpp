#include "game/item/LingeringPotionItem.hpp"
#include "game/core/StringIds.hpp"

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/core/GenericStats.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Potion.hpp"
#include "game/item/PotionUtils.hpp"
#include "game/item/ThrownPotion.hpp"
#include "game/ui/ItemTooltipDataHolder.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"

unsigned int LingeringPotionItem::getPotionType() const
{
    return 2;
}

std::wstring LingeringPotionItem::getName(not_null_ptr<ItemInstance> itemInstance)
{
    return PotionUtils::getPotion(itemInstance)->getName(Potion::LINGERING);
}

void LingeringPotionItem::appendHoverText(not_null_ptr<ItemInstance> itemInstance, boost::shared_ptr<Player> player,
                                          std::vector<HtmlString>* lines, bool)
{
    PotionUtils::addPotionTooltip(itemInstance, lines, 0.25f, player);
}

InteractionResultHolder LingeringPotionItem::use(Level* level, boost::shared_ptr<Player> player,
                                                 InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    not_null_ptr<ItemInstance> thrownItem = player->mAbilities.mInstabuild ? itemInstance->copy()
                                                                           : itemInstance->split(1);

    level->playSound(boost::shared_ptr<Player>(), player->mX, player->mY, player->mZ,
                     SoundEvents::LINGERINGPOTION_THROW, SoundSource::eSoundSource_Neutral, 0.5f,
                     0.4f / (Item::random->nextFloat() * 0.4f + 0.8f), 16.0f);

    if (!level->mIsClientSide)
    {
        boost::shared_ptr<ThrownPotion> potion(new ThrownPotion(level, player, thrownItem));
        potion->shootFromRotation(player, player->mXRot, player->mYRot, -20.0f, 0.5f, 1.0f);
        level->addEntity(potion);
    }

    player->awardStat(GenericStats::itemsUsed(this), GenericStats::param_itemsUsed(player, itemInstance));

    return InteractionResultHolder(InteractionResult::eInteractionResult_Success, itemInstance);
}

unsigned int LingeringPotionItem::GetUseTooltip(const ItemTooltipDataHolder& tooltipData)
{
    unsigned int tooltip = IDS_NONE;
    if (tooltipData.m_eInteractionResult != InteractionResult::eInteractionResult_Fail)
    {
        tooltip = IDS_TOOLTIPS_THROW;
    }
    return tooltip;
}

std::wstring LingeringPotionItem::getBottleTextureName()
{
    return L"potion_bottle_lingering";
}
