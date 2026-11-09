#include "game/item/FishingRodItem.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/IconRegister.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/projectile/FishingHook.hpp"
#include "game/item/EnchantmentHelper.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/Random.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"

FishingRodItem::FishingRodItem()
{
    setMaxDamage(384);
    setMaxStackSize(1);
    addProperty(ResourceLocation(L"cast"), new FishingRodItem_ItemPropertyFunction());
}

unsigned int FishingRodItem::GetUseTooltip(const ItemTooltipDataHolder& tooltipData)
{
    return IDS_TOOLTIPS_USE;
}

InteractionResultHolder FishingRodItem::use(Level* level, boost::shared_ptr<Player> player,
                                            InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    if (player->mFishing)
    {
        itemInstance->hurtAndBreak(player->mFishing->retrieve(), player);
        player->swing(hand);
        level->playSound(boost::shared_ptr<Player>(), player->mX, player->mY, player->mZ, SoundEvents::BOBBER_RETRIEVE,
                         SoundSource::eSoundSource_Neutral, 1.0f, 0.4f / (random->nextFloat() * 0.4f + 0.8f), 16.0f);
    }
    else
    {
        level->playSound(boost::shared_ptr<Player>(), player->mX, player->mY, player->mZ, SoundEvents::BOBBER_THROW,
                         SoundSource::eSoundSource_Neutral, 0.5f, 0.4f / (random->nextFloat() * 0.4f + 0.8f), 16.0f);
        if (!level->mIsClientSide)
        {
            boost::shared_ptr<FishingHook> hook(new FishingHook(level, player));
            int lureSpeed = EnchantmentHelper::getFishingSpeedBonus(itemInstance);
            if (lureSpeed > 0)
            {
                hook->setLureSpeed(lureSpeed);
            }
            int luck = EnchantmentHelper::getFishingLuckBonus(itemInstance);
            if (luck > 0)
            {
                hook->setLuck(luck);
            }
            player->mFishing = hook;
            boost::shared_ptr<Entity> entity = hook;
            level->addEntity(entity);
        }
        player->swing(hand);
        player->awardStat(GenericStats::itemsUsed(this), GenericStats::param_itemsUsed(player, itemInstance));
    }
    return InteractionResultHolder(InteractionResult::eInteractionResult_Success, itemInstance);
}

bool FishingRodItem::isHandEquipped()
{
    return true;
}

bool FishingRodItem::isMirroredArt()
{
    return true;
}

int FishingRodItem::getEnchantmentValue()
{
    return 1;
}

void FishingRodItem::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(getIconName() + L"_uncast");
    mEmptyIcon = iconRegister->registerIcon(getIconName() + L"_cast");
}

TextureAtlasSprite* FishingRodItem::getEmptyIcon()
{
    return mEmptyIcon;
}

float FishingRodItem::FishingRodItem_ItemPropertyFunction::call(not_null_ptr<ItemInstance> itemInstance, Level* level,
                                                                boost::shared_ptr<LivingEntity> entity)
{
    float result = 0.0f;
    if (entity)
    {
        bool inMainHand = entity->getMainHandItem() == itemInstance;
        bool inOffhand = entity->getOffhandItem() == itemInstance;
        if (dynamic_cast<FishingRodItem*>(entity->getMainHandItem()->getItem()) != NULL)
        {
            inOffhand = false;
        }
        if ((inMainHand || inOffhand) && eTYPE_DERIVED_FROM(EntityType::PLAYER, entity->GetType()))
        {
            boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);
            if (player->mFishing)
            {
                result = 1.0f;
            }
        }
    }
    return result;
}
