#ifndef GAME_MINIGAME_SCHEMATICTHEMERULEDEFINITION_HPP
#define GAME_MINIGAME_SCHEMATICTHEMERULEDEFINITION_HPP

#include <string>
#include <vector>

#include "game/minigame/GameRuleDefinition.hpp"
#include "game/world/generation/LayerAttributeChildRuleDefinition.hpp"
#include "game/world/generation/LayerTheme.hpp"

class SchematicThemeRuleDefinition : public LayerAttributeChildRuleDefinition<LayerTheme*>
{
public:
    class AssetRuleDef : public GameRuleDefinition
    {
    public:
        AssetRuleDef();

        virtual ConsoleGameRules::EGameRuleType getActionType();

        std::wstring mFilename;
        int mMinimumWidth;
    };

    SchematicThemeRuleDefinition();
    virtual ~SchematicThemeRuleDefinition();

    virtual ConsoleGameRules::EGameRuleType getActionType();
    virtual LayerTheme* Generate() const;
    virtual GameRuleDefinition* addChild(ConsoleGameRules::EGameRuleType type);

private:
    std::vector<AssetRuleDef*> mAssets;
};

#endif
