#ifndef GAME_ENTITY_AI_DOORINTERACTGOAL_HPP
#define GAME_ENTITY_AI_DOORINTERACTGOAL_HPP

#include <string>

#include "game/entity/ai/Goal.hpp"
#include "game/world/block/BlockPos.hpp"

class Block;
class Mob;

class DoorInteractGoal : public Goal
{
public:
    DoorInteractGoal(Mob* mob);
    virtual ~DoorInteractGoal() {}
    Block* getDoorBlock(const BlockPos& pos);
    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual void start();
    virtual void tick();
    virtual std::wstring getName() { return L"DoorInteract"; }

protected:
    Mob* mMob;
    BlockPos mDoorPos;
    Block* mDoorBlock;
    bool mHasDoor;
    float mDoorX;
    float mDoorZ;
};

#endif
