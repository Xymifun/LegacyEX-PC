#ifndef GAME_ENTITY_MOB_MULTIENTITYMOB_HPP
#define GAME_ENTITY_MOB_MULTIENTITYMOB_HPP

#include <boost/shared_ptr.hpp>

class DamageSource;
class EntityTooltipDataHolder;
class Level;
class MultiEntityMobPart;

class MultiEntityMob
{
public:
    virtual Level* getLevel() = 0;
    virtual bool hurt(boost::shared_ptr<MultiEntityMobPart> part, const DamageSource* source, float amount) = 0;
    virtual void GetInteractTooltip(boost::shared_ptr<MultiEntityMobPart> part,
                                    const EntityTooltipDataHolder& holder) = 0;
};

#endif
