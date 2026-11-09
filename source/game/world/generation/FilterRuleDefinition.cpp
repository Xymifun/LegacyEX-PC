#include "game/world/generation/FilterRuleDefinition.hpp"

#include "game/core/StringConversion.hpp"

#include <string>
#include <vector>

#include "game/core/LayerSize.hpp"
#include "game/data/GameRuleSaveInterface.hpp"
#include "game/minigame/GameRuleDefinition.hpp"
#include "game/world/generation/LayerAttributeChildRuleDefinition.hpp"
#include "game/world/generation/FunctionLayerFill.hpp"
#include "game/world/generation/LayerMetadata.hpp"
#include "game/world/generation/LayerShape.hpp"
#include "game/world/generation/LayerTheme.hpp"

template <class T>
ConsoleGameRules::EGameRuleType FilterRuleDefinition<T>::getActionType()
{
    return ConsoleGameRules::eGameRuleType_FilterTheme;
}

template <class T>
void FilterRuleDefinition<T>::writeAttributes(GameRuleSaveInterface* saveInterface, unsigned int extraAttributeCount)
{
    GameRuleDefinition::writeAttributes(saveInterface, extraAttributeCount + 2);

    saveInterface->saveAttribute(ConsoleGameRules::eGameRuleAttr_variant, _toString<int>(mVariant));
    saveInterface->saveAttribute(ConsoleGameRules::eGameRuleAttr_type, LayerMetadata::toStringFilter(mFilter));
}

template <class T>
void FilterRuleDefinition<T>::getChildren(std::vector<GameRuleDefinition*>* children)
{
    children->push_back(mChild);
}

template <class T>
GameRuleDefinition* FilterRuleDefinition<T>::addChild(ConsoleGameRules::EGameRuleType type)
{
    LayerAttributeChildRuleDefinition<T>* child = mCreateChild(type);
    if (child != NULL)
    {
        if (mChild != NULL)
        {
            delete mChild;
        }

        mChild = child;
        return child;
    }

    return GameRuleDefinition::addChild(type);
}

template <class T>
void FilterRuleDefinition<T>::addAttribute(const std::wstring& name, const std::wstring& value)
{
    if (name.compare(L"variant") == 0)
    {
        mVariant = _fromString<int>(value);
        return;
    }

    if (name.compare(L"type") == 0)
    {
        mFilter = LayerMetadata::fromStringFilter(value);
        return;
    }

    GameRuleDefinition::addAttribute(name, value);
}

template <class T>
T FilterRuleDefinition<T>::Generate() const
{
    return mChild->Generate();
}

template <class T>
bool FilterRuleDefinition<T>::CanUse(const LayerMetadata& metadata) const
{
    int canUse = 0;

    if (mVariant == -1 || mVariant == metadata.m_uGameVariant)
    {
        LayerMetadata::FilterFunc filter = LayerMetadata::getFilter(mFilter);
        if (filter(metadata))
        {
            canUse = 1;
        }
    }

    return canUse;
}

template class FilterRuleDefinition<LayerShape*>;
template class FilterRuleDefinition<LayerSize*>;
template class FilterRuleDefinition<LayerTheme*>;
template class FilterRuleDefinition<LayerFill*>;
