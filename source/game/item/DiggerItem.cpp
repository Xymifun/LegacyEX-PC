#include "game/item/DiggerItem.hpp"

#include "game/core/AttributeModifier.hpp"
#include "game/core/EquipmentSlot.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/core/Minecraft.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockState.hpp"

DiggerItem::DiggerItem(float attackDamage, float attackSpeed, const Item::Tier* tier, arrayWithLength<Block*> blocks)
    : mTier(tier)
{
    _init(attackDamage, attackSpeed, blocks);
}

DiggerItem::DiggerItem(const Item::Tier* tier, arrayWithLength<Block*> blocks) : mTier(tier)
{
    _init(0.0f, 0.0f, blocks);
}

void DiggerItem::_init(float attackDamage, float attackSpeed, arrayWithLength<Block*> blocks)
{
    mBlocks = blocks;
    mMaxStackSize = 1;
    setMaxDamage(mTier->getUses());
    mSpeed = mTier->getSpeed();
    mAttackDamage = attackDamage + mTier->getAttackDamageBonus();
    mAttackSpeed = attackSpeed;
}

float DiggerItem::getDestroySpeed(not_null_ptr<ItemInstance> itemInstance, const BlockState* state)
{
    for (unsigned int i = 0; i < mBlocks.length; i++)
    {
        if (mBlocks.data[i] == state->getBlock())
        {
            return mSpeed;
        }
    }

    return 1.0f;
}

bool DiggerItem::hurtEnemy(not_null_ptr<ItemInstance> itemInstance, boost::shared_ptr<LivingEntity> target,
                           boost::shared_ptr<LivingEntity> attacker)
{
    itemInstance->hurtAndBreak(2, attacker);
    return true;
}

bool DiggerItem::mineBlock(not_null_ptr<ItemInstance> itemInstance, Level* level, const BlockState* state,
                           const BlockPos& pos, boost::shared_ptr<LivingEntity> entity)
{
    if (!level->mIsClientSide && state->getDestroySpeed(level, pos) != 0.0)
    {
        itemInstance->hurtAndBreak(1, entity);
    }

    return true;
}

bool DiggerItem::isHandEquipped()
{
    return true;
}

int DiggerItem::getEnchantmentValue()
{
    return mTier->getEnchantmentValue();
}

const Item::Tier* DiggerItem::getTier()
{
    return mTier;
}

bool DiggerItem::isValidRepairItem(not_null_ptr<ItemInstance> itemInstance, not_null_ptr<ItemInstance> repairItem)
{
    if (mTier->getTierItem() == repairItem->getItem())
    {
        return true;
    }

    return Item::isValidRepairItem(itemInstance, repairItem);
}

boost::unordered_map<eATTRIBUTE_ID, AttributeModifier*>* DiggerItem::getDefaultAttributeModifiers(
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
        (*modifiers)[SharedMonsterAttributes::ATTACK_DAMAGE->getId()] = new AttributeModifier(
            eMODIFIER_ID_BaseAttackDamage, attackDamage, 0);
        (*modifiers)[SharedMonsterAttributes::ATTACK_SPEED->getId()] = new AttributeModifier(
            eMODIFIER_ID_BaseAttackSpeed, mAttackSpeed, 0);
    }

    return modifiers;
}
