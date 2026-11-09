#include "game/entity/mob/MobCache.hpp"

#include "game/entity/mob/EntityIO.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/server/ServerLevel.hpp"

MobCache::MobCache()
{
}

boost::shared_ptr<Mob> MobCache::getTestMob(ServerLevel* level, unsigned long long entityType)
{
    iterator found = find(entityType);

    if (found == end())
    {
        boost::shared_ptr<Entity> entity = EntityIO::newEntity(entityType, level);
        (*this)[entityType] = boost::static_pointer_cast<Mob>(entity);
        found = find(entityType);
    }
    else
    {
        found->second->setLevel(level);
    }

    return found->second;
}

void MobCache::removeFromCache(boost::shared_ptr<Mob> mob)
{
    erase(find(mob->GetType()));
}
