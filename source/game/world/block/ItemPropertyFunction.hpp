#pragma once

#include <boost/shared_ptr.hpp>

#include "game/item/not_null_ptr.hpp"

class ItemInstance;
class Level;
class LivingEntity;

class ItemPropertyFunction
{
public:
    virtual float call(not_null_ptr<ItemInstance> itemInstance, Level* level,
                       boost::shared_ptr<LivingEntity> entity) = 0;
};
