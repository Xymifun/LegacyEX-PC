#ifndef GAME_CORE_CHANGESTATECONSTRAINT_HPP
#define GAME_CORE_CHANGESTATECONSTRAINT_HPP

#include "game/minigame/Tutorial.hpp"
#include "game/minigame/TutorialConstraint.hpp"

class AABB;
class GameType;
class Vec3;

class ChangeStateConstraint : public TutorialConstraint
{
public:
    ChangeStateConstraint(Tutorial* tutorial, eTutorial_State targetState, eTutorial_State* states,
                          unsigned int numStates, double x0, double y0, double z0, double x1, double y1, double z1,
                          bool insideArea, bool changeGameType, const GameType* gameType);

    virtual ~ChangeStateConstraint();

    virtual int getType();
    virtual void tick(int playerId);

    bool isTargetState(int state);
    bool withinTargetStateArea(int playerId);
    bool withinTargetStateArea(Vec3* pos);

    AABB* area;
    bool insideArea;
    bool changeGameType;
    const GameType* gameType;
    const GameType* previousGameType;
    eTutorial_State targetState;
    eTutorial_State* states;
    unsigned int numStates;
    bool stateChanged;
    eTutorial_State previousState;
    bool gameTypeChanged;
    Tutorial* tutorial;
};

#endif
