#include "game/core/DifficultyInstance.hpp"

#include "game/core/Mth.hpp"

const float DifficultyInstance::LEVEL_TIME_OFFSET = -72000.0f;
const float DifficultyInstance::LEVEL_TIME_RANGE = 1440000.0f;
const float DifficultyInstance::INHABITED_TIME_RANGE = 3600000.0f;

DifficultyInstance::DifficultyInstance(const Difficulty* difficulty, long long levelTime, long long inhabitedTime,
                                       float moonPhase)
    : difficulty(difficulty),
      effectiveDifficulty(calculateDifficulty(difficulty, levelTime, inhabitedTime, moonPhase))
{
}

DifficultyInstance::~DifficultyInstance()
{
}

const Difficulty* DifficultyInstance::getDifficulty() const
{
    return difficulty;
}

float DifficultyInstance::calculateDifficulty(const Difficulty* difficulty, long long levelTime,
                                              long long inhabitedTime, float moonPhase) const
{
    if (difficulty == Difficulty::PEACEFUL)
    {
        return 0.0f;
    }

    bool hard = difficulty == Difficulty::HARD;

    float scale = 0.75f;
    float timeFactor = Mth::clamp(((float)levelTime + -72000.0f) / 1440000.0f, 0.0f, 1.0f) * 0.25f;
    scale += timeFactor;

    float bonus = 0.0f;
    bonus += Mth::clamp((float)inhabitedTime / 3600000.0f, 0.0f, 1.0f) * (hard ? 1.0f : 0.75f);
    bonus += Mth::clamp(moonPhase * 0.25f, 0.0f, timeFactor);

    if (difficulty == Difficulty::EASY)
    {
        bonus *= 0.5f;
    }

    scale += bonus;
    return (float)difficulty->getId() * scale;
}

float DifficultyInstance::getEffectiveDifficulty() const
{
    return effectiveDifficulty;
}

float DifficultyInstance::getSpecialMultiplier() const
{
    if (effectiveDifficulty < 2.0f)
    {
        return 0.0f;
    }

    if (effectiveDifficulty > 4.0f)
    {
        return 1.0f;
    }

    return (effectiveDifficulty - 2.0f) * 0.5f;
}

bool DifficultyInstance::isHard()
{
    return difficulty == Difficulty::HARD;
}

bool DifficultyInstance::isHarderThan(float threshold)
{
    return effectiveDifficulty > threshold;
}
