#ifndef GAME_WORLD_GENERATION_CONSOLEGENERATESTRUCTURE_HPP
#define GAME_WORLD_GENERATION_CONSOLEGENERATESTRUCTURE_HPP

#include <vector>

#include "game/minigame/GameRuleDefinition.hpp"
#include "game/world/generation/ConsoleGenerateStructureAction.hpp"
#include "game/world/generation/StructurePiece.hpp"

class BlockPos;
class BoundingBox;
class DataInputStream;
class DataOutputStream;
class GameRuleSaveInterface;
class Level;
class Random;
class StructureManager;

class ConsoleGenerateStructure : public GameRuleDefinition, public StructurePiece
{
public:
    ConsoleGenerateStructure();
    virtual ConsoleGameRules::EGameRuleType getActionType();
    virtual void writeAttributes(GameRuleSaveInterface* saveInterface, unsigned int extraAttributeCount);
    virtual void getChildren(std::vector<GameRuleDefinition*>* children);
    virtual GameRuleDefinition* addChild(ConsoleGameRules::EGameRuleType type);
    virtual void addAttribute(const std::wstring& name, const std::wstring& value);
    virtual BoundingBox* getBoundingBox();
    virtual bool postProcess(Level* level, Random& random, BoundingBox* box);
    virtual int getMinY();
    virtual EStructurePiece GetType();
    virtual void addAdditionalSaveData(DataOutputStream* output);
    virtual void readAdditionalSaveData(DataInputStream* input, StructureManager* structureManager);
    bool checkIntersects(const BlockPos& min, const BlockPos& max);

private:
    int mX;
    int mY;
    int mZ;
    std::vector<ConsoleGenerateStructureAction*> mActions;
    int mDimension;
};

#endif
