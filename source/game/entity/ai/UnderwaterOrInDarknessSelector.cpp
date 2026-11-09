#include "game/entity/ai/UnderwaterOrInDarknessSelector.hpp"

#include "game/entity/mob/Entity.hpp"

bool UnderwaterOrInDarknessSelector::apply(boost::shared_ptr<Entity> entity) const
{
    return entity->isInWater() || entity->getBrightness() < 0.49;
}

UnderwaterOrInDarknessSelector::~UnderwaterOrInDarknessSelector()
{
}
