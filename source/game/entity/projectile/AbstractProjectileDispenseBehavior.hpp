#ifndef GAME_ENTITY_PROJECTILE_ABSTRACTPROJECTILEDISPENSEBEHAVIOR_HPP
#define GAME_ENTITY_PROJECTILE_ABSTRACTPROJECTILEDISPENSEBEHAVIOR_HPP

#include <boost/shared_ptr.hpp>

#include "game/item/DefaultDispenseItemBehavior.hpp"
#include "game/item/not_null_ptr.hpp"

class ItemInstance;
class Level;
class Position;
class Projectile;

class AbstractProjectileDispenseBehavior : public DefaultDispenseItemBehavior
{
public:
    virtual ~AbstractProjectileDispenseBehavior() {}

    virtual not_null_ptr<ItemInstance> execute(BlockSource* blockSource, not_null_ptr<ItemInstance> itemInstance,
                                               DefaultDispenseItemBehavior::eOUTCOME& outcome);
    virtual void playSound(BlockSource* blockSource, DefaultDispenseItemBehavior::eOUTCOME outcome);
    virtual float getUncertainty();
    virtual float getPower();

protected:
    virtual boost::shared_ptr<Projectile> getProjectile(Level* level, Position* position,
                                                        not_null_ptr<ItemInstance> itemInstance) = 0;
};

#endif
