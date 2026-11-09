#include "game/core/Sensing.hpp"

#include "game/entity/mob/Mob.hpp"

Sensing::Sensing(Mob* mob)
{
    mMob = mob;
}

void Sensing::tick()
{
    mSeen.clear();
    mUnseen.clear();
}

bool Sensing::canSee(boost::shared_ptr<Entity> entity)
{
    for (std::vector<boost::weak_ptr<Entity> >::iterator it = mSeen.begin(); it != mSeen.end(); ++it)
    {
        if (entity == it->lock())
            return true;
    }

    for (std::vector<boost::weak_ptr<Entity> >::iterator it = mUnseen.begin(); it != mUnseen.end(); ++it)
    {
        if (entity == it->lock())
            return false;
    }

    bool visible = mMob->canSee(entity);
    if (visible)
    {
        mSeen.push_back(boost::weak_ptr<Entity>(entity));
    }
    else
    {
        mUnseen.push_back(boost::weak_ptr<Entity>(entity));
    }
    return visible;
}
