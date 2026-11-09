#ifndef GAME_ITEM_WATERWORKERENCHANTMENT_HPP
#define GAME_ITEM_WATERWORKERENCHANTMENT_HPP
#include "game/item/Enchantment.hpp"
#include "game/world/biome/arrayWithLength.hpp"
class WaterWorkerEnchantment : public Enchantment
{
public:
    WaterWorkerEnchantment(const Rarity* rarity, arrayWithLength<const EquipmentSlot*> slots);
    virtual ~WaterWorkerEnchantment();
    virtual int getMinCost(int level);
    virtual int getMaxCost(int level);
    virtual int getMaxLevel();
};
#endif
