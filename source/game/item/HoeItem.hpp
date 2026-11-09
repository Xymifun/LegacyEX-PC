#ifndef GAME_ITEM_HOEITEM_HPP
#define GAME_ITEM_HOEITEM_HPP

#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

#include "game/core/InteractionHand.hpp"
#include "game/core/InteractionResult.hpp"
#include "game/item/Item.hpp"

class AttributeModifier;
class BlockPos;
class BlockState;
class Direction;
class EquipmentSlot;
class ItemInstance;
class Level;
class LivingEntity;
class Player;

class HoeItem : public Item
{
public:
    HoeItem(const Item::Tier* tier);

    void setBlock(not_null_ptr<ItemInstance> itemInstance, boost::shared_ptr<Player> player, Level* level,
                  const BlockPos& pos, const BlockState* state);
    virtual InteractionResult::EInteractionResult useOn(boost::shared_ptr<Player> player, Level* level,
                                                        const BlockPos& pos, InteractionHand::EInteractionHand hand,
                                                        const Direction* direction, float hitX, float hitY, float hitZ,
                                                        bool simulate);
    virtual bool hurtEnemy(not_null_ptr<ItemInstance> itemInstance, boost::shared_ptr<LivingEntity> target,
                           boost::shared_ptr<LivingEntity> attacker);
    virtual bool isHandEquipped();
    const Item::Tier* getTier();
    virtual boost::unordered_map<eATTRIBUTE_ID, AttributeModifier*>* getDefaultAttributeModifiers(
        const EquipmentSlot* slot);

private:
    float mAttackDamage;
    const Item::Tier* mTier;
};

#endif
