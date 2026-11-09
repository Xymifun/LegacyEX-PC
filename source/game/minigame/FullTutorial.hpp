#ifndef GAME_MINIGAME_FULLTUTORIAL_HPP
#define GAME_MINIGAME_FULLTUTORIAL_HPP

#include "game/minigame/Tutorial.hpp"

class GameModeConstraints;

class FullTutorial : public Tutorial
{
public:
    FullTutorial(int playerId, GameModeConstraints* constraints, bool enabled);

    void AddTutorialArea(eTutorial_State state);
    void PopulateChest(eTutorial_State state);
    virtual void tick();
    virtual unsigned char isStateCompleted(eTutorial_State state);
    virtual void setStateCompleted(eTutorial_State state);

private:
    bool mEnabledForPlayer;
    char mProgressFlags;
    unsigned char mStateCompleted[44];
};

#endif
