#ifndef GAME_CORE_INPUTCONSTRAINT_HPP
#define GAME_CORE_INPUTCONSTRAINT_HPP

#include "game/minigame/TutorialConstraint.hpp"

class InputConstraint : public TutorialConstraint
{
public:
    InputConstraint(int mapping) : TutorialConstraint(-1), constrainedMapping(mapping) {}

    virtual ~InputConstraint();

    virtual int getType();
    virtual bool isMappingConstrained(int playerId, int mapping);

    int constrainedMapping;
};

#endif
