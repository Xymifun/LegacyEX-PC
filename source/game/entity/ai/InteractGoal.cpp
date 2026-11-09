#include "game/entity/ai/InteractGoal.hpp"

#include <string>

InteractGoal::InteractGoal(Mob* mob, const std::type_info& lookAtType, float lookDistance, float probability)
    : LookAtPlayerGoal(mob, lookAtType, lookDistance, probability)
{
    setRequiredControlFlags(3);
}
