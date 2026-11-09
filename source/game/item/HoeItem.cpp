#include "game/item/HoeItem.hpp"

#include <typeinfo>

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/core/AttributeModifier.hpp"
#include "game/core/Direction.hpp"
#include "game/core/EquipmentSlot.hpp"
#include "game/core/Minecraft.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/DirtBlock.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/generation/Player.hpp"

HoeItem::HoeItem(const Item::Tier* tier) : Item()
{
    mMaxStackSize = 1;
    mTier = tier;
    setMaxDamage(tier->getUses());
    mAttackDamage = 1.0f + tier->getAttackDamageBonus();
}

void HoeItem::setBlock(not_null_ptr<ItemInstance> itemInstance, boost::shared_ptr<Player> player, Level* level,
                       const BlockPos& pos, const BlockState* state)
{
    level->playSound(player, pos, SoundEvents::HOE_TILL, SoundSource::eSoundSource_Blocks, 1.0f, 1.0f, 16.0f);
    if (!level->mIsClientSide)
    {
        level->setBlock(pos, state, Block::UPDATE_ALL | Block::UPDATE_IMMEDIATE, false);
        itemInstance->hurtAndBreak(1, player);
    }
}

InteractionResult::EInteractionResult HoeItem::useOn(boost::shared_ptr<Player> player, Level* level,
                                                     const BlockPos& pos, InteractionHand::EInteractionHand hand,
                                                     const Direction* direction, float hitX, float hitY, float hitZ,
                                                     bool simulate)
{
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    if (!player->mayUseItemAt(pos.relative(direction), direction, itemInstance))
    {
        return InteractionResult::eInteractionResult_Fail;
    }

    const BlockState* state = level->getBlockState(pos);
    Block* block = state->getBlock();
    Block* replacement = NULL;
    InteractionResult::EInteractionResult result = InteractionResult::eInteractionResult_Pass;
    if (direction != Direction::DOWN && level->getBlockState(pos.above())->getBlock()->mMaterial == Material::AIR)
    {
        if (block == Blocks::grass || block == Blocks::grass_path)
        {
            replacement = Blocks::farmland;
            result = InteractionResult::eInteractionResult_Success;
        }
        else if (block == Blocks::dirt)
        {
            switch (state->getValue(DirtBlock::VARIANT)->getData())
            {
            case 0:
                replacement = Blocks::farmland;
                result = InteractionResult::eInteractionResult_Success;
                break;
            case 1:
                replacement = Blocks::dirt;
                result = InteractionResult::eInteractionResult_Success;
                break;
            }
        }
    }

    if (!simulate && result == InteractionResult::eInteractionResult_Success)
    {
        setBlock(itemInstance, player, level, pos, replacement->defaultBlockState());
    }
    return result;
}

bool HoeItem::hurtEnemy(not_null_ptr<ItemInstance> itemInstance, boost::shared_ptr<LivingEntity> target,
                        boost::shared_ptr<LivingEntity> attacker)
{
    const int durability = 1;
    itemInstance->hurtAndBreak(durability, attacker);
    return true;
}

bool HoeItem::isHandEquipped()
{
    return true;
}

const Item::Tier* HoeItem::getTier()
{
    return mTier;
}

boost::unordered_map<eATTRIBUTE_ID, AttributeModifier*>* HoeItem::getDefaultAttributeModifiers(
    const EquipmentSlot* slot)
{
    boost::unordered_map<eATTRIBUTE_ID, AttributeModifier*>* modifiers = Item::getDefaultAttributeModifiers(slot);
    double attackDamage = 0.0;
    if (Minecraft::InMiniGame(eMiniGameId_Any, true))
    {
        const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
        attackDamage = miniGame->getBaseDamage(getId(this), 0.0);
    }
    if (slot == EquipmentSlot::MAINHAND)
    {
        (*modifiers)[SharedMonsterAttributes::ATTACK_DAMAGE->getId()] = new AttributeModifier(
            eMODIFIER_ID_BaseAttackDamage, attackDamage, 0);
        (*modifiers)[SharedMonsterAttributes::ATTACK_SPEED->getId()] = new AttributeModifier(
            eMODIFIER_ID_BaseAttackSpeed, mAttackDamage - 4.0f, 0);
    }
    return modifiers;
}
