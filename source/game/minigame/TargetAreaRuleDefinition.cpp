#include "game/minigame/TargetAreaRuleDefinition.hpp"

#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/core/StringFormat.hpp"

TargetAreaRuleDefinition::TargetAreaRuleDefinition() : mPrimary(false), mPointValue(0), mLaps(0)
{
    addBoolAttribute(ConsoleGameRules::eGameRuleAttr_primary, &mPrimary);
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_laps, &mLaps);
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_points, &mPointValue);
}

TargetAreaRuleDefinition::~TargetAreaRuleDefinition()
{
}

ConsoleGameRules::EGameRuleType TargetAreaRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_TargetArea;
}

int TargetAreaRuleDefinition::getPointValue()
{
    return mPointValue;
}

int TargetAreaRuleDefinition::getSize()
{
    if (mPointValue != 3)
    {
        if (mPointValue == 5)
        {
            return 1;
        }

        if (mPointValue == 7)
        {
            return 0;
        }
    }

    return 2;
}

void TargetAreaRuleDefinition::WriteAttributesAsXML(std::string& xml)
{
    if (!mPrimary)
    {
        if (!mName.empty())
        {
            WriteAttributeAsXML(ConsoleGameRules::eGameRuleAttr_name, xml);
        }

        BlockPos centre(mArea->getCenter()->x, mArea->getCenter()->y, mArea->getCenter()->z);

        xml.append(formatstr(" x=\"%d\"", centre.x));
        xml.append(formatstr(" y=\"%d\"", centre.y));
        xml.append(formatstr(" z=\"%d\"", centre.z));

        double sizeX = mArea->mMaxX - mArea->mMinX;
        double sizeY = mArea->mMaxY - mArea->mMinY;
        double sizeZ = mArea->mMaxZ - mArea->mMinZ;

        if (sizeX < sizeY && sizeX < sizeZ)
        {
            xml.append(" orientation=\"x\"");
        }
        else if (sizeY < sizeX && sizeY < sizeZ)
        {
            xml.append(" orientation=\"y\"");
        }
        else if (sizeZ < sizeX && sizeZ < sizeY)
        {
            xml.append(" orientation=\"z\"");
        }

        if (getPointValue() == 7)
        {
            xml.append(" size=\"small\"");
        }
        else if (getPointValue() == 5)
        {
            xml.append(" size=\"medium\"");
        }
        else
        {
            xml.append(" size=\"large\"");
        }
    }
    else
    {
        NamedAreaRuleDefinition::WriteAttributesAsXML(xml);

        if (mPrimary)
        {
            WriteAttributeAsXML(ConsoleGameRules::eGameRuleAttr_primary, xml);
        }

        if (mLaps != 0)
        {
            WriteAttributeAsXML(ConsoleGameRules::eGameRuleAttr_laps, xml);
        }
    }
}
