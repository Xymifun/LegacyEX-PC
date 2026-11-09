#include "game/world/generation/LayerThemeRuleDefinition.hpp"

#include "game/client/render/TextureThemeRuleDefinition.hpp"
#include "game/minigame/CarpetTrapThemeRuleDefinition.hpp"
#include "game/minigame/GameRuleDefinition.hpp"
#include "game/minigame/NullThemeRuleDefinition.hpp"
#include "game/minigame/SchematicThemeRuleDefinition.hpp"
#include "game/world/block/BlockStackLayerThemeRuleDefinition.hpp"
#include "game/world/block/MushroomBlockThemeRuleDefinition.hpp"
#include "game/world/generation/BasicPatchesLayerThemeRuleDefinition.hpp"
#include "game/world/generation/FunctionPatchesLayerThemeRuleDefinition.hpp"
#include "game/world/generation/RainbowLayerThemeRuleDefinition.hpp"
#include "game/world/generation/ShaftsLayerThemeRuleDefinition.hpp"
#include "game/world/generation/SimplePatchesLayerThemeRuleDefinition.hpp"
#include "game/world/generation/TerracottaLayerThemeRuleDefinition.hpp"

static const std::wstring sPassNames[4] = {L"Look", L"Decorations", L"Hazards", L"Lights"};

LayerThemeRuleDefinition::LayerThemeRuleDefinition()
    : LayerAttributeRuleDefinition<LayerTheme*>(ConsoleGameRules::eGameRuleType_LayerTheme, CreateFuncImpl),
      mPassId(0xFFFFFFFF)
{
}

unsigned int LayerThemeRuleDefinition::GetPassId() const
{
    return mPassId;
}

void LayerThemeRuleDefinition::writeAttributes(GameRuleSaveInterface* saveInterface, unsigned int extraAttributeCount)
{
    GameRuleDefinition::writeAttributes(saveInterface, extraAttributeCount + 1);

    saveInterface->saveAttribute(ConsoleGameRules::eGameRuleAttr_name, sPassNames[mPassId]);
}

void LayerThemeRuleDefinition::addAttribute(const std::wstring& name, const std::wstring& value)
{
    if (name.compare(L"pass") == 0)
    {
        if (sPassNames[0].compare(value) == 0)
        {
            mPassId = 0;
        }

        if (sPassNames[1].compare(value) == 0)
        {
            mPassId = 1;
        }

        if (sPassNames[2].compare(value) == 0)
        {
            mPassId = 2;
        }

        if (sPassNames[3].compare(value) == 0)
        {
            mPassId = 3;
        }
    }
    else
    {
        GameRuleDefinition::addAttribute(name, value);
    }
}

LayerAttributeChildRuleDefinition<LayerTheme*>* LayerThemeRuleDefinition::CreateFuncImpl(
    ConsoleGameRules::EGameRuleType type)
{
    switch (type)
    {
    case ConsoleGameRules::eGameRuleType_NullTheme:
        return new NullThemeRuleDefinition();

    case ConsoleGameRules::eGameRuleType_ShaftsTheme:
        return new ShaftsLayerThemeRuleDefinition();

    case ConsoleGameRules::eGameRuleType_BlockStackTheme:
        return new BlockStackLayerThemeRuleDefinition();

    case ConsoleGameRules::eGameRuleType_RainbowTheme:
        return new RainbowLayerThemeRuleDefinition();

    case ConsoleGameRules::eGameRuleType_TerracottaTheme:
        return new TerracottaLayerThemeRuleDefinition();

    case ConsoleGameRules::eGameRuleType_BasicPatchesTheme:
        return new BasicPatchesLayerThemeRuleDefinition();

    case ConsoleGameRules::eGameRuleType_FunctionPatchesTheme:
        return new FunctionPatchesLayerThemeRuleDefinition();

    case ConsoleGameRules::eGameRuleType_SimplePatchesTheme:
        return new SimplePatchesLayerThemeRuleDefinition();

    case ConsoleGameRules::eGameRuleType_CarpetTrapTheme:
        return new CarpetTrapThemeRuleDefinition();

    case ConsoleGameRules::eGameRuleType_MushroomBlockTheme:
        return new MushroomBlockThemeRuleDefinition();

    case ConsoleGameRules::eGameRuleType_TextureTheme:
        return new TextureThemeRuleDefinition();

    case ConsoleGameRules::eGameRuleType_SchematicTheme:
        return new SchematicThemeRuleDefinition();

    default:
        break;
    }

    return NULL;
}
