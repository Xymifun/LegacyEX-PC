#ifndef GAME_CORE_SENSING_HPP
#define GAME_CORE_SENSING_HPP

#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

class Entity;
class Mob;

class Sensing
{
public:
    Sensing(Mob* mob);
    void tick();
    bool canSee(boost::shared_ptr<Entity> entity);

private:
    Mob* mMob;
    std::vector<boost::weak_ptr<Entity> > mSeen;
    std::vector<boost::weak_ptr<Entity> > mUnseen;
};

#endif
