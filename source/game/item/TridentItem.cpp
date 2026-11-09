#include "game/item/TridentItem.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/core/AttributeModifier.hpp"
#include "game/core/EquipmentSlot.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/InteractionResult.hpp"
#include "game/core/Mth.hpp"
#include "game/core/ThrownTrident.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/entity/projectile/AbstractRotatedItemProjectile.hpp"
#include "game/item/EnchantmentHelper.hpp"
#include "game/item/Enchantments.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/ui/ItemTooltipDataHolder.hpp"
#include "game/util/Random.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/TridentItem_Throwing_ItemPropertyFunction.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"

TridentItem::TridentItem()
{
    mMaxStackSize = 1;
    setMaxDamage(250);

    addProperty(ResourceLocation(std::wstring(L"throwing")), new TridentItem_Throwing_ItemPropertyFunction());
}

int TridentItem::getUseAnimation(not_null_ptr<const ItemInstance> itemInstance) const
{
    return UseAnim_spear;
}

int TridentItem::getUseDuration(not_null_ptr<ItemInstance> itemInstance)
{
    return 72000;
}

void TridentItem::releaseUsing(not_null_ptr<ItemInstance> itemInstance, Level* level,
                               boost::shared_ptr<LivingEntity> entity, int timeLeft)
{
    bool isPlayer = eTYPE_DERIVED_FROM(EntityType::PLAYER, entity->GetType());

    if (!isPlayer)
    {
        return;
    }

    boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);
    int riptide = EnchantmentHelper::getEnchantmentLevel(Enchantments::RIPTIDE, entity);

    if (riptide > 0)
    {
        int useTime = getUseDuration(itemInstance) - timeLeft;

        if (!player->isInWaterOrRain() || player->isPassenger() || useTime < 10)
        {
            return;
        }

        float yRot = player->mYRot;
        float xRot = player->mXRot;
        yRot *= Mth::DEG_TO_RAD;
        xRot *= Mth::DEG_TO_RAD;
        float dirX = -Mth::sin(yRot) * Mth::cos(xRot);
        float dirY = -Mth::sin(xRot);
        float dirZ = Mth::cos(yRot) * Mth::cos(xRot);
        float length = Mth::sqrt(dirX * dirX + dirY * dirY + dirZ * dirZ);
        float speed = 0.25f * ((float)riptide + 1.0f) * 3.0f / length;

        player->startSpinAttack();

        if (player->mOnGround)
        {
            player->move(eMoverType_Self, 0.0, -1.0, 0.0, false);
        }

        entity->push((double)(dirX * speed), (double)(dirY * speed), (double)(dirZ * speed));

        if (riptide >= 3)
        {
            level->playSound(player, BlockPos(player), SoundEvents::TRIDENT_RIPTIDE_3,
                             SoundSource::eSoundSource_Players, 1.0f, 1.0f, 16.0f);
        }
        else if (riptide == 2)
        {
            level->playSound(player, BlockPos(player), SoundEvents::TRIDENT_RIPTIDE_2,
                             SoundSource::eSoundSource_Players, 1.0f, 1.0f, 16.0f);
        }
        else
        {
            level->playSound(player, BlockPos(player), SoundEvents::TRIDENT_RIPTIDE_1,
                             SoundSource::eSoundSource_Players, 1.0f, 1.0f, 16.0f);
        }
    }
    else
    {
        int useTime = getUseDuration(itemInstance) - timeLeft;

        if (useTime < 10)
        {
            return;
        }

        if (!level->mIsClientSide)
        {
            itemInstance->hurtAndBreak(1, player);

            boost::shared_ptr<ThrownTrident> trident(new ThrownTrident(level, player, itemInstance));

            trident->shootFromRotation(player, player->mXRot, player->mYHeadRot, 0.0f, 2.5f + (float)riptide * 0.5f,
                                       1.0f);

            if (player->mAbilities.mInstabuild)
            {
                trident->setPickup(AbstractRotatedItemProjectile::ePickup_CreativeOnly);
            }

            EnchantmentHelper::getEnchantmentLevel(Enchantments::LOYALTY, entity);

            level->addEntity(trident);
        }

        if (!player->mAbilities.mInstabuild)
        {
            player->mInventory->removeItem(itemInstance);
        }

        level->playSound(player, player->mX, player->mY, player->mZ, SoundEvents::TRIDENT_THROW,
                         SoundSource::eSoundSource_Players, 1.0f, 1.0f / (random->nextFloat() * 0.4f + 1.2f) + 0.5f,
                         16.0f);
    }

    player->awardStat(GenericStats::itemsUsed(this), GenericStats::param_itemsUsed(player, itemInstance));
}

InteractionResultHolder TridentItem::use(Level* level, boost::shared_ptr<Player> player,
                                         InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);

    int riptide = EnchantmentHelper::getEnchantmentLevel(Enchantments::RIPTIDE, player);

    if ((riptide > 0 && !player->isInWaterOrRain()) || itemInstance->getDamageValue() >= itemInstance->getMaxDamage())
    {
        return InteractionResultHolder(InteractionResult::eInteractionResult_Fail, itemInstance);
    }

    player->startUsingItem(hand);

    return InteractionResultHolder(InteractionResult::eInteractionResult_Success, itemInstance);
}

bool TridentItem::hurtEnemy(not_null_ptr<ItemInstance> itemInstance, boost::shared_ptr<LivingEntity> target,
                            boost::shared_ptr<LivingEntity> attacker)
{
    itemInstance->hurtAndBreak(1, attacker);

    return true;
}

bool TridentItem::mineBlock(not_null_ptr<ItemInstance> itemInstance, Level* level, const BlockState* state,
                            const BlockPos& pos, boost::shared_ptr<LivingEntity> entity)
{
    if (state->getDestroySpeed(level, pos) != 0.0)
    {
        itemInstance->hurtAndBreak(2, entity);
    }

    return true;
}

boost::unordered_map<eATTRIBUTE_ID, AttributeModifier*>* TridentItem::getDefaultAttributeModifiers(
    const EquipmentSlot* slot)
{
    boost::unordered_map<eATTRIBUTE_ID, AttributeModifier*>* modifiers = Item::getDefaultAttributeModifiers(slot);

    if (slot == EquipmentSlot::MAINHAND)
    {
        modifiers->insert(boost::unordered_map<eATTRIBUTE_ID, AttributeModifier*>::value_type(
            SharedMonsterAttributes::ATTACK_DAMAGE->getId(),
            new AttributeModifier(eMODIFIER_ID_BaseAttackDamage, 8.0f, 0)));
        modifiers->insert(boost::unordered_map<eATTRIBUTE_ID, AttributeModifier*>::value_type(
            SharedMonsterAttributes::ATTACK_SPEED->getId(),
            new AttributeModifier(eMODIFIER_ID_BaseAttackSpeed, -2.9f, 0)));
    }

    return modifiers;
}

int TridentItem::getEnchantmentValue()
{
    return 1;
}

unsigned int TridentItem::GetUseTooltip(const ItemTooltipDataHolder& tooltipData)
{
    bool riptide = EnchantmentHelper::getItemEnchantmentLevel(Enchantments::RIPTIDE, tooltipData.m_pItemInstance) > 0;
    if (riptide)
    {
        if (tooltipData.m_pPlayer->isUsingItem())
        {
            return IDS_TOOLTIPS_DASH;
        }

        if (tooltipData.m_pPlayer->isInWaterOrRain()
            && tooltipData.m_pItemInstance->getDamageValue() < tooltipData.m_pItemInstance->getMaxDamage())
        {
            return IDS_TOOLTIPS_CHARGE;
        }
    }
    else
    {
        if (tooltipData.m_pPlayer->isUsingItem())
        {
            return IDS_TOOLTIPS_THROW;
        }

        if (tooltipData.m_pItemInstance->getDamageValue() < tooltipData.m_pItemInstance->getMaxDamage())
        {
            return IDS_TOOLTIPS_CHARGE;
        }
    }

    return IDS_NONE;
}
