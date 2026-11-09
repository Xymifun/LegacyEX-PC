#include "game/minigame/NamedAreaRuleDefinition.hpp"

#include "game/util/AABB.hpp"
#include "game/world/block/BlockPos.hpp"

NamedAreaRuleDefinition::NamedAreaRuleDefinition()
{
    mName = L"";
    mArea = AABB::newPermanent(0.0, 0.0, 0.0, 0.0, 0.0, 0.0);

    addStringAttribute(ConsoleGameRules::eGameRuleAttr_name, &mName);
    addDoubleAttribute(ConsoleGameRules::eGameRuleAttr_x0, &mArea->mMinX);
    addDoubleAttribute(ConsoleGameRules::eGameRuleAttr_y0, &mArea->mMinY);
    addDoubleAttribute(ConsoleGameRules::eGameRuleAttr_z0, &mArea->mMinZ);
    addDoubleAttribute(ConsoleGameRules::eGameRuleAttr_x1, &mArea->mMaxX);
    addDoubleAttribute(ConsoleGameRules::eGameRuleAttr_y1, &mArea->mMaxY);
    addDoubleAttribute(ConsoleGameRules::eGameRuleAttr_z1, &mArea->mMaxZ);
    addHexAttribute(ConsoleGameRules::eGameRuleAttr_dataTag, &mDataTag);
}

NamedAreaRuleDefinition::~NamedAreaRuleDefinition()
{
    delete mArea;
}

ConsoleGameRules::EGameRuleType NamedAreaRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_NamedArea;
}

AABB* NamedAreaRuleDefinition::getBoundingVolume()
{
    return getArea();
}

AABB* NamedAreaRuleDefinition::getArea()
{
    return mArea;
}

std::wstring NamedAreaRuleDefinition::getName()
{
    return mName;
}

bool NamedAreaRuleDefinition::containsBlock(int blockId, const BlockPos& pos)
{
    return mArea->mMinX <= pos.x && pos.x <= mArea->mMaxX && mArea->mMinY <= pos.y && pos.y <= mArea->mMaxY
           && mArea->mMinZ <= pos.z && pos.z <= mArea->mMaxZ;
}

void NamedAreaRuleDefinition::onAttributesAdded()
{
    mArea->correctMinMax();
}

void NamedAreaRuleDefinition::updateWithPoints(int x0, int y0, int z0, int x1, int y1, int z1)
{
    int minX = x1 < x0 ? x1 : x0;
    int maxX = x0 > x1 ? x0 : x1;
    if (minX == maxX)
    {
        maxX = minX + 1;
    }

    int minY = y1 < y0 ? y1 : y0;
    int maxY = y0 > y1 ? y0 : y1;
    if (minY == maxY)
    {
        maxY = minY + 1;
    }

    int minZ = z1 < z0 ? z1 : z0;
    int maxZ = z0 > z1 ? z0 : z1;
    if (minZ == maxZ)
    {
        maxZ = minZ + 1;
    }

    mArea->mMinX = minX;
    mArea->mMaxX = maxX;
    mArea->mMinY = minY;
    mArea->mMaxY = maxY;
    mArea->mMinZ = minZ;
    mArea->mMaxZ = maxZ;
}

void NamedAreaRuleDefinition::WriteAttributesAsXML(std::string& xml)
{
    if (!mName.empty())
    {
        WriteAttributeAsXML(ConsoleGameRules::eGameRuleAttr_name, xml);
    }

    WriteAttributeAsXML(ConsoleGameRules::eGameRuleAttr_x0, xml);
    WriteAttributeAsXML(ConsoleGameRules::eGameRuleAttr_y0, xml);
    WriteAttributeAsXML(ConsoleGameRules::eGameRuleAttr_z0, xml);
    WriteAttributeAsXML(ConsoleGameRules::eGameRuleAttr_x1, xml);
    WriteAttributeAsXML(ConsoleGameRules::eGameRuleAttr_y1, xml);
    WriteAttributeAsXML(ConsoleGameRules::eGameRuleAttr_z1, xml);

    if (mDataTag != 0)
    {
        WriteAttributeAsXML(ConsoleGameRules::eGameRuleAttr_dataTag, xml);
    }
}
