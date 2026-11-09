#ifndef GAME_ITEM_DIGGERITEM_HPP
#define GAME_ITEM_DIGGERITEM_HPP

#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

#include "game/core/Attribute.hpp"
#include "game/item/Item.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class AttributeModifier;
class Block;
class BlockPos;
class BlockState;
class EquipmentSlot;
class ItemInstance;
class Level;
class LivingEntity;

class DiggerItem : public Item
{
public:
    DiggerItem(float attackDamage, float attackSpeed, const Item::Tier* tier, arrayWithLength<Block*> blocks);
    DiggerItem(const Item::Tier* tier, arrayWithLength<Block*> blocks);

    virtual ~DiggerItem() {}
    virtual float getDestroySpeed(not_null_ptr<ItemInstance> itemInstance, const BlockState* state);
    virtual bool hurtEnemy(not_null_ptr<ItemInstance> itemInstance, boost::shared_ptr<LivingEntity> target,
                           boost::shared_ptr<LivingEntity> attacker);
    virtual bool mineBlock(not_null_ptr<ItemInstance> itemInstance, Level* level, const BlockState* state,
                           const BlockPos& pos, boost::shared_ptr<LivingEntity> entity);
    virtual bool isHandEquipped();
    virtual int getEnchantmentValue();
    virtual bool isValidRepairItem(not_null_ptr<ItemInstance> itemInstance, not_null_ptr<ItemInstance> repairItem);
    virtual boost::unordered_map<eATTRIBUTE_ID, AttributeModifier*>* getDefaultAttributeModifiers(
        const EquipmentSlot* slot);

    const Item::Tier* getTier();

protected:
    void _init(float attackDamage, float attackSpeed, arrayWithLength<Block*> blocks);

    arrayWithLength<Block*> mBlocks;
    float mSpeed;
    float mAttackDamage;
    float mAttackSpeed;
    const Item::Tier* mTier;
};

#endif
