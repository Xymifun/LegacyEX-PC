#include "game/core/Team.hpp"

bool Team::isAlliedTo(Team* other)
{
    if (other != NULL && this == other)
    {
        return true;
    }

    return false;
}
