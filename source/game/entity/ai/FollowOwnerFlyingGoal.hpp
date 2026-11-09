#ifndef GAME_ENTITY_AI_FOLLOWOWNERFLYINGGOAL_HPP
#define GAME_ENTITY_AI_FOLLOWOWNERFLYINGGOAL_HPP

#include <string>

#include "game/entity/ai/FollowOwnerGoal.hpp"

class TamableAnimal;

class FollowOwnerFlyingGoal : public FollowOwnerGoal
{
public:
    FollowOwnerFlyingGoal(TamableAnimal* tamable, double speedModifier, float startDistance, float stopDistance);
    virtual ~FollowOwnerFlyingGoal() {}
    virtual std::wstring getName() { return L"FollowOwnerFlyingGoal"; }
    virtual bool isTeleportFriendlyBlock(int x, int z, int y, int dx, int dz);
};

#endif
