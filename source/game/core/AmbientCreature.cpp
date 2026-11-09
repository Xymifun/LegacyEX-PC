#include "game/core/AmbientCreature.hpp"

AmbientCreature::AmbientCreature(Level* level) : Mob(level)
{
}

bool AmbientCreature::canBeLeashed(const boost::shared_ptr<Player>&)
{
    return false;
}
