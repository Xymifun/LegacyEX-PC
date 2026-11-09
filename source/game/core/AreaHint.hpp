#ifndef GAME_CORE_AREAHINT_HPP
#define GAME_CORE_AREAHINT_HPP

#include "game/minigame/TutorialHint.hpp"

class AABB;

class AreaHint : public TutorialHint
{
public:
    AreaHint(eTutorial_Hint hint, Tutorial* tutorial, eTutorial_State state, eTutorial_State completionState,
             int descriptionId, double x0, double y0, double z0, double x1, double y1, double z1, bool repeatMessage,
             bool insideArea);
    virtual ~AreaHint();
    virtual int tick();

    AABB* area;
    bool insideArea;
    eTutorial_State state;
    eTutorial_State completionState;
};

#endif
