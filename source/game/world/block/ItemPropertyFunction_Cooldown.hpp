#ifndef GAME_WORLD_BLOCK_ITEMPROPERTYFUNCTION_COOLDOWN_HPP
#define GAME_WORLD_BLOCK_ITEMPROPERTYFUNCTION_COOLDOWN_HPP

#include <boost/shared_ptr.hpp>

#include "game/item/not_null_ptr.hpp"
#include "game/world/block/ItemPropertyFunction.hpp"

class ItemInstance;
class Level;
class LivingEntity;

class ItemPropertyFunction_Cooldown : public ItemPropertyFunction
{
public:
    virtual float call(not_null_ptr<ItemInstance> itemInstance, Level* level, boost::shared_ptr<LivingEntity> entity);
};

#endif
