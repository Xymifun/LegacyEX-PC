#include "game/world/generation/ConsoleGenerateStructure.hpp"

#include <algorithm>
#include <string>

#include "game/core/BoundingBox.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/Direction.hpp"
#include "game/data/GameRuleSaveInterface.hpp"
#include "game/item/PlaceContainerRuleDefinition.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/XboxStructureActionPlaceBlock.hpp"
#include "game/world/generation/StructureManager.hpp"
#include "game/world/generation/XboxStructureActionGenerateBox.hpp"
#include "game/world/generation/XboxStructureActionPlaceSpawner.hpp"

template <typename T>
std::wstring _toString(T value);

template <typename T>
T _fromString(const std::wstring& value);

ConsoleGenerateStructure::ConsoleGenerateStructure() : StructurePiece(0)
{
    mZ = 0;
    mY = 0;
    mX = 0;
    mBoundingBox = NULL;
    setOrientation(Direction::NORTH);
    mDimension = 0;
}

ConsoleGameRules::EGameRuleType ConsoleGenerateStructure::getActionType()
{
    return ConsoleGameRules::eGameRuleType_GenerateStructure;
}

void ConsoleGenerateStructure::writeAttributes(GameRuleSaveInterface* saveInterface, unsigned int extraAttributeCount)
{
    GameRuleDefinition::writeAttributes(saveInterface, extraAttributeCount + 5);
    saveInterface->saveAttribute(ConsoleGameRules::eGameRuleAttr_x, _toString(mX));
    saveInterface->saveAttribute(ConsoleGameRules::eGameRuleAttr_y, _toString(mY));
    saveInterface->saveAttribute(ConsoleGameRules::eGameRuleAttr_z, _toString(mZ));
    saveInterface->saveAttribute(ConsoleGameRules::eGameRuleAttr_orientation,
                                 _toString<const Direction*>(getOrientation()));
    saveInterface->saveAttribute(ConsoleGameRules::eGameRuleAttr_dimension, _toString(mDimension));
}

void ConsoleGenerateStructure::getChildren(std::vector<GameRuleDefinition*>* children)
{
    for (std::vector<ConsoleGenerateStructureAction*>::iterator it = mActions.begin(); it != mActions.end(); ++it)
    {
        children->push_back(*it);
    }
}

GameRuleDefinition* ConsoleGenerateStructure::addChild(ConsoleGameRules::EGameRuleType type)
{
    GameRuleDefinition* rule = NULL;
    if (type == ConsoleGameRules::eGameRuleType_GenerateBox)
    {
        rule = new XboxStructureActionGenerateBox();
        mActions.push_back(static_cast<ConsoleGenerateStructureAction*>(rule));
    }
    else if (type == ConsoleGameRules::eGameRuleType_PlaceBlock)
    {
        rule = new XboxStructureActionPlaceBlock();
        mActions.push_back(static_cast<ConsoleGenerateStructureAction*>(rule));
    }
    else if (type == ConsoleGameRules::eGameRuleType_PlaceContainer)
    {
        rule = new PlaceContainerRuleDefinition();
        mActions.push_back(static_cast<ConsoleGenerateStructureAction*>(rule));
    }
    else if (type == ConsoleGameRules::eGameRuleType_PlaceSpawner)
    {
        rule = new XboxStructureActionPlaceSpawner();
        mActions.push_back(static_cast<ConsoleGenerateStructureAction*>(rule));
    }
    return rule;
}

void ConsoleGenerateStructure::addAttribute(const std::wstring& name, const std::wstring& value)
{
    if (name == L"x")
    {
        mX = _fromString<int>(value);
        return;
    }
    if (name == L"y")
    {
        mY = _fromString<int>(value);
        return;
    }
    if (name == L"z")
    {
        mZ = _fromString<int>(value);
        return;
    }
    if (name == L"orientation")
    {
        setOrientation(Direction::from3DDataValue(_fromString<int>(value)));
        return;
    }
    if (name != L"dim")
    {
        GameRuleDefinition::addAttribute(name, value);
        return;
    }
    int dimension = _fromString<int>(value);
    if (dimension + 1U < 3)
    {
        mDimension = dimension;
        return;
    }
    mDimension = 0;
}

bool ConsoleGenerateStructure::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (level->mDimension->getType() == DimensionType::getById(mDimension))
    {
        for (std::vector<ConsoleGenerateStructureAction*>::iterator it = mActions.begin(); it != mActions.end(); it++)
        {
            ConsoleGenerateStructureAction* action = *it;
            switch (action->getActionType())
            {
            case ConsoleGameRules::eGameRuleType_GenerateBox:
                static_cast<XboxStructureActionGenerateBox*>(action)->generateBoxInLevel(this, level, box);
                break;
            case ConsoleGameRules::eGameRuleType_PlaceBlock:
                static_cast<XboxStructureActionPlaceBlock*>(action)->placeBlockInLevel(this, level, box);
                break;
            case ConsoleGameRules::eGameRuleType_PlaceContainer:
                static_cast<PlaceContainerRuleDefinition*>(action)->placeContainerInChunk(this, level, box);
                break;
            case ConsoleGameRules::eGameRuleType_PlaceSpawner:
                static_cast<XboxStructureActionPlaceSpawner*>(action)->placeSpawnerInLevel(this, level, box);
                break;
            default:
                break;
            }
        }
    }

    return false;
}

BoundingBox* ConsoleGenerateStructure::getBoundingBox()
{
    if (mBoundingBox == NULL)
    {
        int maxX = 1;
        int maxY = 1;
        int maxZ = 1;
        for (std::vector<ConsoleGenerateStructureAction*>::iterator it = mActions.begin(); it != mActions.end(); ++it)
        {
            ConsoleGenerateStructureAction* action = *it;
            maxX = std::max(maxX, static_cast<int>(action->getEndX()));
            maxY = std::max(maxY, static_cast<int>(action->getEndY()));
            maxZ = std::max(maxZ, static_cast<int>(action->getEndZ()));
        }
        mBoundingBox = new BoundingBox(mX, mY, mZ, mX + maxX, mY + maxY, mZ + maxZ);
    }
    return mBoundingBox;
}

EStructurePiece ConsoleGenerateStructure::GetType()
{
    return eStructurePiece_MonumentBuilding;
}

void ConsoleGenerateStructure::addAdditionalSaveData(DataOutputStream* output)
{
    (void)output;
}

void ConsoleGenerateStructure::readAdditionalSaveData(DataInputStream* input, StructureManager* structureManager)
{
    (void)input;
    (void)structureManager;
}

bool ConsoleGenerateStructure::checkIntersects(const BlockPos& min, const BlockPos& max)
{
    return getBoundingBox()->intersects(min, max);
}

int ConsoleGenerateStructure::getMinY()
{
    return getBoundingBox()->y0;
}
