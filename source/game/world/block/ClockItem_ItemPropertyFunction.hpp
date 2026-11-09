#ifndef GAME_WORLD_BLOCK_CLOCKITEM_ITEMPROPERTYFUNCTION_HPP
#define GAME_WORLD_BLOCK_CLOCKITEM_ITEMPROPERTYFUNCTION_HPP

#include <boost/shared_ptr.hpp>

#include "game/item/not_null_ptr.hpp"
#include "game/world/block/ItemPropertyFunction.hpp"

class ItemInstance;
class Level;
class LivingEntity;

class ClockItem_ItemPropertyFunction : public ItemPropertyFunction
{
public:
    virtual float call(not_null_ptr<ItemInstance> itemInstance, Level* level, boost::shared_ptr<LivingEntity> entity);

private:
    double mRotation;
    double mWobble;
    long long mLastUpdate;

    double wobble(Level* level, double value);
};

#endif
