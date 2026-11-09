#ifndef GAME_WORLD_GENERATION_CONSOLEGENERATESTRUCTUREACTION_HPP
#define GAME_WORLD_GENERATION_CONSOLEGENERATESTRUCTUREACTION_HPP

#include <stdint.h>

#include "game/minigame/GameRuleDefinition.hpp"

class ConsoleGenerateStructureAction : public GameRuleDefinition
{
public:
    virtual ~ConsoleGenerateStructureAction() {}
    virtual uint32_t getEndX() = 0;
    virtual uint32_t getEndY() = 0;
    virtual uint32_t getEndZ() = 0;
};

#endif
