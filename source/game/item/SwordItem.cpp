#include "game/item/SwordItem.hpp"
#include "game/core/StringIds.hpp"

#include "game/core/AttributeModifier.hpp"
#include "game/core/EquipmentSlot.hpp"
#include "game/core/GameMode.hpp"
#include "game/core/InteractionResult.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/core/Minecraft.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/generation/Player.hpp"

SwordItem::SwordItem(const Item::Tier* tier)
{
    mMaxStackSize = 1;
    mTier = tier;
    setMaxDamage(tier->getUses());
    mAttackDamage = 4.0f + tier->getAttackDamageBonus();
}

unsigned int SwordItem::GetUseTooltip(const ItemTooltipDataHolder& tooltipData)
{
    return IDS_TOOLTIPS_BLOCK;
}

float SwordItem::getTierDamage()
{
    return mTier->getAttackDamageBonus();
}

float SwordItem::getDestroySpeed(not_null_ptr<ItemInstance> itemInstance, const BlockState* state)
{
    if (state->getBlock() == Blocks::web)
    {
        return 15.0f;
    }

    Material* material = state->getMaterial();

    if (material == Material::PLANT || material == Material::REPLACEABLE_PLANT || material == Material::BAMBOO
        || material == Material::CORAL || material == Material::LEAVES || material == Material::VEGETABLE)
    {
        if (((GameMode*)Minecraft::GetInstance()->mGameMode)->isAdventure()
            && (state->getBlock() == Blocks::chorus_flower || state->getBlock() == Blocks::chorus_plant))
        {
            return 1.0f;
        }

        return 1.5f;
    }

    return 1.0f;
}

bool SwordItem::hurtEnemy(not_null_ptr<ItemInstance> itemInstance, boost::shared_ptr<LivingEntity> target,
                          boost::shared_ptr<LivingEntity> attacker)
{
    itemInstance->hurtAndBreak(1, attacker);
    return true;
}

bool SwordItem::mineBlock(not_null_ptr<ItemInstance> itemInstance, Level* level, const BlockState* state,
                          const BlockPos& pos, boost::shared_ptr<LivingEntity> entity)
{
    if (static_cast<double>(state->getDestroySpeed(level, pos)) != 0.0)
    {
        itemInstance->hurtAndBreak(2, entity);
    }

    return true;
}

bool SwordItem::isHandEquipped()
{
    return true;
}

int SwordItem::getUseAnimation(not_null_ptr<const ItemInstance> itemInstance) const
{
    return UseAnim_block;
}

int SwordItem::getUseDuration(not_null_ptr<ItemInstance> itemInstance)
{
    return 72000;
}

InteractionResultHolder SwordItem::use(Level* level, boost::shared_ptr<Player> player,
                                       InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    player->startUsingItem(hand);
    return InteractionResultHolder(InteractionResult::eInteractionResult_Success, itemInstance);
}

bool SwordItem::canDestroySpecial(const BlockState* state)
{
    return state->getBlock() == Blocks::web;
}

int SwordItem::getEnchantmentValue()
{
    return mTier->getEnchantmentValue();
}

const Item::Tier* SwordItem::getTier()
{
    return mTier;
}

bool SwordItem::isValidRepairItem(not_null_ptr<ItemInstance> itemInstance, not_null_ptr<ItemInstance> repairItem)
{
    if (mTier->getTierItem() == repairItem->getItem())
    {
        return true;
    }

    return Item::isValidRepairItem(itemInstance, repairItem);
}

boost::unordered_map<eATTRIBUTE_ID, AttributeModifier*>* SwordItem::getDefaultAttributeModifiers(
    const EquipmentSlot* slot)
{
    boost::unordered_map<eATTRIBUTE_ID, AttributeModifier*>* modifiers = Item::getDefaultAttributeModifiers(slot);
    double attackDamage = mAttackDamage;

    if (Minecraft::InMiniGame(eMiniGameId_Any, true))
    {
        const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
        attackDamage = miniGame->getBaseDamage(getId(this), mAttackDamage);
    }

    if (slot == EquipmentSlot::MAINHAND)
    {
        modifiers->insert(boost::unordered_map<eATTRIBUTE_ID, AttributeModifier*>::value_type(
            SharedMonsterAttributes::ATTACK_DAMAGE->getId(),
            new AttributeModifier(eMODIFIER_ID_BaseAttackDamage, attackDamage, 0)));
        modifiers->insert(boost::unordered_map<eATTRIBUTE_ID, AttributeModifier*>::value_type(
            SharedMonsterAttributes::ATTACK_SPEED->getId(),
            new AttributeModifier(eMODIFIER_ID_BaseAttackSpeed, -2.4f, 0)));
    }

    return modifiers;
}
