#ifndef GAME_WORLD_BLOCK_ITEMPROPERTYFUNCTION_DAMAGE_HPP
#define GAME_WORLD_BLOCK_ITEMPROPERTYFUNCTION_DAMAGE_HPP

#include <boost/shared_ptr.hpp>

#include "game/core/Mth.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/world/block/ItemPropertyFunction.hpp"

class Level;
class LivingEntity;

class ItemPropertyFunction_Damage : public ItemPropertyFunction
{
public:
    virtual float call(not_null_ptr<ItemInstance> itemInstance, Level* level, boost::shared_ptr<LivingEntity> entity)
    {
        return Mth::clamp((float)itemInstance->getDamageValue() / (float)itemInstance->getMaxDamage(), 0.0f, 1.0f);
    }
};

#endif
