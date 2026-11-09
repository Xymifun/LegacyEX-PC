#ifndef GAME_MINIGAME_ANYCOMBINATIONOFRULEDEFINITION_HPP
#define GAME_MINIGAME_ANYCOMBINATIONOFRULEDEFINITION_HPP

#include <string>
#include <vector>

#include "game/minigame/GameRuleDefinition.hpp"
#include "game/world/generation/LayerAttributeChildRuleDefinition.hpp"
#include "game/world/generation/LayerFillRuleDefinition.hpp"
#include "game/world/generation/LayerShapeRuleDefinition.hpp"
#include "game/world/generation/LayerSizeRuleDefinition.hpp"

class CombinationVariationsRuleDefinition;
class LayerGenerationRuleDefinition;
class LayerMetadata;
class LayerTheme;
class LayerThemeRuleDefinition;
class Random;

class AnyCombinationOfRuleDefinition : public GameRuleDefinition
{
public:
    AnyCombinationOfRuleDefinition(LayerGenerationRuleDefinition* parent);
    AnyCombinationOfRuleDefinition(LayerGenerationRuleDefinition* parent, AnyCombinationOfRuleDefinition* definition);

    AnyCombinationOfRuleDefinition* GetRandomLeafChild(const LayerMetadata& metadata, Random& layerRandom,
                                                       Random& random) const;
    void GetPossibleLayerSizes(std::vector<LayerAttributeChildRuleDefinition<LayerSize*>*>& sizes) const;
    void GetPossibleLayerShapes(std::vector<LayerAttributeChildRuleDefinition<LayerShape*>*>& shapes) const;
    void GetPossibleLayerFills(std::vector<LayerAttributeChildRuleDefinition<LayerFill*>*>& fills) const;
    void GetPossibleLayerThemes(int passId, std::vector<LayerAttributeChildRuleDefinition<LayerTheme*>*>& themes) const;

    virtual void getChildren(std::vector<GameRuleDefinition*>* children);
    virtual GameRuleDefinition* addChild(ConsoleGameRules::EGameRuleType type);
    virtual ConsoleGameRules::EGameRuleType getActionType();

private:
    LayerGenerationRuleDefinition* mParent;
    AnyCombinationOfRuleDefinition* mDefinition;
    std::wstring mName;
    std::wstring mFilter;
    CombinationVariationsRuleDefinition* mCombinationVariations;
    LayerSizeRuleDefinition* mSizes;
    LayerShapeRuleDefinition* mShapes;
    LayerFillRuleDefinition* mFills;
    std::vector<LayerThemeRuleDefinition*> mThemes;
};

#endif
