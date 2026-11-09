#include "game/item/SplashPotionItem.hpp"
#include "game/core/StringIds.hpp"

#include "game/core/GenericStats.hpp"
#include "game/core/Minecraft.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Potion.hpp"
#include "game/item/PotionUtils.hpp"
#include "game/item/ThrownPotion.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/ui/ItemTooltipDataHolder.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"

unsigned int SplashPotionItem::getPotionType() const
{
    return 1;
}

std::wstring SplashPotionItem::getName(not_null_ptr<ItemInstance> itemInstance)
{
    return PotionUtils::getPotion(itemInstance)->getName(Potion::SPLASH);
}

void SplashPotionItem::appendHoverText(not_null_ptr<ItemInstance> itemInstance, boost::shared_ptr<Player> player,
                                       std::vector<HtmlString>* lines, bool)
{
    PotionUtils::addPotionTooltip(itemInstance, lines, 0.75f, player);
}

InteractionResultHolder SplashPotionItem::use(Level* level, boost::shared_ptr<Player> player,
                                              InteractionHand::EInteractionHand hand)
{
    bool instabuild = player->mAbilities.mInstabuild;
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    not_null_ptr<ItemInstance> thrownItem = instabuild ? itemInstance->copy() : itemInstance->split(1);
    if (!instabuild)
    {
        itemInstance->shrink(1);
    }

    if (!level->mIsClientSide)
    {
        boost::shared_ptr<ThrownPotion> potion(new ThrownPotion(level, player, thrownItem));
        potion->shootFromRotation(player, player->mXRot, player->mYRot, -20.0f, 0.5f, 1.0f);
        level->addEntity(potion);
    }
    else if (Minecraft::GetInstance()->GetMiniGame()->HasFeatureOverrideHandSwing())
    {
        player->swing(InteractionHand::eInteractionHand_MainHand);
    }

    player->awardStat(GenericStats::itemsUsed(this), GenericStats::param_itemsUsed(player, itemInstance));
    return InteractionResultHolder(InteractionResult::eInteractionResult_Success, itemInstance);
}

unsigned int SplashPotionItem::GetUseTooltip(const ItemTooltipDataHolder& tooltipData)
{
    unsigned int tooltip = IDS_NONE;
    if (tooltipData.m_eInteractionResult != InteractionResult::eInteractionResult_Fail)
    {
        tooltip = IDS_TOOLTIPS_THROW;
    }
    return tooltip;
}

int SplashPotionItem::getUseDuration(not_null_ptr<ItemInstance>)
{
    return 0;
}

std::wstring SplashPotionItem::getBottleTextureName()
{
    return L"potion_splash";
}
