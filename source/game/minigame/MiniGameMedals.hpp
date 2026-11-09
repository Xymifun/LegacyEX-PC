#ifndef GAME_MINIGAME_MINIGAMEMEDALS_HPP
#define GAME_MINIGAME_MINIGAMEMEDALS_HPP

#include <vector>

class GameMedal;

extern std::vector<const GameMedal*> gBattleMedals;
extern std::vector<const GameMedal*> gTumbleMedals;
extern std::vector<const GameMedal*> gGlideMedals;
extern std::vector<const GameMedal*> gBuildMedals;

class MiniGameMedals
{
public:
    static void staticCtor();

    static const GameMedal* getMedalById(int medalId);
};

#endif
