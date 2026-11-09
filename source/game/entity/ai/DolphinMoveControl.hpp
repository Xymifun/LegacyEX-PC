#ifndef GAME_ENTITY_AI_DOLPHINMOVECONTROL_HPP
#define GAME_ENTITY_AI_DOLPHINMOVECONTROL_HPP

#include "game/entity/ai/MoveControl.hpp"

class Dolphin;

class DolphinMoveControl : public MoveControl
{
public:
    DolphinMoveControl(Dolphin* dolphin);
    virtual ~DolphinMoveControl();
    virtual void tick();

private:
    float mMaxYRotChange;
    float mMaxXRotChange;
};

#endif
