#include "game/entity/mob/RideEntityTask.hpp"

#include "game/entity/mob/Entity.hpp"

bool RideEntityTask::isCompleted()
{
    return completed;
}

void RideEntityTask::onRideEntity(boost::shared_ptr<Entity> entity)
{
    unsigned long long type = entity->GetType();

    if ((mEntityType & 0x3F) != 0)
    {
        if (type != (unsigned long long)mEntityType)
        {
            return;
        }
    }
    else if ((type & mEntityType) != (unsigned long long)mEntityType)
    {
        return;
    }

    completed = true;
}
