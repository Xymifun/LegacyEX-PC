#include "game/minigame/ScoreRingRuleDefinition.hpp"

#include "game/minigame/GameRuleDefinition.hpp"
#include "game/minigame/TargetAreaRuleDefinition.hpp"
#include "game/util/AABB.hpp"

ScoreRingRuleDefinition::ScoreRingRuleDefinition()
    : mX(0),
      mY(0),
      mZ(0),
      mOrientation(GlideRingGenerator::eGlideRingOrientation_XY),
      mSize(GlideRingGenerator::eGlideRingSize_Large)
{
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_x, &mX);
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_y, &mY);
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_z, &mZ);
}

ScoreRingRuleDefinition::~ScoreRingRuleDefinition()
{
}

ConsoleGameRules::EGameRuleType ScoreRingRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_ScoreRing;
}

void ScoreRingRuleDefinition::addAttribute(const std::wstring& name, const std::wstring& value)
{
    if (name.compare(L"orientation") == 0)
    {
        if (value.compare(L"y") == 0)
        {
            mOrientation = GlideRingGenerator::eGlideRingOrientation_XZ;
        }
        else if (value.compare(L"z") == 0)
        {
            mOrientation = GlideRingGenerator::eGlideRingOrientation_XY;
        }
        else if (value.compare(L"x") == 0)
        {
            mOrientation = GlideRingGenerator::eGlideRingOrientation_YZ;
        }
    }
    else if (name.compare(L"size") == 0)
    {
        if (value.compare(L"small") == 0)
        {
            mSize = GlideRingGenerator::eGlideRingSize_Small;
        }
        else if (value.compare(L"medium") == 0)
        {
            mSize = GlideRingGenerator::eGlideRingSize_Medium;
        }
        else if (value.compare(L"large") == 0)
        {
            mSize = GlideRingGenerator::eGlideRingSize_Large;
        }
    }
    else
    {
        GameRuleDefinition::addAttribute(name, value);
    }
}

void ScoreRingRuleDefinition::onAttributesAdded()
{
    int radius;

    switch (mSize)
    {
    case GlideRingGenerator::eGlideRingSize_Small:
        radius = 1;
        break;
    case GlideRingGenerator::eGlideRingSize_Medium:
        radius = 2;
        break;
    default:
        radius = 3;
        break;
    }

    switch (mOrientation)
    {
    case GlideRingGenerator::eGlideRingOrientation_XZ:
        mArea->mMinY = mY;
        mArea->mMaxY = mY + 1;
        mArea->mMinX = mX - radius;
        mArea->mMaxX = mX + radius + 1;
        mArea->mMinZ = mZ - radius;
        mArea->mMaxZ = mZ + radius + 1;
        break;
    case GlideRingGenerator::eGlideRingOrientation_XY:
        mArea->mMinZ = mZ;
        mArea->mMaxZ = mZ + 1;
        mArea->mMinX = mX - radius;
        mArea->mMaxX = mX + radius + 1;
        mArea->mMinY = mY - radius;
        mArea->mMaxY = mY + radius + 1;
        break;
    default:
        mArea->mMinX = mX;
        mArea->mMaxX = mX + 1;
        mArea->mMinZ = mZ - radius;
        mArea->mMaxZ = mZ + radius + 1;
        mArea->mMinY = mY - radius;
        mArea->mMaxY = mY + radius + 1;
        break;
    }
}

int ScoreRingRuleDefinition::getPointValue()
{
    switch (mSize)
    {
    case GlideRingGenerator::eGlideRingSize_Small:
        return 7;
    case GlideRingGenerator::eGlideRingSize_Medium:
        return 5;
    default:
        return 3;
    }
}

int ScoreRingRuleDefinition::getSize()
{
    return mSize;
}
