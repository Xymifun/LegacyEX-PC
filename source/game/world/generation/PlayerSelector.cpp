#include "game/world/generation/PlayerSelector.hpp"

#include "game/core/GameMode.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/server/ServerPlayerGameMode.hpp"
#include "game/world/generation/Player.hpp"

bool PlayerSelector::apply(boost::shared_ptr<Player> player) const
{
    return mEntity->distanceToSqr(player) < 2500.0
           && boost::static_pointer_cast<ServerPlayer>(player)->mGameMode->isSurvival();
}
