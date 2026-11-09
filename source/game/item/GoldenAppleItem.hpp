#ifndef GAME_ITEM_GOLDENAPPLEITEM_HPP
#define GAME_ITEM_GOLDENAPPLEITEM_HPP

#include <boost/shared_ptr.hpp>

#include "game/item/FoodItem.hpp"
#include "game/item/not_null_ptr.hpp"

class ItemInstance;
class Level;
class Player;
class Rarity;

class GoldenAppleItem : public FoodItem
{
public:
    GoldenAppleItem(int nutrition, float saturationModifier, bool isMeat);
    virtual bool isFoil(not_null_ptr<ItemInstance> itemInstance);
    virtual const Rarity* getRarity(not_null_ptr<ItemInstance> itemInstance);
    virtual int getUseDescriptionId(int auxValue);
    virtual int getUseDescriptionId(not_null_ptr<ItemInstance> itemInstance);
    virtual void addEatEffect(not_null_ptr<ItemInstance> itemInstance, Level* level, boost::shared_ptr<Player> player);
};

#endif
