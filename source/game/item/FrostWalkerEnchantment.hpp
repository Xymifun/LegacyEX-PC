#ifndef GAME_ITEM_FROSTWALKERENCHANTMENT_HPP
#define GAME_ITEM_FROSTWALKERENCHANTMENT_HPP

#include "game/item/Enchantment.hpp"

class BlockPos;
class Level;

class FrostWalkerEnchantment : public Enchantment
{
public:
    FrostWalkerEnchantment(const Rarity* rarity, arrayWithLength<const EquipmentSlot*> slots);
    virtual ~FrostWalkerEnchantment();

    virtual int getMinCost(int level);
    virtual int getMaxCost(int level);
    virtual bool isTreasureOnly();
    virtual int getMaxLevel();
    static void onEntityMoved(const boost::shared_ptr<LivingEntity>& entity, Level* level, const BlockPos& pos,
                              int levelValue);
    virtual bool checkCompatibility(const Enchantment* other) const;
};

#endif
