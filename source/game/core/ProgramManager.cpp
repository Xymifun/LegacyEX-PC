#include "game/core/ProgramManager.hpp"

static ProgramManager sInstance;

ProgramManager* ProgramManager::getInstance()
{
    return &sInstance;
}

void ProgramManager::releaseProgram(Effect* effect)
{
}
