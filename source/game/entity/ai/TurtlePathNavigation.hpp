#ifndef GAME_ENTITY_AI_TURTLEPATHNAVIGATION_HPP
#define GAME_ENTITY_AI_TURTLEPATHNAVIGATION_HPP

#include "game/entity/ai/WaterBoundPathNavigation.hpp"

class BlockPos;
class Level;
class PathFinder;
class Turtle;

class TurtlePathNavigation : public WaterBoundPathNavigation
{
public:
    TurtlePathNavigation(Turtle* turtle, Level* level);

    virtual PathFinder* createPathFinder();
    virtual bool canUpdatePath();
    virtual bool isStableDestination(const BlockPos& pos);
};

#endif
