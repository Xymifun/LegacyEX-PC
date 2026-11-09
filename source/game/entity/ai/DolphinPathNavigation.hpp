#ifndef GAME_ENTITY_AI_DOLPHINPATHNAVIGATION_HPP
#define GAME_ENTITY_AI_DOLPHINPATHNAVIGATION_HPP

#include "game/entity/ai/WaterBoundPathNavigation.hpp"

class BlockPos;
class Dolphin;
class Level;

class DolphinPathNavigation : public WaterBoundPathNavigation
{
public:
    DolphinPathNavigation(Dolphin* dolphin, Level* level);

    virtual Path* createPath(const BlockPos& pos);
    virtual bool canUpdatePath();
    virtual void updatePath();
    virtual bool isStableDestination(const BlockPos& pos);
};

#endif
