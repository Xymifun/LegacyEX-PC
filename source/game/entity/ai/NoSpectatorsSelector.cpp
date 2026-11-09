#include "game/entity/ai/NoSpectatorsSelector.hpp"
#include "game/core/EntityType.hpp"

#include "game/entity/mob/Entity.hpp"
#include "game/world/generation/Player.hpp"

bool NoSpectatorsSelector::apply(boost::shared_ptr<Entity> entity) const
{
    if (eTYPE_DERIVED_FROM(EntityType::PLAYER, entity->GetType()))
    {
        boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);
        return !player->isSpectator();
    }

    return true;
}

NoSpectatorsSelector::~NoSpectatorsSelector()
{
}
