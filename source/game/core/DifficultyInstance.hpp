#ifndef GAME_CORE_DIFFICULTYINSTANCE_HPP
#define GAME_CORE_DIFFICULTYINSTANCE_HPP

#include "game/core/Difficulty.hpp"

class DifficultyInstance
{
public:
    static const float LEVEL_TIME_OFFSET;
    static const float LEVEL_TIME_RANGE;
    static const float INHABITED_TIME_RANGE;

    DifficultyInstance(const Difficulty* difficulty, long long levelTime, long long inhabitedTime, float moonPhase);
    ~DifficultyInstance();

    const Difficulty* getDifficulty() const;
    float getEffectiveDifficulty() const;
    float getSpecialMultiplier() const;
    bool isHard();
    bool isHarderThan(float threshold);

private:
    float calculateDifficulty(const Difficulty* difficulty, long long levelTime, long long inhabitedTime,
                              float moonPhase) const;

    const Difficulty* difficulty;
    float effectiveDifficulty;
};

#endif
