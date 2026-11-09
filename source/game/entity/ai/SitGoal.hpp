#ifndef GAME_ENTITY_AI_SITGOAL_HPP
#define GAME_ENTITY_AI_SITGOAL_HPP

#include <string>

#include "game/entity/ai/Goal.hpp"

class TamableAnimal;

class SitGoal : public Goal
{
public:
    SitGoal(TamableAnimal* mob);
    virtual ~SitGoal();
    virtual bool canUse();
    virtual void start();
    virtual void stop();
    void wantToSit(bool wantToSit);
    virtual std::wstring getName() { return L"Sit"; }

private:
    TamableAnimal* mMob;
    bool mWantToSit;
};

#endif
