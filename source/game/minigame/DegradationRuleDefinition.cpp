#include "game/minigame/DegradationRuleDefinition.hpp"

#include "game/data/GameRuleSaveInterface.hpp"
#include "game/core/IGameRuleAttribute.hpp"
#include "game/minigame/GameRuleDefinition.hpp"

template <typename T>
std::wstring _toString(T value);

DegradationRuleDefinition::DegradationRuleDefinition() : mTargetLayer(L"[UNINTIALISED]"), mSpeed(2)
{
    addStringAttribute(ConsoleGameRules::eGameRuleAttr_target, &mTargetLayer);
}

DegradationRuleDefinition::~DegradationRuleDefinition()
{
}

void DegradationRuleDefinition::writeAttributes(GameRuleSaveInterface* saveInterface, unsigned int extraAttributeCount)
{
    GameRuleDefinition::writeAttributes(saveInterface, extraAttributeCount + 1);

    saveInterface->saveAttribute(ConsoleGameRules::eGameRuleAttr_speed, _toString(mSpeed));
}

void DegradationRuleDefinition::addAttribute(const std::wstring& name, const std::wstring& value)
{
    if (name.compare(L"speed") == 0)
    {
        if (value.compare(L"Uniform") == 0)
        {
            mSpeed = 2;
        }

        return;
    }

    GameRuleDefinition::addAttribute(name, value);
}

void DegradationRuleDefinition::ClearCaches()
{
}

const std::wstring& DegradationRuleDefinition::getTargetLayer() const
{
    return mTargetLayer;
}
