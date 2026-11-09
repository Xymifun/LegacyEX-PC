#include "game/minigame/KillboxRuleDefinition.hpp"

#include "game/core/StringIds.hpp"
#include "game/minigame/GameRuleDefinition.hpp"
#include "game/minigame/NamedAreaRuleDefinition.hpp"
#include "game/util/AABB.hpp"

KillboxRuleDefinition::KillboxRuleDefinition(AABB* legacyArea) : mDeathMessageIDS(0)
{
    if (legacyArea == NULL)
    {
        addIntAttribute(ConsoleGameRules::eGameRuleAttr_string_IDS, &mDeathMessageIDS);
    }
    else
    {
        ConvertFromLegacy(legacyArea);
    }
}

KillboxRuleDefinition::~KillboxRuleDefinition()
{
}

ConsoleGameRules::EGameRuleType KillboxRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_Killbox;
}

void KillboxRuleDefinition::ConvertFromLegacy(AABB* legacyArea)
{
    updateWithPoints(static_cast<int>(legacyArea->mMinX), static_cast<int>(legacyArea->mMinY),
                     static_cast<int>(legacyArea->mMinZ), static_cast<int>(legacyArea->mMaxX),
                     static_cast<int>(legacyArea->mMaxY), static_cast<int>(legacyArea->mMaxZ));
}

void KillboxRuleDefinition::GetDeathMessageIDS(int& deathMessageIDS, std::wstring& deathMessage)
{
    if (mDeathMessageIDS != 0)
    {
        deathMessageIDS = mDeathMessageIDS;
        deathMessage = L"";
        return;
    }

    if (mDescriptionName.compare(L"") != 0)
    {
        deathMessageIDS = 0;
        deathMessage = mDescriptionName;
        return;
    }

    deathMessageIDS = IDS_DEATH_OUTOFWORLD;
    deathMessage = L"";
}

void KillboxRuleDefinition::WriteAttributesAsXML(std::string& xml)
{
    if (!mName.empty())
    {
        WriteAttributeAsXML(ConsoleGameRules::eGameRuleAttr_name, xml);
    }

    WriteAttributeAsXML(ConsoleGameRules::eGameRuleAttr_string_IDS, xml);

    NamedAreaRuleDefinition::WriteAttributesAsXML(xml);
}
