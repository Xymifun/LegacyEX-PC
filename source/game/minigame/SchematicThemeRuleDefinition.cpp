#include "game/minigame/SchematicThemeRuleDefinition.hpp"

#include "game/core/CommonMasterGameMode.hpp"
#include "game/core/ConsoleSchematicFile.hpp"
#include "game/core/GameRules_IntAttribute.hpp"
#include "game/core/SchematicTheme.hpp"
#include "game/world/LevelGenerationOptions.hpp"

SchematicThemeRuleDefinition::SchematicThemeRuleDefinition()
{
}

SchematicThemeRuleDefinition::~SchematicThemeRuleDefinition()
{
    std::vector<AssetRuleDef*>::iterator end = mAssets.end();
    for (std::vector<AssetRuleDef*>::iterator asset = mAssets.begin(); asset != end; ++asset)
    {
        delete *asset;
    }
}

ConsoleGameRules::EGameRuleType SchematicThemeRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_SchematicTheme;
}

LayerTheme* SchematicThemeRuleDefinition::Generate() const
{
    SchematicTheme* theme = new SchematicTheme();

    std::vector<AssetRuleDef*>::const_iterator end = mAssets.end();
    for (std::vector<AssetRuleDef*>::const_iterator it = mAssets.begin(); it != end; ++it)
    {
        AssetRuleDef* asset = *it;
        std::wstring filename = ConsoleSchematicFile::formatFilenameWithExt(asset->mFilename);
        if (!filename.empty())
        {
            CommonMasterGameMode* gameMode = CommonMasterGameMode::GetMasterOrClient();
            LevelGenerationOptions* options = gameMode->getRoundLevelGenOptions();
            ConsoleSchematicFile* schematic = options->getSchematicFile(filename);
            if (schematic != NULL)
            {
                static_cast<AssetTheme<ConsoleSchematicFile*>*>(theme)->addAsset(
                    static_cast<unsigned int>(asset->mMinimumWidth), schematic);
            }
        }
    }

    return theme;
}

GameRuleDefinition* SchematicThemeRuleDefinition::addChild(ConsoleGameRules::EGameRuleType type)
{
    if (type == ConsoleGameRules::eGameRuleType_LayerAsset)
    {
        AssetRuleDef* asset = new AssetRuleDef();
        mAssets.push_back(asset);
        return asset;
    }

    return GameRuleDefinition::addChild(type);
}

SchematicThemeRuleDefinition::AssetRuleDef::AssetRuleDef() : mFilename(), mMinimumWidth(0)
{
    addStringAttribute(ConsoleGameRules::eGameRuleAttr_filename, &mFilename);
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_size, &mMinimumWidth)->setMin(0)->setMax(128);
}

ConsoleGameRules::EGameRuleType SchematicThemeRuleDefinition::AssetRuleDef::getActionType()
{
    return ConsoleGameRules::eGameRuleType_LayerAsset;
}
