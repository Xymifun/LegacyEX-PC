#ifndef GAME_ENTITY_PROJECTILE_SPECTRALARROWITEM_HPP
#define GAME_ENTITY_PROJECTILE_SPECTRALARROWITEM_HPP

#include <boost/shared_ptr.hpp>

#include "game/entity/projectile/ArrowItem.hpp"

class Arrow;
class Level;
class LivingEntity;
class ItemInstance;

class SpectralArrowItem : public ArrowItem
{
public:
    SpectralArrowItem();

    virtual boost::shared_ptr<Arrow> createArrow(Level* level, not_null_ptr<ItemInstance> itemInstance,
                                                 boost::shared_ptr<LivingEntity> entity);
};

#endif
