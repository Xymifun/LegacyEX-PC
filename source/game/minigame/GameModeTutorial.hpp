#ifndef GAME_MINIGAME_GAMEMODETUTORIAL_HPP
#define GAME_MINIGAME_GAMEMODETUTORIAL_HPP

#include "game/minigame/Tutorial.hpp"

class GameModeTutorial : public Tutorial
{
public:
    GameModeTutorial(int playerId, GameModeConstraints* constraints, int gameMode);
    virtual unsigned char isStateCompleted(eTutorial_State state);
    virtual void setStateCompleted(eTutorial_State state);
    virtual void changeTutorialState(eTutorial_State state, UIScene* scene);

private:
    unsigned char stateCompleted[44];
};

#endif
