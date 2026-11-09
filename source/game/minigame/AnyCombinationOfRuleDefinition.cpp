#include "game/minigame/AnyCombinationOfRuleDefinition.hpp"

#include "game/minigame/CombinationVariationsRuleDefinition.hpp"
#include "game/core/IGameRuleAttribute.hpp"
#include "game/world/generation/LayerGenerationRuleDefinition.hpp"
#include "game/world/generation/LayerMetadata.hpp"
#include "game/world/generation/LayerThemeRuleDefinition.hpp"
#include "game/util/Random.hpp"

AnyCombinationOfRuleDefinition::AnyCombinationOfRuleDefinition(LayerGenerationRuleDefinition* parent)
    : mParent(parent),
      mDefinition(NULL),
      mName(),
      mFilter(L"AnyLayer"),
      mCombinationVariations(NULL),
      mSizes(new LayerSizeRuleDefinition()),
      mShapes(new LayerShapeRuleDefinition()),
      mFills(new LayerFillRuleDefinition())
{
    addStringAttribute(ConsoleGameRules::eGameRuleAttr_name, &mName);
    addStringAttribute(ConsoleGameRules::eGameRuleAttr_type, &mFilter);
}

AnyCombinationOfRuleDefinition::AnyCombinationOfRuleDefinition(LayerGenerationRuleDefinition* parent,
                                                               AnyCombinationOfRuleDefinition* definition)
    : mParent(parent),
      mDefinition(definition),
      mName(),
      mFilter(L"AnyLayer"),
      mCombinationVariations(NULL),
      mSizes(new LayerSizeRuleDefinition()),
      mShapes(new LayerShapeRuleDefinition()),
      mFills(new LayerFillRuleDefinition())
{
    addStringAttribute(ConsoleGameRules::eGameRuleAttr_name, &mName);
    addStringAttribute(ConsoleGameRules::eGameRuleAttr_type, &mFilter);
}

AnyCombinationOfRuleDefinition* AnyCombinationOfRuleDefinition::GetRandomLeafChild(const LayerMetadata& metadata,
                                                                                   Random& layerRandom,
                                                                                   Random& random) const
{
    CombinationVariationsRuleDefinition* variations = mCombinationVariations;
    if (variations != NULL)
    {
        const std::vector<AnyCombinationOfRuleDefinition*>* combinations = variations->GetCombinations();
        bool wholeLevel = variations->IsForWholeLevel();
        size_t count = combinations->size();
        AnyCombinationOfRuleDefinition* child;

        do
        {
            int layerIndex = layerRandom.nextInt(count);
            int randomIndex = random.nextInt(count);
            child = combinations->at(wholeLevel ? layerIndex : randomIndex);
        } while (LayerMetadata::getFilter(child->mFilter)(metadata) == false || child == NULL);

        AnyCombinationOfRuleDefinition* leaf = child->GetRandomLeafChild(metadata, layerRandom, random);
        return leaf;
    }
    return const_cast<AnyCombinationOfRuleDefinition*>(this);
}

void AnyCombinationOfRuleDefinition::GetPossibleLayerSizes(
    std::vector<LayerAttributeChildRuleDefinition<LayerSize*>*>& sizes) const
{
    std::vector<GameRuleDefinition*> children;
    mSizes->getChildren(&children);
    std::vector<GameRuleDefinition*>::iterator it;
    for (it = children.begin(); it != children.end(); ++it)
    {
        sizes.push_back(static_cast<LayerAttributeChildRuleDefinition<LayerSize*>*>(*it));
    }

    if (mDefinition != NULL)
    {
        mDefinition->GetPossibleLayerSizes(sizes);
    }
}

void AnyCombinationOfRuleDefinition::GetPossibleLayerShapes(
    std::vector<LayerAttributeChildRuleDefinition<LayerShape*>*>& shapes) const
{
    std::vector<GameRuleDefinition*> children;
    mShapes->getChildren(&children);
    std::vector<GameRuleDefinition*>::iterator it;
    for (it = children.begin(); it != children.end(); ++it)
    {
        shapes.push_back(static_cast<LayerAttributeChildRuleDefinition<LayerShape*>*>(*it));
    }

    if (mDefinition != NULL)
    {
        mDefinition->GetPossibleLayerShapes(shapes);
    }
}

void AnyCombinationOfRuleDefinition::GetPossibleLayerFills(
    std::vector<LayerAttributeChildRuleDefinition<LayerFill*>*>& fills) const
{
    std::vector<GameRuleDefinition*> children;
    mFills->getChildren(&children);
    std::vector<GameRuleDefinition*>::iterator it;
    for (it = children.begin(); it != children.end(); ++it)
    {
        fills.push_back(static_cast<LayerAttributeChildRuleDefinition<LayerFill*>*>(*it));
    }

    if (mDefinition != NULL)
    {
        mDefinition->GetPossibleLayerFills(fills);
    }
}

void AnyCombinationOfRuleDefinition::GetPossibleLayerThemes(
    int passId, std::vector<LayerAttributeChildRuleDefinition<LayerTheme*>*>& themes) const
{
    const AnyCombinationOfRuleDefinition* definition = this;
    while (definition != NULL)
    {
        LayerThemeRuleDefinition* matchingTheme = NULL;
        std::vector<LayerThemeRuleDefinition*>::const_iterator it;
        for (it = definition->mThemes.begin(); it != definition->mThemes.end(); ++it)
        {
            if ((*it)->GetPassId() == static_cast<unsigned int>(passId))
            {
                matchingTheme = *it;
            }
        }

        if (matchingTheme != NULL)
        {
            std::vector<GameRuleDefinition*> children;
            matchingTheme->getChildren(&children);
            std::vector<GameRuleDefinition*>::iterator child;
            for (child = children.begin(); child != children.end(); ++child)
            {
                themes.push_back(static_cast<LayerAttributeChildRuleDefinition<LayerTheme*>*>(*child));
            }
        }

        definition = definition->mDefinition;
    }
}

void AnyCombinationOfRuleDefinition::getChildren(std::vector<GameRuleDefinition*>* children)
{
    children->push_back(mCombinationVariations);
}

GameRuleDefinition* AnyCombinationOfRuleDefinition::addChild(ConsoleGameRules::EGameRuleType type)
{
    if (type == ConsoleGameRules::eGameRuleType_Variations)
    {
        mCombinationVariations = new CombinationVariationsRuleDefinition(mParent, this);
        return mCombinationVariations;
    }

    if (type == ConsoleGameRules::eGameRuleType_LayerSize)
    {
        return mSizes;
    }

    if (type == ConsoleGameRules::eGameRuleType_LayerShape)
    {
        return mShapes;
    }

    if (type == ConsoleGameRules::eGameRuleType_LayerFill)
    {
        return mFills;
    }

    if (type == ConsoleGameRules::eGameRuleType_LayerTheme)
    {
        LayerThemeRuleDefinition* theme = new LayerThemeRuleDefinition();
        mThemes.push_back(theme);
        return theme;
    }

    return GameRuleDefinition::addChild(type);
}

ConsoleGameRules::EGameRuleType AnyCombinationOfRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_AnyCombinationOf;
}
