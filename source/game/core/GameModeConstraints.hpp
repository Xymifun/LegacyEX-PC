#ifndef GAME_CORE_GAMEMODECONSTRAINTS_HPP
#define GAME_CORE_GAMEMODECONSTRAINTS_HPP

#include <utility>
#include <vector>

#include <boost/unordered_map.hpp>

#include "game/minigame/TutorialConstraint.hpp"

class Vec3;

class GameModeConstraints
{
public:
    typedef std::vector<TutorialConstraint*> ConstraintList;
    typedef std::vector<std::pair<TutorialConstraint*, unsigned char> > DelayedConstraintList;
    typedef boost::unordered_map<int, ConstraintList> ConstraintMap;
    typedef boost::unordered_map<int, DelayedConstraintList> DelayedConstraintMap;
    typedef boost::unordered_map<int, TutorialConstraint*> FailedConstraintMap;

    GameModeConstraints(int playerId);
    ~GameModeConstraints();
    void tick();
    void SetCurrentState(int state);
    bool HasFailedConstraint(int state);
    TutorialConstraint* GetFailedConstraint(int state);
    bool DidCurrentFailedConstraintChange();
    void MoveDelayedConstraints(int oldState, int newState);
    void AddGlobalConstraint(TutorialConstraint* constraint);
    void AddConstraint(TutorialConstraint* constraint, bool takeOwnership);
    void RemoveConstraint(TutorialConstraint* constraint, bool delayed);
    void ClearAllAreaConstraints();
    bool InConstraintArea(Vec3* pos, int state);
    bool NotInConstraintArea(int playerId, int state);
    bool HasGlobalConstraint(int state);
    int canMoveToPosition(double oldX, double oldY, double oldZ, double newX, double newY, double newZ,
                          Vec3* clampedPosition);
    bool CanUsePortal();
    bool isInputAllowed(int mapping);

    ConstraintList globalConstraints;
    ConstraintMap constraints;
    DelayedConstraintMap delayedConstraints;
    ConstraintList ownedConstraints;
    FailedConstraintMap failedConstraints;
    int playerId;
    int currentState;
    bool failedConstraintChanged;
};

#endif
