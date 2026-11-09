#include "game/core/IsLookingAtMePredicate.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/EnderMan.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/world/generation/Player.hpp"

bool IsLookingAtMePredicate::apply(boost::shared_ptr<Entity> entity) const
{
    return entity && mEnderMan->isLookingAtMe(boost::static_pointer_cast<Player>(entity));
}
