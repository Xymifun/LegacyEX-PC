#ifndef GAME_WORLD_GENERATION_XBOXSTRUCTUREACTIONPLACESPAWNER_HPP
#define GAME_WORLD_GENERATION_XBOXSTRUCTUREACTIONPLACESPAWNER_HPP

#include <stdint.h>
#include <string>

#include "game/world/block/XboxStructureActionPlaceBlock.hpp"

class BoundingBox;
class Level;
class StructurePiece;

class XboxStructureActionPlaceSpawner : public XboxStructureActionPlaceBlock
{
public:
    XboxStructureActionPlaceSpawner();
    virtual ~XboxStructureActionPlaceSpawner();

    uint32_t placeSpawnerInLevel(StructurePiece* structurePiece, Level* level, BoundingBox* box);
    virtual ConsoleGameRules::EGameRuleType getActionType();

private:
    std::wstring mEntityId;
};

#endif
