#ifndef GAME_CORE_FOODDATA_HPP
#define GAME_CORE_FOODDATA_HPP

#include <boost/shared_ptr.hpp>

#include "game/item/not_null_ptr.hpp"

class CompoundTag;
class FoodItem;
class ItemInstance;
class Player;

class FoodData
{
public:
    FoodData();
    void eat(int foodLevel, float saturationModifier);
    void eat(FoodItem* foodItem, const not_null_ptr<ItemInstance>& itemInstance);
    void addExhaustion(float exhaustion);
    void tick(const boost::shared_ptr<Player>& player);
    void readAdditionalSaveData(CompoundTag* tag);
    void addAdditonalSaveData(CompoundTag* tag);
    unsigned int getFoodLevel();
    unsigned int getLastFoodLevel();
    bool needsFood();
    bool isStarving();
    float getSaturationLevel();
    void setFoodLevel(int foodLevel);
    void setSaturation(float saturationLevel);
    void setExhaustion(float exhaustionLevel);
    float getExhaustionLevel();

private:
    int foodLevel;
    float saturationLevel;
    float exhaustionLevel;
    int foodTickTimer;
    int lastFoodLevel;
};

#endif
