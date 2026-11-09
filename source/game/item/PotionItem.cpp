#include "game/item/PotionItem.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"

#include "game/core/GenericStats.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Minecraft.hpp"
#include "game/entity/mob/MobEffect.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/Potion.hpp"
#include "game/item/Potions.hpp"
#include "game/item/PotionUtils.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "game/ui/ItemTooltipDataHolder.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"

static std::wstring gBottleTextureName = L"potion";
static std::wstring gPotionOverlayTextureName = L"potion_contents";

PotionItem::PotionItem() : Item(), mIcon(NULL), mLayerIcon(NULL)
{
    setMaxStackSize(1);
}

uint32_t PotionItem::getPotionType() const
{
    return 0;
}

not_null_ptr<ItemInstance> PotionItem::getDefaultInstance()
{
    return PotionUtils::setPotion(Item::getDefaultInstance(), Potions::WATER);
}

not_null_ptr<ItemInstance> PotionItem::finishUsingItem(not_null_ptr<ItemInstance> itemInstance, Level* level,
                                                       boost::shared_ptr<LivingEntity> entity)
{
    boost::shared_ptr<Player> player = eTYPE_DERIVED_FROM(EntityType::PLAYER, entity->GetType())
                                           ? boost::static_pointer_cast<Player>(entity)
                                           : boost::shared_ptr<Player>();
    if (player.get() != NULL)
    {
        player->awardStat(GenericStats::itemsUsed(this), GenericStats::param_itemsUsed(player, itemInstance));
        if (PotionUtils::hasEffect(itemInstance, MobEffects::WATER_BREATHING))
        {
            player->ResetTimerAchievement(Player::eTimerAchievement_SleepWithTheFishes);
        }
    }

    if (!level->mIsClientSide)
    {
        if (player.get() == NULL || !player->mAbilities.mInstabuild)
        {
            itemInstance->shrink(1);
        }

        std::vector<MobEffectInstance*>* effects = PotionUtils::getMobEffects(itemInstance);
        if (effects != NULL)
        {
            std::vector<MobEffectInstance*>::iterator it;
            for (it = effects->begin(); it != effects->end(); ++it)
            {
                MobEffectInstance* effect = *it;
                if (!effect->getEffect()->isInstantenous())
                {
                    entity->addEffect(
                        new MobEffectInstance(effect->getEffect(), effect->getDuration(), effect->getAmplifier()),
                        boost::shared_ptr<Entity>());
                }
                else
                {
                    effect->getEffect()->applyInstantenousEffect(player, player, entity, effect->getAmplifier(), 1.0);
                }
            }
            delete effects;
        }

        if (player.get() == NULL || (!player->mAbilities.mInstabuild && !Minecraft::InMiniGame(eMiniGameId_Any, true)))
        {
            if (itemInstance->isEmpty())
            {
                return not_null_ptr<ItemInstance>(new ItemInstance(Items::glassBottle));
            }
            if (player.get() != NULL)
            {
                player->mInventory->add(not_null_ptr<ItemInstance>(new ItemInstance(Items::glassBottle)));
            }
        }
    }

    return itemInstance;
}

int PotionItem::getUseDuration(not_null_ptr<ItemInstance>)
{
    return 32;
}

int PotionItem::getUseAnimation(not_null_ptr<const ItemInstance>) const
{
    return UseAnim_drink;
}

InteractionResultHolder PotionItem::use(Level*, boost::shared_ptr<Player> player,
                                        InteractionHand::EInteractionHand hand)
{
    player->startUsingItem(hand);
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    return InteractionResultHolder(InteractionResult::eInteractionResult_Success, itemInstance);
}

bool PotionItem::TestUse(Level*, boost::shared_ptr<Player>, InteractionHand::EInteractionHand)
{
    return true;
}

std::wstring PotionItem::getName(not_null_ptr<ItemInstance> itemInstance)
{
    return PotionUtils::getPotion(itemInstance)->getName(Potion::NORMAL);
}

void PotionItem::appendHoverText(not_null_ptr<ItemInstance> itemInstance, boost::shared_ptr<Player> player,
                                 std::vector<HtmlString>* lines, bool)
{
    PotionUtils::addPotionTooltip(itemInstance, lines, 1.0f, player);
}

bool PotionItem::isFoil(not_null_ptr<ItemInstance> itemInstance)
{
    return Item::isFoil(itemInstance) || PotionUtils::isFoil(itemInstance);
}

int PotionItem::getUseDescriptionId(not_null_ptr<ItemInstance> itemInstance)
{
    Potion* potion = PotionUtils::getPotion(itemInstance);
    return potion->getUseDescriptionId(Potion::NORMAL);
}

TextureAtlasSprite* PotionItem::getIcon(int)
{
    return mIcon;
}

TextureAtlasSprite* PotionItem::getLayerIcon(int auxValue, int layer, not_null_ptr<ItemInstance> itemInstance)
{
    if (layer == 0)
    {
        return mLayerIcon;
    }
    return Item::getLayerIcon(auxValue, layer, itemInstance);
}

bool PotionItem::hasMultipleSpriteLayers()
{
    return true;
}

void PotionItem::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(getBottleTextureName());
    mLayerIcon = iconRegister->registerIcon(gPotionOverlayTextureName);
}

unsigned int PotionItem::GetUseTooltip(const ItemTooltipDataHolder& tooltipData)
{
    if (tooltipData.m_eInteractionResult != InteractionResult::eInteractionResult_Fail)
    {
        return IDS_TOOLTIPS_DRINK;
    }
    return IDS_NONE;
}

std::wstring PotionItem::getBottleTextureName()
{
    return gBottleTextureName;
}
