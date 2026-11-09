#ifndef GAME_ENTITY_AI_TURTLEGOTOWATERGOAL_HPP
#define GAME_ENTITY_AI_TURTLEGOTOWATERGOAL_HPP

#include <string>

#include "game/world/block/MoveToBlockGoal.hpp"

class Turtle;

class TurtleGoToWaterGoal : public MoveToBlockGoal
{
public:
    TurtleGoToWaterGoal(Turtle* turtle, double speedModifier);
    virtual ~TurtleGoToWaterGoal();

    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual std::wstring getName() { return L"Turtle Goto Water"; }
    virtual bool isValidTarget(Level* level, const BlockPos& pos);

private:
    Turtle* mTurtle;
};

#endif
