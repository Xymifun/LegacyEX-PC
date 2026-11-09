#ifndef GAME_ENTITY_AI_INTERACTGOAL_HPP
#define GAME_ENTITY_AI_INTERACTGOAL_HPP

#include <string>
#include <typeinfo>

#include "game/world/generation/LookAtPlayerGoal.hpp"

class Mob;

class InteractGoal : public LookAtPlayerGoal
{
public:
    InteractGoal(Mob* mob, const std::type_info& lookAtType, float lookDistance);
    InteractGoal(Mob* mob, const std::type_info& lookAtType, float lookDistance, float probability);
    virtual ~InteractGoal() {}
    virtual std::wstring getName() { return L"Interact"; }
};

#endif
