#ifndef GAME_MINIGAME_THEMESUGGESTIONMEDAL_HPP
#define GAME_MINIGAME_THEMESUGGESTIONMEDAL_HPP

#include "game/minigame/GameMedal.hpp"

class PlayerStats;

class ThemeSuggestionMedal : public GameMedal
{
public:
    ThemeSuggestionMedal(int name, float target, unsigned int weight, float (&scoreFunction)(const PlayerStats*),
                         bool bestOnly);

    virtual float GetScore(PlayerStats* playerStats) const;

    static bool NEEDS_TO_WIN(const PlayerStats* playerStats);
    static bool NEEDS_TO_LOSE_BADLY(const PlayerStats* playerStats);
};

#endif
