#include "game/entity/projectile/SpectralArrowItem.hpp"

#include "game/entity/projectile/SpectralArrow.hpp"

SpectralArrowItem::SpectralArrowItem() : ArrowItem()
{
}

boost::shared_ptr<Arrow> SpectralArrowItem::createArrow(Level* level, not_null_ptr<ItemInstance> itemInstance,
                                                        boost::shared_ptr<LivingEntity> entity)
{
    return boost::shared_ptr<SpectralArrow>(new SpectralArrow(level, entity));
}
