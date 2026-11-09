#ifndef GAME_ITEM_SWORDITEM_HPP
#define GAME_ITEM_SWORDITEM_HPP

#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

#include "game/core/Attribute.hpp"
#include "game/core/InteractionHand.hpp"
#include "game/core/InteractionResultHolder.hpp"
#include "game/item/Item.hpp"
#include "game/item/not_null_ptr.hpp"

class AttributeModifier;
class BlockPos;
class BlockState;
class EquipmentSlot;
class ItemInstance;
class ItemTooltipDataHolder;
class Level;
class LivingEntity;
class Player;

class SwordItem : public Item
{
public:
    SwordItem(const Item::Tier* tier);

    virtual unsigned int GetUseTooltip(const ItemTooltipDataHolder& tooltipData);
    virtual float getDestroySpeed(not_null_ptr<ItemInstance> itemInstance, const BlockState* state);
    virtual InteractionResultHolder use(Level* level, boost::shared_ptr<Player> player,
                                        InteractionHand::EInteractionHand hand);
    virtual bool hurtEnemy(not_null_ptr<ItemInstance> itemInstance, boost::shared_ptr<LivingEntity> target,
                           boost::shared_ptr<LivingEntity> attacker);
    virtual bool mineBlock(not_null_ptr<ItemInstance> itemInstance, Level* level, const BlockState* state,
                           const BlockPos& pos, boost::shared_ptr<LivingEntity> entity);
    virtual bool canDestroySpecial(const BlockState* state);
    virtual bool isHandEquipped();
    virtual int getUseAnimation(not_null_ptr<const ItemInstance> itemInstance) const;
    virtual int getUseDuration(not_null_ptr<ItemInstance> itemInstance);
    virtual int getEnchantmentValue();
    virtual bool isValidRepairItem(not_null_ptr<ItemInstance> itemInstance, not_null_ptr<ItemInstance> repairItem);
    virtual boost::unordered_map<eATTRIBUTE_ID, AttributeModifier*>* getDefaultAttributeModifiers(
        const EquipmentSlot* slot);
    virtual float getTierDamage();

    const Item::Tier* getTier();

private:
    float mAttackDamage;
    const Item::Tier* mTier;
};

#endif
