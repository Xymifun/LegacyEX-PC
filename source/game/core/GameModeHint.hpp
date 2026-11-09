#ifndef GAME_CORE_GAMEMODEHINT_HPP
#define GAME_CORE_GAMEMODEHINT_HPP

#include "game/minigame/TutorialHint.hpp"

class GameModeHint : public TutorialHint
{
public:
    GameModeHint(eTutorial_Hint hint, Tutorial* tutorial, int descriptionId, eTutorial_GameMode* gameModes,
                 unsigned int gameModeCount, bool allowFade, int minimumDisplayTime);
    virtual ~GameModeHint();
    virtual unsigned int onGameModeChange(eTutorial_GameMode gameMode);

private:
    eTutorial_GameMode* gameModes;
    unsigned int gameModeCount;
};

#endif
