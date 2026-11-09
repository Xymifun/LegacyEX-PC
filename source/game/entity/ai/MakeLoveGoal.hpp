#ifndef GAME_ENTITY_AI_MAKELOVEGOAL_HPP
#define GAME_ENTITY_AI_MAKELOVEGOAL_HPP

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <string>

#include "game/entity/ai/Goal.hpp"

class Level;
class Villager;
class Village;

class MakeLoveGoal : public Goal
{
public:
    MakeLoveGoal(Villager* villager);

    bool villageNeedsMoreVillagers();
    virtual bool canUse();
    virtual void start();
    virtual void stop();
    virtual bool canContinueToUse();
    void breed();
    virtual void tick();
    virtual void setLevel(Level* level);
    virtual std::wstring getName() { return L"MakeLove"; }

private:
    Villager* mVillager;
    boost::weak_ptr<Villager> mPartner;
    Level* mLevel;
    int mRunningTicks;
    boost::weak_ptr<Village> mVillage;
};

#endif
