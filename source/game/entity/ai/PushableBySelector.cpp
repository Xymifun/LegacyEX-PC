#include "game/entity/ai/PushableBySelector.hpp"
#include "game/core/EntityType.hpp"

#include "game/entity/mob/Entity.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"

PushableBySelector::PushableBySelector(const boost::shared_ptr<Entity>& pusher)
    : pusher(pusher),
      team(pusher->getTeam())
{
}

bool PushableBySelector::apply(boost::shared_ptr<Entity> entity) const
{
    if (!entity->isPushable(pusher))
    {
        return false;
    }

    if (pusher->mLevel->mIsClientSide
        && (!entity->instanceof(EntityType::PLAYER) || !boost::static_pointer_cast<Player>(entity)->isLocalPlayer()))
    {
        return false;
    }

    if (pusher->mLevel->mIsClientSide && pusher->instanceof(EntityType::LOCAL_PLAYER)
        && entity->instanceof(EntityType::LOCAL_PLAYER))
    {
        return false;
    }

    return true;
}
