#ifndef GAME_ENTITY_AI_PLAYWITHITEMSGOAL_HPP
#define GAME_ENTITY_AI_PLAYWITHITEMSGOAL_HPP

#include <string>

#include "game/entity/ai/Goal.hpp"

class Dolphin;

class PlayWithItemsGoal : public Goal
{
public:
    PlayWithItemsGoal(Dolphin* dolphin);
    virtual ~PlayWithItemsGoal();

    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual void start();
    virtual void stop();
    virtual void tick();
    virtual std::wstring getName();

private:
    Dolphin* mDolphin;
};

#endif
