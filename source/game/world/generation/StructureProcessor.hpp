#ifndef GAME_WORLD_GENERATION_STRUCTUREPROCESSOR_HPP
#define GAME_WORLD_GENERATION_STRUCTUREPROCESSOR_HPP

#include "game/world/generation/StructureTemplate.hpp"

class BlockPos;
class Level;

class StructureProcessor
{
public:
    virtual ~StructureProcessor() {}
    virtual StructureTemplate::StructureBlockInfo* processBlock(Level* level, const BlockPos& position,
                                                                StructureTemplate::StructureBlockInfo* blockInfo) = 0;
};

#endif
